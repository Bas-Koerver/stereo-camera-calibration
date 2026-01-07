#ifndef YACCP_CLI_RECORDING_HPP
#define YACCP_CLI_RECORDING_HPP
#include <CLI/App.hpp>

namespace YACCP::CLI
{
    struct RecordingConfig
    {
        bool showAvailableJobs{};
        std::string jobId{};
        bool showAvailableCams{};
    };

    ::CLI::App* addRecordingCmd(::CLI::App& app, RecordingConfig& config);

} // namespace YACCP::CLI

#endif // YACCP_CLI_RECORDING_HPP
