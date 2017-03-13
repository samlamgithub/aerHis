#include <tango_interface/camera_interface.hpp>
#include <tango_interface/gui_interface.hpp>
#include <tango_interface/imu_interface.hpp>
#include <tango_interface/logger.hpp>
#include <tango_interface/util.hpp>
#include <memory>
#include "aer.hpp"
#include <tango_interface/mylogger.hpp>

namespace aer {

Aer::Aer() {
  if (tango_interface::ImuInterface::initialise()) {
    tango_interface::ImuInterface::register_accelerometer_callback(
      std::bind(&Aer::accelerometer_callback, this, std::placeholders::_1));
    tango_interface::ImuInterface::register_gyroscope_callback(
      std::bind(&Aer::gyroscope_callback, this, std::placeholders::_1));
  }
  tango_interface::CameraInterface::register_raw_frame_callback(
    std::bind(&Aer::raw_frame_callback, this, std::placeholders::_1));
  tango_interface::CameraInterface::register_marker_callback(
    std::bind(&Aer::marker_callback, this, std::placeholders::_1));
  tango_interface::CameraInterface::register_loggerWidthHeight_callback(
       std::bind(&Aer::setLoggerWidthHeight_callback, this, std::placeholders::_1, std::placeholders::_2));
  tango_interface::CameraInterface::register_rgbd_callback(
      std::bind(&Aer::rgbdCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5,
    		  std::placeholders::_6));
  tango_interface::CameraInterface::register_writing_callback(
      std::bind(&Aer::writing_callback, this));
//  tango_interface::Mylogger logger;
//  tango_interface::CameraInterface::setLogger(&logger);

  tango_interface::Logger file_logger;
  file_logger.log(tango_interface::kInfo,"My first default informational log %.02f %i",0.05,42);
}

Aer::~Aer() {

}

void Aer::accelerometer_callback(const tango_interface::AccelerometerEvent& event) {
  // TODO: do something with the values...
}

void Aer::gyroscope_callback(const tango_interface::GyroscopeEvent& event) {
  // TODO: do something with the values...
}

void Aer::marker_callback(const tango_interface::MarkerEvent& event) {
  if (!event.marker_data.empty()) {
    LOGI("Markers detected: %lld", event.timestamp_nanoseconds);
    tango_interface::GuiInterface::display_marker_data(event.marker_data);
  }
  // TODO later: do something...
}

void Aer::setLoggerWidthHeight_callback(const int width, const int height) {
	mylogger.setCamWidthAndheight(width, height);
}

void Aer::rgbdCallback(unsigned char* image, float* depth, double cameraTime, int depth_image_width, int depth_image_height, int depth_image_size) {
  mylogger.rgbdCallback(image, depth, cameraTime, depth_image_width, depth_image_height, depth_image_size);
}

void Aer::writing_callback() {
	 LOGI("logger start writing callback");
}

void Aer::rgbd_callback(const tango_interface::DepthEvent& event) {
//  LOGI("raw depth(t): %lld", event.timestamp_nanoseconds);
}

void Aer::raw_frame_callback(const tango_interface::RawFrameEvent& event) {
  LOGI("raw frame(t): %lld", event.timestamp_nanoseconds);
}

void Aer::aerStartWriting(bool startWriting) {
	LOGI("Aer hello start writing");
	if (startWriting) {
		 LOGI("logger start writing in aerStartWriting");
			 mylogger.startWriting();
	} else {
		  LOGI("logger stop weriting in aerStartWriting");
			mylogger.stopWriting();
	}
}

} // namespace aer
