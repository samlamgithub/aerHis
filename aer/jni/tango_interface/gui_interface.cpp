#include <tango_interface/gui_interface.hpp>

namespace tango_interface {

JavaVM *GuiInterface::jvm_ = nullptr;
jobject GuiInterface::gui_ref_;

bool GuiInterface::initialise(JNIEnv *env, jobject gui) {
  JavaVM *jvm;
  jint rs = env->GetJavaVM(&jvm);
  jvm_ = jvm;
  gui_ref_ = env->NewGlobalRef(gui);
}

bool GuiInterface::set_overlay_camera_size(int width, int height) {
  JNIEnv *env = java_environment();
  if (env == nullptr) {
    return false;
  }
  jclass gui_class = env->GetObjectClass(gui_ref_);
  jmethodID camera_size_ref =
      env->GetMethodID(gui_class, "setCameraSize", "(II)V");
  env->CallVoidMethod(gui_ref_, camera_size_ref, width, height);
  env->DeleteLocalRef(gui_class);
  return true;
}

JNIEnv *GuiInterface::java_environment() {
  JNIEnv *env;
  jint rs = jvm_->AttachCurrentThread(&env, nullptr);
  if (rs != JNI_OK) {
    return nullptr;
  }
  return env;
}
}
