#ifndef STEREO_CAMERA_CALIBRATION_VIDEO_VIEWER_H
#define STEREO_CAMERA_CALIBRATION_VIDEO_VIEWER_H

#include <metavision/sdk/core/utils/frame_composer.h>

namespace YACC {
    struct camData {
        std::string camId;
        unsigned width;
        unsigned height;
        cv::Mat frame;
        std::mutex m;
    };

    class VideoViewer {
    public:
        VideoViewer(unsigned viewsHorizontal, std::vector<camData> &camDatas);

        void start();

    private:
        unsigned viewsHorizontal_ = 2;
        Metavision::FrameComposer frame_composer_;
        std::vector<camData> &camDatas_;

        std::tuple<std::vector<int>, std::vector<int>> calculateBiggestDim();

        std::tuple<int, int> calculateRowColumnIndex(int camIndex);
    };
} // YACC

#endif //STEREO_CAMERA_CALIBRATION_VIDEO_VIEWER_H
