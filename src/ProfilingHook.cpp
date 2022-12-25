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

    REL::Relocation<std::uintptr_t> target{RELOCATION_ID(98130, 104853), REL::VariantOffset(0x7F, 0x7F, 0x7F)};
    _original_func = trampoline.write_call<5>(target.address(), FuncCallHook);
}

static RE::BSFixedString* Profiling::FuncCallHook(std::uint64_t unk0, RE::BSScript::Stack* a_stack,
    std::uint64_t* a_funcCallQuery) {
    return _original_func(unk0, a_stack, a_funcCallQuery);
}