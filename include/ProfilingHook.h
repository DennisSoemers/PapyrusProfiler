#pragma once

#include <RE/Skyrim.h>
#include "ProfilingConfig.h"

namespace Profiling {
#pragma warning(push)
#pragma warning(disable : 4251)

    class __declspec(dllexport) ProfilingHook {
    public:
        [[nodiscard]] static ProfilingHook& GetSingleton() noexcept;

        static void InstallHook();

        void RunConfig(const std::string& configFile);

    private:
        ProfilingHook() = default;
        ProfilingHook(const ProfilingHook&) = delete;
        ProfilingHook(ProfilingHook&&) = delete;
        ~ProfilingHook() = default;

        ProfilingHook& operator=(const ProfilingHook&) = delete;
        ProfilingHook& operator=(ProfilingHook&&) = delete;

        /** Currently active config. */
        std::unique_ptr<ProfilingConfig> activeConfig;
    };

    static RE::BSFixedString* FuncCallHook(
        RE::BSScript::Internal::VirtualMachine* _this,
        RE::BSScript::Stack* a_stack,
        RE::BSTSmartPointer<RE::BSScript::Internal::IFuncCallQuery>& a_funcCallQuery);

    static inline REL::Relocation<decltype(FuncCallHook)> _original_func;
    static std::unique_ptr<spdlog::logger> outputLogger;
    static uint64_t numStacksPrinted = 0;
    static const uint64_t stacksPrintCap = 10'000;

#pragma warning(pop)
}  // namespace Profiling