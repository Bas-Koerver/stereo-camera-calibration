#include "board_runner.hpp"

#include "../utility.hpp"

#include "../tools/create_board.hpp"


namespace YACCP::Executor {
    void runBoardCreation(CLI::CliCmdConfig& cliCmdConfig,
                          std::filesystem::path path,
                          const std::stringstream& dateTime) {
        const std::filesystem::path dataPath{path / "data"};

        // Show jobs that are missing a board image and/or video.
        if (cliCmdConfig.boardCreationCmdConfig.showAvailableJobs) {
            CreateBoard::listJobs(dataPath);
        } else {
            std::filesystem::path jobPath;
            Config::FileConfig fileConfig;


            // If no job ID is given create new folder and load TOML config.
            if (cliCmdConfig.boardCreationCmdConfig.jobId.empty()) {
                // Load config from TOML file
                Config::loadBoardConfig(fileConfig, path, true);

                std::cout << "No job ID given, creating a new one.\n";
                jobPath = dataPath / ("job_" + dateTime.str());
                std::filesystem::create_directories(jobPath);
                // If job ID is given, load the JSON config from the given job ID
            } else {
                // Otherwise generate a board for the given job ID
                jobPath = dataPath / cliCmdConfig.boardCreationCmdConfig.jobId;

                // Check whether the given job exists.
                if (!is_directory(jobPath)) {
                    std::stringstream ss;
                    ss << "Job: " << jobPath.string() << " does not exist in the given path: " << dataPath;
                    throw std::runtime_error(ss.str());
                }

                // Load config from JSON file
                nlohmann::json j = Utility::loadJobDataFromFile(jobPath);
                j.at("config").get_to(fileConfig);
            }

            CreateBoard::charuco(fileConfig, cliCmdConfig.boardCreationCmdConfig, jobPath);

            // Save the board creation variables to a JSON
            std::vector<CamData> empty{};
            Utility::saveJobDataToFile(jobPath, fileConfig, empty);
        }
    }
} // YACCP::Executor
