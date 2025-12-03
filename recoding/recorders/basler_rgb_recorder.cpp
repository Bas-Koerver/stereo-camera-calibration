#define NOMINMAX
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <opencv2/highgui.hpp>

#include <pylon/PylonIncludes.h>

#include "basler_rgb_recorder.hpp"
#include "../utility.hpp"
#include "../VideoViewer.hpp"

#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#include <chrono>
#include <filesystem>
#include <ranges>


class FrameHandler : public Pylon::CImageEventHandler {
public:
    FrameHandler(std::mutex &m, cv::Mat &f, int &id, GenApi::INodeMap &np) : frame_mutex_{m}, frame_{f}, id_{id},
                                                                             nodeMap_{np} {
    }

    void OnImageGrabbed(Pylon::CInstantCamera &cam, const Pylon::CGrabResultPtr &ptrGrabResult) override {
        if (!ptrGrabResult->GrabSucceeded()) {
            return;
        }

        const int height{static_cast<int>(ptrGrabResult->GetHeight())};
        const int width{static_cast<int>(ptrGrabResult->GetWidth())};

        //BayerRG8
        // cv::Mat tempFrame(height, width, CV_8UC1, ptrGrabResult->GetBuffer());
        // cv::Mat tempFrameBGR;
        // cv::cvtColor(tempFrame, tempFrameBGR, cv::COLOR_BayerBG2BGR);
        int tempID{};

        cv::Mat tempFrame(height, width, CV_8UC3, ptrGrabResult->GetBuffer());
        tempID = Pylon::CIntegerParameter(nodeMap_, "CounterValue").GetValue();

        {
            std::lock_guard<std::mutex> lock{frame_mutex_};
            tempFrame.copyTo(frame_);
            id_ = tempID;
        }
    }

private:
    std::mutex &frame_mutex_;
    cv::Mat &frame_;
    int &id_;
    GenApi::INodeMap &nodeMap_;
};

namespace YACC {
    void BaslerRGBWorker::setPixelFormat(GenApi::INodeMap &nodeMap) {
        // Configure pixel format and exposure time (FPS).
        Pylon::CEnumParameter(nodeMap, "PixelFormat").SetValue("BGR8");
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

    std::tuple<int, int> BaslerRGBWorker::getSetNodeMapParameters(GenApi::INodeMap &nodeMap) {
        setPixelFormat(nodeMap);
        setGainControl(nodeMap);
        setTimingInterfaces(nodeMap);
        setCounters(nodeMap);
        return getDims(nodeMap);
    }

    BaslerRGBWorker::BaslerRGBWorker(std::stop_token stopToken,
                                     CamData &camData,
                                     int fps,
                                     const cv::aruco::CharucoDetector &charucoDetector,
                                     const std::string &camId) : stopToken_(stopToken),
                                                                 camData_(camData),
                                                                 fps_(fps),
                                                                 charucoDetector_(charucoDetector) {
        Pylon::PylonInitialize();
        camId_ = camId.c_str();
    }

    void BaslerRGBWorker::listAvailableSources() {
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

    void BaslerRGBWorker::start() {
        (void) utility::createDirs("./data/images/basler_rgb/");
        Pylon::CInstantCamera cam;

        try {
            Pylon::CTlFactory &TlFactory = Pylon::CTlFactory::GetInstance();
            Pylon::DeviceInfoList_t lstDevices;

            (void) TlFactory.EnumerateDevices(lstDevices);
            if (lstDevices.empty()) {
                std::cerr << "No Basler cameras found.\n";
                Pylon::PylonTerminate();
                camData_.exitCode = 2;
            } else {
                if (!camId_.empty()) {
                    cam.Attach(TlFactory.CreateDevice(Pylon::CDeviceInfo().SetFullName(camId_)));
                    camData_.camName = cam.GetDeviceInfo().GetModelName();
                    std::cout << "Using Basler device: " << camData_.camName << "\n";
                    camData_.isOpen = true;

                } else {
                    cam.Attach(TlFactory.CreateDevice(lstDevices[0]));
                    camData_.camName = cam.GetDeviceInfo().GetModelName();
                    std::cout << "Using Basler device: " << camData_.camName << "\n";
                    camData_.isOpen = true;
                }
            }
        } catch (const GenICam::GenericException &e) {
            std::cerr << "Pylon error: " << e.GetDescription() << "\n";
            Pylon::PylonTerminate();
            camData_.exitCode = EXIT_FAILURE;
        }

        if (camData_.isOpen) {
            cam.Open();
            GenApi::INodeMap &nodeMap = cam.GetNodeMap();
            auto [width, height] = getSetNodeMapParameters(nodeMap);
            camData_.width = width;
            camData_.height = height;

            cv::Mat grayFrame;
            cv::Mat overlay{height, width, CV_8UC3, cv::Scalar(0, 0, 0)};

            auto startTime = std::chrono::system_clock::now();

            std::mutex frameMutex;
            cv::Mat frame;
            int frameID;

            FrameHandler frameHandler{frameMutex, frame, frameID, nodeMap};
            cam.RegisterImageEventHandler(&frameHandler, Pylon::RegistrationMode_Append,
                                          Pylon::Cleanup_None);

            cam.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByInstantCamera);

            Pylon::CGrabResultPtr ptrGrabResult;

            camData_.isRunning = cam.IsGrabbing();
            while (cam.IsGrabbing() && !stopToken_.stop_requested()) {
                cv::Mat localFrame;
                int localFrameID;
                {
                    std::unique_lock<std::mutex> lock{frameMutex};
                    if (frame.empty()) {
                        continue;
                    }
                    localFrame = frame.clone();
                    localFrameID = frameID;
                }
                // Frame used for visualisation
                {
                    std::unique_lock<std::mutex> lock{camData_.m};
                    camData_.frame = localFrame.clone();
                }

                // Frame used for verification
                // TODO: Maybe run after every x frames instead of seconds.
                // std::chrono::duration<double> seconds{std::chrono::system_clock::now() - startTime};
                // if (charucoCorners.size() > 3 && seconds.count() > 2.0) {
                //
                //     startTime = std::chrono::system_clock::now();
                // }
            }

            cam.StopGrabbing();
            cam.Close();
            (void) cam.DeregisterImageEventHandler(&frameHandler);
        }
    }

    BaslerRGBWorker::~BaslerRGBWorker() {
        Pylon::PylonTerminate();
    }
}
