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

The plugin can be downloaded from [its NexusMods page](https://www.nexusmods.com/skyrimspecialedition/mods/82770/).

---

# Usage

Using the profiler consists of two separate steps:

1. [Collecting function call data](#1-collecting-function-call-data) at runtime: we'll have to play Skyrim with the plugin installed such that the plugin can collect data and write it to a text file. We can specify when to start and or stop collecting data during our playing session in a few different ways, which are explained below.
2. [Generating the Flame Graph](#2-generating-the-flame-graph): after the plugin has written the data we need to a text file, we can use [a separate tool](https://github.com/brendangregg/FlameGraph) to turn it into a nice, interactive plot.

## 1. Collecting Function Call Data

There are two main ways in which you can make the plugin collect function call data from gameplay:

1. By running console commands to start and stop profiling.
2. By specifying a configuration file (which describes how and when you want profiling to happen) in the plugin's INI file, which will then be used as soon as you launch your game.

### 1.1 Console Commands

In order to be able to start and stop profiling from the in-game console, it is required to have the `PapyrusProfiler.esp` file (which can be downloaded from the [PapyrusProfiler NexusMods page](https://www.nexusmods.com/skyrimspecialedition/mods/82770/)) active. This, in turn, also requires the [NL_CMD Console Command Framework](https://www.nexusmods.com/skyrimspecialedition/mods/62497) to be installed and active.

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

> **Note**: an alternative implementation that uses a different framework to implement console commands may be downloaded from: [https://www.nexusmods.com/skyrimspecialedition/mods/82793/](https://www.nexusmods.com/skyrimspecialedition/mods/82793/)

### 1.2 INI File

The download of Papyrus Profiler includes a `/SKSE/Plguins/PapyrusProfiler.ini` file, right alongside the plugin's `.dll` file. By default, the contents of this INI file are as follows:

```INI
[PapyrusProfilerSettings]

;Filename (or filepath) for Profiling Config to use on startup. Leave empty to not run any config on game load.
StartupConfig = 

```

With these default contents, the profiler does not run any configuration on game startup. However, you may edit this file by providing a filename or filepath for a file that specifies a configuration, and then the profiler will start using this configuration immediately when you load a game (or start a new game). If you only specify a filename (for example: `StartupConfig = Skip30sec_Profile5min.json`), the profiler will look for a configuration with this name in the `/SKSE/Plugins/` directory (where `PapyrusProfiler.dll` and `PapyrusProfiler.ini` are also located). If you specify a full filepath, it could be anywhere on your computer. More details on exactly how configuration files work are provided next.

### 1.3 Configuration Files

A *configuration* file is a JSON file that describes some settings that the profiler should use. Several example files are included in the Papyrus Profiler download (and also available [in this GitHub repo](/contrib/Distribution/ProfilingConfigs)). For example, the contents of the `Skip30sec_Profile5min.json` config are as follows:

```json
{
	"Description": "This configuration skips the first 30 seconds, and then profiles for 5 minutes (300sec + 30sec to account for skipping).",
	"OutFilename": "Skip30sec_Profile5mins_Profile",
	"MaxFilepathSuffix": 3,
	"MaxNumSeconds": 330,
	"NumSkipSeconds": 30,
	"WriteMode": 0
}
```

If this example configuration is used (either by specifying it via the `nl_cmd StartPapyrusProfilingConfig(Skip30sec_Profile5min.json)` console command, or by specifying it as a startup configuration in the INI file), the profiler will behave as follows. For the first 30 seconds, it will do nothing. Over the next 5 minutes (300 seconds), it will collect data on all the Papyrus function calls. After this, it will write all the data it has collected to `<SKSE_LOGS_DIR>/PapyrusProfiler/Skip30sec_Profile5mins_Profile_<i>.log`, where `<i>` cycles through 0, 1, 2, 3, and back to 0. Once that is done, it will do nothing else for the remainder of your playing session (except if you manually re-start profiling again).

**The following properties can be used in config files**:
- `"OutFilename"`: here you should provide the filename (without extension) which you would like the profiler to write its output to when using this configuration.
- `"MaxFilepathSuffix"`: here you can specify an integer, which provides a bound on the number of different output files can be written for the same configuration before the profiler will start overwriting old files. For example, if you use `3`, the profiler can write to up to four different files (0, 1, 2, and 3) before it starts overwriting the oldest file.
- `"MaxNumCalls"`: if you specify a number greater than `0`, the Papyrus Profiler will automatically stop running once it has collected data on this many function calls.
- `"MaxNumSeconds"`: if you specify a number greater than `0`, the Papyrus Profiler will automatically stop running once it has been running for this many seconds.
- `"NumSkipCalls"`: if you specify a number `n` greater than `0`, the Papyrus Profiler will skip the first `n` function calls that it observes (and only start collecting data afterwards).
- `"NumSkipSeconds"`: if you specify a number greater than `0`, the Papyrus Profiler will only start collecting data once this many seconds have elapsed since it was started with this configuration. Note that the profiler *will* include this time also in the time counted for `"MaxNumSeconds"`.
- `"WriteMode"`: you may set this to either `0` or `1`. If you use `0` (default), the Papyrus Profiler will only write all its collected data at once when it is done. This may be either because you stopped it manually with a console command, or because it hit the `"MaxNumSeconds"` or `"MaxNumCalls"` limit. If you set `"WriteMode"` to `1`, the Papyrus Profiler will continously keep writing data immediately as it is collected. This may be considered 'more safe', in the sense that you will not lose data collected so far if the game crashes or if you quit the game without stopping profiling. However, this may lead to **substantially larger output file sizes** (because the profiler cannot compress many lines with identical stacktraces into a single output line), so **I do not recommend this for long profiling sessions**.
- `"IncludeFilters"`: here you can provide an array of strings, each of which is interpreted as a regular expression. If this array is non-empty, only stacktraces that match at least one of these regular expressions will be tracked by the profiler. Other stacktraces will simply be ignored. Regular expressions are always assumed to be case-insensitive. For example, the `OnHit_Config.json` example configuration uses `"^.*\\.OnHit.*$"` as a regular expression to only collect data on stacktraces that include "OnHit".
- `"ExcludeFilters"`: here you can provide an array of strings, each of which is interpreted as a regular expression. Any stacktrace that matches any regular expression provided in this array will be ignored by the profiler. Regular expressions are always assumed to be case-insensitive.

### 1.4 Starting and Stopping Profiling from Papyrus Scripts

While the primary two ways to specify how and when profiling should start and stop are (1) console commands and (2) the INI file setting, a third approach that can be used is to start and stop profiling using Papyrus Scripts. This means that you could, for example, create a mod with a new Spell or Power that starts or stops profiling.

The Papyrus Profiler exposes two new functions to the scripting language, which may be found in the [PapyrusProfilerFunctions.psc](/contrib/Distribution/PapyrusSources/PapyrusProfilerFunctions.psc) file. If an empty string is passed into the function to start profiling, it will use exhibit the same default behaviour as when the `nl_cmd StartPapyrusProfiling()` console command without config file is used. The [PapyrusProfilerQuestScript.psc](/contrib/Distribution/PapyrusSources/PapyrusProfilerQuestScript.psc) file (which is used to implement the console commands!) may be used as an example for how to call these functions.

## 2. Generating the Flame Graph

The process described above for collecting data will have written its output to some output file. For example: `<SKSE_LOGS_DIR>/PapyrusProfiler/PapyrusProfilerOutput_0.log`, where `<SKSE_LOGS_DIR>` is the directory where SKSE and most SKSE plugins write their logs. Typically, this is
`<USER>/Documents/My Games/Skyrim Special Edition/SKSE`.

> **Note**: as a simpler alternative to the following instructions based on command lines, you may also drop your log files into [https://www.speedscope.app/](https://www.speedscope.app/) to obtain similar types of plots. For this website, I recommend switching to "Left Heavy" instead of the default "Time Order" view.

To turn this output into a Flame Graph, we'll use the [FlameGraph repository](https://github.com/brendangregg/FlameGraph). This also requires you to have installed [Perl](https://www.perl.org/get.html). After cloning the FlameGraph repository and opening a command prompt in its root directory, we can run the following command to generate a Flame Graph:

```
perl ./flamegraph.pl "<SKSE_LOGS_DIR>/PapyrusProfiler/PapyrusProfilerOutput_0.log" > "SKSE_LOGS_DIR>/PapyrusProfiler/FlameGraph.svg" --width 1800
```

This will write the `FlameGraph.svg` file right next to the output log, which we can then open and inspect in any internet browser.

The example flamegraph near the top of this page was generated by profiling with the `Skip30sec_Profile5min.json` config, and running the following command to generate the plot:

```
perl ./flamegraph.pl "<SKSE_LOGS_DIR>/PapyrusProfiler/Skip30sec_Profile5mins_Profile_0.log" > "SKSE_LOGS_DIR>/PapyrusProfiler/FlameGraph_5mins_Whiterun.svg" --width 1800 --title "Walking around Whiterun for 5 min"
```

As a second example, the following flamegraph was obtained by profiling with the `OnHit_Config.json` (which only looks for OnHit events) while killing the Giant outside Whiterun, and then running the following command to generate the plot:

```
perl ./flamegraph.pl "<SKSE_LOGS_DIR>/PapyrusProfiler/OnHit_0.log" > "SKSE_LOGS_DIR>/PapyrusProfiler/FlameGraph_OnHit.svg" --width 1200 --title "Killing the Giant outside Whiterun"
```

[![Example image of a generated flamegraph.](https://dennissoemers.github.io/assets/papyrus_profiler/FlameGraph_OnHit.svg)](https://dennissoemers.github.io/assets/papyrus_profiler/FlameGraph_OnHit.svg)

---

## Runtime Requirements (for users)

At runtime, the plugin has the following requirements. Any user of this plugin (or mods that in turn require this plugin) need to have this installed locally.

- The Elder Scrolls V: Skyrim Special Edition.
  - Tested with version 1.5.97 (SSE).
  - Assumed compatible with any version 1.6.xxx (AE).
  - Assumed compatible with VR.
- [Skyrim Script Extender](https://skse.silverlock.org/)
- [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444)

## Build Requirements (for plugin developers)

Building the code in this repository has the following requirements.

- [Visual Studio](https://visualstudio.microsoft.com/).
- [CMake](https://cmake.org/).
- [Vcpkg](https://github.com/microsoft/vcpkg).

This project was set up exactly as in the [CommonLibSSE NG Sample Plugin](https://gitlab.com/colorglass/commonlibsse-sample-plugin), 
and I refer to that repository for highly detailed instructions on installation and building.

## Limitations

Currently, the Papyrus Profiler only counts Papyrus function calls. This is very likely useful, and likely provides useful information that can guide us and help us better identify which scripts (and which functions in them) are **likely** to be taking substantial time/resources in our game. But it is important to keep in mind that it does not tell the whole story, there are some limitations.

### Limitation 1: Not Measuring Time

We are just counting how often every function gets called, not how much time it takes to complete running. This is not necessarily better or worse, it is just different information. In Papyrus, function calls are notoriously expensive, so there is probably some useful correlation. Function calls are also often (depending on which functions they are) points at which scripts "pause" to sync to framerate or wait for resources currently locked by other threads, which again suggests that counting functions may be a useful way to estimate the "stress" that a script places on the game's scripting engine. 

In other words, the profiler can currently show you which functions/stacktraces get called frequently, and which don't, but it doesn't tell us exactly how much time they take. If a function gets called very frequently, that suggests that it may be worthwhile to pay extra attention to that function and, for example, see whether it may be possible to optimise the code inside of it... but if we were to succeed at this and find ways to optimise it, this would likely have been very useful, but we would not notice any difference if we ran the profiler again because we were just counting calls instead of measuring time!

### Limitation 2: Function Calls Only

Papyrus scripts do not only consist of function calls. They also include numerous operations, such as addition, subtraction, multiplication, assigning values to variables, equality/inequality tests, if-statements, casting operations, ... and so on. Function calls are often more expensive than these other operations, but if we have a lot of them, they can still add up. Especially if we're not using [Papyrus Tweaks](https://www.nexusmods.com/skyrimspecialedition/mods/77779) to increase the maximum number of operations per task. However, the Papyrus Profiler is currently completely oblivious to any of these operation: it's only counting function calls.

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
