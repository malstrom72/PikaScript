@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
CD /D "%~dp0\.."

REM Usage: fuzzPikaCmd.cmd [seconds = 600] [parallel jobs = 8]. Build output\PikaCmdFuzz.exe first with buildPikaCmdFuzz.cmd.
SET DURATION=%~1
IF "%DURATION%"=="" SET DURATION=600
SET JOBS=%~2
IF "%JOBS%"=="" SET JOBS=8

IF NOT EXIST output\fuzz\corpus MD output\fuzz\corpus || GOTO error
IF NOT EXIST output\fuzz\crashes MD output\fuzz\crashes || GOTO error
COPY /Y tests\fuzz\seeds\*.pika output\fuzz\corpus\ >NUL || GOTO error
CD output\fuzz
..\PikaCmdFuzz.exe -jobs=%JOBS% -workers=%JOBS% -max_total_time=%DURATION% -max_len=4096 -timeout=10 -dict=..\..\tests\fuzz\pika.dict -artifact_prefix=crashes\ corpus || GOTO error

EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
