#define NOMINMAX
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <opencv2/highgui.hpp>

#include <pylon/PylonIncludes.h>

#include "basler_rgb_recorder.h"
#include "../utility.h"
#include "../VideoViewer.h"

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
            // tempFrameBGR.copyTo(frame_);
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
    void setPixelFormat(GenApi::INodeMap &nodeMap, const int &fps) {
        // Configure pixel format and exposure time (FPS).
        Pylon::CEnumParameter(nodeMap, "PixelFormat").SetValue("BGR8");
        Pylon::CFloatParameter(nodeMap, "ExposureTime").SetValue((1.0 / static_cast<double>(fps)) * 1e6);
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

    void setNodeMapParameters(GenApi::INodeMap &nodeMap, const int &fps) {
        setPixelFormat(nodeMap, fps);
        // setGainControl(nodeMap);
        setTimingInterfaces(nodeMap);
        setCounters(nodeMap);
    }

    BaslerRGBWorker::BaslerRGBWorker(camData &cam,
                                     int fps,
                                     const cv::aruco::CharucoBoard &charucoBoard,
                                     const cv::aruco::CharucoDetector &charuco_detector) : cam_(cam), fps_(fps),
        charucoBoard_(charucoBoard), charucoDetector_(charuco_detector) {
    }

    void BaslerRGBWorker::start() {
        cam_.width = 1920;
        cam_.height = 1200;

        int exitCode = 0;
        (void) utility::createDirs("./data/images/basler_rgb/");


        bool camOpen = false;

        Pylon::PylonInitialize();

        Pylon::CInstantCamera cam;

        try {
            Pylon::CTlFactory &TlFactory = Pylon::CTlFactory::GetInstance();
            Pylon::DeviceInfoList_t lstDevices;

            (void) TlFactory.EnumerateDevices(lstDevices);
            if (lstDevices.empty()) {
                std::cerr << "No Basler cameras found.\n";
                Pylon::PylonTerminate();
                exitCode = 2;
            } else {
                cam.Attach(TlFactory.CreateDevice(lstDevices[0]));
                std::cout << "Using device: " << cam.GetDeviceInfo().GetModelName() << "\n";
                camOpen = true;
            }
        } catch (const GenICam::GenericException &e) {
            std::cerr << "Pylon error: " << e.GetDescription() << "\n";
            Pylon::PylonTerminate();
            exitCode = EXIT_FAILURE;
        }

        if (camOpen) {
            cam.Open();
            GenApi::INodeMap &nodeMap = cam.GetNodeMap();

            setNodeMapParameters(nodeMap, fps_);

            int height{static_cast<int>(Pylon::CIntegerParameter(nodeMap, "Height").GetValue())};
            int width{static_cast<int>(Pylon::CIntegerParameter(nodeMap, "Width").GetValue())};

            cv::Mat grayFrame;
            cv::Mat overlay{height, width, CV_8UC3, cv::Scalar(0, 0, 0)};
            double alpha = 0.2;

            auto startTime = std::chrono::system_clock::now();

            std::mutex frameMutex;
            cv::Mat frame;
            int frameID;

            FrameHandler frameHandler{frameMutex, frame, frameID, nodeMap};
            cam.RegisterImageEventHandler(&frameHandler, Pylon::RegistrationMode_Append,
                                          Pylon::Cleanup_None);

            cam.StartGrabbing(Pylon::GrabStrategy_LatestImageOnly, Pylon::GrabLoop_ProvidedByInstantCamera);

            Pylon::CGrabResultPtr ptrGrabResult;
            bool should_close{false};

            while (cam.IsGrabbing() && !should_close) {
                cv::Mat viz;
                cv::Mat img;
                int localFrameID;
                {
                    std::unique_lock<std::mutex> lock{frameMutex};
                    if (frame.empty()) {
                        continue;
                    }
                    viz = frame.clone();
                    localFrameID = frameID;
                }

                // img = viz.clone();

                cv::cvtColor(viz, grayFrame, cv::COLOR_BGR2GRAY);

                std::vector<int> markerIds;
                std::vector<std::vector<cv::Point2f> > markerCorners;
                std::vector<int> charucoIds;
                std::vector<cv::Point2f> charucoCorners;
                std::vector<cv::Point3f> objectPoints;
                std::vector<cv::Point2f> imagePoints;

                charucoDetector_.detectBoard(grayFrame, charucoCorners, charucoIds, markerCorners, markerIds);


                if (!markerIds.empty())
                    cv::aruco::drawDetectedMarkers(viz, markerCorners, markerIds, cv::Scalar(255., 255., 0.));
                if (!charucoIds.empty())
                    cv::aruco::drawDetectedCornersCharuco(viz, charucoCorners, charucoIds,
                                                          cv::Scalar(0., 255., 255.));

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
                {
                    std::unique_lock<std::mutex> lock{cam_.m};
                    cam_.frame = viz.clone();
                }
                // cv::imshow("basler", viz);
                // int key{cv::waitKey(1)};
                // if (key == 27) {
                //     should_close = true;
                // }
            }

            cam.StopGrabbing();
            cam.Close();
            cv::destroyAllWindows();
            (void) cam.DeregisterImageEventHandler(&frameHandler);
            Pylon::PylonTerminate();
        }
    }
}
