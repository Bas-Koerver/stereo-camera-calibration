#include <metavision/sdk/core/utils/frame_composer.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>

#include "VideoViewer.hpp"

#include <numeric>
#include <vector>

#include "utility.hpp"

namespace YACCP {
    template<typename T>
    T sumVector(std::vector<T> &dimVector, int stop) {
        auto end = dimVector.begin() + stop;
        return std::reduce(dimVector.begin(), end);
    }

    VideoViewer::VideoViewer(std::stop_source &stopSource,
                             int viewsHorizontal,
                             std::vector<CamData> &camDatas,
                             const cv::aruco::CharucoDetector &charucoDetector) : stopSource_(stopSource),
        viewsHorizontal_(viewsHorizontal),
        camDatas_(camDatas),
        charucoDetector_(charucoDetector) {
    }

    void VideoViewer::processFrame(std::stop_token stopToken, CamData &camData, const int camRef,
                                   std::atomic<int> &camDetectMode) {
        while (!stopToken.stop_requested()) {
            cv::Mat localFrame;
            {
                std::unique_lock<std::mutex> lock(camData.m);
                camData.frame.copyTo(localFrame);
            }

            int mode = camDetectMode.load(std::memory_order_relaxed);
            // int mode = camDetectMode;
            if (mode == -1 || mode == camRef) {
                cv::Mat grayFrame;
                cv::cvtColor(localFrame, grayFrame, cv::COLOR_BGR2GRAY);
                auto [boardFound,
                    markerIds,
                    markerCorners,
                    charucoIds,
                    charucoCorners] = utility::findBoard(charucoDetector_, grayFrame);

                if (!markerIds.empty())
                    cv::aruco::drawDetectedMarkers(localFrame, markerCorners, markerIds);
                if (!charucoIds.empty())
                    cv::aruco::drawDetectedCornersCharuco(localFrame, charucoCorners, charucoIds,
                                                          cv::Scalar(0, 255, 0));
            }

            frame_composer_.update_subimage(camRef, localFrame);
        }
    }


    std::tuple<int, int> VideoViewer::calculateRowColumnIndex(int camIndex) const {
        // Determine from which row the max height needs to be calculated, based on the given camera index.
        int rowIndex = std::floor(static_cast<float>(camIndex) / static_cast<float>(viewsHorizontal_));
        // Determine from which column the max width needs to be calculated, based on the given camera index.
        int columnIndex = camIndex % viewsHorizontal_;

        return {rowIndex, columnIndex};
    }

    std::tuple<std::vector<int>, std::vector<int> > VideoViewer::calculateBiggestDim() const {
        // Calculate how many vertical rows their will be based on the amount of cameras and the set amount of cameras
        // showed horizontally.
        const int viewsVertical = std::ceil(
            static_cast<float>(camDatas_.size()) / static_cast<float>(viewsHorizontal_));

        std::vector<int> maxWidthVec;
        std::vector<int> maxHeightVec;

        // Calculate the camera with the widest frame.
        for (int columnIndex = 0; columnIndex < viewsHorizontal_; ++columnIndex) {
            int maxWidth{};
            for (int i = columnIndex; i <= columnIndex + (viewsVertical - 1) * viewsHorizontal_;
                 i += viewsHorizontal_) {
                if (i + 1 > camDatas_.size()) break;
                if (camDatas_[i].width > maxWidth) maxWidth = camDatas_[i].width;
            }
            maxWidthVec.push_back(maxWidth);
        }
        // Calculate the camera with the highest frame.
        for (int rowIndex = 0; rowIndex < viewsVertical; ++rowIndex) {
            int maxHeight{};
            for (int i = rowIndex * viewsHorizontal_; i < (rowIndex + 1) * viewsHorizontal_; i++) {
                if (i + 1 > camDatas_.size()) break;
                if (camDatas_[i].height > maxHeight) maxHeight = camDatas_[i].height;
            }
            maxHeightVec.push_back(maxHeight);
        }

        return {maxWidthVec, maxHeightVec};
    }

