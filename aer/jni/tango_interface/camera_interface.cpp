#include "tango_client_api.h"
#include "tango_support_api.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <jni.h>
#include <memory>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <tango_interface/camera_interface.hpp>
#include <tango_interface/gui_interface.hpp>
#include <tango_interface/util.hpp>
#include <tango_support_api.h>

namespace {
static void onPointCloudAvailable(void *context,
                                  const TangoPointCloud *point_cloud) {
  tango_interface::CameraInterface *cam =
      static_cast<tango_interface::CameraInterface *>(context);
  cam->onPointCloudAvailable2(point_cloud);
}

void onPoseAvailable(void *context, const TangoPoseData *pose) {}
} // namespace

using namespace cv;

namespace tango_interface {

TangoConfig CameraInterface::tango_config_ = nullptr;
std::unique_ptr<GlCameraFrame> CameraInterface::gl_camera_frame_;
std::unique_ptr<double> CameraInterface::frame_timestamp_;

std::unique_ptr<RGBDCallBack> CameraInterface::rgbd_callback_;
std::unique_ptr<LoggerWHCallBack> CameraInterface::loggerWH_callback_;

TangoSupportPointCloudManager *CameraInterface::point_cloud_manager_;

JavaVM *CameraInterface::jvm_ = nullptr;
jobject CameraInterface::activity_ref_;

int CameraInterface::myImageHeight = 0;
int CameraInterface::myImageWidth = 0;
int CameraInterface::myImageSize = 0;
double CameraInterface::myFx = 0;
double CameraInterface::myFy = 0;
double CameraInterface::myCx = 0;
double CameraInterface::myCy = 0;

uint32_t CameraInterface::max_vertex_count = 0;
bool CameraInterface::is_service_connected_ = false;

ThreadMutexObject<int> CameraInterface::countFrame;

void CameraInterface::onPointCloudAvailable2(
    const TangoPointCloud *point_cloud) {
  TangoErrorType err =
      TangoSupport_updatePointCloud(point_cloud_manager_, point_cloud);
  if (err != TANGO_SUCCESS) {
    LOGE("CameraInterface:TangoSupport_updatePointCloud failed");
  }
}

void CameraInterface::register_rgbd_callback(RGBDCallBack function) {
  rgbd_callback_.reset(new RGBDCallBack(function));
}

void CameraInterface::register_loggerWidthHeight_callback(
    LoggerWHCallBack function) {
  loggerWH_callback_.reset(new LoggerWHCallBack(function));
}

bool CameraInterface::initialise(JNIEnv *env, jobject caller_activity,
                                 jobject asset_manager) {
  countFrame.assignValue(0);
  TangoErrorType ret = TangoService_initialize(env, caller_activity);
  if (ret != TANGO_SUCCESS) {
    LOGE("CameraInterface: Failed to initialise the tango service.");
    return false;
  }
  JavaVM *jvm;
  jint rs = env->GetJavaVM(&jvm);
  jvm_ = jvm;
  activity_ref_ = env->NewGlobalRef(caller_activity);
  LOGI("CameraInterface: Successfully initialised.");
  return true;
}

void CameraInterface::destroy() {
  disconnect();
  java_environment()->DeleteGlobalRef(activity_ref_);
  TangoSupport_freePointCloudManager(point_cloud_manager_);
  point_cloud_manager_ = nullptr;
}

void CameraInterface::incrementCounter() {
  LOGI("CameraInterface: incrementCounter called");
  countFrame.assignValue(1);
}

bool CameraInterface::connect() {
  // Initialize TangoSupport context.
  TangoSupport_initializeLibrary();

  // Set up the minimal options necessary for fisheye camera call-backs
  if (!setup_tango_config()) {
    LOGE("CameraInterface: Failed to setup the tango configuration");
    return false;
  }
  // Connect to tango service
  TangoErrorType status = TangoService_connect(nullptr, tango_config_);
  if (status != TANGO_SUCCESS) {
    LOGE("CameraInterface: Failed to connect to the tango service.");
    return false;
  }
  // Initialise and OpenGL camera frame class for rendering raw camera input
  gl_camera_frame_.reset(new GlCameraFrame());
  TangoCameraIntrinsics camera_intrinsics;
  TangoService_getCameraIntrinsics(camera_type_, &camera_intrinsics);
  myImageHeight = camera_intrinsics.height;
  myImageWidth = camera_intrinsics.width;
  myImageSize = myImageHeight * myImageWidth;
  myFx = camera_intrinsics.fx;
  myFy = camera_intrinsics.fy;
  myCx = camera_intrinsics.cx;
  myCy = camera_intrinsics.cy;
  set_frame_view_port(myImageWidth, myImageHeight);
  // Connect callbacks for new camera frames to the OpenGL GlCameraFrame class
  frame_timestamp_.reset(new double);
  status = TangoService_connectTextureId(
      camera_type_, gl_camera_frame_->texture_id(), nullptr,
      &CameraInterface::process_frame_event);
  if (status != TANGO_SUCCESS) {
    LOGE("CameraInterface: Failed to connect texture callbacks for the camera.");
    return false;
  }
  if (TangoService_connectOnPointCloudAvailable(onPointCloudAvailable) !=
      TANGO_SUCCESS) {
    LOGE("CameraInterface: Failed to TangoService_connectOnPointCloudAvailable");
    return false;
  }
  // Use the tango_config to set up the PointCloudManager before we connect
  // the callbacks.
  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    TangoErrorType ret = TangoConfig_getInt32(
        tango_config_, "max_point_cloud_elements", &max_point_cloud_elements);
    if (ret != TANGO_SUCCESS) {
      LOGE("Failed to query maximum number of point cloud elements.");
      std::exit(EXIT_SUCCESS);
    }
    max_vertex_count = static_cast<uint32_t>(max_point_cloud_elements);
    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (ret != TANGO_SUCCESS) {
      LOGE("Failed to TangoSupport_createPointCloudManager");
      std::exit(EXIT_SUCCESS);
    }
  }
  is_service_connected_ = true;
  if (loggerWH_callback_) {
    (*loggerWH_callback_)(myImageWidth, myImageHeight, myFx, myFy, myCx, myCy,
                          max_vertex_count);
  }
  return true;
}

