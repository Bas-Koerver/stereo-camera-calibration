#include "orchestrator.hpp"

#include <iostream>
#include <toml++/toml.hpp>

namespace YACCP::Config {
    void loadConfig(FileConfig& config, const std::filesystem::path& path) {
        // Load TOML configuration file.
        toml::table tbl;
        try {
            tbl = toml::parse_file((path / "config.toml").string());
        } catch (const toml::parse_error& err) {
            std::stringstream ss{};
            ss << err.description() << "\n At: " << err.source() << "\n";
            throw std::runtime_error(ss.str());
        }

        parseBoardConfig(tbl, config.boardConfig);
        parseDetectionConfig(tbl, config.detectionConfig);
        parseRecordingConfig(tbl, config.recordingConfig);
        parseViewingConfig(tbl, config.viewingConfig);
    }
} // YACCP::Config