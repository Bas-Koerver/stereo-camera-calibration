#include <metavision/sdk/core/algorithms/polarity_filter_algorithm.h>
#include <metavision/sdk/core/utils/cd_frame_generator.h>
#include <metavision/sdk/core/algorithms/on_demand_frame_generation_algorithm.h>
#include <metavision/sdk/stream/camera.h>

#include <metavision/hal/facilities/i_hw_identification.h>
#include <metavision/hal/facilities/i_trigger_in.h>
#include <metavision/hal/facilities/i_ll_biases.h>

#include "prophesee_cam_worker.hpp"

#include <filesystem>
#include <iostream>
#include <string>


#include "../Utility.hpp"
#include "camera_worker.hpp"
#include "../detection_validator.hpp"

namespace YACCP {
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
        // TODO: Handle scenarios where the camera doesn't support external triggers
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

    PropheseeCamWorker::PropheseeCamWorker(std::stop_source stopSource,
                                           std::vector<CamData> &camDatas,
                                           const int fps,
                                           const int id,
                                           const int accumulation_time,
                                           const int fallingEdgePolarity,
                                           const std::filesystem::path &outputPath,
                                           const std::string camId)
        : CameraWorker(stopSource, camDatas, fps, id, outputPath, std::move(camId)),
          accumulationTime_(accumulation_time),
          fallingEdgePolarity_(fallingEdgePolarity) {
    }

    void PropheseeCamWorker::listAvailableSources() {
        Metavision::AvailableSourcesList sources{Metavision::Camera::list_online_sources()};

        if (sources.empty()) {
            std::cerr << "No available Prophesee cameras found \n";
            return;
        }

        for (const auto &source: sources) {
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


    void PropheseeCamWorker::start() {
        Metavision::Camera cam;

        if (camId_.empty()) {
            try {
                // open the first available camera
                cam = Metavision::Camera::from_first_available();
                camData_.isOpen = true;
            } catch (Metavision::CameraException &e) {
                std::cerr << e.what() << "\n";
                camData_.exitCode = 2;
                stopSource_.request_stop();
                return;
            }
        } else {
            try {
                // open the first available camera
                cam = Metavision::Camera::from_serial(camId_);
                camData_.isOpen = true;
            } catch (Metavision::CameraException &e) {
                std::cerr << e.what() << "\n";
                camData_.exitCode = 2;
                stopSource_.request_stop();
                return;
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
            bool requestNew{true};

            // Configure facilities like biases en timing interfaces.
            configureFacilities(cam);

            Metavision::CDFrameGenerator cdFrameGenerator{geometry.get_width(), geometry.get_height()};
            cdFrameGenerator.set_display_accumulation_time_us(accumulationTime_);


            Metavision::OnDemandFrameGenerationAlgorithm onDemandFrameGenerator{
                geometry.get_width(),
                geometry.get_height(),
                static_cast<uint32_t>(std::round(1e6 / static_cast<double>(fps_)))
            };


            (void) cdFrameGenerator.start(
                fps_,
                [&cd_frame_mutex, &cdFrame, &cd_frame_ts](const Metavision::timestamp &ts, const cv::Mat &frame) {
                    std::unique_lock<std::mutex> lock(cd_frame_mutex);
                    cd_frame_ts = ts;
                    frame.copyTo(cdFrame);
                });

            (void) cam.ext_trigger().add_callback(
                [this, &onDemandFrameGenerator, &requestNew](const Metavision::EventExtTrigger *begin,
                                                             const Metavision::EventExtTrigger *end) {
                    if (requestedFrame_ < 0) {
                        (void) camData_.frameRequestQ.try_dequeue(requestedFrame_);
                    }

                    for (auto ev = begin; ev != end; ++ev) {
                        // Check for falling edge trigger and increment frame index.
                        if (ev->p == fallingEdgePolarity_) {
                            // Handle falling edge trigger event
                            ++frameIndex_;
                        }

                        // If a frame was requested and the current frame index matches or
                        // is larger than the requested frame, generate and enqueue it.
                        if (frameIndex_ >= requestedFrame_ && requestedFrame_ >= 0) {
                            VerifyTask task;
                            task.id = requestedFrame_;
                            onDemandFrameGenerator.generate(ev->t, task.frame);
                            camData_.frameVerifyQ.enqueue(task);

                            requestedFrame_ = -1;
                        }
                    }
                });

            (void) cam.cd().add_callback(
                [&pol_filter, &cdFrameGenerator, &onDemandFrameGenerator](
            const Metavision::EventCD *begin,
            const Metavision::EventCD *end) {
                    std::vector<Metavision::EventCD> polFilterOut;
                    pol_filter.process_events(begin, end, std::back_inserter(polFilterOut));
                    begin = polFilterOut.data();
                    end = begin + polFilterOut.size();

                    cdFrameGenerator.add_events(begin, end);
                    onDemandFrameGenerator.process_events(begin, end);
                });

            // TODO: Make event file recording optional.
            (void) cam.start();
            (void) cam.start_recording(outputPath_ / "event_file.raw");

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
        } else {
            stopSource_.request_stop();
        }
    }
}