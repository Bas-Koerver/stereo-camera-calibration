#include <metavision/sdk/stream/camera.h>
#include <metavision/sdk/core/algorithms/periodic_frame_generation_algorithm.h>
#include <metavision/sdk/ui/utils/window.h>
#include <metavision/sdk/ui/utils/event_loop.h>

int main(int argc, char *argv[]) {
    try
    {
        std::cout << "Until here it works \n";
        Metavision::Camera cam;
        if (argc >= 2) {
            // if we passed a file path, open it
            cam = Metavision::Camera::from_file(argv[1]);
        } else {
            // open the first available camera
            cam = Metavision::Camera::from_first_available();
        }

        const int w = cam.geometry().get_width();
        const int h = cam.geometry().get_height();

        const std::uint32_t acc = 20000;
        double fps = 50;
        auto frame_gen = Metavision::PeriodicFrameGenerationAlgorithm(w, h, acc, fps);

        Metavision::Window window("Frames", w, h, Metavision::BaseWindow::RenderMode::BGR);

        frame_gen.set_output_callback([&](Metavision::timestamp, cv::Mat &frame){
            window.show(frame);
        });

        cam.cd().add_callback([&](const Metavision::EventCD *begin, const Metavision::EventCD *end){
            frame_gen.process_events(begin, end);
        });

        cam.start();
        while(cam.is_running()){
            Metavision::EventLoop::poll_and_dispatch(20);
        }
        cam.stop();
    } catch (const std::exception &e)
    {
        std::cout << e.what() << "\n";
    }

    return 0;
}