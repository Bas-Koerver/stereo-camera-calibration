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
#include <metavision/hal/facilities/i_hw_identification.h>


#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>

#include "prophesee_dvs_recorder.hpp"

#include "../utility.hpp"
#include "../VideoViewer.hpp"

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

    void printCurrentDevice(Metavision::Camera &cam, CamData &camData) {
        auto &device = cam.get_device();

        // Facility that gives hardware info
        if (auto *hw_id = device.get_facility<Metavision::I_HW_Identification>()) {
            for (const auto &kv: hw_id->get_system_info()) {
                if (kv.first == "device0 name") {
                    std::cout << "Using Prophesee device: " << kv.second << '\n';
                    camData.camName = kv.second;
                }
            }
        } else {
            std::cout << "Using Prophesee device (no HW identification facility)\n";
        }
    }

    PropheseeDVSWorker::PropheseeDVSWorker(std::stop_token stopToken,
                                           CamData &camData,
                                           std::uint16_t fps,
                                           std::uint16_t accumulation_time,
                                           const cv::aruco::CharucoDetector &charucoDetector,
                                           const std::string &camId) : stopToken_(stopToken),
                                                                       camData_(camData),
                                                                       fps_(fps),
                                                                       accumulationTime_(accumulation_time),
                                                                       charucoDetector_(charucoDetector),
                                                                       camId_(camId) {
    }

    void PropheseeDVSWorker::listAvailableSources() {
        Metavision::AvailableSourcesList sources{Metavision::Camera::list_online_sources()};

        if (sources.empty()) {
            std::cerr << "No available Prophesee cameras found \n";
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
        (void) utility::createDirs("./data/eventfile/");

        Metavision::Camera cam;

        if (camId_.empty()) {
            try {
                // open the first available camera
                cam = Metavision::Camera::from_first_available();
                camData_.isOpen = true;
            } catch (Metavision::CameraException &e) {
                std::cerr << e.what() << "\n";
                camData_.exitCode = 2;
            }
        } else {
            try {
                // open the first available camera
                cam = Metavision::Camera::from_serial(camId_);
                camData_.isOpen = true;
            } catch (Metavision::CameraException &e) {
                std::cerr << e.what() << "\n";
                camData_.exitCode = 2;
            }
        }

        printCurrentDevice(cam, camData_);

        if (camData_.isOpen) {
            // TODO: add biases file loading.

            // TODO: add runtime error handling
            cv::Mat cdFrame;
            cv::Mat grayFrame;
            std::mutex cd_frame_mutex;
            Metavision::timestamp cd_frame_ts{0};
            // Set polarity filter to only include events with a positive polarity.
            Metavision::PolarityFilterAlgorithm pol_filter{1};
            const auto &geometry = cam.geometry();
            camData_.width = geometry.get_width();
            camData_.height = geometry.get_height();

            // Configure facilities like biases en timing interfaces.
            configureFacilities(cam);

            Metavision::CDFrameGenerator cdFrameGenerator{geometry.get_width(), geometry.get_height()};
            cdFrameGenerator.set_display_accumulation_time_us(accumulationTime_);

            (void) cdFrameGenerator.start(
                fps_,
                [&cd_frame_mutex, &cdFrame, &cd_frame_ts](const Metavision::timestamp &ts, const cv::Mat &frame) {
                    std::unique_lock<std::mutex> lock(cd_frame_mutex);
                    cd_frame_ts = ts;
                    frame.copyTo(cdFrame);
                });

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

            camData_.isRunning = cam.is_running();
            while (cam.is_running() && !stopToken_.stop_requested()) {
                cv::Mat localFrame;
                {
                    std::unique_lock<std::mutex> lock(cd_frame_mutex);
                    if (cdFrame.empty()) {
                        continue;
                    }
                    localFrame = cdFrame.clone();
                }
                // Frame used for visualisation
                {
                    std::unique_lock<std::mutex> lock{camData_.m};
                    camData_.frame = localFrame.clone();
                }
            }

            (void) cam.stop_recording();
            (void) cam.stop();
        }
    }

    void stop() {
    }
}
