# PikaScript

PikaScript is a minimal interpreted scripting language implemented in C++.
This repository contains the language runtime, standard library, a command-line tool called `PikaCmd`, and various example scripts and documentation.

## Repository Layout

```
src/       - C++ source code of the PikaScript interpreter and standard library
PikaCmd/   - command-line tool sources, build scripts, installation helpers
lab/       - assorted PikaScript samples and experiments
dox/       - generated documentation and helper scripts
```

The main runtime implementation is in `src/PikaScript.cpp`/`PikaScript.h`.
Examples and test scripts can be found in `lab/`, while `dox/` contains HTML documentation describing the standard library.

## Building PikaCmd

The easiest way to build the command-line tool is to run the provided build script under `PikaCmd/SourceDistribution`:

```bash
cd PikaCmd/SourceDistribution
./BuildPikaCmd.sh       # builds PikaCmd and runs unit tests
```

`BuildPikaCmd.sh` calls `BuildCpp.sh` internally to compile `PikaCmdAmalgam.cpp` and execute available unit tests. The relevant lines show the build step and tests:

```
6  ./BuildCpp.sh ./PikaCmd -DPLATFORM_STRING=UNIX PikaCmdAmalgam.cpp
10 echo Testing...
12 ./PikaCmd unittests.pika >/dev/null
18 if [ -e ./systoolsTests.pika ]; then
```

After building, `PikaCmd` can be installed system-wide using `InstallPika.sh`, which copies the binary and helper scripts to `/usr/local/bin`:

```
5 chmod +x ./pika
16 sudo mv ./PikaCmd /usr/local/bin/
17 sudo cp ./pika /usr/local/bin/
18 sudo cp systools.pika /usr/local/bin/
```

The file `DownloadAndInstallReadMe.txt` also provides one-line boot-strap commands for Windows or Unix systems to download and install PikaScript:

```
powershell.exe -Command "(New-Object System.Net.WebClient).DownloadFile('http://nuedge.net/pikascript/install.bat','%TEMP%\\install.bat')" && %TEMP%\\install.bat
curl -s http://nuedge.net/pikascript/install.sh | sh
```

## Running Scripts

Once built, you can run a PikaScript program via:

```bash
./PikaCmd myscript.pika
```

For an interactive REPL experience, use `interactive.pika` (located in `src/`):

```
Enter expressions to evaluate them interactively line by line. E.g.:

    3+3
    print('hello world')
    run('chess.pika')
```

The interactive environment provides several special commands:

```
?                        this help
<page>?                  shows a page from the standard library help system
=                        displays the full definition of the last evaluated expression
<variable>=              displays the full definition of a variable / function / container
%                        re-run last executed PikaScript source file
%['']<path>[''] [args...]  runs a PikaScript source file (optionally with arguments)
!<command>               executes a command with the operating system's command interpreter
exit                     exits
```

## Documentation

HTML documentation for the standard library lives in `dox/help/`. Open `dox/help/index.html` to browse available functions and examples.

## License

PikaScript is released under the BSD 2-Clause license. See [LICENSE](LICENSE) for details.

