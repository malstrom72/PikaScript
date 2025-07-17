#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=beta bash BuildPikaCmd.sh)
tools/PikaCmd/SourceDistribution/PikaCmd tools/ppeg/ppegTest.pika

(cd tools/PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=release bash BuildPikaCmd.sh)
tools/PikaCmd/SourceDistribution/PikaCmd tools/ppeg/ppegTest.pika
