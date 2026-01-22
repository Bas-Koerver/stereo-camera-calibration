#ifndef YACCP_CAMERA_CALIBRATION_HPP
#define YACCP_CAMERA_CALIBRATION_HPP
#include "config/orchestrator.hpp"

#include "recoding/job_data.hpp"

#include <filesystem>

#include <opencv2/objdetect/charuco_detector.hpp>

namespace YACCP::Calibration {
    void monoCalibrate(const cv::aruco::CharucoDetector& charucoDetector,
                       std::vector<CamData>& camDatas,
                       const Config::FileConfig& fileConfig,
                       const std::filesystem::path& jobPath);
}

namespace YACCP {
    class CameraCalibration {
    public:
        CameraCalibration(const cv::aruco::CharucoDetector& charucoDetector,
                          const std::filesystem::path& dataPath,
                          float cornerMin);

        void monoCalibrate(const std::string& jobName);

        void stereoCalibrate(const std::string& jobName);

    private:
        const cv::aruco::CharucoDetector charucoDetector_;
        const std::filesystem::path& dataPath_;
        std::filesystem::path jobPath_{};
        const float cornerMin_;
    };
}


#endif //YACCP_CAMERA_CALIBRATION_HPP
