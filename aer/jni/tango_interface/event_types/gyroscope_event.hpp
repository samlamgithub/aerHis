#ifndef __GYROSCOPE_EVENT_HPP__
#define __GYROSCOPE_EVENT_HPP__

#include "../event_types/timestamp_t.hpp"

namespace tango_interface {

struct GyroscopeEvent {
  timestamp_t timestamp_nanoseconds;
  float rotation_rates[3];
};

} // namespace tango_interface

#endif // __GYROSCOPE_EVENT_HPP__
