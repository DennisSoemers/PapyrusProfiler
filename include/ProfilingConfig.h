#pragma once

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
        std::string outFilepath;

        /** Max suffix we'll add to filepath to avoid overwriting previous outputs (start counting at 0). */
        uint32_t maxFilepathSuffix;

        /** Maximum number of calls we'll collect before we stop profiling. */
        uint32_t maxNumCalls;
	};

#pragma warning(pop)
} // namespace Profiling