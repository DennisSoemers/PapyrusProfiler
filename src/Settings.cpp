#include "Settings.h"

Settings* Settings::GetSingleton() {
    static Settings singleton;
    return std::addressof(singleton);
}

void Settings::Load() {
    constexpr auto path = L"Data/SKSE/Plugins/PapyrusProfiler.ini";

    CSimpleIniA ini;
    ini.SetUnicode();

    ini.LoadFile(path);

    papyrusProfilerSettings.Load(ini);

    ini.SaveFile(path);
}

void Settings::PapyrusProfilerSettings::Load(CSimpleIniA& a_ini) {
    static const char* section = "PapyrusProfilerSettings";

    detail::get_value(a_ini, startupConfig, section, "StartupConfig",
                      ";Filename (or filepath) for Profiling Config to use on startup. Leave empty to not run any config on game load.");
}