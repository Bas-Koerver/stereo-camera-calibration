#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect/aruco_detector.hpp>
#include <opencv2/objdetect/charuco_detector.hpp>
#include <opencv2/highgui.hpp>

#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#include <chrono>
#include <filesystem>
#include <ranges>

#include "utility.h"

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

bool createDirs(const std::string &path) {
    std::filesystem::path p(path);
    return std::filesystem::create_directories(p);
}


int main() {
    int exitCode = 0;

    // ChArUco detector params
    int squaresX{7};
    int squaresY{5};
    float squareLength{0.0295};
    float markerLength{0.0206};
    bool refine{false};
    int fps{30};
    (void)createDirs("./data/images/basler_rgb/");

    cv::aruco::DetectorParameters detParams;
    cv::aruco::CharucoParameters charucoParams;
    if (refine) {
        charucoParams.tryRefineMarkers = true;
    }

    cv::aruco::Dictionary dictionary{cv::aruco::getPredefinedDictionary(cv::aruco::DICT_6X6_50)};
    cv::aruco::CharucoBoard board{cv::Size(squaresX, squaresY), squareLength, markerLength, dictionary};
    cv::aruco::CharucoDetector charuco(board, charucoParams, detParams);

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
        return EXIT_FAILURE;
    }

    if (camOpen) {
        cam.Open();

        GenApi::INodeMap &nodemap = cam.GetNodeMap();
        int height{static_cast<int>(Pylon::CIntegerParameter(nodemap, "Height").GetValue())};
        int width{static_cast<int>(Pylon::CIntegerParameter(nodemap, "Width").GetValue())};

        cv::Mat grayFrame;
        cv::Mat overlay{height, width, CV_8UC3, cv::Scalar(0, 0, 0)};
        double alpha = 0.2;

        auto startTime = std::chrono::system_clock::now();

        // Configure pixel format and exposure time (FPS).
        Pylon::CEnumParameter(nodemap, "PixelFormat").SetValue("BGR8");
        Pylon::CFloatParameter(nodemap, "ExposureTime").SetValue(1.0 / static_cast<double>(fps) * 1e6);

        // Enable trigger signals on exposure active.
        Pylon::CEnumParameter(nodemap, "LineSelector").SetValue("Line2");
        Pylon::CEnumParameter(nodemap, "LineMode").SetValue("Output");
        Pylon::CEnumParameter(nodemap, "LineSource").SetValue("ExposureActive");

        // Enable frame count.
        Pylon::CEnumParameter(nodemap, "CounterSelector").SetValue("Counter1");
        Pylon::CEnumParameter(nodemap, "CounterEventSource").SetValue("ExposureActive");
        Pylon::CEnumParameter(nodemap, "CounterEventActivation").SetValue("RisingEdge");
        Pylon::CCommandParameter(nodemap, "CounterReset").Execute();
        std::cout << Pylon::CEnumParameter(nodemap, "CounterStatus").GetValue();

        std::mutex frameMutex;
        cv::Mat frame;
        int frameID;

        FrameHandler frameHandler{frameMutex, frame, frameID, nodemap};
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

            img = viz.clone();
            cv::cvtColor(viz, grayFrame, cv::COLOR_BGR2GRAY);

            std::vector<int> markerIds;
            std::vector<std::vector<cv::Point2f> > markerCorners;
            std::vector<int> charucoIds;
            std::vector<cv::Point2f> charucoCorners;
            std::vector<cv::Point3f> objectPoints;
            std::vector<cv::Point2f> imagePoints;

            charuco.detectBoard(grayFrame, charucoCorners, charucoIds, markerCorners, markerIds);


            if (!markerIds.empty())
                cv::aruco::drawDetectedMarkers(viz, markerCorners, markerIds, cv::Scalar(255., 255., 0.));
            if (!charucoIds.empty())
                cv::aruco::drawDetectedCornersCharuco(viz, charucoCorners, charucoIds,
                                                      cv::Scalar(0., 255., 255.));

            std::chrono::duration<double> seconds{std::chrono::system_clock::now() - startTime};
            if (charucoCorners.size() > 3 && seconds.count() > 2.0) {
                board.matchImagePoints(charucoCorners, charucoIds, objectPoints, imagePoints);

                if (!objectPoints.empty() && !imagePoints.empty()) {
                    std::vector<cv::Point2f> hullFloat;
                    cv::convexHull(imagePoints, hullFloat);

                    std::vector<cv::Point> hull;
                    hull.reserve(hullFloat.size());
                    for (const auto &p: hullFloat) {
                        (void) hull.emplace_back(cvRound(p.x), cvRound(p.y));
                    }
                    std::vector<std::vector<cv::Point> > hulls{hull};

                    cv::fillPoly(overlay, hulls, cv::Scalar(0., 255., 255.));

                    // Save image where a board was detected.
                    (void)cv::imwrite("./data/images/basler_rgb/frame_" + std::to_string(localFrameID) + ".png", img);
                }
                startTime = std::chrono::system_clock::now();
            }

            cv::addWeighted(overlay, alpha, viz, 1.0 - alpha, 0.0, viz);

            cv::imshow("basler", viz);
            int key{cv::waitKey(1)};
            if (key == 27) {
                should_close = true;
            }
        }

        cam.StopGrabbing();
        cam.Close();
        cv::destroyAllWindows();
        (void) cam.DeregisterImageEventHandler(&frameHandler);
        Pylon::PylonTerminate();

        return exitCode;
    }
}
