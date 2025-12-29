#define NOMINMAX

#include "basler_cam_worker.hpp"

#include <mutex>

#include <opencv2/imgproc.hpp>
#include <opencv2/core/mat.hpp>

#include <pylon/PylonIncludes.h>

#include "../job_data.hpp"
#include "../utility.hpp"
#include "../detection_validator.hpp"


class FrameHandler : public Pylon::CImageEventHandler {
public:
    FrameHandler(std::mutex &m, cv::Mat &f, int &index, GenApi::INodeMap &np)
        : frame_mutex_{m},
          frame_{f},
          id_{index},
          nodeMap_{np} {
    }

    void OnImageGrabbed(Pylon::CInstantCamera &cam, const Pylon::CGrabResultPtr &ptrGrabResult) override {
        if (!ptrGrabResult->GrabSucceeded()) {
            return;
        }

        const int width{static_cast<int>(ptrGrabResult->GetWidth())};
        const int height{static_cast<int>(ptrGrabResult->GetHeight())};
        cv::Mat tempFrameBGR;
        int tempIndex{};

        //BayerRG8 (RGGB)
        cv::Mat tempFrame(height, width, CV_8UC1, ptrGrabResult->GetBuffer());
        cv::cvtColor(tempFrame, tempFrameBGR, cv::COLOR_BayerRGGB2BGR);

        tempIndex = Pylon::CIntegerParameter(nodeMap_, "CounterValue").GetValue();

        {
            std::lock_guard<std::mutex> lock{frame_mutex_};
            tempFrameBGR.copyTo(frame_);
            id_ = tempIndex;
        }
    }

private:
    std::mutex &frame_mutex_;
    cv::Mat &frame_;
    int &id_;
    GenApi::INodeMap &nodeMap_;
};

namespace YACCP {
    void BaslerCamWorker::setPixelFormat(GenApi::INodeMap &nodeMap) {
        // TODO: Add exceptions for unsupported pixel formats.
        // TODO: auto configure pixel format based on camera capabilities.
        // Configure pixel format and exposure time (FPS).
        Pylon::CEnumParameter(nodeMap, "PixelFormat").SetValue("BayerRG8");
        Pylon::CFloatParameter(nodeMap, "ExposureTime").SetValue((1.0 / static_cast<double>(fps_)) * 1e6);
    }

    void setGainControl(GenApi::INodeMap &nodeMap) {
        Pylon::CFloatParameter(nodeMap, "AutoGainLowerLimit").SetValue(0.0);
        Pylon::CFloatParameter(nodeMap, "AutoGainUpperLimit").SetValue(48.0);
        // This controls how bright the automatic control will aim for.
        Pylon::CFloatParameter(nodeMap, "AutoTargetBrightness").SetValue(0.11);
        Pylon::CEnumParameter(nodeMap, "AutoFunctionProfile").SetValue("MinimizeExposureTime");
        Pylon::CEnumParameter(nodeMap, "GainAuto").SetValue("Continuous");
        Pylon::CEnumParameter(nodeMap, "ExposureAuto").SetValue("Off");
    }

    void setTimingInterfaces(GenApi::INodeMap &nodeMap) {
        // TODO: Add exception handling for unsupported timing interfaces.
        // Enable trigger signals on exposure active.
        Pylon::CEnumParameter(nodeMap, "LineSelector").SetValue("Line2");
        Pylon::CEnumParameter(nodeMap, "LineMode").SetValue("Output");
        Pylon::CEnumParameter(nodeMap, "LineSource").SetValue("ExposureActive");
    }

    void setCounters(GenApi::INodeMap &nodeMap) {
        // Enable frame count.
        Pylon::CEnumParameter(nodeMap, "CounterSelector").SetValue("Counter1");
        Pylon::CEnumParameter(nodeMap, "CounterEventSource").SetValue("ExposureActive");
        Pylon::CEnumParameter(nodeMap, "CounterEventActivation").SetValue("RisingEdge");
        Pylon::CCommandParameter(nodeMap, "CounterReset").Execute();
    }

    std::tuple<int, int> getDims(GenApi::INodeMap &nodeMap) {
        return {
            Pylon::CIntegerParameter(nodeMap, "Width").GetValue(),
            Pylon::CIntegerParameter(nodeMap, "Height").GetValue()
        };
    }

    std::tuple<int, int> BaslerCamWorker::getSetNodeMapParameters(GenApi::INodeMap &nodeMap) {
        setPixelFormat(nodeMap);
        // TODO: Make configurable.
        setGainControl(nodeMap);
        setTimingInterfaces(nodeMap);
        setCounters(nodeMap);
        return getDims(nodeMap);
    }

    BaslerCamWorker::BaslerCamWorker(std::stop_source stopSource,
                                     std::vector<CamData> &camDatas,
                                     const int fps,
                                     const int id,
                                     const std::filesystem::path &outputPath,
                                     std::string camId)
        : CameraWorker(stopSource, camDatas, fps, id, outputPath, std::move(camId)) {
        Pylon::PylonInitialize();
        // TODO: Handle scenarios where the camera doesn't support external triggers
    }

