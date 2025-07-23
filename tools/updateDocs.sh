#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"/..

./output/PikaCmd tools/ExportHelp.pika
./output/PikaCmd tools/UpdateHtmlDox.pika
if command -v doxygen >/dev/null; then
    doxygen docs/PikaScriptDoxyfile
else
    echo "doxygen is required to build documentation" >&2
    exit 1
fi
cd docs/latex
make
cd ..
cp latex/refman.pdf PikaScript.pdf
