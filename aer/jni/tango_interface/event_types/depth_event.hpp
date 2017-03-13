#ifndef __DEPTH_EVENT_HPP__
#define __DEPTH_EVENT_HPP__

#include <memory>

#include "../event_types/timestamp_t.hpp"

namespace tango_interface {

struct DepthEvent {
  timestamp_t timestamp_nanoseconds;

  std::shared_ptr<float> data;
};

} // namespace tango_interface

#endif // __DEPTH_EVENT_HPP__
