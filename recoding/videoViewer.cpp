#include <metavision/sdk/core/utils/frame_composer.h>
#include <metavision/sdk/ui/utils/event_loop.h>
#include <metavision/sdk/ui/utils/window.h>
#include <numeric>

#include "VideoViewer.h"

namespace YACC {
    VideoViewer::VideoViewer(unsigned viewsHorizontal, std::vector<camData> &camDatas) : viewsHorizontal_(
            viewsHorizontal),
        camDatas_(camDatas) {
    }

    template<typename T>
    T sumVector(std::vector<T> &dimVector, int stop) {
        auto end = dimVector.begin() + stop;
        return std::reduce(dimVector.begin(), end);
    }

    std::tuple<int, int> VideoViewer::calculateRowColumnIndex(int camIndex) {
        // Determine from which row the max height needs to be calculated, based on the given camera index.
        unsigned rowIndex = std::floor(static_cast<float>(camIndex) / static_cast<float>(viewsHorizontal_));
        // Determine from which column the max width needs to be calculated, based on the given camera index.
        unsigned columnIndex = camIndex % viewsHorizontal_;

        return std::make_tuple(rowIndex, columnIndex);
    }

    std::tuple<std::vector<int>, std::vector<int> > VideoViewer::calculateBiggestDim() {
        // Calculate how many vertical rows their will be based on the amount of cameras and the set amount of cameras
        // showed horizontally.
        const unsigned viewsVertical = std::ceil(static_cast<float>(camDatas_.size()) / static_cast<float>(viewsHorizontal_));

        std::vector<int> maxWidthVec;
        std::vector<int> maxHeightVec;

        // Calculate the camera with the widest frame.
        for (unsigned columnIndex = 0; columnIndex < viewsHorizontal_; ++columnIndex) {
            int maxWidth{};
            for (unsigned i = columnIndex; i <= columnIndex + (viewsVertical - 1) * viewsHorizontal_;
                 i += viewsHorizontal_) {
                if (i + 1 > camDatas_.size()) break;
                if (camDatas_[i].width > maxWidth) maxWidth = camDatas_[i].width;
            }
            maxWidthVec.push_back(maxWidth);
        }
        // Calculate the camera with the highest frame.
        for (unsigned rowIndex = 0; rowIndex < viewsVertical; ++rowIndex) {
            int maxHeight{};
            for (unsigned i = rowIndex * viewsHorizontal_; i < (rowIndex + 1) * viewsHorizontal_; i++) {
                if (i + 1 > camDatas_.size()) break;
                if (camDatas_[i].height > maxHeight) maxHeight = camDatas_[i].height;
            }
            maxHeightVec.push_back(maxHeight);
        }

        return std::make_tuple(maxWidthVec, maxHeightVec);
    }

    void VideoViewer::start() {
        std::atomic should_stop = false;
        std::vector<unsigned> camRefs;
        auto [maxWidthVec, maxHeightVec] = calculateBiggestDim();

        // Determine the topmost x and leftmost y coordinate for a given camera.
        for (unsigned i = 0; i < camDatas_.size(); i++) {
            auto [row, column] = calculateRowColumnIndex(i);

            unsigned paddingX{(maxWidthVec[column] - camDatas_[i].width) >> 1};
            unsigned paddingY{(maxHeightVec[row] - camDatas_[i].height) >> 1};

            unsigned extraSpacingX = column * 10;
            unsigned extraSpacingY = row * 10;

            int x = sumVector(maxWidthVec, column) + paddingX + extraSpacingX;
            int y = sumVector(maxHeightVec, row) + paddingY + extraSpacingY;

            camRefs.emplace_back(frame_composer_.add_new_subimage_parameters(
                    x, y,
                    {camDatas_[i].width, camDatas_[i].height},
                    Metavision::FrameComposer::GrayToColorOptions())
            );
        }

        unsigned screenWidth = 1920;
        unsigned screenHeight = 1080;

        double scaleX{static_cast<double>(screenWidth) / frame_composer_.get_total_width()};
        double scaleY{static_cast<double>(screenHeight) / frame_composer_.get_total_height()};

        double scale{std::min(scaleX, scaleY)};

        unsigned width{static_cast<unsigned>(frame_composer_.get_total_width() * scale)};
        unsigned height{static_cast<unsigned>(frame_composer_.get_total_height() * scale)};

        Metavision::Window window("testing", width, height,
                                  Metavision::Window::RenderMode::BGR);

        window.set_keyboard_callback(
            [&should_stop](Metavision::UIKeyEvent key, int scancode, Metavision::UIAction action, int mods) {
                if (action == Metavision::UIAction::RELEASE) {
                    switch (key) {
                        case Metavision::UIKeyEvent::KEY_ESCAPE:
                        case Metavision::UIKeyEvent::KEY_Q:
                            should_stop = true;
                            break;
                    }
                }
            });


        while (!should_stop) {
            {
                std::unique_lock<std::mutex> lock{camDatas_[0].m};
                frame_composer_.update_subimage(camRefs[0], camDatas_[0].frame);
            }

            {
                std::unique_lock<std::mutex> lock{camDatas_[1].m};
                frame_composer_.update_subimage(camRefs[1], camDatas_[1].frame);
            }


            if (!frame_composer_.get_full_image().empty()) {
                window.show(frame_composer_.get_full_image());
            }

            Metavision::EventLoop::poll_and_dispatch();
        }
    }
}
