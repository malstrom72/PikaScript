#!/bin/bash

cd "${0%/*}"

/usr/local/bin/PikaCmd ExportHelp.pika
/usr/local/bin/PikaCmd UpdateHtmlDox.pika
doxygen PikaScriptDoxyfile
cd latex
make
cd ..
cp latex/refman.pdf PikaScript.pdf
