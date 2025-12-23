#include <csignal>
#include <map>

#include "detection_validator.hpp"
#include "Utility.hpp"
#include "Video_viewer.hpp"

#include "recorders/basler_cam_worker.hpp"
#include "recorders/camera_worker.hpp"
#include "recorders/prophesee_cam_worker.hpp"

#include <GLFW/glfw3.h>

#include "Image_validator.hpp"

int getWorstStopCode(const std::vector<YACCP::CamData> &camDatas) {
    int stopCode = 0;
    for (auto &camData: camDatas) {
        if (camData.exitCode > stopCode) {
            stopCode = camData.exitCode;
        }
    }

    return stopCode;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        raise(SIGABRT);
    }
    auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    // TODO: Add config / CLI
    // CLI to define high level parameters, what to do (create board, record, validate images, calibrate), where
    // config file is saved.
    // config files to define low level parameters, camera types, ids, fps, accumulation time, board parameters etc.
    std::vector<YACCP::WorkerTypes> slaveWorkers;
    YACCP::WorkerTypes masterWorker;
    slaveWorkers = {YACCP::WorkerTypes::prophesee};
    masterWorker = YACCP::WorkerTypes::basler;
    auto squaresX{7};
    auto squaresY{5};
    float squareLength{0.0295};
    float markerLength{0.0206};
    auto refine{false};
    auto fps{30};
    auto acc{33333};
    auto viewsHorizontal{3};
    float cornerMin{.25F};
    std::filesystem::path userPath{""};

    // Current working directory.
    auto workingDir = std::filesystem::current_path();
    std::stop_source stopSource;
    auto numCams{static_cast<int>(slaveWorkers.size() + 1)};
    std::vector<YACCP::CamData> camDatas(numCams);
    std::vector<std::jthread> threads;
    std::vector<std::unique_ptr<YACCP::CameraWorker> > cameraWorkers;
    moodycamel::ReaderWriterQueue<YACCP::ValidatedCornersData> valCornersQ{100};
    cv::aruco::DetectorParameters detParams;
    cv::aruco::CharucoParameters charucoParams;
    std::map<std::string, YACCP::WorkerTypes>{
        {"PropheseeDVS", YACCP::WorkerTypes::prophesee},
        {"BaslerRGB", YACCP::WorkerTypes::basler},
    };

    if (refine) {
        charucoParams.tryRefineMarkers = true;
    }
    auto now = std::chrono::system_clock::now();
    auto localTime = std::chrono::system_clock::to_time_t(now);
    std::stringstream dateTime;
    dateTime << std::put_time(std::localtime(&localTime), "%F_%H-%M-%S");
    // TODO: Catch invalid paths.
    std::filesystem::path path = workingDir / userPath / "data";
    std::filesystem::path outputPath = path / ("job_" + dateTime.str() + "/");

    YACCP::ImageValidator imageValidator(mode->width - 100,
                                         mode->height - 100,
                                         path);
    imageValidator.listJobs();
    imageValidator.validateImages(
        "job_2025-12-21_14-56-20"
    );
    return 0;

    (void) std::filesystem::create_directories(outputPath / "images/raw");
    // (void) std::filesystem::create_directories(outputPath / "images/verified");

    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_50)};
    cv::aruco::CharucoBoard board{cv::Size(squaresX, squaresY), squareLength, markerLength, dictionary};
    cv::aruco::CharucoDetector charucoDetector(board, charucoParams, detParams);

    for (int i = 0; i < numCams - 1; ++i) {
        camDatas[i].isMaster = false;
        camDatas[i].camId = i;

        switch (slaveWorkers[i]) {
            case YACCP::WorkerTypes::prophesee:
                cameraWorkers.emplace_back(std::make_unique<YACCP::PropheseeCamWorker>(
                    stopSource,
                    camDatas,
                    fps,
                    i,
                    acc,
                    1,
                    outputPath
                ));
                break;

            case YACCP::WorkerTypes::basler:
                cameraWorkers.emplace_back(std::make_unique<YACCP::BaslerCamWorker>(
                    stopSource,
                    camDatas,
                    fps,
                    i,
                    outputPath
                ));
                break;
        }

        auto *worker = cameraWorkers.back().get();
        threads.emplace_back([worker] { worker->start(); });

        while (!camDatas[i].isRunning) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            if (stopSource.stop_requested()) {
                return getWorstStopCode(camDatas);
            }
        }
    }

    camDatas.back().isMaster = true;
    camDatas.back().camId = camDatas.size() - 1;
    switch (masterWorker) {
        case YACCP::WorkerTypes::prophesee:
            cameraWorkers.emplace_back(std::make_unique<YACCP::PropheseeCamWorker>(
                stopSource,
                camDatas,
                fps,
                camDatas.size() - 1,
                acc,
                1,
                outputPath
            ));
            break;

        case YACCP::WorkerTypes::basler:
            cameraWorkers.emplace_back(std::make_unique<YACCP::BaslerCamWorker>(
                stopSource,
                camDatas,
                fps,
                camDatas.size() - 1,
                outputPath
            ));
            break;
    }

    auto *worker = cameraWorkers.back().get();
    threads.emplace_back([worker] { worker->start(); });

    // Start the viewing thread.
    while (!camDatas.back().isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (stopSource.stop_requested()) {
            return getWorstStopCode(camDatas);
        }
    }

    YACCP::VideoViewer videoViewer{
        stopSource,
        viewsHorizontal,
        mode->width - 100,
        mode->height - 100,
        camDatas,
        charucoDetector,
        valCornersQ,
        outputPath,
        cornerMin
    };
    threads.emplace_back(&YACCP::VideoViewer::start, &videoViewer);

    YACCP::DetectionValidator detectionValidator{
        stopSource,
        camDatas,
        charucoDetector,
        valCornersQ,
        outputPath,
        cornerMin
    };
    threads.emplace_back(&YACCP::DetectionValidator::start, &detectionValidator);

    for (auto &thread: threads) {
        thread.join();
    }

    return 0;
}
