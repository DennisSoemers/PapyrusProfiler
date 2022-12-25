#include <stddef.h>
#include "OnMeleeHit.h"
#include "PrecisionAPI.h"
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

        OnMeleeHit::OnMeleeHitHook::InstallHook();

        log::trace("Hooks initialized.");
    }

    void MessageHandler(SKSE::MessagingInterface::Message* a_msg) {
        switch (a_msg->type) {
            case SKSE::MessagingInterface::kPostPostLoad: {
                const auto precisionAPI =
                    reinterpret_cast<PRECISION_API::IVPrecision4*>(PRECISION_API::RequestPluginAPI());
                if (precisionAPI) {
                    precisionAPI->AddWeaponWeaponCollisionCallback(SKSE::GetPluginHandle(), OnMeleeHit::PrecisionWeaponsCallback);
                    logger::info("Enabled compatibility with Precision");
                }
            }
                break;
            case SKSE::MessagingInterface::kDataLoaded: {
                auto parryingHandle = GetModuleHandleA("Parrying.dll");
                if (parryingHandle) {
                    logger::error("Warning! ParryingRPG has detected that Parrying.dll is also loaded!");
                    RE::DebugMessageBox("Warning! ParryingRPG has detected that Parrying.dll is also loaded!");
                }

                auto maxsuWeaponParryHandle = GetModuleHandleA("MaxsuWeaponParry.dll");
                if (maxsuWeaponParryHandle) {
                    logger::error("Warning! ParryingRPG has detected that MaxsuWeaponParry.dll is also loaded!");
                    RE::DebugMessageBox("Warning! ParryingRPG has detected that MaxsuWeaponParry.dll is also loaded!");
                }
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

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    Init(skse);

    const auto runtimeVersion = skse->RuntimeVersion();

    if (runtimeVersion < REL::Version{1, 5, 97, 0}) {
        logger::error("Parrying RPG is not compatible with runtime versions below 1.5.97!");
        return false;
    }

    if (runtimeVersion > REL::Version{1, 6, 353, 0}) {
        logger::error("Parrying RPG is not compatible with runtime versions above 1.6.353!");
        return false;
    }

    try {
        Settings::GetSingleton()->Load();
    } catch (...) {
        logger::error("Exception caught when loading settings! Default settings will be used");
    }

    InitializeHooks();
    SKSE::GetMessagingInterface()->RegisterListener(MessageHandler);

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
