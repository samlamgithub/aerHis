#ifndef __GUI_INTERFACE_HPP__
#define __GUI_INTERFACE_HPP__

#include <jni.h>
#include <vector>

#include "camera_interface.hpp"

namespace tango_interface {

class GuiInterface {
public:
  // Must only be called once
  static bool initialise(JNIEnv *env, jobject caller);
  static bool set_overlay_camera_size(int width, int height);

private:
  static JNIEnv *java_environment();
  // Java call handler
  static JavaVM *jvm_;
  static jobject gui_ref_;
};
}

#endif
