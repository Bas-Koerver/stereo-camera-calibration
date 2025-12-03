#ifndef STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#define STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
#include <opencv2/objdetect/charuco_detector.hpp>
#include "../VideoViewer.hpp"
#include <pylon/PylonIncludes.h>

namespace YACC {
    class BaslerRGBWorker {
    public:
        BaslerRGBWorker(std::stop_token stopToken,
                        CamData &camData,
                        int fps,
                        const cv::aruco::CharucoDetector &charucoDetector,
                        const std::string &camId = {});

        void listAvailableSources();

        void start();

        ~BaslerRGBWorker();

    private:
        std::stop_token stopToken_;
        CamData &camData_;
        const int fps_;
        const cv::aruco::CharucoDetector charucoDetector_;
        Pylon::String_t camId_;

        void setPixelFormat(GenApi::INodeMap &nodeMap);

        std::tuple<int, int> getSetNodeMapParameters(GenApi::INodeMap &nodeMap);
    };
}

#endif //STEREO_CAMERA_CALIBRATION_BASLER_RGB_RECORDER_H
