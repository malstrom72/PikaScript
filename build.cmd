@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
CD /D "%~dp0"

PUSHD tools\PikaCmd\SourceDistribution
SET CPP_TARGET=beta
DEL /Q PikaCmd.exe 2>NUL
CALL BuildPikaCmd.cmd
IF ERRORLEVEL 1 GOTO error
POPD
tools\PikaCmd\SourceDistribution\PikaCmd tests\ppegTest.pika
IF ERRORLEVEL 1 GOTO error

tools\PikaCmd\SourceDistribution\PikaCmd examples\ppegDocExample.pika
IF ERRORLEVEL 1 GOTO error
tools\PikaCmd\SourceDistribution\PikaCmd tests\htmlifyTests.pika
IF ERRORLEVEL 1 GOTO error

PUSHD tools\PikaCmd\SourceDistribution
DEL /Q PikaCmd.exe 2>NUL
SET CPP_TARGET=release
CALL BuildPikaCmd.cmd
IF ERRORLEVEL 1 GOTO error
POPD
tools\PikaCmd\SourceDistribution\PikaCmd tests\ppegTest.pika
IF ERRORLEVEL 1 GOTO error

tools\PikaCmd\SourceDistribution\PikaCmd examples\ppegDocExample.pika
IF ERRORLEVEL 1 GOTO error
tools\PikaCmd\SourceDistribution\PikaCmd tests\htmlifyTests.pika
IF ERRORLEVEL 1 GOTO error

EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
