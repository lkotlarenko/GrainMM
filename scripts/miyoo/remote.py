#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Run a reviewed script over an existing authenticated SSH control session."""
import argparse
import json
from pathlib import Path
import subprocess

from deploy import ssh_command


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, required=True)
    parser.add_argument('--script', type=Path, required=True)
    parser.add_argument('--timeout', type=int, default=15)
    args = parser.parse_args()
    if not 1 <= args.timeout <= 60:
        parser.error('timeout must be 1..60 seconds')
    config = json.loads(args.config.read_text())
    try:
        # A host timeout bounds SSH; reviewed runtime probes also use device timeout.
        # Stream script bytes over stdin rather than a large SSH exec request.
        # Keeps quoting predictable and supports constrained embedded SSH servers.
        subprocess.run(ssh_command(config, 'exec sh -s'), input=args.script.read_text(),
                       text=True, check=True, timeout=args.timeout)
    except subprocess.TimeoutExpired:
        parser.exit(1, 'Remote operation timed out; no SSH/server configuration was changed.\n')
    except subprocess.CalledProcessError as error:
        parser.exit(1, f'Remote operation failed with status {error.returncode}.\n')


if __name__ == '__main__':
    main()
