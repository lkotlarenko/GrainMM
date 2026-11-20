#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""One-shot, shared RFB 3.8 raw capture of an existing private-network server."""
import argparse
import datetime
import ipaddress
import json
from pathlib import Path
import socket
import struct
import time
import zlib

ROOT = Path(__file__).resolve().parents[2]
KEYS = {'Up': 0xff52, 'Down': 0xff54, 'Left': 0xff51, 'Right': 0xff53,
        'Escape': 0xff1b, 'Return': 0xff0d, 'Space': 0x20}


def read_exact(connection, size):
    if size < 0 or size > 16 * 1024 * 1024:
        raise ValueError('RFB message exceeds capture limit')
    result = bytearray()
    while len(result) < size:
        data = connection.recv(size-len(result))
        if not data:
            raise ConnectionError('RFB connection closed')
        result.extend(data)
    return bytes(result)


def png(width, height, bgrx):
    def chunk(kind, data):
        return struct.pack('>I', len(data)) + kind + data + struct.pack('>I', zlib.crc32(kind+data))
    rows = bytearray()
    for y in range(height):
        rows.append(0)
        for x in range(width):
            at = (y*width+x)*4
            rows.extend((bgrx[at+2], bgrx[at+1], bgrx[at]))
    return (b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', width, height, 8, 2, 0, 0, 0))
            + chunk(b'IDAT', zlib.compress(rows, 6)) + chunk(b'IEND', b''))


def capture(connection, key=None, repeat=1, view=True):
    if not 1 <= repeat <= 8 or (repeat != 1 and key not in ('Up', 'Down', 'Left', 'Right')):
        raise ValueError('only bounded arrow-key navigation can repeat')
    banner = read_exact(connection, 12)
    if banner != b'RFB 003.008\n':
        raise ValueError('only RFB 3.8 is supported by this diagnostic')
    connection.sendall(b'RFB 003.008\n')
    count = read_exact(connection, 1)[0]
    types = read_exact(connection, count)
    if 1 not in types:
        raise ValueError('existing server does not offer None security; no authentication changes attempted')
    connection.sendall(b'\x01')
    if struct.unpack('>I', read_exact(connection, 4))[0] != 0:
        raise ValueError('RFB security negotiation failed')
    connection.sendall(b'\x01')  # Shared: do not disconnect another viewer.
    initial = read_exact(connection, 24)
    width, height = struct.unpack('>HH', initial[:4])
    name_length = struct.unpack('>I', initial[20:24])[0]
    if not width or not height or width*height > 2*1024*1024 or name_length > 4096:
        raise ValueError('unexpected desktop geometry/name length')
    read_exact(connection, name_length)  # Discard server desktop name; it may identify the host.
    wire_format = struct.pack('>BBBBHHHBBBxxx', 32, 24, 0, 1, 255, 255, 255, 16, 8, 0)
    connection.sendall(b'\x00\0\0\0'+wire_format)
    connection.sendall(struct.pack('>BBHi', 2, 0, 1, 0))  # Raw rectangles only.
    if key is not None:
        for _ in range(repeat):
            connection.sendall(struct.pack('>BBHI', 4, 1, 0, KEYS[key]))
            try:
                time.sleep(0.08)
            finally:
                connection.sendall(struct.pack('>BBHI', 4, 0, 0, KEYS[key]))
            time.sleep(0.15)
        time.sleep(1.0)  # Let the stock 2 FPS capture thread publish the changed display.
    if not view:
        return None, {'protocol': 'RFB 3.8', 'key_sent': key, 'repeat': repeat,
                      'shared': True, 'requested_frames': 0}
    connection.sendall(struct.pack('>BBHHHH', 3, 0, 0, 0, width, height))
    pixels = bytearray(width*height*4)
    for _ in range(16):
        message = read_exact(connection, 1)[0]
        if message == 2:  # Bell, no payload.
            continue
        if message == 3:  # Never record clipboard contents.
            header = read_exact(connection, 7)
            length = struct.unpack('>I', header[3:])[0]
            if length > 65536:
                raise ValueError('clipboard message exceeds bound')
            read_exact(connection, length)
            continue
        if message != 0:
            raise ValueError(f'unhandled RFB server message {message}')
        rectangles = struct.unpack('>H', read_exact(connection, 3)[1:])[0]
        if not rectangles or rectangles > 4096:
            raise ValueError('empty or excessive rectangle update')
        total = 0
        for _ in range(rectangles):
            x, y, w, h, encoding = struct.unpack('>HHHHi', read_exact(connection, 12))
            if encoding != 0 or not w or not h or x+w > width or y+h > height:
                raise ValueError('unsupported or out-of-bounds rectangle')
            total += w*h*4
            if total > 16*1024*1024:
                raise ValueError('frame update exceeds bound')
            data = read_exact(connection, w*h*4)
            for row in range(h):
                at = ((y+row)*width+x)*4
                pixels[at:at+w*4] = data[row*w*4:(row+1)*w*4]
        return png(width, height, pixels), {
            'protocol': 'RFB 3.8', 'security_types': list(types), 'shared': True,
            'width': width, 'height': height, 'server_pixel_format': list(initial[4:20]),
            'wire_pixel_format': 'BGRX32 little endian, true color', 'rectangles': rectangles,
            'requested_frames': 1, 'key_sent': key,
        }
    raise ValueError('no framebuffer update within message bound')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True, help='ignored local connection JSON')
    parser.add_argument('--output', type=Path, help='new PNG under .local/')
    parser.add_argument('--key-only', action='store_true', help='send input without requesting/saving a frame')
    parser.add_argument('--key', choices=KEYS, help='explicit single key press; omit for view-only')
    parser.add_argument('--repeat', type=int, default=1, help='1..8, arrow-key navigation only')
    args = parser.parse_args()
    if args.key_only:
        if not args.key or args.output:
            parser.error('--key-only requires --key and no --output')
        output = None
    else:
        if not args.output:
            parser.error('viewing requires --output')
        output = args.output.resolve()
        if not output.is_relative_to(ROOT / '.local') or output.exists():
            parser.error('capture must exclusively create a private .local/ PNG')
    config = json.loads(args.config.read_text())
    host = config['host']
    addresses = socket.getaddrinfo(host, int(config.get('vnc_port', 5900)), type=socket.SOCK_STREAM)
    if not addresses or any(not ipaddress.ip_address(entry[4][0]).is_private for entry in addresses):
        parser.error('only private-network endpoints are allowed')
    with socket.create_connection((host, int(config.get('vnc_port', 5900))), timeout=5) as connection:
        data, metadata = capture(connection, args.key, args.repeat, view=not args.key_only)
    if output is not None:
        output.parent.mkdir(parents=True, exist_ok=True)
        with output.open('xb') as stream:
            stream.write(data)
    journal = ROOT / '.local/hardware/vnc-events.jsonl'
    journal.parent.mkdir(parents=True, exist_ok=True)
    with journal.open('a') as stream:
        stream.write(json.dumps({'utc': datetime.datetime.now(datetime.timezone.utc).isoformat(),
                                 **metadata, 'output': str(output.relative_to(ROOT)) if output else None})+'\n')
    print(json.dumps(metadata, sort_keys=True))


if __name__ == '__main__':
    main()
