#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"/..

# Usage: updateFuzzCorpus.sh. Minimizes output/fuzz/corpus from fuzzPikaCmd.sh into tests/fuzz/corpus.tar.gz.
rm -rf output/fuzz/minimized
mkdir -p output/fuzz/minimized output/fuzz/crashes
output/PikaCmdFuzz -merge=1 -max_len=4096 -timeout=10 -artifact_prefix=output/fuzz/crashes/ output/fuzz/minimized output/fuzz/corpus
ls output/fuzz/minimized | LC_ALL=C sort >output/fuzz/minimized.txt
(cd output/fuzz/minimized && COPYFILE_DISABLE=1 tar --no-xattrs -czf ../../../tests/fuzz/corpus.tar.gz -T ../minimized.txt)
echo "$(wc -l <output/fuzz/minimized.txt) files in tests/fuzz/corpus.tar.gz"
