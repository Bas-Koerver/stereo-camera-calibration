//
// Created by Bas_K on 2025-11-17.
//

#ifndef STEREO_CAMERA_CALIBRATION_CHARUCO_BOARD_DETECTOR_H
#define STEREO_CAMERA_CALIBRATION_CHARUCO_BOARD_DETECTOR_H

#include <opencv2/objdetect/charuco_detector.hpp>


class utility {
public:
    static bool findShowBoard(const cv::aruco::CharucoDetector& charuco, const cv::Mat& gray, const cv::Mat& frame);
};


#endif //STEREO_CAMERA_CALIBRATION_CHARUCO_BOARD_DETECTOR_H