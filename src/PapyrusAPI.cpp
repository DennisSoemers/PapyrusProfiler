#include "PapyrusAPI.h"

// Interface code based on https://github.com/adamhynek/higgs
// Reference for the implementation:
// https://github.com/Nightfallstorm/DescriptionFramework/blob/main/src/DescriptionFrameworkAPI.cpp

// Stores the API after it has already been fetched
PapyrusProfilerAPI::IPapyrusProfilerInterface001* g_PapyrusProfilerInterface = nullptr;

// Fetches the interface to use from PapyrusProfiler
PapyrusProfilerAPI::IPapyrusProfilerInterface001* PapyrusProfilerAPI::GetPapyrusProfilerInterface001() {
    // If the interface has already been fetched, return the same object
    if (g_PapyrusProfilerInterface) {
        return g_PapyrusProfilerInterface;
    }

    // Dispatch a message to get the plugin interface from PapyrusProfiler
    PapyrusProfilerMessage message;
    const auto skseMessaging = SKSE::GetMessagingInterface();
    skseMessaging->Dispatch(PapyrusProfilerMessage::kMessage_GetInterface, (void*)&message,
                            sizeof(PapyrusProfilerMessage*), PapyrusProfilerPluginName);
    if (!message.GetApiFunction) {
        return nullptr;
    }

    // Fetch the API for this version of the PapyrusProfiler interface
    g_PapyrusProfilerInterface = static_cast<IPapyrusProfilerInterface001*>(message.GetApiFunction(1));
    return g_PapyrusProfilerInterface;
}