@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
CD /D "%~dp0"

PUSHD tools\PikaCmd\SourceDistribution
SET CPP_TARGET=beta
DEL /Q PikaCmd.exe 2>NUL
CALL BuildPikaCmd.cmd || GOTO error
POPD
tools\PikaCmd\SourceDistribution\PikaCmd tests\ppegTest.pika || GOTO error

tools\PikaCmd\SourceDistribution\PikaCmd examples\ppegDocExample.pika || GOTO error

tools\PikaCmd\SourceDistribution\PikaCmd examples\htmlifyTests.pika || GOTO error

PUSHD tools\PikaCmd\SourceDistribution
DEL /Q PikaCmd.exe 2>NUL
SET CPP_TARGET=release
CALL BuildPikaCmd.cmd || GOTO error
POPD
tools\PikaCmd\SourceDistribution\PikaCmd tests\ppegTest.pika || GOTO error

tools\PikaCmd\SourceDistribution\PikaCmd examples\ppegDocExample.pika || GOTO error

tools\PikaCmd\SourceDistribution\PikaCmd examples\htmlifyTests.pika || GOTO error

EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
