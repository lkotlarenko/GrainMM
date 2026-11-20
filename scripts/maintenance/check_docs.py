#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Check repository documentation links without reading private guides or using the network."""
from pathlib import Path
import re
import sys
from urllib.parse import unquote
ROOT = Path(__file__).resolve().parents[2]
ROOT_DOCS = ('README.md', 'ARCHITECTURE.md', 'CONTRIBUTING.md', 'LICENSE_SCOPE.md',
             'CHANGELOG.md', 'SECURITY.md', 'CODE_OF_CONDUCT.md', 'AGENTS.md')


def anchors(text):
    result = set()
    duplicates = {}
    fenced = False
    for line in text.splitlines():
        if line.startswith('```'):
            fenced = not fenced
        if fenced or not line.startswith('#'):
            continue
        heading = line.lstrip('#').strip().lower()
        slug = re.sub(r'[^\w -]', '', heading).replace(' ', '-')
        count = duplicates.get(slug, 0)
        duplicates[slug] = count + 1
        result.add(slug + (f'-{count}' if count else ''))
    return result


def main():
    paths = sorted((ROOT / 'docs').rglob('*.md')) + [ROOT / p for p in ROOT_DOCS]
    errors = []
    for path in paths:
        text = path.read_text()
        for link in re.findall(r'\]\(([^)]+)\)', text):
            if re.match(r'^[a-zA-Z][a-zA-Z0-9+.-]*:', link):
                continue
            name, _, fragment = link.partition('#')
            target = (path.parent / unquote(name)).resolve() if name else path
            if not target.is_relative_to(ROOT) or not target.exists():
                errors.append(f'{path.relative_to(ROOT)}: missing/unsafe link {link}')
            elif fragment and target.is_file() and target.suffix == '.md':
                if unquote(fragment) not in anchors(target.read_text()):
                    errors.append(f'{path.relative_to(ROOT)}: missing heading {link}')
    if errors:
        sys.exit('\n'.join(errors))
    print(f'Documentation gate passed: {len(paths)} documents, local paths/headings checked')


if __name__ == '__main__':
    main()
