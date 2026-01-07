#ifndef YACCP_CONFIG_RECORDING_HPP
#define YACCP_CONFIG_RECORDING_HPP
#include <toml++/toml.hpp>

namespace YACCP {
    enum class WorkerTypes;
}

namespace YACCP::Config {
    struct RecordingConfig {
        struct Prophesee {
            bool saveEventfile{};
            std::vector<std::string> camUuid{};
        };

        struct Basler {
            std::vector<std::string> camUuid{};
        };


        int fps{};
        int accumulationTime{};
        std::vector<WorkerTypes> slaveWorkers{};
        WorkerTypes masterWorker{};
        std::vector<int> camPlacement{};
        int masterCamera{};

        Prophesee prophesee{};
        Basler basler{};
    };

    void parseRecordingConfig(const toml::table& tbl, RecordingConfig& config);
} // YACCP::Config

#endif //YACCP_CONFIG_RECORDING_HPP
