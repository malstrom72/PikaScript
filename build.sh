#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=beta bash BuildPikaCmd.sh)
tools/PikaCmd/SourceDistribution/PikaCmd tests/ppegTest.pika
tools/PikaCmd/SourceDistribution/PikaCmd examples/ppegDocExample.pika
tools/PikaCmd/SourceDistribution/PikaCmd tests/htmlifyTests.pika

mkdir -p output
bash tools/PikaCmd/SourceDistribution/BuildCpp.sh beta native output/PikaCmdFuzzReplay -DLIBFUZZ -DPLATFORM_STRING=UNIX \
		tools/PikaCmd/SourceDistribution/PikaCmdAmalgam.cpp tests/fuzz/FuzzMain.cpp
rm -rf output/fuzzReplay
mkdir -p output/fuzzReplay
tar -xzf tests/fuzz/corpus.tar.gz -C output/fuzzReplay
(ls tests/fuzz/seeds/*.pika && find output/fuzzReplay -type f) | output/PikaCmdFuzzReplay

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=release bash BuildPikaCmd.sh)
mkdir -p output
cp -f tools/PikaCmd/SourceDistribution/PikaCmd output/PikaCmd
cp -f tools/PikaCmd/SourceDistribution/systools.pika output/
output/PikaCmd tests/ppegTest.pika
output/PikaCmd examples/ppegDocExample.pika
output/PikaCmd tests/htmlifyTests.pika
