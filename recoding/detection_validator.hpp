#ifndef STEREO_CAMERA_CALIBRATION_DETECTION_VALIDATOR_H
#define STEREO_CAMERA_CALIBRATION_DETECTION_VALIDATOR_H
#include <readerwriterqueue.h>

#include <opencv2/objdetect/charuco_detector.hpp>

#include "Video_viewer.hpp"

#include "recorders/camera_worker.hpp"

namespace YACCP {
    struct VerifyTask {
        int id;
        cv::Mat frame;
    };

    class DetectionValidator {
    public:
        DetectionValidator(std::stop_source stopSource,
                           std::vector<CamData> &camDatas,
                           const cv::aruco::CharucoDetector &charucoDetector,
                           moodycamel::ReaderWriterQueue<ValidatedCornersData> &valCornersQ,
                           const std::filesystem::path &outputPath,
                           float cornerMin = .5F);


        void start();

    private:
        std::stop_source stopSource_;
        std::stop_token stopToken_;
        std::vector<CamData> &camDatas_;
        const cv::aruco::CharucoDetector charucoDetector_;
        moodycamel::ReaderWriterQueue<ValidatedCornersData> &valCornersQ_;
        const std::filesystem::path &outputPath_;
        float cornerMin_;
    };
} // YACCP

#endif //STEREO_CAMERA_CALIBRATION_DETECTION_VALIDATOR_H