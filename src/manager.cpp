#define NOMINMAX
#include "cli/orchestrator.hpp"
#ifdef NDEBUG
#include <opencv2/core/utils/logger.hpp>
#endif
#include <csignal>
#include <fstream>
#include <map>
#include <CLI/App.hpp>
#include <GLFW/glfw3.h>
#include <toml++/toml.hpp>

#include "camera_calibration.hpp"
#include "utility.hpp"

#include "cli/board_creation.hpp"
#include "cli/calibration.hpp"
#include "cli/recording.hpp"
#include "cli/validation.hpp"

#include "config/orchestrator.hpp"

#include "recoding/detection_validator.hpp"
#include "recoding/job_data.hpp"
#include "recoding/video_viewer.hpp"
#include "recoding/recorders/basler_cam_worker.hpp"
#include "recoding/recorders/camera_worker.hpp"
#include "recoding/recorders/prophesee_cam_worker.hpp"

#include "tools/create_board.hpp"
#include "tools/image_validator.hpp"

int getWorstStopCode(const std::vector<YACCP::CamData>& camDatas) {
    int stopCode = 0;
    for (auto& camData : camDatas) {
        if (camData.runtimeData.exitCode > stopCode) {
            stopCode = camData.runtimeData.exitCode;
        }
    }

    return stopCode;
}

