#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/core/utils/cd_frame_generator.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/mt_window.h>
#include <metavision/sdk/ui/utils/event_loop.h>
#include <metavision/sdk/stream/camera.h>
#include <metavision/sdk/base/events/event_cd.h>

#include <metavision/hal/device/device_discovery.h>
#include <metavision/hal/device/device.h>
#include <metavision/hal/facilities/i_trigger_in.h>
#include <metavision/hal/facilities/i_ll_biases.h>

#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "utility.h"

bool createDirs(const std::string &path) {
    std::filesystem::path p(path);
    return std::filesystem::create_directories(p);
}

int main() {
    createDirs("./data/eventfile/");
    int exitCode = 0;

    // ChArUco detector params
    int squaresX = 7;
    int squaresY = 5;
    float squareLength = 0.0295;
    float markerLength = 0.0206;
    bool refine = false;
    const std::uint32_t acc = 33333;
    int fps = 30;

    cv::aruco::DetectorParameters detParams;
    cv::aruco::CharucoParameters charucoParams;
    if (refine) {
        charucoParams.tryRefineMarkers = true;
    }

    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_50)};
    cv::aruco::CharucoBoard board{cv::Size(squaresX, squaresY), squareLength, markerLength, dictionary};
    cv::aruco::CharucoDetector charuco(board, charucoParams, detParams);

    Metavision::Camera cam;

    bool camOpen = false;

    try {
        // open the first available camera
        cam = Metavision::Camera::from_first_available();
        camOpen = true;
    } catch (Metavision::CameraException &e) {
        std::cerr << e.what() << "\n";
        exitCode = 2;
    }
    if (camOpen) {
        // TODO: add serial handling.

        // TODO: add biases file loading.

        // TODO: add runtime error handling
        cv::Mat cdFrame;
        cv::Mat grayFrame;

        std::mutex cd_frame_generator_mutex;
        std::mutex cd_frame_mutex;

        Metavision::timestamp cd_frame_ts{0};

        // Configure biases for less noise
        auto &device = cam.get_device();
        auto *biases = device.get_facility<::Metavision::I_LL_Biases>();
        (void) biases->set("bias_diff_off", 40);
        (void) biases->set("bias_diff_on", 40);

        // Enable trigger in port
        auto i_trigger_in = cam.get_device().get_facility<Metavision::I_TriggerIn>();
        (void) i_trigger_in->enable(Metavision::I_TriggerIn::Channel::Main);

        // TODO: change this to only save cam geometry as a reference.
        const auto &geometry = cam.geometry();

        Metavision::CDFrameGenerator cdFrameGenerator{geometry.get_width(), geometry.get_height()};
        cdFrameGenerator.set_display_accumulation_time_us(acc);

        (void) cdFrameGenerator.start(
            static_cast<uint16_t>(fps),
            [&cd_frame_mutex, &cdFrame, &cd_frame_ts](const Metavision::timestamp &ts, const cv::Mat &frame) {
                std::unique_lock<std::mutex> lock(cd_frame_mutex);
                cd_frame_ts = ts;
                frame.copyTo(cdFrame);
            });

        Metavision::MTWindow window("MTWindow BGR", geometry.get_width(), geometry.get_height(),
                                    Metavision::Window::RenderMode::BGR);

        window.set_keyboard_callback(
            [&window](Metavision::UIKeyEvent key, int, Metavision::UIAction action, int) {
                if (action == Metavision::UIAction::RELEASE && key == Metavision::UIKeyEvent::KEY_ESCAPE) {
                    window.set_close_flag();
                }
            });

        (void) cam.cd().add_callback(
            [&cd_frame_generator_mutex, &cdFrameGenerator
            ](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
                // frame_gen.process_events(begin, end);
                std::unique_lock<std::mutex> lock(cd_frame_generator_mutex);
                cdFrameGenerator.add_events(begin, end);
            });

        auto now = std::chrono::system_clock::now();
        auto localTime = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;

        ss << std::put_time(std::localtime(&localTime), "%F_%H-%M-%S");

        (void) cam.start();
        (void) cam.start_recording("./data/eventfile/raw_recording_" + ss.str() + ".raw");

        while (cam.is_running() && !window.should_close()) {
            Metavision::EventLoop::poll_and_dispatch();
            cv::Mat localFrame;
            {
                std::unique_lock<std::mutex> lock(cd_frame_mutex);
                if (cdFrame.empty()) {
                    continue;
                }
                localFrame = cdFrame.clone();
            }

            cv::cvtColor(localFrame, grayFrame, cv::COLOR_BGR2GRAY);

            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f> > markerCorners;
            std::vector<int> charucoIds;
            std::vector<cv::Point2f> charucoCorners;

            charuco.detectBoard(grayFrame, charucoCorners, charucoIds, markerCorners, markerIds);

            if (!markerIds.empty())
                cv::aruco::drawDetectedMarkers(localFrame, markerCorners, markerIds);
            if (!charucoIds.empty())
                cv::aruco::drawDetectedCornersCharuco(localFrame, charucoCorners, charucoIds, cv::Scalar(0, 255, 0));

            window.show_async(localFrame);
        }

        (void) cam.stop_recording();
        (void) cam.stop();
    }
    return exitCode;
}
