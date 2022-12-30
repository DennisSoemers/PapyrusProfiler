# Papyrus Profiler
[![C++23](https://img.shields.io/static/v1?label=standard&message=C%2B%2B23&color=blue&logo=c%2B%2B&&logoColor=white&style=flat)](https://en.cppreference.com/w/cpp/compiler_support)
[![Platform](https://img.shields.io/static/v1?label=platform&message=windows&color=dimgray&style=flat)](#)
![GitHub](https://img.shields.io/github/license/DennisSoemers/PapyrusProfiler)
![GitHub release (latest by date)](https://img.shields.io/github/v/release/DennisSoemers/PapyrusProfiler)

A profiler for Skyrim's Papyrus scripting language. 

## Download

The plugin can be downloaded from TODO [its NexusMods page]().

## Usage

### Collecting Performance Data

TODO

### Generating the Flame Graph

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
- Nightfallstorm for the hook and example code taken from [Skyrim Recursion FPS Fix](https://github.com/Nightfallstorm/Skyrim-Recursion-FPS-Fix).
- CharmedBaryon for [CommonLibSSE NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) and the [CommonLibSSE NG Sample Plugin](https://gitlab.com/colorglass/commonlibsse-sample-plugin).
- The SKSE authors for [SKSE](http://skse.silverlock.org/).
- Meh321 for [Address Library for SKSE Plugins](https://www.nexusmods.com/skyrimspecialedition/mods/32444).

## Disclaimer

- THIS MATERIAL IS NOT MADE, GUARANTEED OR SUPPORTED BY ZENIMAX OR ITS AFFILIATES.
- ALL FILES IN THE DOWNLOAD ARE PROVIDED ''AS IS'' WITHOUT ANY WARRANTY OR GUARANTEE OF ANY KIND. IN NO EVENT CAN THE AUTHOR BE HELD RESPONSIBLE FOR ANY CLAIMS, WHETHER OR NOT THEY ARE RELATED TO THE DOWNLOAD.
