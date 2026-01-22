#include "orchestrator.hpp"

#include "../global_variables/program_defaults.hpp"

#include "../recoding/recorders/camera_worker.hpp"

namespace YACCP::Config {
    void loadConfig(FileConfig& config, const std::filesystem::path& path, const bool boardCreation) {
        // Load TOML configuration file.
        toml::table tbl;
        try {
            tbl = toml::parse_file((path / GlobalVariables::configFileName).string());
        } catch (const toml::parse_error& err) {
            std::stringstream ss{};
            ss << err.description() << "\n At: " << err.source();
            throw std::runtime_error(ss.str());
        }

        parseBoardConfig(tbl, config.boardConfig, boardCreation);
        parseDetectionConfig(tbl, config.detectionConfig);
        parseRecordingConfig(tbl, config.recordingConfig);
        parseViewingConfig(tbl, config.viewingConfig);
    }

    void loadBoardConfig(FileConfig& config, const std::filesystem::path& path, const bool boardCreation) {
        // Load TOML configuration file.
        toml::table tbl;
        try {
            tbl = toml::parse_file((path / GlobalVariables::configFileName).string());
        } catch (const toml::parse_error& err) {
            std::stringstream ss{};
            ss << err.description() << "\n At: " << err.source();
            throw std::runtime_error(ss.str());
        }

        parseBoardConfig(tbl, config.boardConfig, boardCreation);
        parseDetectionConfig(tbl, config.detectionConfig);
    }
} // YACCP::Config
