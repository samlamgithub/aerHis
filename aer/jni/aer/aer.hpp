#ifndef __AER_HPP__
#define __AER_HPP__

#include <memory>

#include <tango_interface/event_types/accelerometer_event.hpp>
#include <tango_interface/event_types/gyroscope_event.hpp>
#include <tango_interface/event_types/marker_event.hpp>
#include <tango_interface/event_types/raw_frame_event.hpp>
#include <tango_interface/mylogger.hpp>
#include <tango_client_api.h>

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
//  void accelerometer_callback(const tango_interface::AccelerometerEvent& event);
//  void gyroscope_callback(const tango_interface::GyroscopeEvent& event);

  // Camera callback functions
//  void raw_frame_callback(const tango_interface::RawFrameEvent& event);
//  void rgbd_callback(const tango_interface::DepthEvent& event);
//  void setLoggerWidthHeight_callback(const int width, const int height, const double fx, const double fy, const double cx, const double cy);
  void setLoggerWidthHeight_callback(const int width, const int height, const double fx, const double fy, const double cx, const double cy, const int max_vertex_count);

  void rgbdCallback(unsigned char* image, TangoPointCloud* pointcloud_buffer, double cameraTime);
//  void marker_callback(const tango_interface::MarkerEvent& event);
  void writing_callback();
  void aerStartWriting(bool startWriting);
  tango_interface::Mylogger mylogger;
};

} // namespace aer

#endif // __AER_HPP__
