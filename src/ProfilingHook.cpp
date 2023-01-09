#include <filesystem>
#include <SKSE/SKSE.h>
#include "ProfilingHook.h"

using namespace Profiling;

ProfilingHook& ProfilingHook::GetSingleton() noexcept {
    static ProfilingHook instance;
    return instance;
}

void ProfilingHook::InstallHook() {
    // Thanks to Nightfallstorm for: https://github.com/Nightfallstorm/Skyrim-Recursion-FPS-Fix
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    // Hooks into BSScript::Internal::VirtualMachine::AttemptFunctionCall
    // AttemptFunctionCall is called from from BSScript::Internal::VirtualMachine::ProcessMessageQueue
    // 1.5.97: sub_141261CB0
    // 1.6.640: sub_141388580
    // hooks at offset 0x7F (1.6.640 addr: 0x1413885FF)
    REL::Relocation<std::uintptr_t> target{RELOCATION_ID(98130, 104853), REL::VariantOffset(0x7F, 0x7F, 0x7F)};
    _original_func = trampoline.write_call<5>(target.address(), FuncCallHook);
}

static RE::BSFixedString* Profiling::FuncCallHook(
        RE::BSScript::Internal::VirtualMachine* _this,
        RE::BSScript::Stack* a_stack,
        RE::BSTSmartPointer<RE::BSScript::Internal::IFuncCallQuery>& a_funcCallQuery) {

    if (a_stack && a_funcCallQuery) {
        try {
            // Get info from the call
            RE::BSScript::Internal::IFuncCallQuery::CallType callType;
            RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> scriptInfo;
            RE::BSScript::Variable self;
            RE::BSScrapArray<RE::BSScript::Variable> args;
            RE::BSFixedString functionName;

            const auto owningTasklet = a_stack->owningTasklet.get();
            a_funcCallQuery->GetFunctionCallInfo(callType, scriptInfo, functionName, self, args);
            if (scriptInfo.get()) {
                ProfilingHook& profilingHook = ProfilingHook::GetSingleton();
                ProfilingHook::ProfilerCallResponse callResponse = profilingHook.GetNextCallResponse();

                if (callResponse == ProfilingHook::ProfilerCallResponse::Record) {
                    if (!profilingHook.printedStartProfileMessage) {
                        RE::DebugMessageBox("Papyrus Profiling starts now.");
                        profilingHook.printedStartProfileMessage = true;
                    }

                    // Collect info from this stack
                    std::string stackTraceStr = std::format("{}.{}", scriptInfo.get()->GetName(), functionName.c_str());

                    RE::BSScript::StackFrame* stackFrame = a_stack->top;
                    if (stackFrame) {
                        while (stackFrame) {
                            if (stackFrame->owningFunction && stackFrame->owningFunction.get()) {
                                const auto scriptName = stackFrame->owningFunction.get()->GetObjectTypeName().c_str();
                                const auto funcName = stackFrame->owningFunction.get()->GetName().c_str();
                                stackTraceStr = std::format("{}.{};", scriptName, funcName) + stackTraceStr;
                            }
                            stackFrame = stackFrame->previousFrame;
                        }
                    }

                    if (profilingHook.outputLogger &&
                        profilingHook.activeConfig->writeMode == ProfilingConfig::ProfileWriteMode::WriteLive) {
                        profilingHook.outputLogger->info(std::format("{} {}", stackTraceStr, 1));
                    } else if (profilingHook.activeConfig->writeMode != ProfilingConfig::ProfileWriteMode::WriteLive) {
                        {
                            std::lock_guard<std::mutex> lockGuard(profilingHook.callCountsMapMutex);
                            profilingHook.stackCallCounts[stackTraceStr] =
                                profilingHook.stackCallCounts[stackTraceStr] + 1;
                        }
                    }
                } else if (profilingHook.activeConfig) {
                    if (callResponse == ProfilingHook::ProfilerCallResponse::LimitHit) {
                        // Stop profiling
                        if (profilingHook.outputLogger) {
                            if (profilingHook.activeConfig->writeMode ==
                                ProfilingConfig::ProfileWriteMode::WriteAtEnd) {
                                // We'll now write everything we've collected.
                                {
                                    std::lock_guard<std::mutex> lockGuard(profilingHook.callCountsMapMutex);
                                    for (const auto& [stackTraceStr, callCount] : profilingHook.stackCallCounts) {
                                        profilingHook.outputLogger->info(
                                            std::format("{} {}", stackTraceStr, callCount));
                                    }
                                }
                            }

                            profilingHook.outputLogger->flush();
                            spdlog::drop(profilingHook.activeConfig->outFilename);
                            profilingHook.outputLogger.reset();
                        }

                        profilingHook.activeConfig.reset();
                        RE::DebugMessageBox("Papyrus Profiling is now finished.");
                    }
                }
            }
        } catch (...) {
            logger::error("Exception caught in Papyrus Profiler!");
        }
    }

    // Let the engine run its normal code
    return _original_func(_this, a_stack, a_funcCallQuery);
}

