#include <ctime>
#include <cstring>
#include <string>
#include <sstream>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#include <memory>
#include "tango_client_api.h"
#include "tango_support_api.h"
//#include <tango-gl/conversions.h>
//#include "mylogger.hpp"
//#include <tango-gl/util.h>
#include <tango_support_api.h>
#include <tango_interface/camera_interface.hpp>
#include <tango_interface/gui_interface.hpp>
#include <tango_interface/util.hpp>

namespace {
static void onPointCloudAvailable(void* context, const TangoPointCloud* point_cloud) {
//  memcpy(depth_data_buffer, point_cloud->points, point_cloud->num_points * 4 * sizeof(float));
  // Copy any other necessary data out of point_cloud.
  // Number of points in the point cloud.
//    float average_depth;
//
//    // Calculate the average depth.
//    average_depth = 0;
//    // Each xyzc point has 4 coordinates.
//    for (size_t i = 0; i < point_cloud->num_points; ++i) {
//      average_depth += point_cloud->points[i][2];
//    }
//    if (point_cloud->num_points) {
//      average_depth /= point_cloud->num_points;
//    }
  // Log the number of points and average depth.
	  // Log the number of points and average depth.
//	LOGI("CameraInterface: Point count: %d. Average depth (m): %.3f",
//	       point_cloud->num_points, average_depth);
    tango_interface::CameraInterface* cam =
               static_cast<tango_interface::CameraInterface*>(context);
    cam->onPointCloudAvailable2(point_cloud);
}


void onPoseAvailable(void* context, const TangoPoseData* pose) {
  LOGI("onPoseAvailable: Timstamp: %f, status code: %d, Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
		  pose->timestamp, pose->status_code,
       pose->translation[0], pose->translation[1], pose->translation[2],
       pose->orientation[0], pose->orientation[1], pose->orientation[2],
       pose->orientation[3]);
}

}

using namespace cv;

