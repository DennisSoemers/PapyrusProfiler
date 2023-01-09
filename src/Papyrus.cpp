#include "Papyrus.h"
#include "ProfilingHook.h"

namespace Profiling {

    namespace Papyrus {
        constexpr std::string_view PapyrusProfilerFunctions = "PapyrusProfilerFunctions";

        void StartProfiling(RE::StaticFunctionTag*, std::string config) {
            // TODO
        }

        void StopProfiling(RE::StaticFunctionTag*) { 
            ProfilingHook::GetSingleton().StopCurrentConfig();
        }

        /**
         * Provide bindings for all our Papyrus functions.
         */
        bool Bind(RE::BSScript::IVirtualMachine* vm) {
            vm->RegisterFunction("StartProfiling", PapyrusProfilerFunctions, StartProfiling, false);
            vm->RegisterFunction("StopProfiling", PapyrusProfilerFunctions, StopProfiling, false);

            return true;
        }
    }

}  // namespace Profiling