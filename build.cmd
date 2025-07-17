@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
CD /D "%~dp0"

PUSHD tools\PikaCmd\SourceDistribution
SET CPP_TARGET=beta
DEL /Q PikaCmd.exe 2>NUL
CALL BuildPikaCmd.cmd
IF ERRORLEVEL 1 GOTO error
.\PikaCmd ..\..\tests\ppegTest.pika
IF ERRORLEVEL 1 GOTO error

DEL /Q PikaCmd.exe 2>NUL
SET CPP_TARGET=release
CALL BuildPikaCmd.cmd
IF ERRORLEVEL 1 GOTO error
.\PikaCmd ..\..\tests\ppegTest.pika
IF ERRORLEVEL 1 GOTO error

POPD

EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
