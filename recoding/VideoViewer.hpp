#ifndef STEREO_CAMERA_CALIBRATION_VIDEO_VIEWER_H
#define STEREO_CAMERA_CALIBRATION_VIDEO_VIEWER_H

#include <metavision/sdk/core/utils/frame_composer.h>

namespace YACC {
    struct CamData {
        bool isOpen{false};
        bool isRunning{false};
        int exitCode;
        std::string camName;
        int width{};
        int height{};
        cv::Mat frame;
        std::mutex m;
    };

    class VideoViewer {
    public:
        VideoViewer(std::stop_source &stopSource,
                    int viewsHorizontal,
                    std::vector<CamData> &camDatas,
                    const cv::aruco::CharucoDetector &charucoDetector);

        void start();

    private:
        std::stop_source &stopSource_;
        int viewsHorizontal_ = 2;
        Metavision::FrameComposer frame_composer_;
        std::vector<CamData> &camDatas_;
        const cv::aruco::CharucoDetector &charucoDetector_;

        void processFrame(std::stop_token stopToken, CamData &camData, int camRef, std::atomic<int> &camDetectMode);

        std::tuple<std::vector<int>, std::vector<int> > calculateBiggestDim() const;

        std::tuple<int, int> calculateRowColumnIndex(int camIndex) const;
    };
} // YACC

#endif //STEREO_CAMERA_CALIBRATION_VIDEO_VIEWER_H

// std::vector<int> markerIds;
// std::vector<std::vector<cv::Point2f> > markerCorners;
// std::vector<int> charucoIds;
// std::vector<cv::Point2f> charucoCorners;
// std::vector<cv::Point3f> objectPoints;
// std::vector<cv::Point2f> imagePoints;

// std::chrono::duration<double> seconds{std::chrono::system_clock::now() - startTime};
// if (charucoCorners.size() > 3 && seconds.count() > 2.0) {
//     charucoBoard_.matchImagePoints(charucoCorners, charucoIds, objectPoints, imagePoints);
//
//     if (!objectPoints.empty() && !imagePoints.empty()) {
//         std::vector<cv::Point2f> hullFloat;
//         cv::convexHull(imagePoints, hullFloat);
//
//         std::vector<cv::Point> hull;
//         hull.reserve(hullFloat.size());
//         for (const auto &p: hullFloat) {
//             (void) hull.emplace_back(cvRound(p.x), cvRound(p.y));
//         }
//         std::vector<std::vector<cv::Point> > hulls{hull};
//
//         cv::fillPoly(overlay, hulls, cv::Scalar(0., 255., 255.));
//
//         // Save image where a board was detected.
//         (void) cv::imwrite("./data/images/basler_rgb/frame_" + std::to_string(localFrameID) + ".png",
//                            img);
//     }
//     startTime = std::chrono::system_clock::now();
// }
//
// cv::addWeighted(overlay, alpha, viz, 1.0 - alpha, 0.0, viz);
