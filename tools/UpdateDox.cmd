CD /D "%~dp0\.."
tools\PikaCmd\build\Windows\MSVC10\Release\Win32\PikaCmd.exe tools\ExportHelp.pika
tools\PikaCmd\build\Windows\MSVC10\Release\Win32\PikaCmd.exe tools\UpdateHtmlDox.pika
doxygen docs\PikaScriptDoxyfile
CD docs\latex
pdflatex refman.tex
CD ..
move latex\refman.pdf PikaScript.pdf
