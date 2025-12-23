//
// Created by Bas_K on 2025-12-08.
//

#include "camera_worker.hpp"

#include <iostream>
#include <stop_token>
#include <utility>

namespace YACCP {
    CameraWorker::CameraWorker(std::stop_source stopSource,
                               std::vector<CamData> &camDatas,
                               const int fps,
                               const int id,
                               std::filesystem::path outputPath,
                               std::string camId)
        : stopSource_(stopSource),
          stopToken_(stopSource.get_token()),
          camDatas_(camDatas),
          camData_(camDatas.at(id)),
          fps_(fps),
          id_(id),
          outputPath_(std::move(outputPath)),
          camId_(std::move(camId)) {
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