#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"/..

# Usage: replayFuzzCorpus.sh [valgrind]. Replays the seeds and tests/fuzz/corpus.tar.gz through the fuzz target, built
# beta without fuzzer instrumentation. `valgrind` runs the replay under Valgrind's memcheck, for reads of uninitialized
# memory, which the sanitizers miss (Linux only). Each input then gets 25 times the CPU time, to get as far as without.
runner=""
timeLimit="(CLOCKS_PER_SEC/50)"
if [[ "${1:-}" == "valgrind" ]]; then
	runner="valgrind -q --error-exitcode=1"
	timeLimit="(CLOCKS_PER_SEC/2)"
elif [[ $# -gt 0 ]]; then
	echo "Usage: replayFuzzCorpus.sh [valgrind]"
	exit 1
fi
mkdir -p output
bash tools/PikaCmd/SourceDistribution/BuildCpp.sh beta native output/PikaCmdFuzzReplay -DLIBFUZZ "-DLIBFUZZ_TIME_LIMIT=$timeLimit" \
		-DPLATFORM_STRING=UNIX \
		tools/PikaCmd/SourceDistribution/PikaCmdAmalgam.cpp tests/fuzz/FuzzMain.cpp
rm -rf output/fuzzReplay
mkdir -p output/fuzzReplay
tar -xzf tests/fuzz/corpus.tar.gz -C output/fuzzReplay
(ls tests/fuzz/seeds/*.pika && find output/fuzzReplay -type f) | $runner output/PikaCmdFuzzReplay
