#include <metavision/sdk/core/utils/frame_composer.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>

#include "video_viewer.hpp"

#include <fstream>

#include <nlohmann/json.hpp>
#include <numeric>
#include <vector>

#include "job_data.hpp"
#include "utility.hpp"

cv::Scalar getColourGradient(int index, int maxIndex) {
    const double ratio{255.0 / (static_cast<double>(maxIndex) / 2.0)};
    const int red = std::min(255, static_cast<int>(std::round(2 * 255 - ratio * index)));
    const int green = std::min(255, static_cast<int>(std::round(ratio * index)));
    return cv::Scalar(46., green, red);
}

namespace YACCP {
    template<typename T>
    T sumVector(std::vector<T> &dimVector, int stop) {
        auto end = dimVector.begin() + stop;
        return std::reduce(dimVector.begin(), end);
    }

    VideoViewer::VideoViewer(std::stop_source stopSource,
                             int viewsHorizontal,
                             int resolutionWidth,
                             int resolutionHeight,
                             std::vector<CamData> &camDatas,
                             const cv::aruco::CharucoDetector &charucoDetector,
                             moodycamel::ReaderWriterQueue<ValidatedCornersData> &valCornersQ,
                             const std::filesystem::path &outputPath,
                             float cornerMin)
        : stopSource_(stopSource),
          stopToken_(stopSource.get_token()),
          viewsHorizontal_(viewsHorizontal),
          resolutionWidth_(resolutionWidth),
          resolutionHeight_(resolutionHeight),
          camDatas_(camDatas),
          charucoDetector_(charucoDetector),
          valCornersQ_(valCornersQ),
          outputPath_(outputPath),
          cornerMin_(cornerMin) {
    }

    void VideoViewer::processFrame(std::stop_token stopToken,
                                   CamData &camData,
                                   const int camRef,
                                   std::atomic<int> &camDetectMode) {
        while (!stopToken.stop_requested()) {
            cv::Mat localFrame;
            {
                std::unique_lock<std::mutex> lock(camData.runtimeData.m);
                camData.runtimeData.frame.copyTo(localFrame);
            }

            int mode = camDetectMode.load(std::memory_order_relaxed);
            // int mode = camDetectMode;
            if (mode == -1 || mode == camRef) {
                cv::Mat grayFrame;
                cv::cvtColor(localFrame, grayFrame, cv::COLOR_BGR2GRAY);

                CharucoResults charucoResults{Utility::findBoard(charucoDetector_, grayFrame, 0)};

                if (!charucoResults.markerIds.empty())
                    cv::aruco::drawDetectedMarkers(localFrame, charucoResults.markerCorners, charucoResults.markerIds);
                if (!charucoResults.charucoIds.empty())
                    cv::aruco::drawDetectedCornersCharuco(localFrame, charucoResults.charucoCorners,
                                                          charucoResults.charucoIds,
                                                          cv::Scalar(0, 255, 0));
            }

            frameComposer_.update_subimage(camRef, localFrame);
        }
    }

    std::tuple<std::vector<int>, std::vector<int> > VideoViewer::calculateBiggestDims() const {
        // Calculate how many vertical rows their will be based on the amount of cameras and the set amount of cameras
        // showed horizontally.
        const int viewsVertical = static_cast<const int>(std::ceil(
            static_cast<float>(camDatas_.size()) / static_cast<float>(viewsHorizontal_)));

        std::vector<int> maxWidthVec;
        std::vector<int> maxHeightVec;

        // Calculate the camera with the widest frame.
        for (int columnIndex = 0; columnIndex < viewsHorizontal_; ++columnIndex) {
            int maxWidth{};
            for (int i = columnIndex; i <= columnIndex + (viewsVertical - 1) * viewsHorizontal_;
                 i += viewsHorizontal_) {
                if (i + 1 > camDatas_.size()) break;
                if (camDatas_[i].info.resolution.width > maxWidth) maxWidth = camDatas_[i].info.resolution.width;
            }
            maxWidthVec.push_back(maxWidth);
        }
        // Calculate the camera with the highest frame.
        for (int rowIndex = 0; rowIndex < viewsVertical; ++rowIndex) {
            int maxHeight{};
            for (int i = rowIndex * viewsHorizontal_; i < (rowIndex + 1) * viewsHorizontal_; ++i) {
                if (i + 1 > camDatas_.size()) break;
                if (camDatas_[i].info.resolution.height > maxHeight) maxHeight = camDatas_[i].info.resolution.height;
            }
            maxHeightVec.push_back(maxHeight);
        }

        return {maxWidthVec, maxHeightVec};
    }

