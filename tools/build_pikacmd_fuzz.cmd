@ECHO OFF
SETLOCAL
CD /D "%~dp0\.."
IF NOT DEFINED CPP_COMPILER SET "CPP_COMPILER=clang++"
IF NOT EXIST output MKDIR output
%CPP_COMPILER% -DLIBFUZZ -DPLATFORM_STRING=WINDOWS -fsanitize=fuzzer,address ^
        -I src tools/PikaCmd/PikaCmd.cpp tools/PikaCmd/BuiltIns.cpp src/PikaScript.cpp src/QStrings.cpp ^
        -o output\PikaCmdFuzz.exe || GOTO error
EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
