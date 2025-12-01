#include <metavision/sdk/core/utils/frame_composer.h>
#include <opencv2/core/types.hpp>

#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "recorders/prophesee_dvs_recorder.h"
#include "utility.h"
#include "VideoViewer.h"
#include "recorders/basler_rgb_recorder.h"


int main() {
    (void) YACC::utility::createDirs("./data/images/");
    const int squaresX{7};
    const int squaresY{5};
    const float squareLength{0.0295};
    const float markerLength{0.0206};
    const bool refine{false};
    const int fps{30};
    const int acc{33333};
    const int numCams{2};
    const int viewsHorizontal{3};

    std::vector<YACC::camData> camDatas(numCams);
    std::vector<std::thread> threads;
    cv::aruco::DetectorParameters detParams;
    cv::aruco::CharucoParameters charucoParams;
    if (refine) {
        charucoParams.tryRefineMarkers = true;
    }

    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_50)};
    cv::aruco::CharucoBoard board{cv::Size(squaresX, squaresY), squareLength, markerLength, dictionary};
    cv::aruco::CharucoDetector charucoDetector(board, charucoParams, detParams);


    YACC::PropheseeDVSWorker propheseeDVSWorker(camDatas[0], fps, acc, charucoDetector);
    // propheseeDVSWorker.listAvailableSources();

    YACC::BaslerRGBWorker baslerRGBWorker(camDatas[1], fps, board, charucoDetector);

    threads.emplace_back(&YACC::PropheseeDVSWorker::start, &propheseeDVSWorker);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    threads.emplace_back(&YACC::BaslerRGBWorker::start, &baslerRGBWorker);

    std::this_thread::sleep_for(std::chrono::seconds(5));
    YACC::VideoViewer videoViewer{viewsHorizontal, camDatas};
    videoViewer.start();

    for (auto &thread: threads) {
        thread.join();
    }

    return 0;
}
