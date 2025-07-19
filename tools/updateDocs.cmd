CD /D "%~dp0\.."
tools\PikaCmd\SourceDistribution\PikaCmd.exe tools\ExportHelp.pika
tools\PikaCmd\SourceDistribution\PikaCmd.exe tools\UpdateHtmlDox.pika
doxygen docs\PikaScriptDoxyfile
CD docs\latex
pdflatex refman.tex
CD ..
move latex\refman.pdf PikaScript.pdf
