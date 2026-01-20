#ifndef YACCP_RECORDING_VIDEO_VIEWER_HPP
#define YACCP_RECORDING_VIDEO_VIEWER_HPP
#include "recorders/camera_worker.hpp"

#include <readerwriterqueue.h>
// #include <stop_token>
// #include <vector>

// #include <opencv2/objdetect/charuco_detector.hpp>

#include <metavision/sdk/core/utils/frame_composer.h>



namespace YACCP {
    struct ValidatedCornersData {
        int id;
        int camId;
        std::vector<int> charucoIds;
        std::vector<cv::Point2f> charucoCorners;
        int validatedImagePair;
        int validatedCorners;
    };

    class VideoViewer {
    public:
        VideoViewer(std::stop_source stopSource,
                    int viewsHorizontal,
                    int resolutionWidth,
                    int resolutionHeight,
                    std::vector<CamData>& camDatas,
                    const cv::aruco::CharucoDetector& charucoDetector,
                    moodycamel::ReaderWriterQueue<ValidatedCornersData>& valCornersQ,
                    const std::filesystem::path& outputPath,
                    float cornerMin = .5F);

        void start();

    private:
        std::stop_source stopSource_;
        std::stop_token stopToken_;
        int viewsHorizontal_;
        int resolutionWidth_;
        int resolutionHeight_;
        Metavision::FrameComposer frameComposer_;
        std::vector<CamData>& camDatas_;
        const cv::aruco::CharucoDetector charucoDetector_;
        moodycamel::ReaderWriterQueue<ValidatedCornersData>& valCornersQ_;
        const std::filesystem::path& outputPath_;
        float cornerMin_;

        void processFrame(std::stop_token stopToken, CamData& camData, int camRef, std::atomic<int>& camDetectMode);

        [[nodiscard]] std::tuple<std::vector<int>, std::vector<int>> calculateBiggestDims() const;

        [[nodiscard]] std::tuple<int, int> calculateRowColumnIndex(int camIndex) const;

        [[nodiscard]] std::vector<cv::Point> correctCoordinates(const ValidatedCornersData& validatedCornersData);
    };
} // YACCP

#endif //YACCP_RECORDING_VIDEO_VIEWER_HPP