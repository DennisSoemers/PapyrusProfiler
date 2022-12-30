#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Profiling {
#pragma warning(push)
#pragma warning(disable : 4251)

	class ProfilingConfig {
    public:
        ProfilingConfig() = default;
        ProfilingConfig(const std::string& configPath);

        /** 
         * Filepath we want to write our output to (without file extension). 
         * A suffix and file extension will be added to this. 
         */
        std::string outFilepath = "";

        /** Max suffix we'll add to filepath to avoid overwriting previous outputs (start counting at 0). */
        uint32_t maxFilepathSuffix = 0;

        /** Maximum number of calls we'll collect before we stop profiling. Zero is treated as no limit. */
        uint32_t maxNumCalls = 0;

        static void PopulateConfig(ProfilingConfig& config, const json& jsonData);
	};

#pragma warning(pop)
} // namespace Profiling