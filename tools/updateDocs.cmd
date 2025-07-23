CD /D "%~dp0\.."
output\PikaCmd.exe tools\ExportHelp.pika
output\PikaCmd.exe tools\UpdateHtmlDox.pika
doxygen docs\PikaScriptDoxyfile
CD docs\latex
pdflatex refman.tex
CD ..
move latex\refman.pdf PikaScript.pdf
