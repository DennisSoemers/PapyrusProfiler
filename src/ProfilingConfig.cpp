#include <filesystem>
#include "ProfilingConfig.h"
#include "tojson.hpp"

Profiling::ProfilingConfig::ProfilingConfig(const std::string& configPath) {
    failedLoadFromFile = true;

    // Thanks to doodlum for most of the JSON/YAML parsing code (taken from: https://github.com/doodlum/skyrim-srd)
    auto path = std::filesystem::path(configPath).filename();
    auto filename = path.string();

	try {
        std::ifstream i(configPath);
        if (!i.good()) {
            // Try if the specified path is just a name in Data/SKSE/Plugins
            i = std::ifstream(R"(Data\SKSE\Plugins\)" + configPath);
        }

        if (i.good()) {
            json data;
            if (path.extension() == ".yaml"sv) {
                try {
                    data = tojson::loadyaml(configPath);
                } catch (const std::exception& exc) {
                    std::string errorMessage =
                        std::format("Failed to convert {} to JSON object\n{}", filename, exc.what());
                    logger::error("{}", errorMessage);
                }
            } else {
                data = json::parse(i, nullptr, true, true);
            }
            i.close();

            // Put data from the file in our config object
            Profiling::ProfilingConfig::PopulateConfig(*this, data, configPath);
        } else {
            std::string errorMessage = std::format("Failed to parse {}\nBad file stream", filename);
            logger::error("{}", errorMessage);
        }
    } catch (const std::exception& exc) {
        std::string errorMessage = std::format("Failed to parse {}\n{}", filename, exc.what());
        logger::error("{}", errorMessage);
    }
}

void Profiling::ProfilingConfig::PopulateConfig(Profiling::ProfilingConfig& config, const json& jsonData, const std::string& configPath) {
    config.configKey = configPath;
    bool detectedFailure = false;

    for (const std::string& filter : jsonData["IncludeFilters"]) {
        config.includeFilters.push_back(filter);
    }

    for (const std::string& filter : jsonData["ExcludeFilters"]) {
        config.excludeFilters.push_back(filter);
    }

    config.outFilepath = jsonData["OutFilepath"];
    config.maxFilepathSuffix = jsonData["MaxFilepathSuffix"];

    config.maxNumCalls = jsonData["MaxNumCalls"];
    config.maxNumSeconds = jsonData["MaxNumSeconds"];

    uint32_t writeMode = jsonData["WriteMode"];
    if (writeMode < static_cast<uint32_t>(ProfilingConfig::ProfileWriteMode::Invalid)) {
        config.writeMode = static_cast<ProfilingConfig::ProfileWriteMode>(writeMode);
    } else {
        detectedFailure = true;
        config.writeMode = ProfilingConfig::ProfileWriteMode::Invalid;
        logger::error("Invalid write mode: {}", writeMode);
    }

    config.failedLoadFromFile = detectedFailure;
}