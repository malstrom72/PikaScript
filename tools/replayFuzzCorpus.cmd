@ECHO OFF
SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION
CD /D "%~dp0\.."

REM Usage: replayFuzzCorpus.cmd. Replays the seeds and tests\fuzz\corpus.tar.gz through the fuzz target, built beta
REM without fuzzer instrumentation. Like replayFuzzCorpus.sh, whose `valgrind` option is Linux only.
IF "%~1"=="valgrind" (
	ECHO Valgrind runs on Linux only: use replayFuzzCorpus.sh there.
	EXIT /B 1
)
IF NOT "%~1"=="" (
	ECHO Usage: replayFuzzCorpus.cmd
	EXIT /B 1
)
IF NOT EXIST output MD output
CALL tools\PikaCmd\SourceDistribution\BuildCpp.cmd beta native output\PikaCmdFuzzReplay.exe /D LIBFUZZ /D "LIBFUZZ_TIME_LIMIT=(CLOCKS_PER_SEC/50)" /D "PLATFORM_STRING=WINDOWS" tools\PikaCmd\SourceDistribution\PikaCmdAmalgam.cpp tests\fuzz\FuzzMain.cpp || GOTO error
IF EXIST output\fuzzReplay RD /S /Q output\fuzzReplay
MD output\fuzzReplay || GOTO error
tar -xzf tests\fuzz\corpus.tar.gz -C output\fuzzReplay || GOTO error
DIR /B /S /A-D tests\fuzz\seeds\*.pika output\fuzzReplay | output\PikaCmdFuzzReplay.exe || GOTO error

EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
