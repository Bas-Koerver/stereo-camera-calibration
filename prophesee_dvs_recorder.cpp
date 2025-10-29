#include <metavision/sdk/stream/camera.h>

int main()
{
    Metavision::Camera cam; // create the camera
    try
    {
        cam = Metavision::Camera::from_first_available();
    }
    catch (Metavision::CameraException& e)
    {
        std::cout << e.what() << std::endl;
    }

    cam.start();

    // keep running while the camera is on or the recording is not finished
    while (cam.is_running()) {
        std::cout << "Camera is running!" << std::endl;
    }

    // the recording is finished, stop the camera.
    // Note: we will never get here with a live camera
    cam.stop();

    // const Metavision::I_HW_Identification::SensorInfo si = cam.get_device().get_facility<Metavision::I_HW_Identification>()->get_sensor_info();
    // std::cout << "Camera Version: " << si.major_version_ << "." << si.minor_version_ << std::endl;
}