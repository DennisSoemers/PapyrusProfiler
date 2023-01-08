#include "Papyrus.h"

namespace Profiling {

    namespace Papyrus {
        constexpr std::string_view PapyrusProfilerFunctions = "PapyrusProfilerFunctions";

        void StartProfiling(RE::StaticFunctionTag*, std::string config) {
            // TODO
        }

        void StopProfiling(RE::StaticFunctionTag*) {
            // TODO
        }

        /**
         * Provide bindings for all our Papyrus functions.
         */
        bool Bind(RE::BSScript::IVirtualMachine* vm) {
            vm->RegisterFunction("StartProfiling", PapyrusProfilerFunctions, StartProfiling, true);
            vm->RegisterFunction("StopProfiling", PapyrusProfilerFunctions, StopProfiling, true);

            return true;
        }
    }

}  // namespace Profiling