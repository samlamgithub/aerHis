#ifndef __CAMERA_INTERFACE_HPP__
#define __CAMERA_INTERFACE_HPP__

//#include <AR/ar.h>
//#include <AR/arMulti.h>
#include <Eigen/Core>
#include <functional>
#include <jni.h>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"

#include "event_types/marker_event.hpp"
#include "event_types/raw_frame_event.hpp"
//#include "event_types/depth_event.hpp"
#include "gl_util.hpp"

//#include <tango-gl/util.h>
#include <atomic>
#include <jni.h>
#include <memory>
#include <string>

#include <opencv2/opencv.hpp>

// #include "mylogger.hpp"
#include "myElasticFusion.hpp"

// test
#include "../cudatest/nativeCUDA.cuh"

//#include <tango-gl/util.h>
#include <tango_client_api.h> // NOLINT
//#include <tango-gl/tango-gl.h>
//#include <tango-gl/util.h>
#include <tango_3d_reconstruction_api.h>
#include <tango_support_api.h>

namespace tango_interface {

// typedef std::function<void (const RawFrameEvent& event)> RawFrameCallBack;
// typedef std::function<void (const MarkerEvent& event)> MarkerCallBack;
typedef std::function<void(unsigned char *image,
                           TangoPointCloud *pointcloud_buffer,
                           double color_timestamp, TangoPoseData pose)>
    RGBDCallBack;
// typedef std::function<void (const int width, const int height, const double
// fx, const double fy, const double cx, const double cy)> LoggerWHCallBack;
typedef std::function<void(const int width, const int height, const double fx,
                           const double fy, const double cx, const double cy,
                           const int maxVerCount)>
    LoggerWHCallBack;
// typedef std::function<void ()> WritingCallBack;

class CameraInterface {
public:
  // Main interface methods are via callbacks
  //  static void register_raw_frame_callback(RawFrameCallBack function);
  //  static void register_marker_callback(MarkerCallBack function);
  static void register_rgbd_callback(RGBDCallBack function);
  static void register_loggerWidthHeight_callback(LoggerWHCallBack function);
  // static void register_writing_callback(WritingCallBack);
  // register_elasticfusion_callback

  static void onPointCloudAvailable2(const TangoPointCloud *point_cloud);
  //  static TangoCameraIntrinsics TangoGetIntrinsics();
  //  static void OnDrawFrame(std::shared_ptr<unsigned char> frame);
  // These provide the frame size for RawFrame events
  static int get_frame_height();
  static int get_frame_width();
  // The gl initialisation are separated as they must be called from the gl
  // thread
  static bool initialise(JNIEnv *env, jobject caller_activity,
                         jobject asset_manager);
  static void destroy();

  // These are used for every pause/resume event
  static bool connect();
  static void disconnect();

  // These are the main display variables
  static void render();
  static void set_display_view_port(int width, int height);
  static void set_frame_view_port(int width, int height);

  // DO NOT CALL: This must be public for the Tango API only
  static void process_frame_event(void *, TangoCameraId);
  static bool setup_tango_config();

  static int myImageHeight;
  static int myImageWidth;
  static int myImageSize;
  static double myFx;
  static double myFy;
  static double myCx;
  static double myCy;

private:
  static bool is_service_connected_;
  const static TangoCameraId camera_type_ = TANGO_CAMERA_COLOR;
  static TangoConfig tango_config_;
  static std::unique_ptr<double> frame_timestamp_;
  static uint32_t max_vertex_count;

  // OpenGl Content and methods
  static std::unique_ptr<GlCameraFrame> gl_camera_frame_;
  static void initialise_gl_content();
  static void destroy_gl_content();

  // These pointers store the functions for callbacks with event updates
  //  static std::unique_ptr<RawFrameCallBack> raw_frame_callback_;
  //  static std::unique_ptr<MarkerCallBack> marker_callback_;
  static std::unique_ptr<RGBDCallBack> rgbd_callback_;
  static std::unique_ptr<LoggerWHCallBack> loggerWH_callback_;
  // static std::unique_ptr<WritingCallBack> writing_callback_;

  // These are used for accessing the render_gl request
  static JNIEnv *java_environment();
  static void request_render();
  static JavaVM *jvm_;
  static jobject activity_ref_;

  // The point_cloud_manager allows for thread safe reading and
  // writing of the point cloud data.
  static TangoSupportPointCloudManager *point_cloud_manager_;

  // Private ARToolkit methods for marker detection
  //  static bool initialise_artoolkit();
  //  static void destroy_artoolkit();
  //  static void load_ar_config(JNIEnv* env, jobject asset_manager);
  //  static bool detect_markers(unsigned char* image_buffer,
  //  std::vector<MarkerData>& markers);
  //
  //  static bool ar_config_available_;
  //  static ARParam ar_param_;
  //  static ARParamLT* ar_param_lt_;
  //  static ARHandle* ar_handle_;
  //  static AR3DHandle* ar_3dhandle_;
  //  static bool ar_initialised_;
};
}

#endif
