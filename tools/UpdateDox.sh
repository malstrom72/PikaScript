#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"/..

/usr/local/bin/PikaCmd tools/ExportHelp.pika
/usr/local/bin/PikaCmd tools/UpdateHtmlDox.pika
doxygen docs/PikaScriptDoxyfile
cd docs/latex
make
cd ..
cp latex/refman.pdf PikaScript.pdf
