#include "recording_runner.hpp"

#include "../utility.hpp"

#include "../global_variables/program_defaults.hpp"

#include "../recoding/recorders/basler_cam_worker.hpp"
#include "../recoding/recorders/prophesee_cam_worker.hpp"

#include <thread>

#include <GLFW/glfw3.h>

namespace YACCP::Executor {
    int getWorstStopCode(const std::vector<CamData>& camDatas) {
        auto stopCode{0};
        for (const auto& [info, runtimeData] : camDatas) {
            if (runtimeData.exitCode > stopCode) {
                stopCode = runtimeData.exitCode;
            }
        }

        return stopCode;
    }


    int runRecording(CLI::CliCmdConfig& cliCmdConfig,
                     const std::filesystem::path& path,
                     const std::string& dateTime) {
        const std::filesystem::path dataPath{path / "data"};

        if (cliCmdConfig.recordingCmdConfig.showAvailableCams) {
            BaslerCamWorker::listAvailableSources();
            PropheseeCamWorker::listAvailableSources();
        } else if (cliCmdConfig.recordingCmdConfig.showAvailableJobs) {
            CameraWorker::listJobs(dataPath);
        } else {
            std::filesystem::path jobPath;
            Config::FileConfig fileConfig;
            // Get the current video mode of the primary monitor.
            // This mode will later be used to retrieve the width and height.
            const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

            // Loading in config variables depending on user defined CLI flags and current directory status.
            if (cliCmdConfig.recordingCmdConfig.jobId.empty()) {
                std::cout << "No job ID given, checking if most recent job already has recording data\n";
                // Get the most recent job
                std::filesystem::path jobPathMostRecent;
                for (const auto& entry : std::filesystem::directory_iterator(dataPath)) {
                    if (!entry.is_directory()) {
                        continue;
                    }
                    if (jobPathMostRecent.empty() || entry.path().filename() > jobPathMostRecent.filename()) {
                        jobPathMostRecent = entry.path();
                    }
                }

                // If most recent job already has recording data create a new job and use that job,
                // otherwise use most recent job.
                if (Utility::isNonEmptyDirectory(jobPathMostRecent / "images" / "raw")) {
                    Config::FileConfig tomlConfig;
                    std::cout <<
                        "The most recent job ID already has recoding data, creating a new job and copying job config from previous one. \n";
                    jobPath = dataPath / ("job_" + dateTime);
                    (void)std::filesystem::create_directories(jobPath);
                    std::filesystem::copy(jobPathMostRecent / GlobalVariables::jobDataFileName,
                                          jobPath / GlobalVariables::jobDataFileName);
                } else {
                    jobPath = jobPathMostRecent;
                }

                Config::FileConfig jsonConfig;

                // Load config from JSON file
                nlohmann::json j = Utility::loadJobDataFromFile(jobPath);
                (void)j.at("config").get_to(jsonConfig);

                // Load config from TOML file
                YACCP::Config::loadConfig(fileConfig, path);

                if (std::tie(fileConfig.boardConfig.boardSize, fileConfig.detectionConfig.openCvDictionaryId) !=
                    std::tie(jsonConfig.boardConfig.boardSize, jsonConfig.detectionConfig.openCvDictionaryId)) {
                    std::cerr << "Warning configured TOML does not coincide with stored JSON\n"
                        "This is a warning just to inform you that JSON config will be used for board setup\n";
                }
                fileConfig.boardConfig.boardSize = jsonConfig.boardConfig.boardSize;
                fileConfig.detectionConfig.openCvDictionaryId = jsonConfig.detectionConfig.openCvDictionaryId;
            } else {
                jobPath = dataPath / cliCmdConfig.recordingCmdConfig.jobId;
                Config::FileConfig jsonConfig;

                Utility::checkJobPath(dataPath, cliCmdConfig.recordingCmdConfig.jobId);

                if (Utility::isNonEmptyDirectory(jobPath / "images" / "raw")) {
                    std::cout <<
                        "There is already recording data present for this job ID, do you want to override it?  (y/n): \n";

                    if (Utility::askYesNo()) {
                        (void)std::filesystem::remove_all(jobPath / "images" / "raw");
                    } else {
                        std::cout << "Aborting recording to avoid overwriting existing data.\n\n";
                        return 0;
                    }
                }

                // Load config from JSON file
                nlohmann::json j = Utility::loadJobDataFromFile(jobPath);
                (void)j.at("config").get_to(jsonConfig);

                // Load config from TOML file
                YACCP::Config::loadConfig(fileConfig, path);

                if (std::tie(fileConfig.boardConfig.boardSize, fileConfig.detectionConfig.openCvDictionaryId) !=
                    std::tie(jsonConfig.boardConfig.boardSize, jsonConfig.detectionConfig.openCvDictionaryId)) {
                    std::cerr << "Warning configured TOML does not coincide with stored JSON\n"
                        "This is a warning just to inform you that JSON config will be used for board setup\n";
                }
                fileConfig.boardConfig.boardSize = jsonConfig.boardConfig.boardSize;
                fileConfig.detectionConfig.openCvDictionaryId = jsonConfig.detectionConfig.openCvDictionaryId;
            }

            Utility::AlternativeBuffer buffer;
            buffer.enable();

            // Variable setup based on config.
            cv::aruco::Dictionary dictionary{
                cv::aruco::getPredefinedDictionary(fileConfig.detectionConfig.openCvDictionaryId)
            };
            cv::aruco::CharucoParameters charucoParams;
            cv::aruco::DetectorParameters detParams;
            cv::aruco::CharucoBoard board{
                fileConfig.boardConfig.boardSize,
                fileConfig.boardConfig.squareLength,
                fileConfig.boardConfig.markerLength,
                dictionary
            };
            cv::aruco::CharucoDetector charucoDetector(board, charucoParams, detParams);

            std::stop_source stopSource;
            auto numCams{static_cast<int>(fileConfig.recordingConfig.workers.size())};
            std::vector<CamData> camDatas(numCams);
            std::vector<std::jthread> threads;
            std::vector<std::unique_ptr<CameraWorker> > cameraWorkers(numCams);
            moodycamel::ReaderWriterQueue<ValidatedCornersData> valCornersQ{100};

            (void)std::filesystem::create_directories(jobPath / "images" / "raw");

            for (auto i{0}; i < numCams; ++i) {
                const int index{fileConfig.recordingConfig.workers[i].placement};
                camDatas[index].info.camIndexId = index;
                camDatas[index].info.isMaster = index == fileConfig.recordingConfig.masterWorker;

                std::visit([&](const auto& backend) {
                               using T = std::decay_t<decltype(backend)>;

                               if constexpr (std::is_same_v<T, Config::Basler>) {
                                   cameraWorkers[index] =
                                       std::make_unique<BaslerCamWorker>(stopSource,
                                                                         camDatas,
                                                                         fileConfig.recordingConfig,
                                                                         backend,
                                                                         index,
                                                                         jobPath);
                               } else if constexpr (std::is_same_v<T, Config::Prophesee>) {
                                   cameraWorkers[index] =
                                       std::make_unique<PropheseeCamWorker>(stopSource,
                                                                            camDatas,
                                                                            fileConfig.recordingConfig,
                                                                            backend,
                                                                            index,
                                                                            jobPath);
                               }
                           },
                           fileConfig.recordingConfig.workers[i].configBackend);

                if (camDatas[index].info.isMaster) continue;
                auto* worker = cameraWorkers[index].get();
                threads.emplace_back([worker] {
                    worker->start();
                });
            }

            // Wait till all slave cameras have started
            auto allSlavesRunning{false};
            while (!allSlavesRunning) {
                if (stopSource.stop_requested()) {
                    return getWorstStopCode(camDatas);
                }

                allSlavesRunning = true;
                for (auto i{0}; i < numCams; ++i) {
                    if (camDatas[i].info.isMaster) continue;
                    if (!camDatas[i].runtimeData.isRunning.load()) {
                        allSlavesRunning = false;
                        break;
                    }
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            // Start the master camera
            {
                auto* worker = cameraWorkers[fileConfig.recordingConfig.masterWorker].get();
                threads.emplace_back([worker] {
                    worker->start();
                });
            }

            // Before the videoViewer can be started, the master camera needs to be running first,
            // wait until this is true.
            while (!camDatas[fileConfig.recordingConfig.masterWorker].runtimeData.isRunning.load()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (stopSource.stop_requested()) {
                    return getWorstStopCode(camDatas);
                }
            }

            // Start the videoViwer
            VideoViewer videoViewer{
                stopSource,
                fileConfig.viewingConfig.viewsHorizontal,
                mode->width - 100,
                mode->height - 100,
                camDatas,
                charucoDetector,
                valCornersQ,
                jobPath,
                fileConfig.detectionConfig.cornerMin,
            };
            threads.emplace_back(&VideoViewer::start, &videoViewer);

            // Start the detectionValidator
            DetectionValidator detectionValidator{
                stopSource,
                camDatas,
                charucoDetector,
                valCornersQ,
                jobPath,
                fileConfig.detectionConfig.cornerMin
            };
            threads.emplace_back(&DetectionValidator::start, &detectionValidator);

            // Join all threads when they've finished executing.
            for (auto& thread : threads) {
                thread.join();
            }
            buffer.disable();

            // Create a JSON object with all information on this job,
            // that includes the configured parameters in the config.toml and information about the job itself.
            Utility::saveJobDataToFile(jobPath, fileConfig, &camDatas);
        }
        return 0;
    }
} // YACCP::Executor