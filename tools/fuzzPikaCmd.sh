#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"/..

# Usage: fuzzPikaCmd.sh [seconds = 600] [parallel jobs = 8]. Build output/PikaCmdFuzz first with buildPikaCmdFuzz.sh.
DURATION="${1:-600}"
JOBS="${2:-8}"

mkdir -p output/fuzz/corpus output/fuzz/crashes
cp tests/fuzz/seeds/*.pika output/fuzz/corpus/
tar -xzf tests/fuzz/corpus.tar.gz -C output/fuzz/corpus
cd output/fuzz
../PikaCmdFuzz -jobs="$JOBS" -workers="$JOBS" -max_total_time="$DURATION" -max_len=4096 -timeout=10 \
		-dict=../../tests/fuzz/pika.dict -artifact_prefix=crashes/ corpus
