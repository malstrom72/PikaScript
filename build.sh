#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=beta bash BuildPikaCmd.sh)
tools/PikaCmd/SourceDistribution/PikaCmd tests/ppegTest.pika
tools/PikaCmd/SourceDistribution/PikaCmd examples/ppegDocExample.pika
tools/PikaCmd/SourceDistribution/PikaCmd tests/htmlifyTests.pika

bash tools/replayFuzzCorpus.sh

bash tools/PikaCmd/SourceDistribution/BuildCpp.sh beta native output/StackTest src/PikaScript.cpp src/QStrings.cpp tests/StackTest.cpp
(ulimit -s 1024 && output/StackTest)

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=release bash BuildPikaCmd.sh)
mkdir -p output
cp -f tools/PikaCmd/SourceDistribution/PikaCmd output/PikaCmd
cp -f tools/PikaCmd/SourceDistribution/systools.pika output/
output/PikaCmd tests/ppegTest.pika
output/PikaCmd examples/ppegDocExample.pika
output/PikaCmd tests/htmlifyTests.pika

bash tools/PikaCmd/SourceDistribution/BuildCpp.sh release native output/StackTest src/PikaScript.cpp src/QStrings.cpp tests/StackTest.cpp
(ulimit -s 1024 && output/StackTest)