    void BaslerCamWorker::listAvailableSources() {
        Pylon::CTlFactory &TlFactory = Pylon::CTlFactory::GetInstance();
        Pylon::DeviceInfoList_t lstDevices;

        try {
            (void) TlFactory.EnumerateDevices(lstDevices);
            if (lstDevices.empty()) {
                std::cerr << "No available Basler cameras found \n";
                Pylon::PylonTerminate();
            }
        } catch (const GenICam::GenericException &e) {
            std::cerr << "Pylon error: " << e.GetDescription() << "\n";
            Pylon::PylonTerminate();
        }

        for (auto device: lstDevices) {
            const auto type = device.GetDeviceFactory();
            const auto id = device.GetFullName();

            std::cout << "Source type: " << type << "\n";
            std::cout << "- ID: " << id << "\n";
        }
    }

    // void BaslerRGBWorker::operator()() {
    // start();
    // }

    void BaslerCamWorker::start() {
        Pylon::CInstantCamera cam;

        try {
            Pylon::CTlFactory &TlFactory = Pylon::CTlFactory::GetInstance();
            Pylon::DeviceInfoList_t lstDevices;

            (void) TlFactory.EnumerateDevices(lstDevices);
            if (lstDevices.empty()) {
                std::cerr << "No Basler cameras found.\n";
                // Pylon::PylonTerminate();
                camData_.runtimeData.exitCode = 2;
                stopSource_.request_stop();
                return;
            } else {
                if (!camId_.empty()) {
                    cam.Attach(TlFactory.CreateDevice(Pylon::CDeviceInfo().SetFullName(camId_.data())));
                    camData_.info.camName = cam.GetDeviceInfo().GetModelName();
                    std::cout << "Using Basler device: " << camData_.info.camName << "\n";
                    camData_.runtimeData.isOpen = true;
                } else {
                    cam.Attach(TlFactory.CreateDevice(lstDevices[0]));
                    camData_.info.camName = cam.GetDeviceInfo().GetModelName();
                    std::cout << "Using Basler device: " << camData_.info.camName << "\n";
                    camData_.runtimeData.isOpen = true;
                }
            }
        } catch (const GenICam::GenericException &e) {
            std::cerr << "Pylon error: " << e.GetDescription() << "\n";
            Pylon::PylonTerminate();
            camData_.runtimeData.exitCode = EXIT_FAILURE;
            stopSource_.request_stop();
            return;
        }

        if (camData_.runtimeData.isOpen) {
            cam.Open();
            GenApi::INodeMap &nodeMap = cam.GetNodeMap();
            auto [width, height] = getSetNodeMapParameters(nodeMap);
            camData_.info.resolution.width = width;
            camData_.info.resolution.height = height;

            cv::Mat grayFrame;
            cv::Mat overlay{height, width, CV_8UC3, cv::Scalar(0, 0, 0)};

            std::mutex frameMutex;
            cv::Mat frame;
            int frameIndex;

            FrameHandler frameHandler{frameMutex, frame, frameIndex, nodeMap};
            cam.RegisterImageEventHandler(&frameHandler, Pylon::RegistrationMode_Append,
                                          Pylon::Cleanup_None);

            cam.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByInstantCamera);

            camData_.runtimeData.isRunning = cam.IsGrabbing();

            if (camData_.info.isMaster) {
                requestedFrame_ = 1 + fps_ * detectionInterval_;
                for (auto &camData: camDatas_) {
                    if (camData.info.isMaster) {
                        continue;
                    }
                    camData.runtimeData.frameRequestQ.enqueue(requestedFrame_);
                }

                while (cam.IsGrabbing() && !stopToken_.stop_requested()) {
                    cv::Mat localFrame;
                    int localFrameIndex;
                    {
                        std::unique_lock<std::mutex> lock{frameMutex};
                        if (frame.empty()) {
                            continue;
                        }
                        localFrame = frame.clone();
                        localFrameIndex = frameIndex;
                    }
                    // Frame used for visualisation
                    {
                        std::unique_lock<std::mutex> lock{camData_.runtimeData.m};
                        camData_.runtimeData.frame = localFrame.clone();
                    }

                    if (localFrameIndex >= requestedFrame_) {
                        VerifyTask frameData;
                        frameData.id = requestedFrame_;
                        frameData.frame = localFrame.clone();
                        camData_.runtimeData.frameVerifyQ.enqueue(frameData);

                        requestedFrame_ = localFrameIndex + fps_ * detectionInterval_;
                        // lastRequestedFrame = requestedFrame_;

                        for (auto &camData: camDatas_) {
                            if (camData.info.isMaster) {
                                continue;
                            }
                            camData.runtimeData.frameRequestQ.enqueue(requestedFrame_);
                        }
                    }
                }
            } else {
                throw std::logic_error("Function not yet implemented");
            }

            cam.StopGrabbing();
            cam.Close();
            (void) cam.DeregisterImageEventHandler(&frameHandler);
        } else {
            stopSource_.request_stop();
        }
    }

    BaslerCamWorker::~BaslerCamWorker() {
        Pylon::PylonTerminate();
    }
}