void CameraInterface::disconnect() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
    TangoService_disconnect();
    is_service_connected_ = false;
  }
  LOGI("CameraInterface: Finished disconnecting.");
}

void CameraInterface::render() {
  TangoErrorType status =
      TangoService_updateTexture(camera_type_, frame_timestamp_.get());
  if (status == TANGO_SUCCESS && (*frame_timestamp_) > 0) {
    if (gl_camera_frame_) {
      gl_camera_frame_->render();

      std::shared_ptr<unsigned char> frame = gl_camera_frame_->get_frame(4);

      if (!is_service_connected_) {
        return;
      }

      double color_timestamp = 0.0;
      double depth_timestamp = 0.0;
      bool new_points = false;
      TangoPointCloud *pointcloud_buffer;
      TangoErrorType err = TangoSupport_getLatestPointCloudAndNewDataFlag(
          point_cloud_manager_, &pointcloud_buffer, &new_points);
      if (err != TANGO_SUCCESS) {
        LOGE("CameraInterface: Failed to "
             "TangoSupport_getLatestPointCloudAndNewDataFlag");
        return;
      }
      if (!new_points) {
        LOGI("CameraInterface: point could data is not new, return");
        return;
      } else {
        LOGI("CameraInterface: point could data is new");
      }
      depth_timestamp = pointcloud_buffer->timestamp;

      // We need to make sure that we update the texture associated with the
      // color image.
      if (TangoService_updateTextureExternalOes(
              TANGO_CAMERA_COLOR, gl_camera_frame_->texture_id(),
              &color_timestamp) != TANGO_SUCCESS) {
        LOGE("CameraInterface: Failed to get a color image.");
        return;
      }

      TangoCoordinateFramePair frames_of_reference;
      frames_of_reference.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
      frames_of_reference.target = TANGO_COORDINATE_FRAME_DEVICE;
      TangoPoseData pose;
      TangoErrorType e =
          TangoService_getPoseAtTime(0.0, frames_of_reference, &pose);
      if (e == TANGO_SUCCESS) {
        LOGI("TangoService_getPoseAtTime success");
      } else {
        LOGI("TangoService_getPoseAtTime failed");
      }

      if (rgbd_callback_) {
        (*rgbd_callback_)(frame.get(), pointcloud_buffer, color_timestamp,
                          pose);
      }
    }
  }
}

