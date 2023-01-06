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

        /** Start running a new config */
        void RunConfig(const std::string& configFile);

        /** Resets all internal data (e.g., counts of collected/skipped function calls, ...) */
        void ResetData();

        /** Do we want to collect the next call, based on currently active config (if any)? */
        bool ShouldCollectCall();

        /** Currently active config. */
        std::unique_ptr<ProfilingConfig> activeConfig;

        /** Number of function calls we've already collected for current config. */
        uint64_t numFuncCallsCollected = 0;
        /** Number of function calls we deliberately skipped. */
        uint64_t numSkippedCalls = 0;
        /** Logger to write output to. */
        std::unique_ptr<spdlog::logger> outputLogger;

    private:
        ProfilingHook() = default;
        ProfilingHook(const ProfilingHook&) = delete;
        ProfilingHook(ProfilingHook&&) = delete;
        ~ProfilingHook() = default;

        ProfilingHook& operator=(const ProfilingHook&) = delete;
        ProfilingHook& operator=(ProfilingHook&&) = delete;
    };

    static RE::BSFixedString* FuncCallHook(
        RE::BSScript::Internal::VirtualMachine* _this,
        RE::BSScript::Stack* a_stack,
        RE::BSTSmartPointer<RE::BSScript::Internal::IFuncCallQuery>& a_funcCallQuery);

    static inline REL::Relocation<decltype(FuncCallHook)> _original_func;

#pragma warning(pop)
}  // namespace Profiling