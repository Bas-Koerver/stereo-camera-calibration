#include "image_validator.hpp"

#include "../utility.hpp"

#include <fstream>
// #include <iostream>

#include <metavision/sdk/ui/utils/event_loop.h>
#include <metavision/sdk/ui/utils/window.h>

// #include <nlohmann/json.hpp>

namespace YACCP {
    void ImageValidator::updateSubimages(Metavision::FrameComposer& frameComposer,
                                         const std::vector<std::filesystem::path>& files,
                                         const std::vector<std::filesystem::path>& cams,
                                         const std::vector<int>& camRefs) const {
        for (auto i{0}; i < cams.size(); ++i) {
            frameComposer.update_subimage(
                camRefs[i],
                cv::imread(
                    (jobPath_ / "images/raw" / cams[i] / files[currentFileIndex_]).string()));
        }
    }

    void ImageValidator::listJobs(const std::filesystem::path& dataPath) {
        std::cout << "Available jobs to validate: \n";
        for (auto const& entry : std::filesystem::directory_iterator(dataPath)) {
            if (!entry.is_directory()) continue;

            std::filesystem::path rawPath = entry.path() / "images" / "raw";
            std::filesystem::path verifiedPath = entry.path() / "images" / "verified";

            // Are there files in the verified folder?
            if (Utility::isNonEmptyDirectory(verifiedPath)) continue;

            // Are there no files in the raw folder?
            if (!Utility::isNonEmptyDirectory(rawPath)) continue;

            std::cout << "  " << entry.path().filename() << "\n";
        }

        std::cout << "\nJobs already validated: \n";
        for (auto const& entry : std::filesystem::directory_iterator(dataPath)) {
            if (!entry.is_directory()) continue;

            std::filesystem::path verifiedPath = entry.path() / "images" / "verified";

            // Are there already files in the verified folder?
            if (!Utility::isNonEmptyDirectory(verifiedPath)) continue;

            std::cout << "  " << entry.path().filename() << "\n";
        }
    }

