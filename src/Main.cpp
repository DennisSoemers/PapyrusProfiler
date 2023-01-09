#include <stddef.h>
#include "Papyrus.h"
#include "ProfilingHook.h"
#include "Settings.h"

using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {
    /**
     * Setup logging.
     */
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>("Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }

#ifndef NDEBUG
        const auto level = spdlog::level::trace;
#else
        const auto level = spdlog::level::info;
#endif

        log->set_level(level);
        log->flush_on(level);

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    }

    /**
     * Initialize the hooks.
     */
    void InitializeHooks() {
        log::trace("Initializing hooks...");

        Profiling::ProfilingHook::InstallHook();

        log::trace("Hooks initialized.");
    }

    /**
     * Register new Papyrus functions.
     */
    void InitializePapyrus() {
        log::trace("Initializing Papyrus bindings...");
        if (GetPapyrusInterface()->Register(Profiling::Papyrus::Bind)) {
            log::debug("Papyrus functions bound.");
        } else {
            stl::report_and_fail("Failure to register Papyrus bindings.");
        }
    }

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) { 
            // Whenever we load a game (or start a new game), we want to start up the profiling config
            // as specified in the .ini file
            case SKSE::MessagingInterface::kNewGame:
            case SKSE::MessagingInterface::kPreLoadGame:
                // In case we already have some profiling running, stop that first
                Profiling::ProfilingHook::GetSingleton().StopCurrentConfig();

                // Start up any config that may be specified as startup-config in .ini
                const std::string& startupConfig = Settings::GetSingleton()->papyrusProfilerSettings.startupConfig;
                if (startupConfig.empty()) {
                    logger::info("Not starting any profiling config, .ini setting is empty.");
                } else {
                    logger::info("Starting up profiling config from .ini: {}", startupConfig);
                    Profiling::ProfilingHook::GetSingleton().RunConfig(startupConfig);
                }
                break;
        }
    }
}  // namespace

/**
 * This is the main callback for initializing the SKSE plugin, called just before Skyrim runs its main function.
 */
SKSEPluginLoad(const LoadInterface* skse) {
    InitializeLogging();
// just define this in the CXX flags if you want it to wait for a debugger to attach before the game loads
#ifdef _DEBUG_WAIT_FOR_ATTACH
    log::info("Waiting for debugger to attach...");
    while (!IsDebuggerPresent())
    {
        Sleep(10);
    }

    Sleep(1000 * 4);
    logger::info("Debugger attached!");
#endif
    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);

    try {
        Settings::GetSingleton()->Load();
    } catch (...) {
        logger::error("Exception caught when loading settings! Default settings will be used");
    }

    InitializeHooks();
    InitializePapyrus();
    SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
