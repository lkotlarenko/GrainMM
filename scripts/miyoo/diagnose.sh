#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
# Read-only, POSIX/BusyBox compatible. Redirect stdout to an SD-card file yourself.
# No serials, MAC/IP addresses, SSIDs, usernames, environment dump or kernel cmdline.
set -u
printf '%s\n' 'GrainMM read-only diagnostics v1' 'Kernel:'
uname -srm
printf '%s\n' 'CPU (identifiers excluded):'
if [ -r /proc/cpuinfo ]; then
    sed -n '/^processor[[:space:]]*:/p; /^model name[[:space:]]*:/p; /^CPU architecture[[:space:]]*:/p; /^Features[[:space:]]*:/p' /proc/cpuinfo
fi
printf '%s\n' 'Memory:'
sed -n '/^MemTotal:/p; /^MemAvailable:/p; /^MemFree:/p; /^SwapTotal:/p' /proc/meminfo
printf '%s\n' 'Onion version (review before sharing):'
if [ -r /mnt/SDCARD/.tmp_update/onionVersion/version.txt ]; then
    head -c 160 /mnt/SDCARD/.tmp_update/onionVersion/version.txt
    printf '\n'
else
    printf '%s\n' 'Not found; record manually in device checklist.'
fi
printf '%s\n' 'Framebuffer geometry:'
for name in virtual_size bits_per_pixel; do
    path=/sys/class/graphics/fb0/$name
    if [ -r "$path" ]; then printf '%s: ' "$name"; head -c 80 "$path"; printf '\n'; fi
done
printf '%s\n' 'ALSA card indices (names/IDs excluded):'
for path in /proc/asound/card[0-9]; do
    [ -d "$path" ] && printf '%s\n' "${path##*/}"
done
if [ -r /proc/asound/pcm ]; then
    sed -n 's/^\([0-9][0-9]-[0-9][0-9]\):.*playback \([0-9]*\).*$/PCM \1 playback \2/p' /proc/asound/pcm
fi
printf '%s\n' 'Input event node presence (no event capture):'
for path in /dev/input/event[0-9]*; do
    [ -e "$path" ] && printf '%s\n' "$path"
done
printf '%s\n' 'Audio node presence (not opened):'
for path in /dev/dsp /dev/sound/dsp /dev/mi_ao; do
    [ -e "$path" ] && printf '%s\n' "$path"
done
printf '%s\n' 'Library paths (no execution of target libraries):'
for directory in /lib /usr/lib /customer/lib /config/lib /mnt/SDCARD/.tmp_update/lib; do
    for path in "$directory"/libSDL* "$directory"/libasound* "$directory"/libstdc++* "$directory"/libgcc_s* "$directory"/libc.so* "$directory"/ld-linux*; do
        [ -e "$path" ] && printf '%s\n' "$path"
    done
done
printf '%s\n' 'Tools:'
for tool in aplay arecord readelf ldd python3; do command -v "$tool" 2>/dev/null || :; done
printf '%s\n' 'No PCM opened, keys captured, mounts changed or system settings written.'
exit 0
