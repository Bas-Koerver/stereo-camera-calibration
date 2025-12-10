//
// Created by Bas_K on 2025-12-08.
//

#include "camera_worker.hpp"

#include <iostream>
#include <stop_token>

namespace YACCP {
    CameraWorker::CameraWorker(std::stop_source stopSource,
                               CamData &camData,
                               int fps,
                               int id,
                               std::string camId) : stopSource_(stopSource),
                                                    camData_(camData),
                                                    fps_(fps),
                                                    id_(id),
                                                    camId_(std::move(camId)) {
        stopToken_ = stopSource_.get_token();
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
