#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/core/algorithms/polarity_filter_algorithm.h>
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

#include "prophesee_dvs_recorder.h"

#include "../utility.h"
#include "../VideoViewer.h"

namespace YACC {
    bool createDirs(const std::string &path) {
        std::filesystem::path p(path);
        return std::filesystem::create_directories(p);
    }

    std::string sourceTypeToString(Metavision::OnlineSourceType type) {
        switch (type) {
            case Metavision::OnlineSourceType::EMBEDDED:
                return "EMBEDDED";
            case Metavision::OnlineSourceType::USB:
                return "USB";
            case Metavision::OnlineSourceType::REMOTE:
                return "REMOTE";
            default:
                return "UNKNOWN";
        }
    }

    void configureBiases(Metavision::Device &device) {
        auto *biases = device.get_facility<::Metavision::I_LL_Biases>();
        (void) biases->set("bias_diff_off", 56);
        (void) biases->set("bias_diff_on", 56);
    }

    void configureTimingInterfaces(Metavision::Device &device) {
        auto i_trigger_in = device.get_facility<Metavision::I_TriggerIn>();
        (void) i_trigger_in->enable(Metavision::I_TriggerIn::Channel::Main);
    }

    void configureFacilities(Metavision::Camera &camera) {
        Metavision::Device &device = camera.get_device();
        configureBiases(device);
        configureTimingInterfaces(device);
    }

    PropheseeDVSWorker::PropheseeDVSWorker(camData &cam, std::uint16_t fps, std::uint16_t accumulation_time,
                                           const cv::aruco::CharucoDetector &charuco_detector, const std::string &id)
        : cam_(cam),
          fps_(fps),
          accumulationTime_(accumulation_time),
          charucoDetector_(charuco_detector), id_(id) {
    }

    void PropheseeDVSWorker::listAvailableSources() {
        Metavision::AvailableSourcesList sources{Metavision::Camera::list_online_sources()};

        if (sources.empty()) {
            std::cerr << "No available sources found \n";
            return;
        }

        for (auto source: sources) {
            const auto &type = source.first;
            const auto &ids = source.second;
            std::string typeName;

            typeName = sourceTypeToString(type);

            std::cout << "Source type: " << typeName << "\n";

            // Print connected Prophesee cameras
            for (const auto &id: ids) {
                std::cout << "- ID: " << id << "\n";
            }
        }
    }

    void PropheseeDVSWorker::start() {
        cam_.width = 1280;
        cam_.height = 720;


        (void) utility::createDirs("./data/eventfile/");
        int exitCode = 0;

        Metavision::Camera cam;

        bool camOpen = false;

        if (id_.empty()) {
            try {
                // open the first available camera
                cam = Metavision::Camera::from_first_available();
                camOpen = true;
            } catch (Metavision::CameraException &e) {
                std::cerr << e.what() << "\n";
                exitCode = 2;
            }
        } else {
            try {
                // open the first available camera
                cam = Metavision::Camera::from_serial(id_);
                camOpen = true;
            } catch (Metavision::CameraException &e) {
                std::cerr << e.what() << "\n";
                exitCode = 2;
            }
        }


        if (camOpen) {
            // TODO: add biases file loading.

            // TODO: add runtime error handling
            cv::Mat cdFrame;
            cv::Mat grayFrame;

            std::mutex cd_frame_mutex;

            Metavision::timestamp cd_frame_ts{0};

            // Configure facilities like biases en timing interfaces.
            configureFacilities(cam);

            // Set polarity filter to only include events with a positive polarity.
            Metavision::PolarityFilterAlgorithm pol_filter(1);

            const auto &geometry = cam.geometry();

            Metavision::CDFrameGenerator cdFrameGenerator{geometry.get_width(), geometry.get_height()};
            cdFrameGenerator.set_display_accumulation_time_us(accumulationTime_);

            (void) cdFrameGenerator.start(
                fps_,
                [&cd_frame_mutex, &cdFrame, &cd_frame_ts](const Metavision::timestamp &ts, const cv::Mat &frame) {
                    std::unique_lock<std::mutex> lock(cd_frame_mutex);
                    cd_frame_ts = ts;
                    frame.copyTo(cdFrame);
                });

            // Metavision::MTWindow window("MTWindow BGR", geometry.get_width(), geometry.get_height(),
            //                             Metavision::Window::RenderMode::BGR);
            //
            // window.set_keyboard_callback(
            //     [&window](Metavision::UIKeyEvent key, int, Metavision::UIAction action, int) {
            //         if (action == Metavision::UIAction::RELEASE && key == Metavision::UIKeyEvent::KEY_ESCAPE) {
            //             window.set_close_flag();
            //         }
            //     });

            (void) cam.cd().add_callback(
                [&cdFrameGenerator, &pol_filter](const Metavision::EventCD *begin, const Metavision::EventCD *end) {
                    std::vector<Metavision::EventCD> polFilterOut;
                    pol_filter.process_events(begin, end, std::back_inserter(polFilterOut));
                    begin = polFilterOut.data();
                    end = begin + polFilterOut.size();
                    cdFrameGenerator.add_events(begin, end);
                });

            auto now = std::chrono::system_clock::now();
            auto localTime = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;

            ss << std::put_time(std::localtime(&localTime), "%F_%H-%M-%S");

            (void) cam.start();
            (void) cam.start_recording("./data/eventfile/raw_recording_" + ss.str() + ".raw");
            // && !window.should_close()
            while (cam.is_running() ) {
                // Metavision::EventLoop::poll_and_dispatch();
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

                charucoDetector_.detectBoard(grayFrame, charucoCorners, charucoIds, markerCorners, markerIds);

                if (!markerIds.empty())
                    cv::aruco::drawDetectedMarkers(localFrame, markerCorners, markerIds);
                if (!charucoIds.empty())
                    cv::aruco::drawDetectedCornersCharuco(localFrame, charucoCorners, charucoIds,
                                                          cv::Scalar(0, 255, 0));

                // window.show_async(localFrame);
                {
                    std::unique_lock<std::mutex> lock{cam_.m};
                    localFrame.copyTo(cam_.frame);
                }
            }

            (void) cam.stop_recording();
            (void) cam.stop();
        }
    }

    void stop() {
    }
}
