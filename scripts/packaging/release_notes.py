#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Validate a release version and assemble user-oriented notes without network access."""
import argparse
import json
import re
from pathlib import Path
import sys
from package import ROOT, release_version


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--version', required=True)
    parser.add_argument('--public', action='store_true', help='require explicit source-publication metadata')
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    version = release_version(args.version)
    if args.public and not json.loads((ROOT / 'packaging/release.json').read_text()).get('source_publication_authorized', False):
        raise ValueError('native/source publication is withheld; explicit owner authorization required')
    notes = ROOT / f'docs/releases/{version}.md'
    if notes.is_symlink() or not notes.is_file():
        raise ValueError('reviewed release notes are missing')
    # Release descriptions resolve relative links against the release URL, not
    # the repository tree. Pin screenshots and guide links to this exact tag.
    def link(match):
        target = match[1]
        if re.match(r'^[a-zA-Z][a-zA-Z0-9+.-]*:', target) or target.startswith('#'):
            return match[0]
        name, sep, fragment = target.partition('#')
        path = (notes.parent / name).resolve()
        if not path.is_relative_to(ROOT) or not path.is_file():
            raise ValueError('release note link must name a repository file')
        relative = path.relative_to(ROOT).as_posix()
        base = ('https://raw.githubusercontent.com/lkotlarenko/GrainMM/'
                if path.suffix.lower() in {'.png', '.jpg', '.jpeg', '.webp'} else
                'https://github.com/lkotlarenko/GrainMM/blob/')
        return '](' + base + 'v' + version + '/' + relative + (sep + fragment if sep else '') + ')'
    rendered = re.sub(r'\]\(([^)]+)\)', link, notes.read_text())
    with args.output.open('x') as stream:
        stream.write(rendered)
    print(f'Release notes validated for {version}')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, KeyError) as error:
        sys.exit(f'Release rejected: {error}')
