# Papyrus Profiler
[![C++23](https://img.shields.io/static/v1?label=standard&message=C%2B%2B23&color=blue&logo=c%2B%2B&&logoColor=white&style=flat)](https://en.cppreference.com/w/cpp/compiler_support)
[![Platform](https://img.shields.io/static/v1?label=platform&message=windows&color=dimgray&style=flat)](#)
![GitHub](https://img.shields.io/github/license/DennisSoemers/PapyrusProfiler)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/DennisSoemers/PapyrusProfiler)

A profiler for Skyrim's [Papyrus](https://www.creationkit.com/index.php?title=Category:Papyrus) scripting language. 

## Introduction

With the plugin in this repository, we can collect information about the function calls that are running in Skyrim's scripting language (Papyrus) at runtime, and convert this into nice plots that help us analyse which scripts (and which functions in them) are being called a lot. For example, I obtain the following graph after running around outside and inside Whiterun for 5 minutes (with a large list of installed mods).

[![Example image of a generated flamegraph.](https://dennissoemers.github.io/assets/papyrus_profiler/FlameGraph_5mins_Whiterun.svg)](https://dennissoemers.github.io/assets/papyrus_profiler/FlameGraph_5mins_Whiterun.svg)

If you open this graph in a new browser tab, you will also be able to mouse over individual elements of the graph to get information about them, or click them to zoom in on certain stacks.

Note that, while the game already has some basic built-in support for Papyrus profiling, that is always restricted to specific [scripts](https://www.creationkit.com/index.php?title=StartScriptProfiling_-_Debug), [stacks](https://www.creationkit.com/index.php?title=StartStackProfiling_-_Debug), or [objects](https://www.creationkit.com/index.php?title=StartObjectProfiling_-_Form). In contrast, the profiling from this plugin rather gives insights into the scripting load of your entire game installation (with potentially hundreds of mods you may have installed), rather than the performance of a small part of your game's scripts.

## Download

The plugin can be downloaded from TODO [its NexusMods page]().

---

# Usage

Using the profiler consists of two separate steps:

1. [Collecting function call data](#collecting-function-call-data) at runtime: we'll have to play Skyrim with the plugin installed such that the plugin can collect data and write it to a text file. We can specify when to start and or stop collecting data during our playing session in a few different ways, which are explained below.
2. [Generating the Flame Graph](#generating-the-flame-graph): after the plugin has written the data we need to a text file, we can use [a separate tool](https://github.com/brendangregg/FlameGraph) to turn it into a nice, interactive plot.

## 1. Collecting Function Call Data

There are two main ways in which you can make the plugin collect function call data from gameplay:

1. By running console commands to start and stop profiling.
2. By specifying a configuration file (which describes how and when you want profiling to happen) in the plugin's INI file, which will then be used as soon as you launch your game.

### 1.1 Console Commands

In order to be able to start and stop profiling from the in-game console, it is required to have the `PapyrusProfiler.esp` file (which can be downloaded from the [TODO PapyrusProfiler NexusMods page]()) active. This, in turn, also requires the [NL_CMD Console Command Framework](https://www.nexusmods.com/skyrimspecialedition/mods/62497) to be installed and active.

In the in-game console (by default opened using the ~ key), Papyrus profiling can be started using the following command:

```
nl_cmd StartPapyrusProfiling()
```

![Screenshot of in-game console window with the command to start profiling entered.](/screenshots/StartPapyrusProfiling.jpg)

After entering this command, the profiler will start collecting data on all Papyrus function calls. Note that it will not write any of this data to a file until you manually stop the profiling. So, if you quit your game (or it crashes) without having stopped the profiler, no profiling data will be retained. Papyrus profiling can be stopped with the following console command:

```
nl_cmd StopPapyrusProfiling()
```

![Screenshot of in-game console window with the command to stop profiling entered.](/screenshots/StopPapyrusProfiling.jpg)

After entering this command, the profiler will write all the data it has collected to `<SKSE_LOGS_DIR>/PapyrusProfiler/PapyrusProfilerOutput_<i>.log`. `<SKSE_LOGS_DIR>` is the directory where SKSE and most SKSE plugins write their logs. Typically, this is `<USER>/Documents/My Games/Skyrim Special Edition/SKSE`. The `<i>` is simply an index, which can be 0, 1, 2, or 3. It will use 0 for the first time a file is written, 1 for the second, etc. If all four possible files already exist, it will overwrite whichever file has not been edited for the longest amount of time (meaning that it will typically just keep cycling from 0 to 3 and back to 0).

It is also possible to modify the behaviour of the profiler, by using a different console command to start the profiling:

```
nl_cmd StartPapyrusProfilingConfig(<filename>)
```

In this command, `<filename>` is expected to be the name of a `.json` or `.yaml` file which specifies the *configuration* you wish to use for profiling. How such files work is explained in more detail [below](#13-configuration-files). Depending on the exact configuration you use, it may not even be necessary anymore to manually stop the profiling (though you always can).

### 1.2 INI File

The download of Papyrus Profiler includes a `/SKSE/Plguins/PapyrusProfiler.ini` file, right alongside the plugin's `.dll` file. By default, the contents of this INI file are as follows:

```INI
[PapyrusProfilerSettings]

;Filename (or filepath) for Profiling Config to use on startup. Leave empty to not run any config on game load.
StartupConfig = 

```

With these default contents, the profiler does not run any configuration on game startup. However, you may edit this file by providing a filename or filepath for a file that specifies a configuration, and then the profiler will start using this configuration immediately when you load a game (or start a new game). If you only specify a filename (for example: `StartupConfig = Skip30sec_Profile5min.json`), the profiler will look for a configuration with this name in the `/SKSE/Plugins/` directory (where `PapyrusProfiler.dll` and `PapyrusProfiler.ini` are also located). If you specify a full filepath, it could be anywhere on your computer. More details on exactly how configuration files work are provided next.

### 1.3 Configuration Files



### 1.4 Starting and Stopping Profiling from Papyrus Scripts

## 2. Generating the Flame Graph

The above process will have written its output to `<SKSE_LOGS_DIR>/PapyrusProfiler/PapyrusProfilerOutput.log`, where 
`<SKSE_LOGS_DIR>` is the directory where SKSE and most SKSE plugins write their logs. Typically, this is
`<USER>/Documents/My Games/Skyrim Special Edition/SKSE`.

To turn this output into a Flame Graph, we'll use the [FlameGraph repository](https://github.com/brendangregg/FlameGraph).
After cloning that and opening a command prompt in its root directory, we can run the following command to generate
a Flame Graph:

```
perl ./flamegraph.pl "<SKSE_LOGS_DIR>/PapyrusProfiler/PapyrusProfilerOutput.log" > "SKSE_LOGS_DIR>/PapyrusProfiler/FlameGraph.svg" --width 1800
```

This will write the `FlameGraph.svg` file right next to the output log, which we can then open and inspect
in any internet browser.

---

## Limitations

TODO

## Runtime Requirements (for users)

At runtime, the plugin has the following requirements. Any user of this plugin (or mods that in turn require this plugin) need to have this installed locally.

- The Elder Scrolls V: Skyrim Special Edition.
  - Tested with version 1.5.97 (SSE).
- [Skyrim Script Extender](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)

## Build Requirements (for plugin developers)

Building the code in this repository has the following requirements.

- [Visual Studio](https://visualstudio.microsoft.com/).
- [CMake](https://cmake.org/).
- [Vcpkg](https://github.com/microsoft/vcpkg).

This project was set up exactly as in the [CommonLibSSE NG Sample Plugin](https://gitlab.com/colorglass/commonlibsse-sample-plugin), 
and I refer to that repository for highly detailed instructions on installation and building.

## Credits

Thanks to:
- [Nikitalita](https://github.com/nikitalita/) for direct code contributions, as well as discussions about the project and its code.
- [Nightfallstorm](https://github.com/Nightfallstorm/) for the hook and example code taken from [Skyrim Recursion FPS Fix](https://github.com/Nightfallstorm/Skyrim-Recursion-FPS-Fix), as well as discussions about the project and its code.
- [CharmedBaryon](https://github.com/CharmedBaryon/) for [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) and the [CommonLibSSE NG Sample Plugin](https://gitlab.com/colorglass/commonlibsse-sample-plugin).
- The SKSE authors for [SKSE](http://skse.silverlock.org/).
- Meh321 for [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444).

## Disclaimer

- THIS MATERIAL IS NOT MADE, GUARANTEED OR SUPPORTED BY ZENIMAX OR ITS AFFILIATES.
- ALL FILES IN THE DOWNLOAD ARE PROVIDED ''AS IS'' WITHOUT ANY WARRANTY OR GUARANTEE OF ANY KIND. IN NO EVENT CAN THE AUTHOR BE HELD RESPONSIBLE FOR ANY CLAIMS, WHETHER OR NOT THEY ARE RELATED TO THE DOWNLOAD.