    void ImageValidator::validateImages(int resolutionWidth,
                                        int resolutionHeight,
                                        const std::filesystem::path& dataPath,
                                        const std::string& jobId) {
        jobPath_ = dataPath / jobId;
        if (!exists(jobPath_)) {
            std::cerr << "Job: " << jobId << " does not exist in the given path: " << dataPath << "\n";
            return;
        }

        if (!Utility::isNonEmptyDirectory(jobPath_ / "images" / "raw")) {
            std::cerr << "\nNo raw images found for job: " << jobId << "\n";
            return;
        }

        std::vector<std::filesystem::path> cams;
        std::vector<std::filesystem::path> images;
        std::vector<int> camRefs;

        for (auto const& entry : std::filesystem::directory_iterator(jobPath_ / "images" / "raw")) {
            if (!entry.is_directory()) continue;
            cams.push_back(entry.path().filename());
        }
        for (auto const& entry : std::filesystem::directory_iterator(jobPath_ / "images" / "raw" / cams[0])) {
            if (!entry.is_regular_file()) continue;
            images.push_back(entry.path().filename());
        }

        std::ranges::sort(images);

        if (!exists(jobPath_ / "job_data.json")) {
            std::cerr << "No camera data was found.\nStopping! \n";
            return;
        }

        std::ifstream file{jobPath_ / "job_data.json"};
        if (!file) {
            std::cerr << "Camera data exists but cannot be opened.\nStopping!\n";
            return;
        }
        nlohmann::json j{nlohmann::json::parse(file)};

        Metavision::FrameComposer frameComposer;
        try {
            for (const auto& cam : cams) {
                const auto& camj = j.at("cams").at(cam.string());
                const auto& view = camj.at("view");
                const auto& resolution = camj.at("resolution");

                const int topLeftX = view.at("windowX").get<int>();
                const int topLeftY = view.at("windowY").get<int>();
                const unsigned width = resolution.at("width").get<unsigned>();
                const unsigned height = resolution.at("height").get<unsigned>();
                camRefs.emplace_back(
                    frameComposer.add_new_subimage_parameters(topLeftX,
                                                              topLeftY,
                                                              {width, height},
                                                              Metavision::FrameComposer::GrayToColorOptions()
                    )
                );
            }
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << "\n";
            std::cerr << "The job_data.json has the wrong format";
            return;
        }

        double scaleX{static_cast<double>(resolutionWidth) / frameComposer.get_total_width()};
        double scaleY{static_cast<double>(resolutionHeight) / frameComposer.get_total_height()};
        double scale{std::min(scaleX, scaleY)};
        Utility::AlternativeBuffer buffer;
        buffer.enable();

        int width{(frameComposer.get_total_width())};
        int height{(frameComposer.get_total_height())};
        cv::Mat display{height, width, CV_8UC3};

        // Close the window when receiving a should close flag.
        {
            cv::Scalar textColour;
            Metavision::Window window("Validating recorded detections",
                                      width * scale,
                                      height * scale,
                                      Metavision::Window::RenderMode::BGR);

            window.set_keyboard_callback(
                [this,
                    &window,
                    &cams,
                    &frameComposer,
                    &images,
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
                            if (std::ranges::find(indexesToDiscard_, currentFileIndex_) != indexesToDiscard_.
                                end()) {
                                std::erase(indexesToDiscard_, currentFileIndex_);
                            } else {
                                indexesToDiscard_.emplace_back(currentFileIndex_);
                            }
                            break;
                        case Metavision::UIKeyEvent::KEY_LEFT:
                            if (currentFileIndex_ <= 0) {
                                Utility::clearScreen();
                                std::cout << "Reached the beginning of the image set.\n Looping to end.\n\n";
                                currentFileIndex_ = images.size() - 1;
                            } else {
                                currentFileIndex_--;
                            }
                            // Go to previous image.
                            updateSubimages(frameComposer, images, cams, camRefs);
                            break;
                        case Metavision::UIKeyEvent::KEY_RIGHT:
                            if (currentFileIndex_ >= images.size() - 1) {
                                Utility::clearScreen();
                                std::cout << "Reached the end of the image set.\n Looping back to start.\n\n";
                                currentFileIndex_ = 0;
                            } else {
                                currentFileIndex_++;
                            }

                            // Go to next image.
                            updateSubimages(frameComposer, images, cams, camRefs);
                            break;
                        }
                    }
                }
            );

            updateSubimages(frameComposer, images, cams, camRefs);

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
                            images[currentFileIndex_].filename().string(),
                            cv::Point(10, height - 60),
                            cv::FONT_HERSHEY_SIMPLEX,
                            0.8,
                            textColour,
                            2);

                window.show(display);
                Metavision::EventLoop::poll_and_dispatch(20);;
            }
        }

        if (Utility::isNonEmptyDirectory(jobPath_ / "images" / "verified")) {
            Utility::clearScreen();
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
                std::filesystem::remove_all(jobPath_ / "images" / "verified");
            } else {
                buffer.disable();
                std::cout << "Aborting verification process to avoid overwriting existing data.\n\n";
                return;
            }
        }

        std::filesystem::create_directories(jobPath_ / "images/verified");

        for (auto i{0}; i < cams.size(); ++i) {
            std::filesystem::create_directories(jobPath_ / "images" / "verified" / cams[i]);
        }

        // Copy the remaining images to the verified folder.
        for (auto i{0}; i < images.size(); ++i) {
            if (std::ranges::find(indexesToDiscard_, i) != indexesToDiscard_.end()) {
                continue;
            }

            // TODO: Change to direct vector indexing.
            for (auto j{0}; j < cams.size(); ++j) {
                try {
                    std::filesystem::copy(jobPath_ / "images" / "raw" / cams[j] / images[i],
                                          jobPath_ / "images" / "verified" / cams[j] / images[i]
                    );
                } catch (const std::filesystem::filesystem_error& e) {
                    std::cerr << e.what() << "\n";
                }
            }
        }
    }
} // YACCP