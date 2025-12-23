//
// Created by Bas_K on 2025-12-20.
//

#include "Image_validator.hpp"

#include <fstream>
#include <iostream>
#include <signal.h>

#include <metavision/sdk/core/utils/frame_composer.h>
#include <metavision/sdk/ui/utils/event_loop.h>
#include <metavision/sdk/ui/utils/window.h>

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

namespace YACCP {
    void ImageValidator::updateSubimages(Metavision::FrameComposer &frameComposer,
                                         const std::vector<std::filesystem::path> &files,
                                         const std::vector<std::filesystem::path> &cams,
                                         const std::vector<int> &camRefs) {
        for (auto i{0}; i < cams.size(); ++i) {
            frameComposer.update_subimage(
                camRefs[i],
                cv::imread(
                    (jobPath_ / "images/raw" / cams[i] / files[currentFileIndex_]).string()));
        }
    }

    ImageValidator::ImageValidator(int resolutionWidth,
                                   int resolutionHeight,
                                   const std::filesystem::path &dataPath)
        : resolutionWidth_(resolutionWidth),
          resolutionHeight_(resolutionHeight),
          dataPath_(dataPath) {
    }

    void ImageValidator::listJobs() {
        std::cout << "Available jobs to validate: \n";
        for (auto const &entry: std::filesystem::directory_iterator(dataPath_)) {
            if (!entry.is_directory()) continue;
            std::cout << entry.path().filename() << "\n";
        }
    }

