#include "board_creation.hpp"


namespace YACCP::CLI {
    ::CLI::App* addBoardCreationCmd(::CLI::App& app, BoardCreationConfig& config) {
        ::CLI::App* subCmd = app.add_subcommand("create-board", "Board creation");

        subCmd->add_option("-s, --square-length", config.squareLength, "The square length in pixels")
              ->default_val(config.squareLength)
              ->check(::CLI::PositiveNumber);

        subCmd
            ->add_option("-m, --marker-length", config.markerLength, "The ArUco marker length in pixels")
            ->default_val(config.markerLength)
            ->check(::CLI::PositiveNumber);

        subCmd
            ->add_option("-e, --marker-border",
                         config.border,
                         "The border size (margins) of the ArUco marker in pixels")
            ->default_str("square-length - marker-length")
            ->check(::CLI::PositiveNumber);

        subCmd
            ->add_option("-b, --border-point",
                         config.borderPoints,
                         "The amount of points (pixels) for the border")
            ->default_val(config.borderPoints)
            ->check(::CLI::PositiveNumber);

        subCmd
            ->add_flag("!-i, !--image",
                       config.generateImage,
                       "Whether to generate an image of the generated board")
            ->default_str("true");

        subCmd
            ->add_flag("-v, --video",
                       config.generateVideo,
                       "Whether to generate an event video of the generated board")
            ->default_str("false");

        subCmd->parse_complete_callback([&config]
        {
            if (!config.border)
            {
                config.border = config.squareLength - config.markerLength;
            }
        });

        return subCmd;
    }
} // namespace YACCP::CLI
