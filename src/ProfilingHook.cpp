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
        ProfilingHook& profilingHook = ProfilingHook::GetSingleton();

        if (profilingHook.ShouldCollectCall()) {
            // Get info from the call
            RE::BSScript::Internal::IFuncCallQuery::CallType callType;
            RE::BSTSmartPointer<RE::BSScript::ObjectTypeInfo> scriptInfo;
            RE::BSScript::Variable self;
            RE::BSScrapArray<RE::BSScript::Variable> args;
            RE::BSFixedString functionName;

            const auto owningTasklet = a_stack->owningTasklet.get();
            a_funcCallQuery->GetFunctionCallInfo(callType, scriptInfo, functionName, self, args);
            if (scriptInfo.get()) {
                // Print this stack
                ++profilingHook.numFuncCallsCollected;
                std::string stackTraceStr = std::format("{}.{}", scriptInfo.get()->GetName(), functionName.c_str());

                RE::BSScript::StackFrame* stackFrame = a_stack->top;
                if (stackFrame) {
                    stackFrame = stackFrame->previousFrame;  // Already got info for top, so start with previous
                    while (stackFrame) {
                        if (stackFrame->owningFunction && stackFrame->owningFunction.get()) {
                            const auto scriptName = stackFrame->owningFunction.get()->GetObjectTypeName().c_str();
                            const auto funcName = stackFrame->owningFunction.get()->GetName().c_str();
                            stackTraceStr = std::format("{}.{};", scriptName, funcName) + stackTraceStr;
                        }
                        stackFrame = stackFrame->previousFrame;
                    }
                }

                if (profilingHook.outputLogger && profilingHook.activeConfig->writeMode == ProfilingConfig::ProfileWriteMode::WriteLive) {
                    profilingHook.outputLogger->info(std::format("{} {}", stackTraceStr, 1));
                }
            }
        } else if (profilingHook.activeConfig.get()) {
            ++profilingHook.numSkippedCalls;
        }
    }

    // Let the engine run its normal code
    return _original_func(_this, a_stack, a_funcCallQuery);
}

void ProfilingHook::RunConfig(const std::string& configFile) { 
    if (activeConfig.get()) {
        // TODO first stop the already-active config, have it finish any writing it should do

        if (outputLogger) {
            spdlog::drop(activeConfig->outFilename);
            outputLogger.reset();
        }

        activeConfig.reset();
    }

    ResetData();

    logger::info("Loading config: {}", configFile);
    activeConfig = std::make_unique<ProfilingConfig>(configFile);

    if (activeConfig->failedLoadFromFile) {
        logger::error("Not running config because it failed to load: {}", configFile);
        activeConfig.reset();
    } else {
        if (activeConfig->writeMode == ProfilingConfig::ProfileWriteMode::WriteAtEnd ||
            activeConfig->writeMode == ProfilingConfig::ProfileWriteMode::WriteLive) {
            // Create new output logger
            auto basePath = SKSE::log::log_directory();
            *basePath /= SKSE::PluginDeclaration::GetSingleton()->GetName();
            auto path = *basePath / std::format("{}_{}.log", activeConfig->outFilename, 0);

            if (std::filesystem::exists(path)) {
                // File already exists. We'll see if we can find a suffix that doesn't exist, or
                // otherwise reuse the oldest file.
                int idxToUse = 0;
                auto oldestWriteTime = std::filesystem::last_write_time(path);
                
                for (int i = 1; i <= activeConfig->maxFilepathSuffix; ++i) {
                    path = *basePath / std::format("{}_{}.log", activeConfig->outFilename, i);
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

                path = *basePath / std::format("{}_{}.log", activeConfig->outFilename, idxToUse);
            }

            outputLogger = std::make_unique<spdlog::logger>(
                activeConfig->outFilename, std::make_unique<spdlog::sinks::basic_file_sink_mt>(path.string(), true));

            outputLogger->set_level(spdlog::level::info);
            outputLogger->flush_on(spdlog::level::info);
            outputLogger->set_pattern("%v");
        }
    }
}

void ProfilingHook::ResetData() { 
    numFuncCallsCollected = 0; 
    numSkippedCalls = 0;
}

bool ProfilingHook::ShouldCollectCall() {
    Profiling::ProfilingConfig* const config = activeConfig.get();
    if (!config) {
        return false;
    }

    if (config->maxNumCalls > 0 && numFuncCallsCollected >= config->maxNumCalls) {
        return false;
    }

    return true;
}