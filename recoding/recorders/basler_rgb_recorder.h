#ifndef STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#include <opencv2/objdetect/charuco_detector.hpp>
#include "../VideoViewer.h"


namespace YACC {
    class BaslerRGBWorker {
    public:
        BaslerRGBWorker(camData &cam,
                        int fps,
                        const cv::aruco::CharucoBoard &charucoBoard,
                        const cv::aruco::CharucoDetector &charuco_detector);

        void start();

    private:
        camData &cam_;
        const int fps_;
        const cv::aruco::CharucoBoard charucoBoard_;
        const cv::aruco::CharucoDetector charucoDetector_;
    };
}

#endif //STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