namespace tango_interface {

TangoConfig CameraInterface::tango_config_ = nullptr;
std::unique_ptr<GlCameraFrame> CameraInterface::gl_camera_frame_;
std::unique_ptr<double> CameraInterface::frame_timestamp_;

//std::unique_ptr<RawFrameCallBack> CameraInterface::raw_frame_callback_;
//std::unique_ptr<MarkerCallBack> CameraInterface::marker_callback_;
std::unique_ptr<RGBDCallBack> CameraInterface::rgbd_callback_;
std::unique_ptr<LoggerWHCallBack> CameraInterface::loggerWH_callback_;
std::unique_ptr<WritingCallBack> CameraInterface::writing_callback_;

TangoSupportPointCloudManager* CameraInterface::point_cloud_manager_;

JavaVM* CameraInterface::jvm_ = nullptr;
jobject CameraInterface::activity_ref_;


int CameraInterface::myImageHeight = 0;
int CameraInterface::myImageWidth = 0;
int CameraInterface::myImageSize = 0;
double CameraInterface::myFx = 0;
double CameraInterface::myFy = 0;
double CameraInterface::myCx = 0;
double CameraInterface::myCy = 0;

//bool CameraInterface::ar_config_available_ = false;
//
//ARParam CameraInterface::ar_param_;
//ARParamLT* CameraInterface::ar_param_lt_ = nullptr;
//ARHandle* CameraInterface::ar_handle_ = nullptr;
//AR3DHandle* CameraInterface::ar_3dhandle_ = nullptr;
//bool CameraInterface::ar_initialised_ = false;
uint32_t CameraInterface::max_vertex_count = 0;
bool CameraInterface::is_service_connected_ = false;

//Mylogger* CameraInterface::mylogger;

void CameraInterface::onPointCloudAvailable2(const TangoPointCloud* point_cloud) {
	  // Allocate the depth buffer (Each point has four values: X, Y, Z and C)

	TangoErrorType err = TangoSupport_updatePointCloud(point_cloud_manager_, point_cloud);
	if (err != TANGO_SUCCESS) {
		LOGE("CameraInterface:TangoSupport_updatePointCloud failed");
//	}
//	if (!point_cloud_manager_) {
//		LOGE("CameraInterface:point_cloud_manager_ null");
//	} else {
//		LOGE("CameraInterface:point_cloud_manager_ not null");
//	}
//	if (!point_cloud) {
//			LOGE("CameraInterface:point_cloud null");
//	} else {
//		LOGE("CameraInterface:point_cloud not null");
//	}
//	auto depth_data_buffer = new float[4 * max_vertex_count];
//	 memcpy(depth_data_buffer, point_cloud->points, point_cloud->num_points * 4 * sizeof(float));
//	     DepthEvent depth_event;
//	     depth_event.timestamp_nanoseconds = static_cast<int64_t>((*frame_timestamp_) * 1000000000.0);
//	     depth_event.data = depth_data_buffer;
//	 	          (*depth_callback_)(depth_event);
}
//
//void CameraInterface::register_raw_frame_callback(RawFrameCallBack function) {
//  raw_frame_callback_.reset(new RawFrameCallBack(function));
}

void CameraInterface::register_rgbd_callback(RGBDCallBack function) {
	rgbd_callback_.reset(new RGBDCallBack(function));
}

void CameraInterface::register_loggerWidthHeight_callback(LoggerWHCallBack function) {
	loggerWH_callback_.reset(new LoggerWHCallBack(function));
}
//
//void CameraInterface::register_marker_callback(MarkerCallBack function) {
//  marker_callback_.reset(new MarkerCallBack(function));
//  if (ar_config_available_ && !ar_initialised_) {
//    ar_initialised_ = initialise_artoolkit();
//  }
//}

void CameraInterface::register_writing_callback(WritingCallBack function) {
  writing_callback_.reset(new WritingCallBack(function));
}

bool CameraInterface::initialise(JNIEnv* env, jobject caller_activity, jobject asset_manager) {
//  if (!ar_initialised_) {
    TangoErrorType ret = TangoService_initialize(env, caller_activity);
    if (ret != TANGO_SUCCESS) {
      LOGE("CameraInterface: Failed to initialise the tango service.");
      return false;
    }
//    load_ar_config(env,asset_manager);
//    if (!ar_initialised_ && ar_config_available_ && marker_callback_ != nullptr) {
//      ar_initialised_ = initialise_artoolkit();
//    }
//  }
  JavaVM *jvm;
  jint rs = env->GetJavaVM(&jvm);
  jvm_ = jvm;
  activity_ref_ = env->NewGlobalRef(caller_activity);
  LOGI("CameraInterface: Successfully initialised.");
  return true;
}

void CameraInterface::destroy() {
  disconnect();
//  if (ar_initialised_) {
//    destroy_artoolkit();
//  }
  java_environment()->DeleteGlobalRef(activity_ref_);
  TangoSupport_freePointCloudManager(point_cloud_manager_);
   point_cloud_manager_ = nullptr;
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
//  LOGI("camera_intrinsics: %d", camera_intrinsics);
  myImageHeight = camera_intrinsics.height;
  myImageWidth = camera_intrinsics.width;
  myImageSize = myImageHeight * myImageWidth;
  myFx = camera_intrinsics.fx;
  myFy = camera_intrinsics.fy;
  myCx = camera_intrinsics.cx;
  myCy = camera_intrinsics.cy;
	LOGI("CameraInterface TangoService_getCameraIntrinsics: %d, %d, %f, %f, %f, %f ", myImageWidth, myImageHeight, myFx, myFy, myCx, myCy);
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
  if (TangoService_connectOnPointCloudAvailable(onPointCloudAvailable) != TANGO_SUCCESS) {
	  LOGE("CameraInterface: Failed to TangoService_connectOnPointCloudAvailable");
	   return false;
  }
  // Connect color camera texture. The callback is ignored because the
//  // color camera is polled.
//  TangoErrorType ret = TangoService_connectOnTextureAvailable(TANGO_CAMERA_COLOR, nullptr,
//                                               nullptr);
//  if (ret != TANGO_SUCCESS) {
//    LOGE(
//        "SynchronizationApplication: Failed to connect texture callback with "
//        "errorcode: %d",
//        ret);
//    std::exit(EXIT_SUCCESS);
//  }
  // Use the tango_config to set up the PointCloudManager before we connect
  // the callbacks.
  if (point_cloud_manager_ == nullptr) {
    int32_t max_point_cloud_elements;
    TangoErrorType ret = TangoConfig_getInt32(tango_config_, "max_point_cloud_elements",
                               &max_point_cloud_elements);
    if (ret != TANGO_SUCCESS) {
      LOGE("Failed to query maximum number of point cloud elements.");
      std::exit(EXIT_SUCCESS);
    }
    max_vertex_count = static_cast<uint32_t>(max_point_cloud_elements);
//    std::string s = std::to_string(max_vertex_count);
//    char const *pchar = s.c_str();
//    LOGE(pchar);
    __android_log_print(ANDROID_LOG_INFO, "MyTag", "The max_vertex_count is %d", max_vertex_count);
    LOGI("max_vertex_count:");
    ret = TangoSupport_createPointCloudManager(max_point_cloud_elements,
                                               &point_cloud_manager_);
    if (ret != TANGO_SUCCESS) {
    	LOGE("Failed to TangoSupport_createPointCloudManager");
      std::exit(EXIT_SUCCESS);
    }
  }
  is_service_connected_ = true;
//  TangoCameraIntrinsics rgb_camera_intrinsics_ = CameraInterface::TangoGetIntrinsics();
//  int depth_image_width = rgb_camera_intrinsics_.width;
//  int depth_image_height = rgb_camera_intrinsics_.height;
//  mylogger = (new Mylogger(depth_image_width, depth_image_height));
  LOGI("setCamWidthAndheight1:width  %d", myImageWidth);
//  Mylogger logger;
  LOGI("setCamWidthAndheight1.5:height  %d", myImageHeight);
//  mylogger->setCamWidthAndheight(depth_image_width, depth_image_height);
//  LOGI("setCamWidthAndheight2:");
//  mylogger = &logger;
  if (loggerWH_callback_) {
      (*loggerWH_callback_)(myImageWidth, myImageHeight, myFx, myFy, myCx, myCy);
  }
//  LOGI("setCamWidthAndheight3:");
  if (writing_callback_) {
       (*writing_callback_)();
   }
  // TangoCoordinateFramePair is used to tell Tango Service about the frame of
	  // references that the applicaion would like to listen to.
//	  TangoCoordinateFramePair pair;
//	  pair.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
//	  pair.target = TANGO_COORDINATE_FRAME_DEVICE;
//	  if (TangoService_connectOnPoseAvailable(1, &pair, onPoseAvailable) !=
//	      TANGO_SUCCESS) {
//	    LOGE("Camera interface :: connectOnPoseAvailable error.");
//	    std::exit(EXIT_SUCCESS);
//	  }
  return true;
}
//
//void CameraInterface::setLogger(Mylogger* logger) {
////   mylogger = logger;
//}

void CameraInterface::disconnect() {
  if (tango_config_ != nullptr) {
    TangoConfig_free(tango_config_);
    tango_config_ = nullptr;
    TangoService_disconnect();
    is_service_connected_ = false;
//    TangoSupport_freePointCloudManager(point_cloud_manager_);
  }
  LOGI("CameraInterface: Finished disconnecting.");
}

void CameraInterface::render() {
  TangoErrorType status = TangoService_updateTexture(camera_type_, frame_timestamp_.get());
  if (status == TANGO_SUCCESS && (*frame_timestamp_) > 0) {
    if (gl_camera_frame_) {
      gl_camera_frame_->render();
      std::shared_ptr<unsigned char> frame = gl_camera_frame_->get_frame();
//      CameraInterface::OnDrawFrame(frame);
//      if (frame) {
//        if (raw_frame_callback_) {
////          RawFrameEvent raw_frame_event;
////          raw_frame_event.timestamp_nanoseconds = static_cast<int64_t>((*frame_timestamp_) * 1000000000.0);
////          raw_frame_event.data = frame;
////          (*raw_frame_callback_)(raw_frame_event);
//        }
//        if (marker_callback_ && ar_initialised_) {
//          MarkerEvent marker_event;
//          if (detect_markers(frame.get(),marker_event.marker_data)) {
//            marker_event.timestamp_nanoseconds = static_cast<int64_t>((*frame_timestamp_) * 1000000000.0);
//            (*marker_callback_)(marker_event);
//          } else {
//            LOGE("CameraInterface: There was a problem detecting markers");
//          }
//        }
//      }
//    }
//  }
//}
//
//void CameraInterface::OnDrawFrame(std::shared_ptr<unsigned char> frame) {
  // If tracking is lost, further down in this method Scene::Render
  // will not be called. Prevent flickering that would otherwise
  // happen by rendering solid black as a fallback.
//  main_scene_.Clear();
//
  if (!is_service_connected_) {
    return;
  }
//  LOGI("chck1");
//  mylogger->sayHello();
//  LOGI("chck2");
  double color_timestamp = 0.0;
  double depth_timestamp = 0.0;
  bool new_points = false;
  TangoPointCloud* pointcloud_buffer;
  TangoErrorType err = TangoSupport_getLatestPointCloudAndNewDataFlag(
      point_cloud_manager_, &pointcloud_buffer, &new_points);
  if (err != TANGO_SUCCESS) {
    LOGE("CameraInterface: Failed to TangoSupport_getLatestPointCloudAndNewDataFlag");
    return;
  }
  if (!new_points) {
	 LOGI("CameraInterface: point could data is not new, return");
	 return;
  } else {
	  LOGI("CameraInterface: point could data is new");
  }
  depth_timestamp = pointcloud_buffer->timestamp;
//  uint32_t num_points = pointcloud_buffer->num_points;
//  LOGI( "depth_timestamp: %f , %d, %d", depth_timestamp, new_points, num_points);
  // We need to make sure that we update the texture associated with the color
  // image.
  if (TangoService_updateTextureExternalOes(
          TANGO_CAMERA_COLOR, gl_camera_frame_->texture_id(), &color_timestamp) !=
      TANGO_SUCCESS) {
    LOGE("CameraInterface: Failed to get a color image.");
    return;
  }
  LOGI("camera type %d", camera_type_);
  //=============================
  TangoPoseData pose_color_image_t1_T_depth_image_t0;
   if (TangoSupport_calculateRelativePose(
		   color_timestamp, TANGO_COORDINATE_FRAME_CAMERA_COLOR, depth_timestamp,
           TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
           &pose_color_image_t1_T_depth_image_t0) != TANGO_SUCCESS) {
     LOGE(
         "SynchronizationApplication: Could not find a valid relative pose at "
         "time for color and "
         " depth cameras.");
     return;
   } else {
	 LOGI("color_timestamp: %f", color_timestamp);
	 LOGI("depth_timestamp: %f", depth_timestamp);
     LOGI("CameraInterface Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
          pose_color_image_t1_T_depth_image_t0.translation[0], pose_color_image_t1_T_depth_image_t0.translation[1], pose_color_image_t1_T_depth_image_t0.translation[2],
          pose_color_image_t1_T_depth_image_t0.orientation[0], pose_color_image_t1_T_depth_image_t0.orientation[1], pose_color_image_t1_T_depth_image_t0.orientation[2],
          pose_color_image_t1_T_depth_image_t0.orientation[3]);
   }




   return;
  //===============================

  // Define what motion is requested.
//  TangoService_Experimental_getPoseAtTime2(double timestamp,
//  TangoCoordinateFrameId base_frame_id, TangoCoordinateFrameId target_frame_id, TangoPoseData *return_pose)
  //
  TangoCoordinateFramePair frames_of_reference;
  frames_of_reference.base = TANGO_COORDINATE_FRAME_START_OF_SERVICE;
  frames_of_reference.target = TANGO_COORDINATE_FRAME_DEVICE;
  TangoPoseData pose;
  TangoErrorType e =   TangoService_getPoseAtTime(0.0, frames_of_reference, &pose);
  if (e == TANGO_SUCCESS) {
	LOGI("TangoService_getPoseAtTime success");
	 LOGI("onPoseAvailable: Timstamp: %f, status code: %d, Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
			  pose.timestamp, pose.status_code,
	       pose.translation[0], pose.translation[1], pose.translation[2],
	       pose.orientation[0], pose.orientation[1], pose.orientation[2],
	       pose.orientation[3]);
  } else {
  	LOGI("TangoService_getPoseAtTime failed");
  }
  //
  if (rgbd_callback_) {
//    	float* depth = &depth_map_buffer_[0];
          (*rgbd_callback_)(frame.get(), pointcloud_buffer, color_timestamp);
     }
    }
  }
}
  //
  //  }
//  main_scene_.Render(color_image_.GetTextureId(), depth_image_.GetTextureId(),
//                     color_camera_to_display_rotation_);
//}
//
//TangoCameraIntrinsics CameraInterface::TangoGetIntrinsics() {
//  // Get the intrinsics for the color camera and pass them on to the depth
//  // image. We need these to know how to project the point cloud into the color
//  // camera frame.
//  TangoCameraIntrinsics color_camera_intrinsics;
//  TangoErrorType err = TangoService_getCameraIntrinsics(
//      TANGO_CAMERA_COLOR, &color_camera_intrinsics);
//  if (err != TANGO_SUCCESS) {
//    LOGE(
//        "SynchronizationApplication: Failed to get the intrinsics for the color"
//        "camera.");
//    std::exit(EXIT_SUCCESS);
//  }
//
//  return color_camera_intrinsics;
////  depth_image_.SetCameraIntrinsics(color_camera_intrinsics);
//}

void CameraInterface::set_display_view_port(int width, int height) {
  if (gl_camera_frame_) {
    gl_camera_frame_->set_display_view_port(width,height);
  }
}

void CameraInterface::set_frame_view_port(int width, int height) {
  if (gl_camera_frame_) {
    gl_camera_frame_->set_frame_view_port(width,height);
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
void CameraInterface::process_frame_event(void* context, TangoCameraId id) {
  // Must request a render to receive on the OpenGL thread context
  request_render();
}

bool CameraInterface::setup_tango_config() {
  tango_config_ = TangoService_getConfig(TANGO_CONFIG_DEFAULT);
  if (tango_config_ == nullptr) {
    return false;
  }
  // This enables auto-exposure white-balancing with the colour camera
  TangoErrorType ret = TangoConfig_setBool(tango_config_, "config_enable_color_camera", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable color camera.");
    return false;
  }
  /*ret = TangoConfig_setBool(tango_config_, "config_color_mode_auto", false);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable auto-exposure.");
    return false;
  }*/
  // Absolutely every other option is explicitly turned off here.
  ret = TangoConfig_setBool(tango_config_, "config_enable_auto_recovery", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable recovery from motion tracking.");
    return false;
  }

  ret = TangoConfig_setBool(tango_config_, "config_enable_depth", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable depth.");
    return false;
  }
  ret = TangoConfig_setBool(tango_config_, "config_enable_low_latency_imu_integration", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable low latency imu integration.");
    return false;
  }
  ret = TangoConfig_setBool(tango_config_, "config_enable_motion_tracking", true);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to enable motion tracking.");
    return false;
  }
  ret = TangoConfig_setBool(tango_config_, "config_enable_learning_mode", false);
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
  //ret = TangoConfig_setBool(tango_config_, "config_enable_dataset_recording", false);
  //if (ret != TANGO_SUCCESS) {
  //  LOGE("Failed to disable dataset recording.");
  //  return false;
  //}
  /*
  ret = TangoConfig_setBool(tango_config_, "config_experimental_high_accuracy_small_scale_adf", false);
  if (ret != TANGO_SUCCESS) {
    LOGE("Failed to disable experimental high accuracy small scale ADF.");
    return false;
  }*/
  //======
//  ret = TangoConfig_setBool(tango_config_, "config_high_rate_pose", false);
//  if (ret != TANGO_SUCCESS) {
//    LOGE("Failed to disable high rate pose.");
//    return false;
//  }
//  ret = TangoConfig_setBool(tango_config_, "config_smooth_pose", false);
//  if (ret != TANGO_SUCCESS) {
//    LOGE("Failed to disable smooth pose.");
//    return false;
//  }
//  ret = TangoConfig_setBool(tango_config_, "config_experimental_enable_scene_reconstruction", false);
//  if (ret != TANGO_SUCCESS) {
//    LOGE("Failed to disable experimental scene reconstruction.");
//    return false;
//  }
  return true;
}

// JVM Functions
JNIEnv* CameraInterface::java_environment() {
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
    jmethodID request_render_ref = env->GetMethodID(activity_class, "requestRender", "()V");
    env->CallVoidMethod(activity_ref_, request_render_ref);
    env->DeleteLocalRef(activity_class);
  }
}
//
//// ARToolkit internal functions
//bool CameraInterface::initialise_artoolkit() {
//  // Setup camera parameters and ar handles
//  ar_param_lt_ = arParamLTCreate(&ar_param_, 15);
//  if (ar_param_lt_ == nullptr) {
//    destroy_artoolkit();
//    LOGE("An error occurred creating the ar distortion lookup table");
//    return false;
//  }
//  ar_handle_ = arCreateHandle(ar_param_lt_);
//  if (ar_handle_ == nullptr) {
//    destroy_artoolkit();
//    LOGE("An error occurred creating the ar handle");
//    return false;
//  }
//  arSetPixelFormat(ar_handle_,AR_PIXEL_FORMAT_MONO);
//  arSetPatternDetectionMode(ar_handle_,AR_MATRIX_CODE_DETECTION);
//  arSetMatrixCodeType(ar_handle_,AR_MATRIX_CODE_3x3);
//  ar_3dhandle_ = ar3DCreateHandle(&ar_param_);
//  if (ar_3dhandle_ == nullptr) {
//    destroy_artoolkit();
//    LOGE("An error occurred creating the ar 3D handle");
//    return false;
//  }
//  LOGI("ARToolkit successfully initialised");
//  return true;
//}
//
//void CameraInterface::destroy_artoolkit() {
//  if (ar_3dhandle_ != nullptr) {
//    ar3DDeleteHandle(&ar_3dhandle_);
//  }
//  if (ar_handle_ != nullptr) {
//    arDeleteHandle(ar_handle_);
//  }
//  if (ar_param_lt_ != nullptr) {
//    arParamLTFree(&ar_param_lt_);
//  }
//  ar_config_available_ = false;
//  ar_initialised_ = false;
//}
//
//bool CameraInterface::detect_markers(unsigned char* image_buffer,
//                                     std::vector<MarkerData>& markers) {
//  int result = arDetectMarker(ar_handle_, image_buffer);
//  if (result) {
//    LOGE("There was an error detecting markers");
//    return false;
//  }
//  int num_markers = arGetMarkerNum(ar_handle_);
//  ARMarkerInfo* detected_markers = arGetMarker(ar_handle_);
//  ARdouble marker_transformation_matrix[3][4];
//  for (int marker_idx = 0; marker_idx < num_markers; ++marker_idx) {
//    if (detected_markers[marker_idx].id >= 0) {
//      MarkerData marker_data;
//      marker_data.id = detected_markers[marker_idx].id;
//      for (int corner_idx = 0; corner_idx < 4; ++corner_idx) {
//        marker_data.estimated_raw_corners(0,corner_idx) =
//          detected_markers[marker_idx].raw_vertex[corner_idx][0];
//        marker_data.estimated_raw_corners(1,corner_idx) =
//          detected_markers[marker_idx].raw_vertex[corner_idx][1];
//        marker_data.estimated_idealised_corners(0,corner_idx) =
//          detected_markers[marker_idx].vertex[corner_idx][0];
//        marker_data.estimated_idealised_corners(1,corner_idx) =
//          detected_markers[marker_idx].vertex[corner_idx][1];
//      }
//      arGetTransMatSquare(ar_3dhandle_, &(ar_handle_->markerInfo[marker_idx]),80.0,
//                          marker_transformation_matrix);
//      marker_data.transformation_matrix = Eigen::Matrix4f::Identity();
//      for (int i = 0; i < 3; ++i) {
//        for (int j = 0; j < 4; ++j) {
//          marker_data.transformation_matrix(i,j) =
//            marker_transformation_matrix[i][j];
//        }
//      }
//      markers.push_back(marker_data);
//    }
//  }
//  return true;
//}
//
//void CameraInterface::load_ar_config(JNIEnv* env, jobject java_asset_manager) {
//  std::string camera_filename;
//  if (camera_type_ == TANGO_CAMERA_FISHEYE) {
//    camera_filename = "fisheye_params.dat";
//  } else if (camera_type_ == TANGO_CAMERA_COLOR) {
//    camera_filename = "color_params.dat";
//  }
//  AAssetManager* mgr = AAssetManager_fromJava(env,java_asset_manager);
//  AAssetDir* assetDir = AAssetManager_openDir(mgr, "");
//  const char* filename = (const char*)nullptr;
//  char writable[4096];
//  while ((filename = AAssetDir_getNextFileName(assetDir)) != nullptr) {
//    std::string filename_str(filename);
//    AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
//    if (filename_str == camera_filename) {
//      // Load the camera parameters into the ARParam struct
//      AAsset* asset = AAssetManager_open(mgr, filename, AASSET_MODE_BUFFER);
//      unsigned int size = AAsset_getLength(asset);
//      AAsset_read(asset,&writable,size);
//      writable[size] = '\0';
//      std::stringstream camera_param_ss(writable);
//      std::string parameter_line;
//      std::string parameter_name;
//      int parameter_int;
//      double parameter_double;
//      while(std::getline(camera_param_ss,parameter_line)) {
//        std::stringstream parameter_line_ss(parameter_line);
//        parameter_line_ss >> parameter_name;
//        if (parameter_name == std::string("xsize")) {
//          parameter_line_ss >> parameter_int;
//          ar_param_.xsize = parameter_int;
//        } else if (parameter_name == std::string("ysize")) {
//          parameter_line_ss >> parameter_int;
//          ar_param_.ysize = parameter_int;
//        } else if (parameter_name == std::string("mat")) {
//          int i, j;
//          parameter_line_ss >> i >> j >> parameter_double;
//          ar_param_.mat[i][j] = parameter_double;
//        } else if (parameter_name == std::string("dist_factor")) {
//          int i;
//          parameter_line_ss >> i >> parameter_double;
//          ar_param_.dist_factor[i] = parameter_double;
//        } else if (parameter_name == std::string("dist_function_version")) {
//          parameter_line_ss >> parameter_int;
//          ar_param_.dist_function_version = parameter_int;
//        }
//      }
//    }
//    AAsset_close(asset);
//  }
//  AAssetDir_close(assetDir);
//  ar_config_available_ = true;
//}

}
