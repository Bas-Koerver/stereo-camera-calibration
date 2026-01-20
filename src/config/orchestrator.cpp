#include "orchestrator.hpp"

#include "../recoding/recorders/camera_worker.hpp"

// #include <iostream>
// #include <toml++/toml.hpp>

namespace YACCP::Config {
    WorkerTypes stringToWorkerType(const std::string& worker) {
        if (const auto it = workerTypesMap.find(worker); it != workerTypesMap.end()) {
            return it->second;
        }
        throw std::runtime_error("Unknown worker type: " + worker);
    }

    std::string workerTypeToString(WorkerTypes workerType) {
        for (const auto& [key, value] : workerTypesMap) {
            if (value == workerType) return key;
        }
        return "Not found";
    }

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

    void loadBoardConfig(FileConfig& config, const std::filesystem::path& path) {
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
    }
} // YACCP::Config
