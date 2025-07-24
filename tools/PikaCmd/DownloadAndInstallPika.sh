#!/usr/bin/env bash
set -e -o pipefail -u
cd "$(dirname "$0")"

if [ -z "${TMPDIR:-}" ]; then
        TMPDIR="/tmp/"
fi

cd "${TMPDIR}"
ARCHIVE="tar.gz"
if ! curl -fL https://github.com/malstrom72/PikaScript/releases/latest/download/PikaCmdSourceDistribution.tar.gz -o PikaCmdSourceDistribution.tar.gz; then
        ARCHIVE="zip"
        curl -fL https://github.com/malstrom72/PikaScript/releases/latest/download/PikaCmdSourceDistribution.zip -o PikaCmdSourceDistribution.zip
fi
mkdir -p "PikaCmd"
cd "PikaCmd"
rm -rf "SourceDistribution" || true
if [ "$ARCHIVE" = "tar.gz" ]; then
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
