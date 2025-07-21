#!/usr/bin/env python3
"""Simplified Python port of htmlify.pika.
Supports headers, preformatted blocks, lists and basic inline markup.
"""
import re
import sys

HTML_XLATE = {
    ord(' '): ' ',
    ord('\n'): ' <br>\n',
    ord('<'): '&lt;',
    ord('>'): '&gt;',
    ord('&'): '&amp;',
    229: '&aring;', 228: '&auml;', 246: '&ouml;',
    197: '&Aring;', 196: '&Auml;', 214: '&Ouml;',
    233: '&eacute;', 201: '&Eacute;', 252: '&uuml;',
    220: '&Uuml;', 241: '&ntilde;', 209: '&Ntilde;',
}

HEADER_CHARS = {'#': 'h1', '=': 'h2', '-': 'h3'}

INLINE_PATTERNS = [
    (re.compile(r'\*([^\n*]+)\*'), r'<b>\1</b>'),
    (re.compile(r'/([^\n/]+)/'), r'<i>\1</i>'),
    (re.compile(r'_([^\n_]+)_'), r'<u>\1</u>'),
    (re.compile(r'`([^`]+)`'), r'<tt>\1</tt>'),
    (re.compile(r'(https?://[\w./?&;%-=#]+)'), r'<a href="\1">\1</a>'),
]

def html(text: str) -> str:
    out = []
    for ch in text:
        cp = ord(ch)
        if cp in HTML_XLATE:
            out.append(HTML_XLATE[cp])
        elif cp < 128:
            out.append(ch)
        else:
            out.append(f'&#{cp};')
    return ''.join(out)

def format_inline(text: str) -> str:
    for pat, repl in INLINE_PATTERNS:
        text = pat.sub(repl, text)
    return text

def expand_tabs(text: str, width: int = 4) -> str:
    return text.expandtabs(width)

def parse_sections(src: str):
    pos = 0
    n = len(src)
    while pos < n:
        end = src.find('\n\n', pos)
        if end == -1:
            end = n
        section = src[pos:end]
        pos = end + 2 if end + 2 <= n else n
        yield section

def htmlify(src: str) -> str:
    src = src.replace('\r\n', '\n')
    src = expand_tabs(src)
    if src.endswith('\n'):
        src = src[:-1]

    body = []
    sections = list(parse_sections(src))

    for sec in sections:
        lines = sec.split('\n')
        first = lines[0]
        if len(lines) == 2 and lines[1] and set(lines[1]) <= set('=#-') and len(lines[1]) == len(first):
            tag = HEADER_CHARS.get(lines[1][0])
            if tag:
                head = html(first)
                anchor = first.replace(' ', '_')
                body.append(f'<{tag}><a name="{anchor}">{head}</a></{tag}>\n')
                continue
        leading = len(first) - len(first.lstrip(' '))
        if leading >= 4:
            body.append('<pre>')
            for l in lines:
                body.append(html(l[leading:]) + '<br>')
            body.append('</pre>\n')
            continue
        bullet_match = re.match(r'\s*(?:[0-9]+\)|\*\)|-)\s+(.*)', first)
        if bullet_match:
            body.append('<ul>\n')
            for l in lines:
                m = re.match(r'\s*(?:[0-9]+\)|\*\)|-)\s+(.*)', l)
                if m:
                    body.append(f'<li>{format_inline(html(m.group(1)))}</li>\n')
            body.append('</ul>\n')
            continue
        if first.strip().startswith('===='):
            body.append('<hr>\n')
            continue
        paragraph = format_inline(html(sec))
        body.append(f'<p>{paragraph}</p>\n')

    return ''.join(body)

def main(argv=None):
    import argparse
    parser = argparse.ArgumentParser(description='htmlify text')
    parser.add_argument('src', help='input text file')
    args = parser.parse_args(argv)
    with open(args.src, 'r', encoding='utf-8') as f:
        text = f.read()
    sys.stdout.write(htmlify(text))

if __name__ == '__main__':
    main()
