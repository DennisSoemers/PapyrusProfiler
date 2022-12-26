#pragma once

#include <RE/Skyrim.h>

namespace Profiling {
#pragma warning(push)
#pragma warning(disable : 4251)

    class __declspec(dllexport) ProfilingHook {
    public:
        [[nodiscard]] static ProfilingHook& GetSingleton() noexcept;

        static void InstallHook();

    private:
        ProfilingHook() = default;
        ProfilingHook(const ProfilingHook&) = delete;
        ProfilingHook(ProfilingHook&&) = delete;
        ~ProfilingHook() = default;

        ProfilingHook& operator=(const ProfilingHook&) = delete;
        ProfilingHook& operator=(ProfilingHook&&) = delete;
    };

    static RE::BSFixedString* FuncCallHook(std::uint64_t unk0, RE::BSScript::Stack* a_stack,
                                           std::uint64_t* a_funcCallQuery);

    static inline REL::Relocation<decltype(FuncCallHook)> _original_func;
    static std::unique_ptr<spdlog::logger> outputLogger;
    static uint64_t numStacksPrinted = 0;
    static const uint64_t stacksPrintCap = 10'000;

#pragma warning(pop)
}  // namespace Profiling