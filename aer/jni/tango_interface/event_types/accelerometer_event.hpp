#ifndef __ACCELEROMETER_EVENT_HPP__
#define __ACCELEROMETER_EVENT_HPP__

#include "../event_types/timestamp_t.hpp"

namespace tango_interface {

struct AccelerometerEvent {
  timestamp_t timestamp_nanoseconds;
  float accelerations[3];
};

} // namespace tango_interface

#endif // __ACCELEROMETER_EVENT_HPP__
