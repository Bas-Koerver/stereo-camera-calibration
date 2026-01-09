//
// Created by Bas_K on 2026-01-02.
//

#include "validation.hpp"

namespace YACCP::CLI
{
    ::CLI::App* addValidationCmd(::CLI::App &app, ValidationCmdConfig &config) {
        ::CLI::App* subCmd = app.add_subcommand("validate", "Validate recorded images");

        subCmd->add_flag("-s, --show-jobs", config.showAvailableJobs, "Show available jobs");
        subCmd->add_option("-j, --job-id", config.jobId, "Give a specific job ID to validate");

        subCmd->require_option(1);

        return subCmd;
    }
} // namespace YACCP::CLI