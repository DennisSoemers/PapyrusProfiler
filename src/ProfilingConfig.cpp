#include "ProfilingConfig.h"
#include "tojson.hpp"

Profiling::ProfilingConfig ProfilingConfig(const std::string& configPath) {
    Profiling::ProfilingConfig config;

    // Thanks to doodlum for most of the JSON/YAML parsing code (taken from: https://github.com/doodlum/skyrim-srd)
    auto path = std::filesystem::path(configPath).filename();
    auto filename = path.string();

	try {
        std::ifstream i(configPath);
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
            Profiling::ProfilingConfig::PopulateConfig(config, data);
        } else {
            std::string errorMessage = std::format("Failed to parse {}\nBad file stream", filename);
            logger::error("{}", errorMessage);
        }
    } catch (const std::exception& exc) {
        std::string errorMessage = std::format("Failed to parse {}\n{}", filename, exc.what());
        logger::error("{}", errorMessage);
    }

	return config;
}

void Profiling::ProfilingConfig::PopulateConfig(Profiling::ProfilingConfig& config, const json& jsonData) {
    config.outFilepath = jsonData["OutFilepath"];
    config.maxFilepathSuffix = jsonData["MaxFilepathSuffix"];
    config.maxNumCalls = jsonData["MaxNumCalls"];
}