    std::tuple<int, int> VideoViewer::calculateRowColumnIndex(int camIndex) const {
        // Determine from which row the max height needs to be calculated, based on the given camera index.
        int rowIndex = static_cast<int>(
            std::floor(static_cast<float>(camIndex) / static_cast<float>(viewsHorizontal_)));
        // Determine from which column the max width needs to be calculated, based on the given camera index.
        int columnIndex = camIndex % viewsHorizontal_;

        return {rowIndex, columnIndex};
    }

    std::vector<cv::Point> VideoViewer::correctCoordinates(const ValidatedCornersData &validatedCornersData) {
        cv::Point2f offset{
            static_cast<float>(camDatas_[validatedCornersData.camId].info.camViewData.windowX),
            static_cast<float>(camDatas_[validatedCornersData.camId].info.camViewData.windowY)
        };
        std::vector<cv::Point> correctedCorners;

        for (const auto &corner: validatedCornersData.charucoCorners) {
            correctedCorners.emplace_back(static_cast<cv::Point>(corner + offset));
        }
        return correctedCorners;
    }

    void VideoViewer::start() {
        std::vector<int> camRefs;
        std::vector<std::jthread> threads;
        std::vector<std::vector<cv::Point> > pts;
        std::atomic camDetectMode = -2;
        std::atomic detectLayerMode = true;
        std::atomic detectLayerClean = false;
        auto validatedImagePairs{0};
        auto validatedCorners{0};
        cv::Scalar textColour;

        auto [maxWidthVec, maxHeightVec] = calculateBiggestDims();

        // Determine the topmost x and leftmost y coordinate for a given camera.
        // Create a subimage in the frame composer for each camera.
        for (auto i{0}; i < camDatas_.size(); ++i) {
            auto [row, column] = calculateRowColumnIndex(i);

            int paddingX{(maxWidthVec[column] - camDatas_[i].info.resolution.width) >> 1};
            int paddingY{(maxHeightVec[row] - camDatas_[i].info.resolution.height) >> 1};

            int extraSpacingX = column * 10;
            int extraSpacingY = row * 10;

            int x = sumVector(maxWidthVec, column) + paddingX + extraSpacingX;
            int y = sumVector(maxHeightVec, row) + paddingY + extraSpacingY;

            camDatas_[i].info.camViewData.windowX = x;
            camDatas_[i].info.camViewData.windowY = y;

            camRefs.emplace_back(frameComposer_.add_new_subimage_parameters(
                    x, y,
                    {
                        static_cast<unsigned>(camDatas_[i].info.resolution.width),
                        static_cast<unsigned>(camDatas_[i].info.resolution.height)
                    },
                    Metavision::FrameComposer::GrayToColorOptions())
            );
        }






        double scaleX{static_cast<double>(resolutionWidth_) / frameComposer_.get_total_width()};
        double scaleY{static_cast<double>(resolutionHeight_) / frameComposer_.get_total_height()};
        double scale{std::min(scaleX, scaleY)};

        int width{(frameComposer_.get_total_width())};
        int height{(frameComposer_.get_total_height())};
        cv::Mat overlay = cv::Mat::zeros(height, width, CV_8UC3);
        cv::Mat mask = cv::Mat::zeros(height, width, CV_8UC1);
        cv::Mat display{height, width, CV_8UC3};

        Metavision::Window window("Recording board detections", width * scale, height * scale,
                                  Metavision::Window::RenderMode::BGR);

        window.set_keyboard_callback(
            [
                this,
                &camDetectMode,
                &camRefs,
                &detectLayerMode,
                &detectLayerClean
            ](Metavision::UIKeyEvent key, int scancode, Metavision::UIAction action, int mods) {
                int mode;
                bool layerClean;
                bool layerMode;
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
                                std::cout << "Showing detections on camera: " << camDatas_[camDetectMode].info.camName
                                        <<
                                        "\n";
                            }
                            break;
                        case Metavision::UIKeyEvent::KEY_D:
                            mode = camDetectMode.load(std::memory_order_relaxed);
                            if (mode < static_cast<int>(camRefs.size()) - 1 && mode >= -1) {
                                camDetectMode.store(mode + 1, std::memory_order_relaxed);
                                std::cout << "Showing detections on camera: " << camDatas_[camDetectMode].info.camName
                                        <<
                                        "\n";
                            }
                            break;
                        case Metavision::UIKeyEvent::KEY_L:
                            layerClean = detectLayerClean.load(std::memory_order_relaxed);
                            if (!layerClean) {
                                detectLayerClean.store(true, std::memory_order_relaxed);
                                std::cout << "Cleaning detection overlay\n";
                            }
                            break;
                        case Metavision::UIKeyEvent::KEY_Z:
                            layerMode = detectLayerMode.load(std::memory_order_relaxed);
                            if (!layerMode) {
                                detectLayerMode.store(true, std::memory_order_relaxed);
                                std::cout << "Enabling detection overlay mode\n";
                            } else {
                                detectLayerMode.store(false, std::memory_order_relaxed);
                                std::cout << "Disabling detection overlay mode\n";
                            }
                    }
                }
            });

        for (auto i{0}; i < static_cast<int>(camDatas_.size()); ++i) {
            threads.emplace_back(
                [this, i, &camDetectMode, &camRefs](std::stop_token st) {
                    processFrame(st, camDatas_[i], camRefs[i], camDetectMode);
                }
            );
        }


        while (!stopToken_.stop_requested()) {
            bool layerClean{detectLayerClean.load(std::memory_order_relaxed)};
            bool layerMode{detectLayerMode.load(std::memory_order_relaxed)};
            cv::Mat frame{frameComposer_.get_full_image()};
            if (frame.empty()) {
                continue;
            }
            frame.copyTo(display);

            ValidatedCornersData validatedCornersData;
            if (valCornersQ_.try_dequeue(validatedCornersData)) {
                // Update the validated counts.
                validatedImagePairs = validatedCornersData.validatedImagePair;
                validatedCorners = validatedCornersData.validatedCorners;

                auto correctedCorners = correctCoordinates(validatedCornersData);
                pts.clear();
                pts.emplace_back(correctedCorners);
                cv::polylines(overlay, pts, false, cv::Scalar{0., 255., 0.}, 2);
                cv::polylines(mask, pts, false, cv::Scalar{255.}, 2);
            }
            if (layerClean) {
                overlay.setTo(cv::Scalar{0., 0., 0.});
                mask.setTo(cv::Scalar{0.});
                detectLayerClean.store(false, std::memory_order_relaxed);
            }
            if (layerMode) {
                overlay.copyTo(display, mask);
            }

            textColour = getColourGradient(validatedCorners, 960);

            cv::putText(display,
                        "Validated image pairs: " + std::to_string(validatedImagePairs),
                        cv::Point(10, height - 60),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.8,
                        textColour,
                        2);

            cv::putText(display,
                        "Validated corners: " + std::to_string(validatedCorners),
                        cv::Point(10, height - 30),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.8,
                        textColour,
                        2);

            // TODO: Add verified image count and amount of verified points/corners.
            window.show(display);
            Metavision::EventLoop::poll_and_dispatch();
        }
    }
}