int main(int argc, char** argv) {
    // Decrease log level to warning for release builds.
#ifdef NDEBUG
    cv::utils::logging::setLogLevel(cv::utils::logging::LogLevel::LOG_LEVEL_WARNING);
#endif

    // Try to initialise GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        raise(SIGABRT);
    }
    // Get the current video mode of the primary monitor.
    // This mode will later be used to retrieve the width and height.
    auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    /*
     * CLI
     */
    YACCP::CLI::CliCmdConfig cliCmdConfig;
    YACCP::CLI::CliCmds cliCmds;
    YACCP::CLI::addCli(cliCmdConfig, cliCmds);
    CLI11_PARSE(cliCmds.app, argc, argv);

    // Construct the path where everything needs to be stored
    std::stringstream dateTime;
    dateTime << YACCP::Utility::getCurrentDateTime();
    auto workingDir = std::filesystem::current_path();
    std::filesystem::path path = workingDir / cliCmdConfig.appCmdConfig.userPath;
    std::filesystem::path dataPath = path / "data";

    /*
     * Config file loading
     */
    YACCP::Config::FileConfig fileConfig;
    try {
        YACCP::Config::loadConfig(fileConfig, path);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        return 1;
    }

    /*
     * Variable decleration
     */
    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(fileConfig.detectionConfig.openCvDictionaryId)};
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
    auto numCams{static_cast<int>(fileConfig.recordingConfig.slaveWorkers.size() + 1)};
    std::vector<YACCP::CamData> camDatas(numCams);
    std::vector<std::jthread> threads;
    std::vector<std::unique_ptr<YACCP::CameraWorker>> cameraWorkers;
    moodycamel::ReaderWriterQueue<YACCP::ValidatedCornersData> valCornersQ{100};
    std::map<std::string, YACCP::WorkerTypes>{
        {"PropheseeDVS", YACCP::WorkerTypes::prophesee},
        {"BaslerRGB", YACCP::WorkerTypes::basler},
    };
    std::filesystem::path jobPath;

    if (*cliCmds.boardCreationCmd) {
        jobPath = dataPath / ("job_" + dateTime.str());
        std::filesystem::create_directories(jobPath);
        YACCP::CreateBoard::charuco(fileConfig, boardCreationConfig, jobPath);
    }


    if (*cliCmds.recordingCmd) {
        if (recordingConfig.jobId.empty()) {
            std::cout << "No job ID given, using most recent one.\n";
            std::vector<std::filesystem::path> jobs;
            for (const auto& entry : std::filesystem::directory_iterator(dataPath))
            {
                jobs.emplace_back(entry.path());
            }
            jobPath = jobs.back();
            std::cout << jobPath << "\n\n";
        } else {
            jobPath = dataPath / recordingConfig.jobId;
            if (!is_directory(jobPath)) {
                std::cerr << "Job: " << jobPath.string() << " does not exist in the given path: " << dataPath << "\n";
            }
        }

        (void)std::filesystem::create_directories(jobPath / "images/raw");

        for (int i = 0; i < numCams - 1; ++i) {
            camDatas[i].info.isMaster = false;
            camDatas[i].info.camId = i;

            switch (fileConfig.recordingConfig.slaveWorkers[i]) {
            case YACCP::WorkerTypes::prophesee:
                cameraWorkers.emplace_back(
                    std::make_unique<YACCP::PropheseeCamWorker>(stopSource,
                                                                camDatas,
                                                                fileConfig.recordingConfig.fps,
                                                                i,
                                                                fileConfig.recordingConfig.accumulationTime,
                                                                1,
                                                                jobPath));
                break;

            case YACCP::WorkerTypes::basler:
                cameraWorkers.emplace_back(
                    std::make_unique<YACCP::BaslerCamWorker>(stopSource,
                                                             camDatas,
                                                             fileConfig.recordingConfig.fps,
                                                             i,
                                                             jobPath));
                break;
            }

            auto* worker = cameraWorkers.back().get();
            threads.emplace_back([worker] { worker->start(); });

            while (!camDatas[i].runtimeData.isRunning) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (stopSource.stop_requested()) {
                    return getWorstStopCode(camDatas);
                }
            }
        }

        camDatas.back().info.isMaster = true;
        camDatas.back().info.camId = camDatas.size() - 1;
        switch (fileConfig.recordingConfig.masterWorker) {
        case YACCP::WorkerTypes::prophesee:
            cameraWorkers.emplace_back(std::make_unique<YACCP::PropheseeCamWorker>(
                stopSource,
                camDatas,
                fileConfig.recordingConfig.fps,
                camDatas.size() - 1,
                fileConfig.recordingConfig.accumulationTime,
                1,
                jobPath));
            break;

        case YACCP::WorkerTypes::basler:
            cameraWorkers.emplace_back(
                std::make_unique<YACCP::BaslerCamWorker>(stopSource,
                                                         camDatas,
                                                         fileConfig.recordingConfig.fps,
                                                         camDatas.size() - 1,
                                                         jobPath));
            break;
        }

        auto* worker = cameraWorkers.back().get();
        threads.emplace_back([worker] { worker->start(); });

        // Start the viewing thread.
        while (!camDatas.back().runtimeData.isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (stopSource.stop_requested()) {
                return getWorstStopCode(camDatas);
            }
        }

        YACCP::VideoViewer videoViewer{
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
        threads.emplace_back(&YACCP::VideoViewer::start, &videoViewer);

        YACCP::DetectionValidator detectionValidator{
            stopSource,
            camDatas,
            charucoDetector,
            valCornersQ,
            jobPath,
            fileConfig.detectionConfig.cornerMin
        };
        threads.emplace_back(&YACCP::DetectionValidator::start, &detectionValidator);

        for (auto& thread : threads) {
            thread.join();
        }
        // Create json object with camera data.
        std::cout << "\nWriting job_data.json\n";
        // YACCP::CharucoConfig charucoConfig;
        // charucoConfig.openCvDictionaryId = dictId;
        // charucoConfig.boardSize = cv::Size(squaresX, squaresY);
        // charucoConfig.squareLength = squareLength;
        // charucoConfig.markerLength = markerLength;
        // charucoConfig.charucoParams = charucoParams;
        // charucoConfig.detParams = detParams;
        nlohmann::json j;
        j["openCv"] = CV_VERSION;
        // j["camPlacement"] = camPlacement;
        // j["charuco"] = charucoConfig;
        j["cams"] = nlohmann::json::object();

        for (auto i{0}; i < camDatas.size(); ++i) {
            j["cams"]["cam_" + std::to_string(i)] = camDatas[i].info;
        }

        // Save json to file.
        std::ofstream file(jobPath / "job_data.json");
        file << j.dump(4);
    }


    if (*validationCmd) {
        if (validationConfig.showAvailableJobs) YACCP::ImageValidator::listJobs(dataPath);

        YACCP::ImageValidator imageValidator;
        imageValidator.validateImages(mode->width - 100,
                                      mode->height - 100,
                                      dataPath,
                                      validationConfig.jobId);
    }

    if (*calibrationCmd.calibration) {
        YACCP::CameraCalibration calibration(charucoDetector, path, fileConfig.detectionConfig.cornerMin);
        //
        // calibration.monoCalibrate("job_2025-12-28_18-54-04");
        //
        // return 0;
        if (*calibrationCmd.mono) {
        } else if (*calibrationCmd.stereo) {
        }
    }

    return 0;
}
