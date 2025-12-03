#ifndef STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
#include <cstdint>

#include <opencv2/objdetect/charuco_detector.hpp>

#include "../VideoViewer.hpp"


namespace YACC {
    class PropheseeDVSWorker {
    public:
        PropheseeDVSWorker(std::stop_token stopToken,
                           CamData &camData,
                           std::uint16_t fps,
                           std::uint16_t accumulation_time,
                           const cv::aruco::CharucoDetector &charucoDetector,
                           const std::string &camId = {});

        void listAvailableSources();

        void start();

    private:
        std::stop_token stopToken_;
        CamData &camData_;
        const std::uint16_t fps_;
        const std::uint16_t accumulationTime_;
        const cv::aruco::CharucoDetector charucoDetector_;
        const std::string camId_;
    };
}
#endif //STEREO_CAMERA_CALIBRATION_PROPHESEE_DVS_RECORDER_H
