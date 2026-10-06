@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
CD /D "%~dp0\.."

REM Usage: updateFuzzCorpus.cmd. Minimizes output\fuzz\corpus from fuzzPikaCmd.cmd into tests\fuzz\corpus.tar.gz.
IF EXIST output\fuzz\minimized RD /S /Q output\fuzz\minimized
MD output\fuzz\minimized || GOTO error
IF NOT EXIST output\fuzz\crashes MD output\fuzz\crashes || GOTO error
output\PikaCmdFuzz.exe -merge=1 -max_len=4096 -timeout=10 -artifact_prefix=output\fuzz\crashes\ output\fuzz\minimized output\fuzz\corpus || GOTO error
DIR /B /ON /A-D output\fuzz\minimized >output\fuzz\minimized.txt || GOTO error
PUSHD output\fuzz\minimized
tar --no-xattrs -czf ..\..\..\tests\fuzz\corpus.tar.gz -T ..\minimized.txt || GOTO error
POPD
FOR /F %%N IN ('FIND /C /V "" ^<output\fuzz\minimized.txt') DO ECHO %%N files in tests\fuzz\corpus.tar.gz
EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
