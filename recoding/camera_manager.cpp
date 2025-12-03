#include <metavision/sdk/core/utils/frame_composer.h>
#include <opencv2/core/types.hpp>

#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "recorders/prophesee_dvs_recorder.hpp"
#include "utility.hpp"
#include "VideoViewer.hpp"
#include "recorders/basler_rgb_recorder.hpp"


int main() {
    (void) YACC::utility::createDirs("./data/images/");
    // TODO: Add config / CLI
    const int squaresX{7};
    const int squaresY{5};
    const float squareLength{0.0295};
    const float markerLength{0.0206};
    const bool refine{false};
    const int fps{30};
    const int acc{33333};
    const int numCams{2};
    const int viewsHorizontal{3};
    std::stop_source stopSource;

    std::vector<YACC::CamData> camDatas(numCams);
    std::vector<std::jthread> threads;
    cv::aruco::DetectorParameters detParams;
    cv::aruco::CharucoParameters charucoParams;
    if (refine) {
        charucoParams.tryRefineMarkers = true;
    }

    bool signalCaught{false};

    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_50)};
    cv::aruco::CharucoBoard board{cv::Size(squaresX, squaresY), squareLength, markerLength, dictionary};
    cv::aruco::CharucoDetector charucoDetector(board, charucoParams, detParams);


    YACC::PropheseeDVSWorker propheseeDVSWorker(stopSource.get_token(), camDatas[0], fps, acc, charucoDetector);
    YACC::BaslerRGBWorker baslerRGBWorker(stopSource.get_token(), camDatas[1], fps, charucoDetector);
    YACC::VideoViewer videoViewer{stopSource, viewsHorizontal, camDatas, charucoDetector};

    threads.emplace_back(&YACC::PropheseeDVSWorker::start, &propheseeDVSWorker);

    while (!camDatas[0].isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    threads.emplace_back(&YACC::BaslerRGBWorker::start, &baslerRGBWorker);

    while (!camDatas[1].isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    threads.emplace_back(&YACC::VideoViewer::start, &videoViewer);

    for (auto &thread: threads) {
        thread.join();
    }

    return 0;
}
