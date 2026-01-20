#include "camera_worker.hpp"

#include "../job_data.hpp"

// #include <iostream>
// #include <stop_token>
// #include <utility>

namespace YACCP {
    CameraWorker::CameraWorker(std::stop_source stopSource,
                               std::vector<CamData>& camDatas,
                               Config::RecordingConfig& recordingConfig,
                               const int index,
                               std::filesystem::path jobPath)
        : stopSource_(stopSource),
          stopToken_(stopSource.get_token()),
          camDatas_(camDatas),
          camData_(camDatas.at(index)),
          recordingConfig_(recordingConfig),
          index_(index),
          jobPath_(std::move(jobPath)) {
    }

    void CameraWorker::listAvailableSources() {
        std::cerr << "The function to list the available sources is not implemented \n";
        throw std::logic_error("Function not yet implemented");
    }

    void CameraWorker::start() {
        std::cerr << "The function to start the camera worker is not implemented \n";
        throw std::logic_error("Function not yet implemented");
    }
} // YACCP