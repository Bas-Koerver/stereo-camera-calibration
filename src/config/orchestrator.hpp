#ifndef YACCP_CONFIG_ORCHESTRATOR_HPP
#define YACCP_CONFIG_ORCHESTRATOR_HPP
#include "board.hpp"
#include "detection.hpp"
#include "recording.hpp"
#include "viewing.hpp"

// #include <filesystem>

namespace YACCP::Config {
    struct FileConfig {
        BoardConfig boardConfig;
        DetectionConfig detectionConfig;
        RecordingConfig recordingConfig;
        ViewingConfig viewingConfig;
    };

    template <typename T>
    [[nodiscard]] T requireVariable(const toml::table& tbl, std::string_view key, std::string_view keyPath = {}) {
        if (auto value = tbl[key].value < T > ())
            return *value;

        if (keyPath.empty())
            throw std::runtime_error("Variable: " + std::string(key) + " is missing or invalid");

        throw std::runtime_error(
            "Variable: " + std::string(key) + " at [" + std::string(keyPath) + "] is missing or invalid");
    }

    WorkerTypes stringToWorkerType(const std::string& worker);

    void loadConfig(FileConfig& config, const std::filesystem::path& path);

    void loadBoardConfig(FileConfig& config, const std::filesystem::path& path);

    inline void to_json(nlohmann::json& j, const FileConfig& f) {
        j = {
            {"boardConfig", f.boardConfig},
            {"detectionConfig", f.detectionConfig},
            {"recordingConfig", f.recordingConfig}
        };
    }

    inline void from_json(const nlohmann::json& j, FileConfig& f) {
        j.at("boardConfig").get_to(f.boardConfig);
        j.at("detectionConfig").get_to(f.detectionConfig);
        j.at("recordingConfig").get_to(f.recordingConfig);
    }
} // YACCP::Config

#endif //YACCP_CONFIG_ORCHESTRATOR_HPP