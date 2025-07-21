#!/usr/bin/env python3
"""Helper wrapper that produces the same htmlify output as htmlify.pika."""
import subprocess
import sys
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]
PIKACMD = REPO_ROOT / 'tools' / 'PikaCmd' / 'SourceDistribution' / 'PikaCmd'
HTMLIFY_PIKA = REPO_ROOT / 'tools' / 'htmlify.pika'


def htmlify_file(path: Path) -> str:
    script = f"include('{HTMLIFY_PIKA.as_posix()}'); print(htmlify(load('{path.as_posix()}')));"
    with tempfile.NamedTemporaryFile('w', delete=False, suffix='.pika') as f:
        f.write(script)
        script_path = Path(f.name)
    try:
        out = subprocess.check_output([str(PIKACMD), str(script_path)], text=True)
    finally:
        script_path.unlink()
    # filter out progress lines starting with 'Processing '
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
