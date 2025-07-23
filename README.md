# PikaScript

PikaScript is a minimal interpreted scripting language implemented in C++. This repository contains the language runtime, standard library, a command-line tool called `PikaCmd`, and various example scripts and documentation.

## History

PikaScript began as a tiny scripting experiment and has hardly changed since 2008. It is now something of a legacy technology, but it remains very stable and is still maintained and used at Sonic Charge. See [docs/PikaScript History.txt](docs/PikaScript%20History.txt) for more background.

## Repository Layout

```
src/            - C++ source code of the PikaScript interpreter and standard library
tools/PikaCmd/  - command-line tool sources, build scripts, installation helpers
examples/       - samples and experiments (various quality, many of them unfinished)
docs/           - documentation (text and HTML formats)
tests/          - tests and helper scripts
```

## Building PikaCmd

The easiest way to build the command-line tool is to run the build script from
the repository root:

```bash
bash tools/PikaCmd/SourceDistribution/BuildPikaCmd.sh
```

`BuildPikaCmd.sh` calls `BuildCpp.sh` internally to compile `PikaCmdAmalgam.cpp` and execute available unit tests.

After building, `PikaCmd` can be installed system-wide using `InstallPika.sh`, which copies the binary and helper scripts to `/usr/local/bin`.

Equivalent scripts are also available for Windows.

For a detailed overview of `PikaCmd`, the helper script `systools.pika` and how to write portable build scripts, see [docs/PikaCmd Documentation.txt](docs/PikaCmd%20Documentation.txt).

The file `DownloadAndInstallReadMe.txt` also provides one-line boot-strap commands for Windows or Unix systems to download and install PikaScript:

```bat
powershell.exe -Command "(New-Object System.Net.WebClient).DownloadFile('http://nuedge.net/pikascript/install.bat','%TEMP%\\install.bat')" && %TEMP%\\install.bat
```
```bash
curl -s http://nuedge.net/pikascript/install.sh | sh
```

## Running Scripts

Once built, you can run a PikaScript program via:

```bash
output/PikaCmd myscript.pika
```

For an interactive REPL experience, run `output/PikaCmd` without arguments (this will invoke `interactive.pika`):

```
Enter expressions to evaluate them interactively line by line. E.g.:

    3+3
    print('hello world')
    run('chess.pika')
```

The interactive environment provides several special commands:

```
?                           this help
<page>?                     shows a page from the standard library help system
=                           displays the full definition of the last evaluated expression
<variable>=                 displays the full definition of a variable / function / container
%                           re-run last executed PikaScript source file
%['']<path>[''] [args...]   runs a PikaScript source file (optionally with arguments)
!<command>                  executes a command with the operating system's command interpreter
exit                        exits
```

## Debugging

To enable assertions and debugging helpers, include the built-in `debug.pika` script at the start of your program:

```pika
include('debug.pika');
```

This installs a tracer that prints error offsets and a short call stack whenever an exception is caught. It also defines the `debug()` function for step-by-step execution and exposes `trace()` helpers for more verbose output.

## PPEG Parser Generator

PPEG is a self-hosting parser generator. All PPEG related files live under `tools/ppeg`. For details on how to run them, see [docs/PPEG Documentation.txt](docs/PPEG%20Documentation.txt).

## Documentation

HTML documentation for the standard library lives in `docs/help/`. Open `docs/help/index.html` to browse available functions and examples.

## Testing

The standard library examples shown in `docs/help` also serve as the unit tests. `tests/unittests.pika` loads each documentation page, extracts the example code, and executes it. This double-checking system keeps the docs accurate and ensures the library functions behave as documented.

## License

PikaScript is released under the BSD 2-Clause license. See [LICENSE](LICENSE) for details.

