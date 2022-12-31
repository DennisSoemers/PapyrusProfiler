#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Profiling {
#pragma warning(push)
#pragma warning(disable : 4251)

	class ProfilingConfig {
    public:

        /** 
         * Different modes for writing.
         */
        enum class ProfileWriteMode : std::uint32_t { 
            WriteAtEnd = 0,         // Write all data at once at the end
            WriteLive = 1,          // Continously write data as we collect it
            NoWrite = 2,            // Don't write anything at all (to a file)

            Invalid = 3
        };

        ProfilingConfig() = default;
        ProfilingConfig(const std::string& configPath);

        /** If not empty, stacktraces must match at least one of these filters (regex) to be included in collected data. */
        std::vector<std::string> includeFilters;

        /** Any stacktraces that match any of these filters (regex) are excluded from collected data. */
        std::vector<std::string> excludeFilters;

        /** 
         * Key identifying this config. Can only have one config active at any time per key. 
         * Will use filename / filepath as key when loading configs from files.
         */
        std::string configKey = "";

        /** 
         * Filepath we want to write our output to (without file extension). 
         * A suffix and file extension will be added to this. 
         */
        std::string outFilepath = "";

        /** Max suffix we'll add to filepath to avoid overwriting previous outputs (start counting at 0). */
        uint32_t maxFilepathSuffix = 0;

        /** Maximum number of calls we'll collect before we stop profiling. Zero is treated as no limit. */
        uint32_t maxNumCalls = 0;

        /** Maximum number of seconds for which we'll profile. Zero is treated as no limit. */
        uint32_t maxNumSeconds = 0;

        /** How/when do we want to write data to files? */
        ProfileWriteMode writeMode = ProfileWriteMode::WriteAtEnd;

        /** Did we fail to correctly load this config from a file? */
        bool failedLoadFromFile = false;

        static void PopulateConfig(ProfilingConfig& config, const json& jsonData, const std::string& configPath);
	};

#pragma warning(pop)
} // namespace Profiling