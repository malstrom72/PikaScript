#!/bin/bash

cd "${0%/*}"

chmod +x ./BuildPikaCmd.sh >/dev/null 2>&1

if [ -e ./PikaCmd ]; then
	chmod +x ./PikaCmd >/dev/null 2>&1
else
	chmod +x ./BuildCpp.sh >/dev/null 2>&1
	./BuildCpp.sh ./PikaCmd -DPLATFORM_STRING=UNIX PikaCmd.cpp BuiltIns.cpp ../src/*.cpp
	if [ $? -ne 0 ]; then
		exit 1
	fi
fi

./PikaCmd UpdateBuiltIns.pika
if [ $? -ne 0 ]; then
	echo Failed updating built-in files
	exit 1
fi

cat ../src/PikaScript.h ../src/PikaScriptImpl.h ../src/QStrings.h ../src/QuickVars.h ../src/PikaScript.cpp ../src/QStrings.cpp BuiltIns.cpp PikaCmd.cpp >SourceDistribution/PikaCmdAmalgam.cpp
if [ $? -ne 0 ]; then
	echo Failed concatenating files
	exit 1
fi

cd SourceDistribution
cp -f ../../src/unittests.pika . && cp -f ../systools.pika . && cp -f ../systoolsTests.pika .
if [ $? -ne 0 ]; then
	echo Failed copying files
	exit 1
fi

rm -f ./PikaCmd
./BuildPikaCmd.sh
if [ $? -ne 0 ]; then
	exit 1
fi

cp -f ./PikaCmd ../
cd ..

exit 0
