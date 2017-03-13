#ifndef __RAW_FRAME_EVENT_HPP__
#define __RAW_FRAME_EVENT_HPP__

#include <memory>

#include "../event_types/timestamp_t.hpp"

namespace tango_interface {

struct RawFrameEvent {
  timestamp_t timestamp_nanoseconds;
  // This is a shared pointer to the grayscale camera data. Whether the camera
  // data is fisheye, or the regular colour camera depends on the static const
  // variable camera_type_ in the CameraInterface class.
  // The data itself is a list of single byte intensity values in row major order
  // with the height and width given by the functions available in the
  // CameraInterface methods get_frame_height() and get_frame_width()
  // If you require use of the data itself after it is returned to you in a
  // callback then you must copy the data at that point.  Memory is only safe for
  // the duration of the callback.
  std::shared_ptr<unsigned char> data;
};

} // namespace tango_interface

#endif // __RAW_FRAME_EVENT_HPP__
