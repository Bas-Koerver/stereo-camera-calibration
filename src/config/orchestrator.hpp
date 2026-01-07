#ifndef YACCP_CONFIG_ORCHESTRATOR_HPP
#define YACCP_CONFIG_ORCHESTRATOR_HPP
#include <filesystem>

#include "board.hpp"
#include "detection.hpp"
#include "recording.hpp"
#include "viewing.hpp"

namespace YACCP::Config {
    struct FileConfig {
        BoardConfig boardConfig;
        DetectionConfig detectionConfig;
        RecordingConfig recordingConfig;
        ViewingConfig viewingConfig;
    };

    void loadConfig(FileConfig& config, const std::filesystem::path& path);
} // YACCP::Config

#endif //YACCP_CONFIG_ORCHESTRATOR_HPP