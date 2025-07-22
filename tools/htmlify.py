#!/usr/bin/env python3
"""Wrapper around htmlify.pika."""
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
PIKACMD = REPO_ROOT / 'tools' / 'PikaCmd' / 'SourceDistribution' / 'PikaCmd'
HTMLIFY_PIKA = REPO_ROOT / 'tools' / 'htmlify.pika'
def htmlify_file(path: Path) -> str:
    src = Path(path).read_text()
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
