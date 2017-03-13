#ifndef __GUI_INTERFACE_HPP__
#define __GUI_INTERFACE_HPP__

#include <jni.h>
#include <vector>

#include "camera_interface.hpp"

namespace tango_interface {

class GuiInterface {
public:
  // Must only be called once
  static bool initialise(JNIEnv* env, jobject caller);
  static bool set_overlay_camera_size(int width, int height);
  static bool display_marker_data(const std::vector<MarkerData>& marker_data);
  static bool add_marker(int label_id, int x1, int y1, int x2, int y2, 
                          int x3, int y3, int x4, int y4);
private:
  static JNIEnv* java_environment();
  // Java call handler
  static JavaVM* jvm_;
  static jobject gui_ref_;
};

}

#endif
