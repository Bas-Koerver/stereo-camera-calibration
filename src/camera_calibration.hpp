#ifndef YACCP_CAMERA_CALIBRATION_HPP
#define YACCP_CAMERA_CALIBRATION_HPP
#include <filesystem>

#include <opencv2/objdetect/charuco_detector.hpp>

namespace YACCP {
    class CameraCalibration {
    public:
        CameraCalibration(const cv::aruco::CharucoDetector& charucoDetector,
                          const std::filesystem::path& dataPath,
                          float cornerMin);

        void monoCalibrate(const std::string& jobName);

    private:
        const cv::aruco::CharucoDetector charucoDetector_;
        const std::filesystem::path& dataPath_;
        std::filesystem::path jobPath_{};
        const float cornerMin_;

        // void processImage(std::vector<cv::Point3f> objPoints, std::vector<cv::Point2f> imagePoints);
    };
}


#endif //YACCP_CAMERA_CALIBRATION_HPP