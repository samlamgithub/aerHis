#include <tango_interface/gui_interface.hpp>


namespace tango_interface {

JavaVM* GuiInterface::jvm_ = nullptr;
jobject GuiInterface::gui_ref_;

bool GuiInterface::initialise(JNIEnv* env, jobject gui) {
  JavaVM *jvm;
  jint rs = env->GetJavaVM(&jvm);
  jvm_ = jvm;
  gui_ref_ = env->NewGlobalRef(gui);
}
//
//bool GuiInterface::display_marker_data(const std::vector<MarkerData>& marker_data) {
//  for (auto marker : marker_data) {
//      tango_interface::GuiInterface::add_marker(marker.id,
//            marker.estimated_raw_corners(0,0), marker.estimated_raw_corners(1,0),
//            marker.estimated_raw_corners(0,1), marker.estimated_raw_corners(1,1),
//            marker.estimated_raw_corners(0,2), marker.estimated_raw_corners(1,2),
//            marker.estimated_raw_corners(0,3), marker.estimated_raw_corners(1,3));
//  }
//}
//
//bool GuiInterface::add_marker(int label_id, int x1, int y1, int x2, int y2,
//                              int x3, int y3, int x4, int y4) {
//  JNIEnv *env = java_environment();
//  if (env == nullptr) {
//    return false;
//  }
//  jclass gui_class = env->GetObjectClass(gui_ref_);
//  jmethodID marker_overlay_ref = env->GetMethodID(gui_class, "addMarker", "(IIIIIIIII)V");
//  env->CallVoidMethod(gui_ref_, marker_overlay_ref, label_id,x1,y1,x2,y2,x3,y3,x4,y4);
//  env->DeleteLocalRef(gui_class);
//  return true;
//}

bool GuiInterface::set_overlay_camera_size(int width, int height) {
  JNIEnv *env = java_environment();
  if (env == nullptr) {
    return false;
  }
  jclass gui_class = env->GetObjectClass(gui_ref_);
  jmethodID camera_size_ref = env->GetMethodID(gui_class, "setCameraSize", "(II)V");
  env->CallVoidMethod(gui_ref_, camera_size_ref , width, height);
  env->DeleteLocalRef(gui_class);
  return true;
}

JNIEnv* GuiInterface::java_environment() {
  JNIEnv *env;
  jint rs = jvm_->AttachCurrentThread(&env, nullptr);
  if (rs != JNI_OK) {
    return nullptr;
  }
  return env;
}

}
