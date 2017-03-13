#ifndef __SENSOR_HPP__
#define __SENSOR_HPP__

#include <functional>
#include <memory>
#include <vector>

#include "event_types/accelerometer_event.hpp"
#include "event_types/gyroscope_event.hpp"

class ASensorEventQueue;

namespace tango_interface {

typedef std::function<void (const AccelerometerEvent& event)> AccelerometerCallBack;
typedef std::function<void (const GyroscopeEvent& event)> GyroscopeCallBack;

class ImuInterface {
 public:
  // Main callback functions
  static void register_accelerometer_callback(AccelerometerCallBack function);
  static void register_gyroscope_callback(GyroscopeCallBack function);

  // This must be called before call-backs will be returned
  static bool initialise();
  static void destroy();
  static void pause();
  static void resume();

  // DO NOT CALL: This is needed for the Android API only
  static int process_events(int fd, int events, void* data);
 private:
  static const int id_ = 42;

  static ASensorEventQueue* sensor_event_queue_;
  static std::unique_ptr<AccelerometerCallBack> accelerometer_callback_;
  static std::unique_ptr<GyroscopeCallBack> gyroscope_callback_;
  static bool sensor_initialised_;
  static bool pause_callbacks_;
};

}

#endif
