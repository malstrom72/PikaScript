..\PikaCmd\build\Windows\MSVC10\Release\Win32\PikaCmd.exe ExportHelp.pika
..\PikaCmd\build\Windows\MSVC10\Release\Win32\PikaCmd.exe UpdateHtmlDox.pika
doxygen PikaScriptDoxyfile
cd latex
pdflatex refman.tex
cd ..
move latex\refman.pdf PikaScript.pdf
