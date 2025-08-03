#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"/..

mkdir -p output

CPP_COMPILER="${CPP_COMPILER:-clang++}" \
CPP_OPTIONS="-fsanitize=fuzzer,address" \
bash tools/PikaCmd/SourceDistribution/BuildCpp.sh debug native output/PikaCmdFuzz \
		-DLIBFUZZ -DPLATFORM_STRING=UNIX -I src \
		tools/PikaCmd/PikaCmd.cpp tools/PikaCmd/BuiltIns.cpp src/PikaScript.cpp src/QStrings.cpp