    void ImageValidator::validateImages(const std::string &jobName) {
        jobPath_ = dataPath_ / jobName;
        if (!exists(jobPath_)) {
            std::cerr << "Job: " << jobName << " does not exist in the given path: " << dataPath_ << "\n";
            return;
        }

        std::vector<std::filesystem::path> cams;
        std::vector<std::filesystem::path> files;
        std::vector<int> camRefs;

        for (auto const &entry: std::filesystem::directory_iterator(jobPath_ / "images/raw")) {
            if (!entry.is_directory()) continue;
            cams.push_back(entry.path().filename());
        }
        for (auto const &entry: std::filesystem::directory_iterator(jobPath_ / "images/raw" / cams[0])) {
            if (!entry.is_regular_file()) continue;
            files.push_back(entry.path().filename());
        }

        if (!exists(jobPath_ / "camera_data.json")) {
            std::cerr << "No camera data was found.\n Stopping! \n";
            return;
        }

        std::ifstream file{jobPath_ / "camera_data.json"};
        nlohmann::json jsonObj{nlohmann::json::parse(file)};

        Metavision::FrameComposer frameComposer;
        for (auto i{0}; i < cams.size(); ++i) {
            camRefs.emplace_back(
                frameComposer.add_new_subimage_parameters(
                    jsonObj["cam_" + std::to_string(i)]["windowX"],
                    jsonObj["cam_" + std::to_string(i)]["windowY"],
                    {
                        jsonObj["cam_" + std::to_string(i)]["width"],
                        jsonObj["cam_" + std::to_string(i)]["height"]
                    },
                    Metavision::FrameComposer::GrayToColorOptions()
                )
            );
        }

        double scaleX{static_cast<double>(resolutionWidth_) / frameComposer.get_total_width()};
        double scaleY{static_cast<double>(resolutionHeight_) / frameComposer.get_total_height()};
        double scale{std::min(scaleX, scaleY)};

        int width{(frameComposer.get_total_width())};
        int height{(frameComposer.get_total_height())};
        cv::Mat display{height, width, CV_8UC3};

        // Close the window when receiving a should close flag.
        {
            cv::Scalar textColour;
            Metavision::Window window("Validating recorded detections", width * scale, height * scale,
                                      Metavision::Window::RenderMode::BGR);

            window.set_keyboard_callback(
                [this,
                    &window,
                    &cams,
                    &frameComposer,
                    &files,
                    &camRefs](Metavision::UIKeyEvent key,
                                       int scancode,
                                       Metavision::UIAction action,
                                       int mods) {
                    if (action == Metavision::UIAction::RELEASE) {
                        switch (key) {
                            case Metavision::UIKeyEvent::KEY_ESCAPE:
                            case Metavision::UIKeyEvent::KEY_Q:
                                window.set_close_flag();
                                break;
                            case Metavision::UIKeyEvent::KEY_D:
                                // Toggle whether an image should be marked as valid or not.
                                if (std::ranges::find(indexesToDiscard_, currentFileIndex_) != indexesToDiscard_.end()) {
                                    std::erase(indexesToDiscard_, currentFileIndex_);
                                } else {
                                    indexesToDiscard_.emplace_back(currentFileIndex_);
                                }
                                break;
                            case Metavision::UIKeyEvent::KEY_LEFT:
                                if (currentFileIndex_ <= 0) {
                                    std::cout << "Reached the beginning of the image set.\n Looping to end.\n\n";
                                    currentFileIndex_ = files.size() - 1;
                                } else {
                                    currentFileIndex_--;
                                }
                                // Go to previous image.
                                updateSubimages(frameComposer, files, cams, camRefs);
                                break;
                            case Metavision::UIKeyEvent::KEY_RIGHT:
                                if (currentFileIndex_ >= files.size() - 1) {
                                    std::cout << "Reached the end of the image set.\n Looping back to start.\n\n";
                                    currentFileIndex_ = 0;
                                } else {
                                    currentFileIndex_++;
                                }

                                // Go to next image.
                                updateSubimages(frameComposer, files, cams, camRefs);
                                break;
                        }
                    }
                }
            );

            updateSubimages(frameComposer, files, cams, camRefs);

            while (!window.should_close()) {
                cv::Mat frame{frameComposer.get_full_image()};
                if (frame.empty()) {
                    continue;
                }
                frame.copyTo(display);

                if (std::ranges::find(indexesToDiscard_, currentFileIndex_) != indexesToDiscard_.end()) {
                    // Red for discard.
                    textColour = cv::Scalar(0, 0, 255);
                } else {
                    // Green for keep.
                    textColour = cv::Scalar(0, 255, 0);
                }

                cv::putText(display,
                            files[currentFileIndex_].filename().string(),
                            cv::Point(10, height - 60),
                            cv::FONT_HERSHEY_SIMPLEX,
                            0.8,
                            textColour,
                            2);

                window.show(display);
                Metavision::EventLoop::poll_and_dispatch(20);;
            }
        }

        if (std::filesystem::exists(jobPath_ / "images/verified")) {
            std::cout << "Verified directory already present, do you want to overwrite it? (y/n): ";
            char response;
            bool keepAsking{true};

            while (keepAsking) {
                std::cin >> response;
                response = std::tolower(response);

                if (response != 'n' && response != 'y') {
                    std::cout << "Please enter 'y' or 'n': ";
                } else {
                    keepAsking = false;
                }
            }

            if (response == 'y') {
                std::filesystem::remove_all(jobPath_ / "images/verified");
            } else {
                std::cout << "Aborting verification process to avoid overwriting existing data.\n";
                return;
            }
        }

        std::filesystem::create_directories(jobPath_ / "images/verified");

        for (auto i{0}; i < cams.size(); ++i) {
            std::filesystem::create_directories(jobPath_ / "images/verified" / cams[i]);
        }

        // Copy the remaining images to the verified folder.
        for (auto i{0}; i < files.size(); ++i) {
            if (std::ranges::find(indexesToDiscard_, i) != indexesToDiscard_.end()) {
                continue;
            }

            for (auto j{0}; j < cams.size(); ++j) {
                try {
                    std::filesystem::copy(jobPath_ / "images/raw" / cams[j] / files[i],
                                          jobPath_ / "images/verified" / cams[j] / files[i]
                    );
                } catch (const std::filesystem::filesystem_error &e) {
                    std::cerr << e.what() << "\n";
                }
            }
        }
    }
} // YACCP