void CameraInterface::set_display_view_port(int width, int height) {
  if (gl_camera_frame_) {
    gl_camera_frame_->set_display_view_port(width, height);
  }
}

void CameraInterface::set_frame_view_port(int width, int height) {
  if (gl_camera_frame_) {
    gl_camera_frame_->set_frame_view_port(width, height);
    // Set the overlay camera size at the same time to ensure alignment
    GuiInterface::set_overlay_camera_size(width, height);
  }
}

int CameraInterface::get_frame_height() {
  if (gl_camera_frame_) {
    return gl_camera_frame_->get_frame_view_height();
  }
  return 0;
}

int CameraInterface::get_frame_width() {
  if (gl_camera_frame_) {
    return gl_camera_frame_->get_frame_view_width();
  }
  return 0;
}

// Tango specific public functions
void CameraInterface::process_frame_event(void *context, TangoCameraId id) {
  // Must request a render to receive on the OpenGL thread context
  request_render();
}

bool CameraInterface::setup_tango_config() {
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    return false;
  }
  TangoErrorType ret =
      TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable color camera.");
    return false;
  }
  ret =
      TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", false);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to disable recovery from motion tracking.");
    return false;
  }

  ret = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable depth.");
    return false;
  }
  ret = TangoConfig_setBool(tango_config_,
                            "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable low latency imu integration.");
    return false;
  }
  ret =
      TangoConfig_setBool(tango_config_, "config_enable_motion_tracking", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable motion tracking.");
    return false;
  }
  ret =
      TangoConfig_setBool(tango_config_, "config_enable_learning_mode", false);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to disable learning mode.");
    return false;
  }
  ret = TangoConfig_setInt32(tango_config_, "config_depth_mode",
                             TANGO_POINTCLOUD_XYZC);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable config_depth_mode TANGO_POINTCLOUD_XYZC.");
    return false;
  }

  return true;
}

// JVM Functions
JNIEnv *CameraInterface::java_environment() {
  JNIEnv *env;
  jint rs = jvm_->AttachCurrentThread(&env, nullptr);
  if (rs != JNI_OK) {
    return nullptr;
  }
  return env;
}

void CameraInterface::request_render() {
  JNIEnv *env = java_environment();
  if (env != nullptr) {
    jclass activity_class = env->GetObjectClass(activity_ref_);
    jmethodID request_render_ref =
        env->GetMethodID(activity_class, "requestRender", "()V");
    if (countFrame.getValue() == 1) {
      jmethodID incrementCounter_ref =
          env->GetMethodID(activity_class, "incrementCounter", "()V");
      env->CallVoidMethod(activity_ref_, incrementCounter_ref);
      countFrame.assignValue(0);
    } else if (countFrame.getValue() == -1) {
      jmethodID resetCounter_ref =
          env->GetMethodID(activity_class, "resetCounter", "()V");
      env->CallVoidMethod(activity_ref_, resetCounter_ref);
      countFrame.assignValue(0);
    }
    env->CallVoidMethod(activity_ref_, request_render_ref);
    env->DeleteLocalRef(activity_class);
  }
}

} // namespace tango_interface
