#ifndef YACCP_CLI_CALIBRATION_HPP
#define YACCP_CLI_CALIBRATION_HPP
#include <CLI/App.hpp>

namespace YACCP::CLI {
    struct CalibrationCmdConfig {
        bool showAvailableJobs{};
        std::string jobId{};
    };

    struct CalibrationCmds {
        ::CLI::App* calibration{};
        ::CLI::App* mono{};
        ::CLI::App* stereo{};
    };

    CalibrationCmds addCalibrationCmds(::CLI::App& app, CalibrationCmdConfig& config);
} // namespace YACCP::CLI

#endif // YACCP_CLI_CALIBRATION_HPP
