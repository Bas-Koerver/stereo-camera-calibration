#include <map>

#include "utility.hpp"
#include "VideoViewer.hpp"

#include "recorders/basler_rgb_recorder.hpp"
#include "recorders/camera_worker.hpp"
#include "recorders/prophesee_dvs_recorder.hpp"

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
    (void) YACCP::utility::createDirs("./data/images/");
    std::map<std::string, YACCP::WorkerTypes>{
        {"PropheseeDVS", YACCP::WorkerTypes::propheseeDVS},
        {"BaslerRGB", YACCP::WorkerTypes::baslerRGB},
    };

    // TODO: Add config / CLI
    std::vector<YACCP::WorkerTypes> slaveWorkers{YACCP::WorkerTypes::propheseeDVS};
    YACCP::WorkerTypes masterWorker{YACCP::WorkerTypes::baslerRGB};
    int squaresX{7};
    int squaresY{5};
    float squareLength{0.0295};
    float markerLength{0.0206};
    bool refine{false};
    int fps{30};
    int acc{33333};
    int viewsHorizontal{3};
    std::stop_source stopSource;
    std::vector<std::unique_ptr<YACCP::CameraWorker> > cameraWorkers;

    int numCams{static_cast<int>(slaveWorkers.size() + 1)};
    std::vector<YACCP::CamData> camDatas(numCams);
    std::vector<std::jthread> threads;
    cv::aruco::DetectorParameters detParams;
    cv::aruco::CharucoParameters charucoParams;
    if (refine) {
        charucoParams.tryRefineMarkers = true;
    }

    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_50)};
    cv::aruco::CharucoBoard board{cv::Size(squaresX, squaresY), squareLength, markerLength, dictionary};
    cv::aruco::CharucoDetector charucoDetector(board, charucoParams, detParams);

    for (int i = 0; i < numCams - 1; i++) {
        camDatas[i].isMaster = false;

        switch (slaveWorkers[i]) {
            case YACCP::WorkerTypes::propheseeDVS:
                cameraWorkers.emplace_back(std::make_unique<YACCP::PropheseeDVSWorker>(
                    stopSource,
                    camDatas[i],
                    fps,
                    i,
                    acc
                ));
                break;

            case YACCP::WorkerTypes::baslerRGB:
                cameraWorkers.emplace_back(std::make_unique<YACCP::BaslerRGBWorker>(
                    stopSource,
                    camDatas[i],
                    fps,
                    i
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
    switch (masterWorker) {
        case YACCP::WorkerTypes::propheseeDVS:
            cameraWorkers.emplace_back(std::make_unique<YACCP::PropheseeDVSWorker>(
                stopSource,
                camDatas.back(),
                fps,
                camDatas.size() - 1,
                acc
            ));
            break;

        case YACCP::WorkerTypes::baslerRGB:
            cameraWorkers.emplace_back(std::make_unique<YACCP::BaslerRGBWorker>(
                stopSource,
                camDatas.back(),
                fps,
                camDatas.size() - 1
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

    YACCP::VideoViewer videoViewer{stopSource, viewsHorizontal, camDatas, charucoDetector};
    threads.emplace_back(&YACCP::VideoViewer::start, &videoViewer);

    for (auto &thread: threads) {
        thread.join();
    }

    return 0;
}
