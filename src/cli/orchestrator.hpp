#ifndef YACCP_SRC_CLI_ORCHESTRATOR_HPP
#define YACCP_SRC_CLI_ORCHESTRATOR_HPP
#include "board_creation.hpp"
#include "calibration.hpp"
#include "recording.hpp"
#include "validation.hpp"


namespace YACCP::CLI {
    struct AppCmdConfig {
        std::filesystem::path userPath;
    };

    struct CliCmdConfig {
        AppCmdConfig appCmdConfig{};
        BoardCreationCmdConfig boardCreationCmdConfig{};
        RecordingCmdConfig recordingCmdConfig{};
        ValidationCmdConfig validationCmdConfig{};
        CalibrationCmdConfig calibrationCmdConfig{};
    };

    struct CliCmds {
        ::CLI::App app{};
        ::CLI::App* boardCreationCmd{};
        ::CLI::App* recordingCmd{};
        ::CLI::App* validationCmd{};
        CalibrationCmds calibrationCmds{};
    };

    void addCli(CliCmdConfig& cliCmdConfig, CliCmds& cliCmds);
} // YACCP::CLI


#endif //YACCP_SRC_CLI_ORCHESTRATOR_HPP
