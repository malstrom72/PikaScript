#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"

(cd PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=beta bash BuildPikaCmd.sh)

(cd PikaCmd/SourceDistribution && rm -f PikaCmd && CPP_TARGET=release bash BuildPikaCmd.sh)
