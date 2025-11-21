#include <metavision/sdk/stream/camera.h>
#include <metavision/sdk/base/events/event_ext_trigger.h>

#include <iostream>
#include <atomic>
#include <mutex>
#include <optional>

std::optional<Metavision::EventExtTrigger>
get_nth_positive_trigger(const std::string &filename, std::size_t N) {
    Metavision::Camera cam = Metavision::Camera::from_file(filename);

    if (!cam.has_ext_trigger()) {
        std::cerr << "File has no ext trigger events.\n";
        return std::nullopt;
    }

    std::atomic<bool> found{false};
    std::atomic<std::size_t> count{0};

    std::mutex ev_mutex;
    std::optional<Metavision::EventExtTrigger> result;

    cam.ext_trigger().add_callback(
        [&](const Metavision::EventExtTrigger *begin, const Metavision::EventExtTrigger *end) {
            if (found.load(std::memory_order_relaxed))
                return;

            for (auto it = begin; it != end; ++it) {
                const auto &ev = *it;

                // positive edge (rising) usually encoded as p == 1
                if (ev.p == 1) {
                    std::size_t current = ++count;
                    if (current == N) {
                        {
                            std::lock_guard<std::mutex> lock(ev_mutex);
                            result = ev; // copy the event
                        }
                        found.store(true, std::memory_order_release);
                        break;
                    }
                }
            }
        });

    cam.start();

    // For a file, poll() will push events to callbacks until EOF
    while (cam.is_running() && !found.load(std::memory_order_acquire)) {
        if (!cam.poll()) {
            // End of file reached
            break;
        }
    }

    cam.stop();

    if (!found.load(std::memory_order_acquire)) {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(ev_mutex);
    return result;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " recording.hd5 N\n";
        return 1;
    }

    const std::string filename = argv[1];
    const std::size_t N = static_cast<std::size_t>(std::stoull(argv[2]));

    auto ev_opt = get_nth_positive_trigger(filename, N);
    if (!ev_opt) {
        std::cout << "Less than " << N << " positive trigger events in file.\n";
        return 0;
    }

    const auto &ev = *ev_opt;
    std::cout << N << "th positive trigger:\n"
              << "  t  = " << ev.t << "\n"
              << "  p  = " << ev.p << "\n"
              << "  id = " << ev.id << "\n";

    return 0;
}
