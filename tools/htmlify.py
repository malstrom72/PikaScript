#!/usr/bin/env python3
"""Wrapper around htmlify.pika with pre-processing for blank lines.

Blank lines are padded with indentation when the following line is indented
as part of a pre block (four or more spaces) or a bullet list item. This
ensures that inputs with trimmed whitespace produce the same output as their
untrimmed counterparts.
"""
import subprocess
import sys
import tempfile
import re
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
PIKACMD = REPO_ROOT / 'tools' / 'PikaCmd' / 'SourceDistribution' / 'PikaCmd'
HTMLIFY_PIKA = REPO_ROOT / 'tools' / 'htmlify.pika'


_BULLET_RE = re.compile(r'^(\s*)(?:[-*]|\d+\))\s+')


def _preprocess(text: str) -> str:
    """Pad blank lines so htmlify handles them consistently."""
    lines = text.splitlines()
    res = []
    for i, line in enumerate(lines):
        if line.strip():
            res.append(line)
            continue

        next_line = lines[i + 1] if i + 1 < len(lines) else ''
        prev_line = lines[i - 1] if i > 0 else ''

        next_indent = len(next_line) - len(next_line.lstrip(' '))
        prev_indent = len(prev_line) - len(prev_line.lstrip(' '))
        bullet_match = _BULLET_RE.match(prev_line)

        if prev_indent >= 4 and next_indent >= 4:
            indent = 4
        elif bullet_match and next_indent >= prev_indent:
            indent = len(bullet_match.group(0))
        elif prev_indent > 0 and next_indent >= prev_indent:
            indent = prev_indent
        else:
            indent = 0

        res.append(' ' * indent)

    return '\n'.join(res) + ('\n' if text.endswith('\n') else '')


def htmlify_file(path: Path) -> str:
    src = _preprocess(path.read_text())
    with tempfile.NamedTemporaryFile('w', delete=False, suffix='.txt') as t:
        t.write(src)
        tpath = Path(t.name)

    script = (
        f"include('{HTMLIFY_PIKA.as_posix()}'); "
        f"print(htmlify(load('{tpath.as_posix()}')));"
    )
    with tempfile.NamedTemporaryFile('w', delete=False, suffix='.pika') as f:
        f.write(script)
        script_path = Path(f.name)

    try:
        out = subprocess.check_output([str(PIKACMD), str(script_path)], text=True)
    finally:
        script_path.unlink()
        tpath.unlink()

    lines = [l for l in out.splitlines() if not l.startswith('Processing ')]
    return '\n'.join(lines)


def main(argv=None):
    import argparse
    parser = argparse.ArgumentParser(description='htmlify text via PikaScript')
    parser.add_argument('src', help='input text file')
    args = parser.parse_args(argv)
    html = htmlify_file(Path(args.src))
    sys.stdout.write(html)

if __name__ == '__main__':
    main()
