#include "recording.hpp"

#include <CLI/App.hpp>

namespace YACCP::CLI
{
    ::CLI::App* addRecordingCmd(::CLI::App& app, RecordingConfig& config) {
        ::CLI::App* subCmd = app.add_subcommand("record", "Record detections");

        subCmd->add_flag("-s, --show-jobs", config.showAvailableJobs, "Show available jobs");
        subCmd->add_option("-j, --job-id", config.jobId, "Give a specific job ID to record to") -> default_str("Latest job ID");
        subCmd->add_flag("-c, --show-cams", config.showAvailableCams, "Show all available cameras");

        subCmd->parse_complete_callback([&config]
        {
           if (config.showAvailableJobs && config.showAvailableCams)
           {
               throw std::runtime_error("Show available jobs and show available cameras are mutually exclusive, you can only use one at a time\n");
           }
        });

       return subCmd;
    }
} // namespace YACCP