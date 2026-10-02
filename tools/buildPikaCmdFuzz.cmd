@ECHO OFF
SETLOCAL ENABLEEXTENSIONS
CD /D "%~dp0\.."

IF NOT EXIST output MD output || GOTO error

REM 8 MB stack like Linux: AddressSanitizer's larger stack frames overflow the 1 MB Windows default.
SET CPP_OPTIONS=/fsanitize=fuzzer /fsanitize=address /F 8388608
CALL tools\PikaCmd\SourceDistribution\BuildCpp.cmd beta native output\PikaCmdFuzz.exe /D LIBFUZZ /D "PLATFORM_STRING=WINDOWS" /I src tools\PikaCmd\PikaCmd.cpp tools\PikaCmd\BuiltIns.cpp src\PikaScript.cpp src\QStrings.cpp || GOTO error

REM The AddressSanitizer runtime DLL must be next to the executable to run it outside a Visual Studio prompt.
FOR /F "delims=" %%F IN ('DIR /B /S "%ProgramFiles%\Microsoft Visual Studio\clang_rt.asan_dynamic-x86_64.dll" 2^>NUL ^| FINDSTR /I "\\Hostx64\\x64\\"') DO SET ASAN_DLL=%%F
IF NOT DEFINED ASAN_DLL (
	ECHO Could not find clang_rt.asan_dynamic-x86_64.dll
	EXIT /b 1
)
COPY /Y "%ASAN_DLL%" output\ >NUL || GOTO error

EXIT /b 0
:error
EXIT /b %ERRORLEVEL%
