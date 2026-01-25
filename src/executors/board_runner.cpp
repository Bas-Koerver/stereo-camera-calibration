#include "board_runner.hpp"

#include "../utility.hpp"

#include "../tools/create_board.hpp"


namespace YACCP::Executor {
    int runBoardCreation(const CLI::CliCmdConfig& cliCmdConfig,
                         const std::filesystem::path& path,
                         const std::string& dateTime) {
        const std::filesystem::path dataPath{path / "data"};

        // Show jobs that are missing a board image and/or video.
        if (cliCmdConfig.boardCreationCmdConfig.showAvailableJobs) {
            Utility::checkDataPath(dataPath);
            CreateBoard::listJobs(dataPath);
        } else {
            std::filesystem::path jobPath;
            Config::FileConfig fileConfig;

            // If no job ID is given create new folder and load TOML config.
            if (cliCmdConfig.boardCreationCmdConfig.jobId.empty()) {
                // Load config from TOML file
                Config::loadBoardConfig(fileConfig, path, true);

                std::cout << "No job ID given, creating a new one.\n";
                jobPath = dataPath / ("job_" + dateTime);
                (void)std::filesystem::create_directories(jobPath);
            } else {
                // If job ID is given, load the JSON config from the given job ID
                // Otherwise generate a board for the given job ID
                jobPath = dataPath / cliCmdConfig.boardCreationCmdConfig.jobId;

                // Check whether the given job std::filesystem::exists.
                Utility::checkJobPath(dataPath, cliCmdConfig.boardCreationCmdConfig.jobId);

                // Load config from JSON file
                nlohmann::json j = Utility::loadJobDataFromFile(jobPath);
                (void)j.at("config").get_to(fileConfig);
            }

            CreateBoard::charuco(fileConfig, cliCmdConfig.boardCreationCmdConfig, jobPath);

            // Save the board creation variables to a JSON
            Utility::saveJobDataToFile(jobPath, fileConfig);
        }

        return 0;
    }
} // YACCP::Executor