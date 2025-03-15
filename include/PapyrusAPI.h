#pragma once
#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

// Interface code based on https://github.com/adamhynek/higgs
// Reference for the implementation:
// https://github.com/Nightfallstorm/DescriptionFramework/blob/main/src/DescriptionFrameworkAPI.h

namespace PapyrusProfilerAPI {
    constexpr const auto PapyrusProfilerPluginName = "PapyrusProfiler";

    // A message used to fetch PapyrusProfilers's interface
    struct PapyrusProfilerMessage {
        enum : uint32_t { kMessage_GetInterface = 0x4ff88a1f };  // Randomly generated
        void* (*GetApiFunction)(unsigned int revisionNumber) = nullptr;
    };

    // Returns an IPapyrusProfiler001 object compatible with the API shown below
    // This should only be called after SKSE sends kMessage_PostLoad to your plugin
    struct IPapyrusProfilerInterface001;
    IPapyrusProfilerInterface001* GetPapyrusProfilerInterface001();

    // This object provides access to PapyrusProfiler's mod support API
    struct IPapyrusProfilerInterface001 {
        virtual unsigned int GetBuildNumber() = 0;

        virtual void StartProfilingConfig(const std::string& configName) = 0;
        virtual void StopProfiling() = 0;
        virtual void LoadSettings() = 0;
    };

}  // namespace PapyrusProfilerAPI
extern PapyrusProfilerAPI::IPapyrusProfilerInterface001* g_PapyrusProfilerInterface;