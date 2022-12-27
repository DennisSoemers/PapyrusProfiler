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

    // Also create our output logger
    auto path = SKSE::log::log_directory();
    *path /= SKSE::PluginDeclaration::GetSingleton()->GetName();
    *path /= L"PapyrusProfilerOutput.log";
    outputLogger = std::make_unique<spdlog::logger>(
        "OutputLogger", std::make_unique<spdlog::sinks::basic_file_sink_mt>(path->string(), true));

    outputLogger->set_level(spdlog::level::info);
    outputLogger->flush_on(spdlog::level::info);
    outputLogger->set_pattern("%v");
}

static RE::BSFixedString* Profiling::FuncCallHook(
        RE::BSScript::Internal::VirtualMachine* _this,
        RE::BSScript::Stack* a_stack,
        RE::BSTSmartPointer<RE::BSScript::Internal::IFuncCallQuery>& a_funcCallQuery) {

    if (numStacksPrinted < stacksPrintCap && a_stack) {
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
            ++numStacksPrinted;
            std::string stackTrace = std::format("{}.{}", scriptInfo.get()->GetName(), functionName.c_str());

            RE::BSScript::StackFrame* stackFrame = a_stack->top;
            if (stackFrame) {
                stackFrame = stackFrame->previousFrame;  // Already printed for top, so start with previous
                while (stackFrame) {
                    if (stackFrame->owningFunction && stackFrame->owningFunction.get()) {
                        const auto scriptName = stackFrame->owningFunction.get()->GetObjectTypeName().c_str();
                        const auto funcName = stackFrame->owningFunction.get()->GetName().c_str();
                        stackTrace = std::format("{}.{};", scriptName, funcName) + stackTrace;
                    }
                    stackFrame = stackFrame->previousFrame;
                }
            }

            outputLogger->info(std::format("{} {}", stackTrace, 1));
        }
    }

    // Let the engine run its normal code
    return _original_func(_this, a_stack, a_funcCallQuery);
}