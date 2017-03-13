#ifndef __AER_HPP__
#define __AER_HPP__

#include <memory>

#include <tango_interface/event_types/accelerometer_event.hpp>
#include <tango_interface/event_types/gyroscope_event.hpp>
#include <tango_interface/event_types/marker_event.hpp>
#include <tango_interface/event_types/raw_frame_event.hpp>

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
  void marker_callback(const tango_interface::MarkerEvent& event);
};

} // namespace aer

#endif // __AER_HPP__
