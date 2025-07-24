WINDOWS
-------

One line install boot strap:
	
powershell.exe -Command "(New-Object System.Net.WebClient).DownloadFile('https://github.com/malstrom72/PikaScript/releases/latest/download/install.bat','%TEMP%\install.bat')" && %TEMP%\install.bat


MAC / UNIX
----------

One line install boot strap:
	
curl -fsL https://github.com/malstrom72/PikaScript/releases/latest/download/install.sh | sh

On Ubuntu you may have to first install curl, g++ and 32-bit libraries:

	sudo apt-get install curl g++ g++-multilib

On FreeBSD you may have to first install sudo and gcc:

        pkg install sudo gcc

To manually download the distribution on Unix systems you can run:

        curl -fL -o PikaCmdSourceDistribution.tar.gz \
                https://github.com/malstrom72/PikaScript/releases/latest/download/PikaCmdSourceDistribution.tar.gz \
        || curl -fL -o PikaCmdSourceDistribution.zip \
                https://github.com/malstrom72/PikaScript/releases/latest/download/PikaCmdSourceDistribution.zip
        if [ -f PikaCmdSourceDistribution.tar.gz ]; then
                tar -xzvf PikaCmdSourceDistribution.tar.gz
        else
                unzip PikaCmdSourceDistribution.zip
        fi