    void VideoViewer::start() {
        std::vector<int> camRefs;
        std::vector<std::jthread> threads;
        std::atomic camDetectMode = -2;

        auto [maxWidthVec, maxHeightVec] = calculateBiggestDim();

        // Determine the topmost x and leftmost y coordinate for a given camera.
        for (int i{0}; i < camDatas_.size(); i++) {
            auto [row, column] = calculateRowColumnIndex(i);

            int paddingX{(maxWidthVec[column] - camDatas_[i].width) >> 1};
            int paddingY{(maxHeightVec[row] - camDatas_[i].height) >> 1};

            int extraSpacingX = column * 10;
            int extraSpacingY = row * 10;

            int x = sumVector(maxWidthVec, column) + paddingX + extraSpacingX;
            int y = sumVector(maxHeightVec, row) + paddingY + extraSpacingY;

            camRefs.emplace_back(frame_composer_.add_new_subimage_parameters(
                    x, y,
                    {static_cast<unsigned>(camDatas_[i].width), static_cast<unsigned>(camDatas_[i].height)},
                    Metavision::FrameComposer::GrayToColorOptions())
            );
        }

        int screenWidth = 1920;
        int screenHeight = 1080;

        double scaleX{static_cast<double>(screenWidth) / frame_composer_.get_total_width()};
        double scaleY{static_cast<double>(screenHeight) / frame_composer_.get_total_height()};

        double scale{std::min(scaleX, scaleY)};

        int width{static_cast<int>(frame_composer_.get_total_width() * scale)};
        int height{static_cast<int>(frame_composer_.get_total_height() * scale)};

        Metavision::Window window("testing", width, height,
                                  Metavision::Window::RenderMode::BGR);

        window.set_keyboard_callback(
            [&camDetectMode, &camRefs, this](Metavision::UIKeyEvent key, int scancode, Metavision::UIAction action,
                                             int mods) {
                int mode;
                if (action == Metavision::UIAction::RELEASE) {
                    switch (key) {
                        case Metavision::UIKeyEvent::KEY_ESCAPE:
                        case Metavision::UIKeyEvent::KEY_Q:
                            stopSource_.request_stop();
                            break;
                        case Metavision::UIKeyEvent::KEY_A:
                            if (camDetectMode == -1) {
                                camDetectMode = -2;
                                std::cout << "Showing no detections\n";
                            } else {
                                camDetectMode = -1;
                                std::cout << "Showing detections on all cameras \n";
                            }
                            break;
                        case Metavision::UIKeyEvent::KEY_S:
                            mode = camDetectMode.load(std::memory_order_relaxed);
                            if (mode > 0) {
                                camDetectMode.store(mode - 1, std::memory_order_relaxed);
                                std::cout << "Showing detections on camera: " << camDatas_[camDetectMode].camName <<
                                        "\n";
                            }
                            break;
                        case Metavision::UIKeyEvent::KEY_D:
                            mode = camDetectMode.load(std::memory_order_relaxed);
                            if (mode < static_cast<int>(camRefs.size()) - 1 && mode >= -1) {
                                camDetectMode.store(mode + 1, std::memory_order_relaxed);
                                std::cout << "Showing detections on camera: " << camDatas_[camDetectMode].camName <<
                                        "\n";
                            }
                            break;
                    }
                }
            });

        for (int i{0}; i < static_cast<int>(camDatas_.size()); ++i) {
            threads.emplace_back(
                [this, i, &camDetectMode, &camRefs](std::stop_token st) {
                    processFrame(st, camDatas_[i], camRefs[i], camDetectMode);
                }
            );
        }

        while (!stopSource_.stop_requested()) {
            if (!frame_composer_.get_full_image().empty()) {
                // TODO: Add countdown till next capture.
                window.show(frame_composer_.get_full_image());
            }

            Metavision::EventLoop::poll_and_dispatch();
        }
    }
}
