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

    void pairWiseStereoCalibrate(const cv::aruco::CharucoDetector& charucoDetector,
                                 std::vector<CamData>& camDatas,
                                 std::vector<StereoCalibData>& stereoCalibDatas,
                                 const Config::FileConfig& fileConfig,
                                 const std::filesystem::path& jobPath);
}


#endif //YACCP_CAMERA_CALIBRATION_HPP
