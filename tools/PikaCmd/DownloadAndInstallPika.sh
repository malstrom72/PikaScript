#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"

if [ -z "${TMPDIR:-}" ]; then
	TMPDIR="/tmp/"
fi

cd "${TMPDIR}"
echo "Downloading, please wait..."
ARCH="tar"
if ! curl -fL https://github.com/malstrom72/PikaScript/releases/latest/download/PikaCmdSourceDistribution.tar.gz -o PikaCmdSourceDistribution.tar.gz; then
	ARCH="zip"
	curl -fL https://github.com/malstrom72/PikaScript/releases/latest/download/PikaCmdSourceDistribution.zip -o PikaCmdSourceDistribution.zip
fi
mkdir -p "PikaCmd"
cd "PikaCmd"
rm -rf "SourceDistribution" || true
echo "Extracting..."
if [ "$ARCH" = "tar" ]; then
	tar -xzf "../PikaCmdSourceDistribution.tar.gz"
	rm -f "../PikaCmdSourceDistribution.tar.gz"
else
	unzip -q "../PikaCmdSourceDistribution.zip"
	rm -f "../PikaCmdSourceDistribution.zip"
fi
cd "SourceDistribution"
bash BuildPikaCmd.sh
bash InstallPika.sh
cd ..
rm -rf "SourceDistribution"
echo SUCCESS
