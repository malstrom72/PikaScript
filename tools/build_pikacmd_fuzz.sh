#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"/..

CPP_COMPILER="${CPP_COMPILER:-clang++}"

mkdir -p output

"$CPP_COMPILER" -DLIBFUZZ -DPLATFORM_STRING=UNIX -fsanitize=fuzzer,address -I src \
	tools/PikaCmd/PikaCmd.cpp tools/PikaCmd/BuiltIns.cpp src/PikaScript.cpp src/QStrings.cpp \
		-o output/PikaCmdFuzz

