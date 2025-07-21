#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=beta bash BuildPikaCmd.sh)
tools/PikaCmd/SourceDistribution/PikaCmd tests/ppegTest.pika
tools/PikaCmd/SourceDistribution/PikaCmd examples/ppegDocExample.pika
tools/PikaCmd/SourceDistribution/PikaCmd tests/htmlifyTests.pika

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=release bash BuildPikaCmd.sh)
tools/PikaCmd/SourceDistribution/PikaCmd tests/ppegTest.pika
tools/PikaCmd/SourceDistribution/PikaCmd examples/ppegDocExample.pika
tools/PikaCmd/SourceDistribution/PikaCmd tests/htmlifyTests.pika
