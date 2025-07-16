#!/bin/sh

set -e

if [ -z "${TMPDIR}" ]; then
	TMPDIR="/tmp/"
fi

cd "${TMPDIR}"
curl http://nuedge.net/pikascript/PikaCmdSourceDistribution.tar.gz -o PikaCmdSourceDistribution.tar.gz
mkdir -p "PikaCmd"
cd "PikaCmd"
rm -rf "SourceDistribution" || true
tar -xzf "../PikaCmdSourceDistribution.tar.gz"
rm -f PikaCmdSourceDistribution.tar.gz
cd "SourceDistribution"
sh BuildPikaCmd.sh
sh InstallPika.sh
cd ..
rm -rf "SourceDistribution"
echo SUCCESS