void ProfilingHook::RunConfig(const std::string& configFile) {
    StopCurrentConfig();

    std::shared_ptr<ProfilingConfig> config;

    if (configFile.empty()) {
        logger::info("Starting default config.");
        config = std::make_shared<ProfilingConfig>();
    } else {
        logger::info("Loading config: {}", configFile);
        config = std::make_shared<ProfilingConfig>(configFile);
    }

    if (config->failedLoadFromFile) {
        logger::error("Not running config because it failed to load: {}", configFile);
        config.reset();
    } else {
        if (config->writeMode == ProfilingConfig::ProfileWriteMode::WriteAtEnd ||
            config->writeMode == ProfilingConfig::ProfileWriteMode::WriteLive) {
            // Create new output logger
            auto basePath = SKSE::log::log_directory();
            *basePath /= SKSE::PluginDeclaration::GetSingleton()->GetName();
            auto path = *basePath / std::format("{}_{}.log", config->outFilename, 0);

            if (std::filesystem::exists(path)) {
                // File already exists. We'll see if we can find a suffix that doesn't exist, or
                // otherwise reuse the oldest file.
                int idxToUse = 0;
                auto oldestWriteTime = std::filesystem::last_write_time(path);

                for (int i = 1; i <= config->maxFilepathSuffix; ++i) {
                    path = *basePath / std::format("{}_{}.log", config->outFilename, i);
                    if (!std::filesystem::exists(path)) {
                        // File with this suffix doesn't exist yet, just use this.
                        idxToUse = i;
                        break;
                    } else {
                        const auto writeTime = std::filesystem::last_write_time(path);
                        if (writeTime < oldestWriteTime) {
                            // This index has an older last-write-time, so use this
                            idxToUse = i;
                            oldestWriteTime = writeTime;
                        }
                    }
                }

                path = *basePath / std::format("{}_{}.log", config->outFilename, idxToUse);
            }

            outputLogger = std::make_unique<spdlog::logger>(
                config->outFilename, std::make_unique<spdlog::sinks::basic_file_sink_mt>(path.string(), true));

            outputLogger->set_level(spdlog::level::info);
            outputLogger->flush_on(spdlog::level::info);
            outputLogger->set_pattern("%v");
        }
    }

    if (config) {
        // Do this assignment at the end for thread-safety, want everything fully
        // initialised/processed first.
        activeConfig = config;
    }
}

void ProfilingHook::StopCurrentConfig() {
    if (activeConfig.get()) {
        // First stop already-active config
        hitLimits = true;  // Set this to true so we stop interacting with hooked calls for now.
        if (outputLogger) {
            // Let already-active config finish any writing if it wants to
            if (activeConfig->writeMode == ProfilingConfig::ProfileWriteMode::WriteAtEnd) {
                // We'll now write everything we've collected.
                {
                    std::lock_guard<std::mutex> lockGuard(callCountsMapMutex);

                    for (const auto& [stackTraceStr, callCount] : stackCallCounts) {
                        outputLogger->info(std::format("{} {}", stackTraceStr, callCount));
                    }

                    stackCallCounts.clear();
                }
            }

            spdlog::drop(activeConfig->outFilename);
            outputLogger.reset();
        }

        activeConfig.reset();
        RE::DebugMessageBox("Papyrus Profiling is now finished.");
    }

    ResetData();
}

void ProfilingHook::ResetData() { 
    numFuncCallsCollected = 0; 
    numSkippedCalls = 0;
    stackCallCounts.clear();
    hitLimits = false;
    printedStartProfileMessage = false;
}

ProfilingHook::ProfilerCallResponse ProfilingHook::GetNextCallResponse() {
    Profiling::ProfilingConfig* const config = activeConfig.get();
    if (!config) {
        return ProfilerCallResponse::Skip;
    }

    // All the following we want to synchronize, for reading/writing of 
    // numSkippedCalls and numFuncCallsCollected, and return appropriate
    // responses.
    static std::mutex mtx;

    {
        std::lock_guard<std::mutex> lockGuard(mtx);

        if (hitLimits) {
            return ProfilerCallResponse::LimitExceeded;
        }

        if (config->numSkipCalls > 0 && numSkippedCalls < config->numSkipCalls) {
            ++numSkippedCalls;
            return ProfilerCallResponse::Skip;
        }

        if (config->maxNumCalls > 0 && numFuncCallsCollected >= config->maxNumCalls) {
            hitLimits = true;
            return ProfilerCallResponse::LimitHit;
        }

        ++numFuncCallsCollected;
    }

    return ProfilerCallResponse::Record;
}