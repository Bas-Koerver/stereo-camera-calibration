#ifndef YACCP_TOOLS_IMAGE_VALIDATOR_HPP
#define YACCP_TOOLS_IMAGE_VALIDATOR_HPP
#include <filesystem>

#include <metavision/sdk/core/utils/frame_composer.h>

#include <opencv2/core/mat.hpp>

namespace YACCP {
    class ImageValidator {
    public:
        static void listJobs(const std::filesystem::path& dataPath);

        void validateImages(int resolutionWidth,
                            int resolutionHeight,
                            const std::filesystem::path& dataPath,
                            const std::string& jobName);

        [[nodiscard]] static bool isNonEmptyDirectory(const std::filesystem::path& path);

    private:
        std::filesystem::path jobPath_;
        int currentFileIndex_{0};
        std::vector<int> indexesToDiscard_;

        void updateSubimages(Metavision::FrameComposer& frameComposer,
                             const std::vector<std::filesystem::path>& files,
                             const std::vector<std::filesystem::path>& cams,
                             const std::vector<int>& camRefs) const;
    };
} // YACCP

#endif //YACCP_TOOLS_IMAGE_VALIDATOR_HPP
