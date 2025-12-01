#ifndef STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#include <cstdint>

#include <opencv2/objdetect/charuco_detector.hpp>

#include "../VideoViewer.h"


namespace YACC {
    class PropheseeDVSWorker {
    public:
        PropheseeDVSWorker(camData &cam,
                           std::uint16_t fps,
                           std::uint16_t accumulation_time,
                           const cv::aruco::CharucoDetector &charuco_detector, const std::string &id = {});

        void listAvailableSources();

        void start();

    private:
        const std::uint16_t fps_;
        const std::uint16_t accumulationTime_;
        const cv::aruco::CharucoDetector charucoDetector_;
        const std::string id_;
        camData &cam_;
    };
}
#endif //STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
