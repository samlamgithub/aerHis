#ifndef __AER_HPP__
#define __AER_HPP__

#include <memory>

#include <tango_interface/event_types/accelerometer_event.hpp>
#include <tango_interface/event_types/gyroscope_event.hpp>
#include <tango_interface/event_types/marker_event.hpp>
#include <tango_interface/event_types/raw_frame_event.hpp>
#include <tango_interface/mylogger.hpp>

namespace aer {

class Aer {
public:
  Aer();
  ~Aer();

  // TODO: Fill this out to be a proper state returned according to Stefan's
  // criteria for testing maybe?? Otherwise delete
  // You must implement a function that returns your state
  double x_location() { return 3.0; };

  // IMU callback functions
  void accelerometer_callback(const tango_interface::AccelerometerEvent& event);
  void gyroscope_callback(const tango_interface::GyroscopeEvent& event);

  // Camera callback functions
  void raw_frame_callback(const tango_interface::RawFrameEvent& event);
  void rgbd_callback(const tango_interface::DepthEvent& event);
  void setLoggerWidthHeight_callback(const int width, const int height);
  void rgbdCallback(std::shared_ptr<unsigned char> image, std::shared_ptr<float> depth, double cameraTime, int depth_image_width, int depth_image_height, int depth_image_size);
  void marker_callback(const tango_interface::MarkerEvent& event);

  tango_interface::Mylogger mylogger;
};

} // namespace aer

#endif // __AER_HPP__
