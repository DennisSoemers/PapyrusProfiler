#include <filesystem>
#include "ProfilingConfig.h"
#include "tojson.hpp"

// Default config which we use if console command with no args is used to start profiling
Profiling::ProfilingConfig::ProfilingConfig() 
    : outFilename("PapyrusProfilerOutput"), maxFilepathSuffix(3), writeMode(ProfileWriteMode::WriteAtEnd) {}

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

void Profiling::ProfilingConfig::PopulateConfig(Profiling::ProfilingConfig& config, const json& jsonData,
                                                const std::string& configPath) {
    bool detectedFailure = false;

    if (jsonData.contains("IncludeFilters")) {
        for (const std::string& filter : jsonData["IncludeFilters"]) {
            config.includeFilters.emplace_back(filter);
        }
    }

    if (jsonData.contains("ExcludeFilters")) {
        for (const std::string& filter : jsonData["ExcludeFilters"]) {
            config.excludeFilters.emplace_back(filter);
        }
    }

    if (jsonData.contains("OutFilename")) {
        config.outFilename = jsonData["OutFilename"];
    } 
    if (jsonData.contains("MaxFilepathSuffix")) {
        config.maxFilepathSuffix = jsonData["MaxFilepathSuffix"];
    }

    if (jsonData.contains("MaxNumCalls")) {
        config.maxNumCalls = jsonData["MaxNumCalls"];
    }
    if (jsonData.contains("MaxNumSeconds")) {
        config.maxNumSeconds = jsonData["MaxNumSeconds"];
    }

    if (jsonData.contains("NumSkipCalls")) {
        config.numSkipCalls = jsonData["NumSkipCalls"];
    }

    if (jsonData.contains("WriteMode")) {
        uint32_t writeMode = jsonData["WriteMode"];
        if (writeMode < static_cast<uint32_t>(ProfileWriteMode::Invalid)) {
            config.writeMode = static_cast<ProfileWriteMode>(writeMode);

            if (config.outFilename.empty() && (config.writeMode == ProfileWriteMode::WriteAtEnd || config.writeMode == ProfileWriteMode::WriteLive)) {
                detectedFailure = true;
                logger::error("The config has WriteMode {}, but no specified OutFilepath", writeMode);
            }
        } else {
            detectedFailure = true;
            config.writeMode = ProfileWriteMode::Invalid;
            logger::error("Invalid write mode: {}", writeMode);
        }
    }

    config.failedLoadFromFile = detectedFailure;
}