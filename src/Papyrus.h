#pragma once

#include <RE/Skyrim.h>

namespace Profiling {
#pragma warning(push)
#pragma warning(disable : 4251)

    namespace Papyrus {
        /** Bind all the new Papyrus functions */
        bool Bind(RE::BSScript::IVirtualMachine* vm);
    } // namespace Papyrus

#pragma warning(pop)
}  // namespace Profiling