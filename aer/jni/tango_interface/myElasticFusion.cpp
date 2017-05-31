#include "myElasticFusion.hpp"

#include <tango_interface/util.hpp>
// #include "logger.hpp"
#include "util.hpp"
#include <cstdarg>
#include <ctime>
#include <jni.h>
#include <memory>
#include <sys/stat.h>
#include <tango_client_api.h>
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/core/core_c.h>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/imgproc/imgproc_c.h>
//#include <opencv2/imgproc/types_c.h>
//#include <opencv2/highgui/highgui_c.h>

static const char *glErrorStringef(GLenum err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "Invalid Enum";
  case GL_INVALID_VALUE:
    return "Invalid Value";
  case GL_INVALID_OPERATION:
    return "Invalid Operation";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  // case GL_STACK_OVERFLOW: return "Stack Overflow";
  // case GL_STACK_UNDERFLOW: return "Stack Underflow";
  case GL_OUT_OF_MEMORY:
    return "error Out of Memory";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
  default:
    return "Unknown Error";
  }
}

inline void glCheckFramebufferStatusMyElasticFusioncpp() {
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    LOGI("MY elasitcfusion MyElasticFusioncpp GL_FRAMEBUFFER_COMPLETE");
  } else if (status == GL_FRAMEBUFFER_UNDEFINED) {
    LOGI("error MY elasitcfusion MyElasticFusioncpp  GL_FRAMEBUFFER_UNDEFINED");
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    LOGI("error MY elasitcfusion MyElasticFusioncpp "
         "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    LOGI("error MY elasitcfusion MyElasticFusioncpp "
         "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    LOGI("error MY elasitcfusion MyElasticFusioncpp  GL_FRAMEBUFFER_UNSUPPORTED");
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    LOGI("error MY elasitcfusion MyElasticFusioncpp  "
         "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE");
  } else if (status == GL_INVALID_ENUM) {
    LOGI("error MY elasitcfusion MyElasticFusioncpp GL_INVALID_ENUM");
  } else {
    LOGI(" error MY elasitcfusion MyElasticFusioncpp glCheckFramebufferStatus else %d",
         status);
  }
}

inline void check_gl_errorEF() {
  glCheckFramebufferStatusMyElasticFusioncpp();
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_error MyElasticFusion cpp My elastic-fusion "
         "CheckGlDieOnError after "
         "%s() glError (0x%x)\n",
         glErrorStringef(error), error);
  }
}

namespace tango_interface {

MyElasticFusion::MyElasticFusion()
    : lastProcessed(-1), elasticFusionThread(0), depth_image_width(0),
      depth_image_height(0) {
  //    std::string deviceId = "#1";
  //    setupDevice(deviceId);
}

MyElasticFusion::~MyElasticFusion() {
  free(depth_compress_buf);
  runningElasticFusion.assignValue(false);
  shouldSavePly.assignValue(false);
  elasticFusionThread->join();
  // if(encodedImage != 0) {
  //     cvReleaseMat(&encodedImage);
  // }
  //
  //    for(int i = 0; i < 10; i++)
  //    {
  //        free(imageBuffers[i].first);
  //    }

  //    for(int i = 0; i < 10; i++)
  //    {
  //        free(frameBuffers[i].first.first);
  //        free(frameBuffers[i].first.second);
  //    }
  for (int i = 0; i < 50; i++) {
    free(frameBuffers[i].image);
    free(frameBuffers[i].pointCloudPoints);
    //            delete frameBuffers[i];
    //            frameBuffers[i] = NULL;
  }
}

void MyElasticFusion::setCamWidthAndheight(int width, int height, double fx,
                                           double fy, double cx, double cy,
                                           int maxVerCount) {
  LOGI("MyElasticFusion setCamWidthAndheight start");
  depth_image_width = width, depth_image_height = height;
  myImageSize = width * height;
  myFx = fx;
  myFy = fy;
  myCx = cx;
  myCy = cy;
  LOGI("MyElasticFusion setCamWidthAndheight intrinsic: %d, %d, %f, %f, %f, %f "
       ", myImageSize: %d",
       depth_image_width, depth_image_height, fx, fy, cx, cy, myImageSize);
  // int imageSize = width * height;
  depth_compress_buf_size = myImageSize * sizeof(int16_t) * 4;
  depth_compress_buf = (unsigned char *)malloc(depth_compress_buf_size);
  // encodedImage = 0;
  runningElasticFusion.assignValue(false);
  shouldSavePly.assignValue(false);
  //	    latestDepthIndex.assignValue(-1);
  latestBufferIndex.assignValue(-1);
  //	    for(int i = 0; i < 10; i++)
  //	    {
  //	        unsigned char * newImage = (unsigned char *)calloc(imageSize *
  // 3, sizeof(unsigned char)); 	        imageBuffers[i] =
  // std::pair<unsigned  char *, int64_t>(newImage, 0);
  //	    }
  //	    for(int i = 0; i < 10; i++) {
  for (int i = 0; i < 50; i++) {
    //	        unsigned char * newDepth = (unsigned char *)calloc(myImageSize *
    // 2, sizeof(unsigned char));
    float *newDepth = (float *)calloc(4 * maxVerCount, sizeof(float));
    unsigned char *newImage =
        (unsigned char *)calloc(myImageSize * 4, sizeof(unsigned char));
    //	        frameBuffers[i] = std::pair<std::pair<unsigned char *, unsigned
    // char *>, int64_t>(std::pair<unsigned char *, unsigned char *>(newDepth,
    // newImage), 0);
    struct ElasticFusionData rgbdData;
    rgbdData.colorTimeStamp = 0.0;
    rgbdData.image = newImage;
    rgbdData.pointCloudTimestamp = 0.0;
    rgbdData.pointCloudNumpoints = 0;
    rgbdData.pointCloudPoints = newDepth;
    rgbdData.m_lastTimestamp = 0;
    //	       rgbdData.pose = NULL;
    frameBuffers[i] = rgbdData;
  }
  LOGI("MyElasticFusion setCamWidthAndheight done");
}
//
// void MyElasticFusion::encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data) {
// 	LOGI("Logger Encoding start: %d, %d", depth_image_height,
// depth_image_width); 	int step = depth_image_width*3*sizeof(unsigned char);
// //	LOGI("step: %d", step);
//     cv::Mat3b rgb(depth_image_height, depth_image_width, rgb_data, step);
// //    LOGI("depth: %d, channels: %d, row: %d, cols : %d", rgb.depth(),
// rgb.channels(),rgb.rows, rgb.cols);
// //    if (rgb.isContinuous())  {
// //    	LOGI("is continuous");
// //    } else {
// //    	LOGI("is not continuous");
// //    }
//     IplImage * img = new IplImage(rgb);
//
//     int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};
//
//     if(encodedImage != 0)
//     {
//         cvReleaseMat(&encodedImage);
//     }
//
//     encodedImage = cvEncodeImage(".jpg", img, jpeg_params);
//
//     delete img;
//     LOGI("Logger Encoding done");
// }

void MyElasticFusion::rgbdCallback(unsigned char *image,
                                   TangoPointCloud *pointcloud_buffer,
                                   double color_timestamp, TangoPoseData pose) {
  LOGI("MyElasticFusion  thread rgbdCallback start ");
  //===========================================================
  boost::posix_time::ptime time =
      boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration duration(time.time_of_day());
  m_lastFrameTime = duration.total_microseconds();
  //    int bufferIndex = (latestBufferIndex.getValue() + 1) % 10;
  int bufferIndex = (latestBufferIndex.getValue() + 1) % 50;
  //    for (auto const& c : depth_map_buffer_) {
  //    	 LOGI("dep: %d", c);
  //    }
  //     std::string depStr(depth_map_buffer_.begin(), depth_map_buffer_.end());
  //     LOGI("dep: %s", depStr.c_str());
  //    LOGI("size of float: %d", sizeof(float));
  //    LOGI("size of unsigned char: %d", sizeof(unsigned char));
  //    LOGI("size of int16_t: %d", sizeof(int16_t));
  //    LOGI("size of unsigned char: %d", sizeof(unsigned char));
  //    LOGI("size of unsigned short: %d", sizeof(unsigned short));
  //    LOGI("size of short: %d", sizeof(short));
  //    double  pointCloudTimestamp;
  //      uint32_t  pointCloudNumpoints;
  //      float *  pointCloudPoints;
  //      unsigned char *   image;
  //      double colorTimeStamp;
  //      int64_t m_lastTimestamp;
  frameBuffers[bufferIndex].pointCloudTimestamp = pointcloud_buffer->timestamp;
  frameBuffers[bufferIndex].pointCloudNumpoints = pointcloud_buffer->num_points;
  memcpy(frameBuffers[bufferIndex].pointCloudPoints, pointcloud_buffer->points,
         (pointcloud_buffer->num_points) * 4 * sizeof(float));
  int rgbPixeldatacount = myImageSize * 4;
  memcpy(frameBuffers[bufferIndex].image, image, rgbPixeldatacount);
  frameBuffers[bufferIndex].colorTimeStamp = color_timestamp;
  frameBuffers[bufferIndex].m_lastTimestamp = m_lastFrameTime;
  frameBuffers[bufferIndex].pose = pose;
  //    memcpy(frameBuffers[bufferIndex].first.first, reinterpret_cast<unsigned
  //    char*>(depth), myImageSize * 2); int rgbPixeldatacount = myImageSize *
  //    3; LOGI("rgbPixeldatacount %d", rgbPixeldatacount);
  //    memcpy(frameBuffers[bufferIndex].first.second, reinterpret_cast<unsigned
  //    char*>(image), rgbPixeldatacount); frameBuffers[bufferIndex].second =
  //    m_lastFrameTime;
  latestBufferIndex++;
  LOGI("MyElasticFusion thread rgbdCallback done ");
}

glm::mat4 MyElasticFusion::GetMatrixFromPose(const TangoPoseData *pose_data) {
  glm::vec3 translation =
      glm::vec3(pose_data->translation[0], pose_data->translation[1],
                pose_data->translation[2]);
  glm::quat rotation =
      glm::quat(pose_data->orientation[3], pose_data->orientation[0],
                pose_data->orientation[1], pose_data->orientation[2]);
  return glm::translate(glm::mat4(1.0f), translation) *
         glm::mat4_cast(rotation);
}

// void MyElasticFusion::UpdateAndUpsampleDepth(const glm::mat4&
// color_t1_T_depth_t0, 		const TangoPointCloud*
// render_point_cloud_buffer, 		std::vector<unsigned short>
//&depth_map_buffer_) {
void MyElasticFusion::UpdateAndUpsampleDepth(
    const glm::mat4 &color_t1_T_depth_t0,
    const float *render_point_cloud_buffer,
    std::vector<unsigned short> &depth_map_buffer_, int point_cloud_size) {
  //  TangoCameraIntrinsics rgb_camera_intrinsics_ =
  //  CameraInterface::TangoGetIntrinsics(); int depth_image_width =
  //  rgb_camera_intrinsics_.width; int depth_image_height =
  //  rgb_camera_intrinsics_.height; int depth_image_size = myImageHeight *
  //  myImageWidth;
  ////  std::vector<float> depth_map_buffer_;
  //	LOGI("UpdateAndUpsampleDepth intrinsic: %d, %d, %d,  %f, %f, %f, %f
  //",depth_image_width , depth_image_height,
  //			myImageSize, myFx, myFy, myCx, myCy);
  depth_map_buffer_.resize(myImageSize);
  //  grayscale_display_buffer_.resize(depth_image_size);
  std::fill(depth_map_buffer_.begin(), depth_map_buffer_.end(), 0);
  //  std::fill(grayscale_display_buffer_.begin(),
  //  grayscale_display_buffer_.end(),
  //            0);
  //  int point_cloud_size = render_point_cloud_buffer->num_points;
  for (int i = 0; i < point_cloud_size; ++i) {
    //    float x = render_point_cloud_buffer->points[i][0];
    //    float y = render_point_cloud_buffer->points[i][1];
    //    float z = render_point_cloud_buffer->points[i][2];
    float x = render_point_cloud_buffer[3 * i];
    float y = render_point_cloud_buffer[3 * i + 1];
    float z = render_point_cloud_buffer[3 * i + 2];
    //    LOGI("UpdateAndUpsampleDepth 1: %f, %f ,%f ", x, y, z);
    // depth_t0_point is the point in depth camera frame on timestamp t0.
    // (depth image timestamp).
    glm::vec4 depth_t0_point = glm::vec4(x, y, z, 1.0);
    // color_t1_point is the point in camera frame on timestamp t1.
    // (color image timestamp).
    glm::vec4 color_t1_point = color_t1_T_depth_t0 * depth_t0_point;
    int pixel_x, pixel_y;
    //    LOGI("UpdateAndUpsampleDepth 2: %f, %f ,%f ",color_t1_point.x,
    //    color_t1_point.y, color_t1_point.z); LOGI("UpdateAndUpsampleDepth 2:
    //    %f, %f ,%f, | %f, %f ,%f ", x, y, z, color_t1_point.x,
    //    color_t1_point.y, color_t1_point.z);
    // get the coordinate on image plane.
    pixel_x =
        static_cast<int>((myFx) * (color_t1_point.x / color_t1_point.z) + myCx);
    pixel_y =
        static_cast<int>((myFy) * (color_t1_point.y / color_t1_point.z) + myCy);
    //    LOGI("UpdateAndUpsampleDepth 3: %d, %d ",pixel_x,pixel_y);
    // Color value is the GL_LUMINANCE value used for displaying the depth
    // image.
    // We can query for depth value in mm from grayscale image buffer by
    // getting a `pixel_value` at (pixel_x,pixel_y) and calculating
    // pixel_value * (kMaxDepthDistance / USHRT_MAX)
    float depth_value = color_t1_point.z;
    //    unsigned char grayscale_value =
    //        (color_t1_point.z * kMeterToMillimeter) * UCHAR_MAX /
    //        kMaxDepthDistance;
    UpSampleDepthAroundPoint(depth_value, pixel_x, pixel_y, &depth_map_buffer_);
  }
  //
  //  this->CreateOrBindCPUTexture();
  //  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, depth_image_width,
  //  depth_image_height,
  //                  GL_LUMINANCE, GL_UNSIGNED_BYTE,
  //                  grayscale_display_buffer_.data());
  //  tango_gl::util::CheckGlError("DepthImage glTexSubImage2D");
  //  glBindTexture(GL_TEXTURE_2D, 0);
  //
  //  texture_id_ = cpu_texture_id_;
}

// Window size for splatter upsample
static const int kWindowSize = 7;

void MyElasticFusion::UpSampleDepthAroundPoint(
    float depth_value, int pixel_x, int pixel_y,
    std::vector<unsigned short> *depth_map_buffer) {
  //  int image_width = rgb_camera_intrinsics_.width;
  //  int image_height = rgb_camera_intrinsics_.height;
  //  int image_size = myImageHeight * myImageWidth;
  // Set the neighbour pixels to same color.
  //    LOGI("UpSampleDepthAroundPoint ===: %d, %d ,%d , %d, %d, %f", pixel_x,
  //    pixel_y, depth_image_width, depth_image_height,
  //    		(unsigned short)round(depth_value * 1000), depth_value);
  for (int a = -kWindowSize; a <= kWindowSize; ++a) {
    for (int b = -kWindowSize; b <= kWindowSize; ++b) {
      if (pixel_x > depth_image_width || pixel_y > depth_image_height ||
          pixel_x < 0 || pixel_y < 0) {
        //    	  LOGI("UpSampleDepthAroundPoint failed");
        //    	  LOGI("UpSampleDepthAroundPoint failed: %d, %d ,%d , %d, %f",
        //    pixel_x, pixel_y, depth_image_width, depth_image_height,
        //    depth_value);
        continue;
      }
      int pixel_num = (pixel_x + a) + (pixel_y + b) * depth_image_width;
      if (pixel_num > 0 && pixel_num < myImageSize) {
        (*depth_map_buffer)[pixel_num] =
            (unsigned short)round(depth_value * 1000);
        //        LOGI("UpSampleDepthAroundPoint success");
        //        LOGI("UpSampleDepthAroundPoint success: %d, %d ,%d , %d, %d,
        //        %f", pixel_x, pixel_y, depth_image_width, depth_image_height,
        //        		(*depth_map_buffer)[pixel_num], depth_value);
      }
    }
  }
}

void MyElasticFusion::startElasticFusion() {
  // if (elasticFusionThread) {
  //   LOGI("MyElasticFusion startElasticFusion elasticFusionThread yes");
  // } else {
  //     LOGI("MyElasticFusion startElasticFusion elasticFusionThread no");
  // }
  // if (runningElasticFusion.getValue()) {
  //   LOGI("MyElasticFusion startElasticFusion runningElasticFusion yes");
  // } else {
  //     LOGI("MyElasticFusion startElasticFusion runningElasticFusion no");
  // }
  assert(!elasticFusionThread && !runningElasticFusion.getValue());
  LOGI("MyElasticFusion startElasticFusion running");
  //    this->filename = filename;
  runningElasticFusion.assignValue(true);
  elasticFusionThread =
      new boost::thread(boost::bind(&MyElasticFusion::runEF, this));
  LOGI("MyElasticFusion startElasticFusion running done");
}

void MyElasticFusion::stopElasticFusion() {
  assert(elasticFusionThread && runningElasticFusion.getValue());
  LOGI("MyElasticFusion stopElasticFusion running");
  runningElasticFusion.assignValue(false);
  elasticFusionThread->join();
  elasticFusionThread = 0;
  LOGI("MyElasticFusion stopElasticFusion running done");
  // LOGI("logger stop logging3");
}

void MyElasticFusion::savePly() {
  assert(elasticFusionThread && runningElasticFusion.getValue());
  LOGI("MyElasticFusion is running, should save ply");
  shouldSavePly.assignValue(true);
  LOGI("MyElasticFusion is running, should save ply done");
}

bool MyElasticFusion::file_exists(const std::string &filename) const {
  struct stat st;
  int result = stat(filename.c_str(), &st);
  return (result == 0);
}

std::string MyElasticFusion::to_string(int value) const {
  char buf[128];
  snprintf(buf, 128, "%d", value);
  return std::string(buf);
}

std::string MyElasticFusion::current_date_time() const {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
  return std::string(buf);
}

void MyElasticFusion::runEF() {
  /**
   * int32_t at file beginning for frame count
   */
  // std::string RGBfilename("/sdcard/mymy_imperial_tango_RGB" +
  // current_date_time());
  //  int RGBversion = 0;
  //   std::string RGBversion_suffix(".klg");
  //   while (file_exists(RGBfilename + RGBversion_suffix)) {
  //     RGBversion_suffix = "_" + to_string(++RGBversion) + ".klg";
  //   }
  //   // Finish opening the file
  //   RGBfilename += RGBversion_suffix;
  //   RGBlog_file_ = fopen(RGBfilename.c_str(),"wb+");
  //   if (RGBlog_file_ == NULL) {
  //     LOGE("Logger: There was a problem opening the RGB log
  //     file:%s",RGBfilename.c_str());
  //   }
  //
  //   std::string Depthfilename("/sdcard/mymy_imperial_tango_Depth" +
  //   current_date_time()); 	 int Depthversion = 0; 	  std::string
  //   Depthversion_suffix(".klg"); 	  while (file_exists(Depthfilename +
  //   Depthversion_suffix)) { 		Depthversion_suffix = "_" +
  //   to_string(++Depthversion) + ".klg";
  //   	  }
  //   	  // Finish opening the file
  //   	Depthfilename += Depthversion_suffix;
  //   	Depthlog_file_ = fopen(Depthfilename.c_str(),"wb+");
  //   	  if (Depthlog_file_ == NULL) {
  //   	    LOGE("Logger: There was a problem opening the Depth log
  //   file:%s",Depthfilename.c_str());
  //   	  }
  //==================================
  LOGI("MyElasticFusion runEF egl context setup start ...");
  const EGLint configAttribs[] = {EGL_RENDERABLE_TYPE,
                                  EGL_OPENGL_ES2_BIT,
                                  EGL_SURFACE_TYPE,
                                  EGL_PBUFFER_BIT,
                                  EGL_BLUE_SIZE,
                                  8,
                                  EGL_GREEN_SIZE,
                                  8,
                                  EGL_RED_SIZE,
                                  8,
                                  EGL_ALPHA_SIZE,
                                  8,
                                  EGL_DEPTH_SIZE,
                                  16,
                                  EGL_NONE};

  const EGLint contextAttribs[] = {EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE};

  EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display == EGL_NO_DISPLAY || eglGetError() != EGL_SUCCESS) {
    LOGI("MyElasticFusion runEF egl  eglGetDisplay error failed");
  }

  EGLint major, minor;
  if (!eglInitialize(display, &major, &minor) || eglGetError() != EGL_SUCCESS) {
    LOGI("MyElasticFusion runEF egl  eglInitialize error failed");
  }
  LOGI("MyElasticFusion  runEFEGL init with version %d.%d", major, minor);
  EGLint numConfigs;
  EGLConfig config;
  if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) ||
      eglGetError() != EGL_SUCCESS) {
    LOGI("MyElasticFusion runEF egl eglChooseConfig error failed");
  }

  EGLSurface surface;
  const EGLint surfaceAttr[] = {EGL_WIDTH, 1280, EGL_HEIGHT, 720, EGL_NONE};

  EGLContext context;

  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS) {
    LOGI("MyElasticFusion runEF egl eglCreateContext error failed");
  }

  // PixmapSurface和PBufferSurface
  surface = eglCreatePbufferSurface(display, config, surfaceAttr);
  //	    surface = eglCreateWindowSurface(display, config, mWindow, NULL);
  if (surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS) {
    LOGI("MyElasticFusion runEF egl eglCreatePbufferSurface  error failed");
  }

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE ||
      eglGetError() != EGL_SUCCESS) {
    LOGI("MyElasticFusion runEF egl eglMakeCurrent error failed");
  }

  if (eglGetError() != EGL_SUCCESS) {
    LOGI("MyElasticFusion runEF egl may failed error");
  }

  LOGI("MyElasticFusion runEF egl context setup done ...");
  //==================================
  bool success = LoadOpenGLExtensionsManually();
  if (!success) {
    LOGE("MyElasticFusion runEF LoadOpenGLExtensionsManually failed");
  } else {
    LOGE("MyElasticFusion runEF LoadOpenGLExtensionsManually success");
  }
  //============================
  LOGI("MyElasticFusion runEF elasticfusion Initialising ... myFx: %d, myFy: %d, myCx: %d, myCy: %d", myFx, myFy, myCx, myCy);
  Resolution::getInstance(depth_image_width, depth_image_height);
  Intrinsics::getInstance(myFx, myFy, myCx, myCy);
  LOGI("MyElasticFusion runEF elasticfusion Initialising done ...");
  LOGI("MyElasticFusion runEF elasticfusion Setting parameters...");
  float confidence = 10.0f;   // fusion的confidence阈值
  float depth = 12.0f;         //去掉depth大于某个阈值的帧
  float icp = 10.0f;          // icp的阈值
  float icpErrThresh = 5e-05; // icp错误阈值
  float covThresh = 1e-05;
  float photoThresh = 115;
  float fernThresh = 0.3095f; //新fern的阈值
  int timeDelta = 200;
  int icpCountThresh = 35000;
  // int start = 1;
  // int end = std::numeric_limits<unsigned short>::max(); //Funny bound, since
  // we predict times in this format really!
  // bool openLoop = 0; //open loop模式：不开启
  bool openLoop = 1; // open loop模式：开启
  bool iclnuim = 0;  //使用icl dataset:不使用
  bool reloc = 0;    //重定位模式：先做重建，不开启重定位
  // bool fastOdom = 0; //Fast odometry (single level pyramid) mode :不开启
  bool fastOdom = 1; // Fast odometry (single level pyramid) mode :开启
  // bool so3 = 1; //SO(3) pre-alignment in tracking：开启
  bool so3 = 0;             // SO(3) pre-alignment in tracking：不开启
  bool frameToFrameRGB = 0; //只做rgb图像的tracking：不开启
  int timestamp = 0;
  LOGI("MyElasticFusion runEF elasticfusion Setting parameters done.");
  LOGI("MyElasticFusion runEF elasticfusion Building eFusion...");
  // pangolin::Params windowParams;
  // windowParams.Set("SAMPLE_BUFFERS", 0);
  // windowParams.Set("SAMPLES", 0);
  // pangolin::CreateWindowAndBind("Main", width, height, windowParams);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  ElasticFusion eFusion(openLoop ? std::numeric_limits<int>::max() / 2
                                 : timeDelta,
                        icpCountThresh, icpErrThresh, covThresh, !openLoop,
                        iclnuim, reloc, photoThresh, confidence, depth, icp,
                        fastOdom, fernThresh, so3, frameToFrameRGB);
  LOGI("MyElasticFusion runEF elasticfusion Building eFusion done");
  check_gl_errorEF();
  //待处理文件的位置和下标
  // std::string filedir = "../pic/";
  // int file_start = 1;
  // int file_end = 782;
  std::vector<Eigen::Matrix4f> posesEigen;
  int32_t numFrames = 0;
  // size_t result1 = fwrite(&numFrames, sizeof(int32_t), 1, RGBlog_file_);
  // LOGI("Logger fwrite: %d", result1);
  //    int result = fputs("\n:testest     \n", log_file_);
  //    LOGI("Logger puts: %d", result);
  Eigen::Matrix4f *previousPose = 0;
  previousPose = new Eigen::Matrix4f;
  previousPose->setIdentity();
  LOGI("MyElasticFusion runEF elasticfusion: good");
  while (runningElasticFusion.getValueWait(1)) {
    // LOGI("MyElasticFusion runEF elasticfusion: while loop in");
    int bufferIndex = latestBufferIndex.getValue();
    if (bufferIndex == -1) {
      continue;
    }
    //   bufferIndex = bufferIndex % 10;
    bufferIndex = bufferIndex % 50;
    if (bufferIndex == lastProcessed) {
      continue;
    }
    //==============================================
    LOGI("MyElasticFusion thread Processing start ,bufferIndex: %d, "
         "lastProcessed: %d",
         bufferIndex, lastProcessed);
    // double depth_timestamp = 0.0;
    //  depth_timestamp = pointcloud_buffer->timestamp;
    double depth_timestamp = frameBuffers[bufferIndex].pointCloudTimestamp;
    // uint32_t num_points = pointcloud_buffer->num_points;
    uint32_t num_points = frameBuffers[bufferIndex].pointCloudNumpoints;
    LOGI("MyElasticFusion depth_timestamp: %f , num_points:  %d",
         depth_timestamp, num_points);
    double color_timestamp = frameBuffers[bufferIndex].colorTimeStamp;
    // In the following code, we define t0 as the depth timestamp and t1 as the
    // color camera timestamp.
    // Calculate the relative pose between color camera frame at timestamp
    // color_timestamp t1 and depth camera frame at depth_timestamp t0.
    TangoPoseData pose_color_image_t1_T_depth_image_t0;
    TangoErrorType err = TangoSupport_calculateRelativePose(
        color_timestamp, TANGO_COORDINATE_FRAME_CAMERA_COLOR, depth_timestamp,
        TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
        &pose_color_image_t1_T_depth_image_t0);
    if (err == TANGO_SUCCESS) {
      LOGI("CameraInterface MyElasticFusion: success get valid relative pose "
           "at %f time for color and depth cameras :%f , color > depth: %d",
           color_timestamp, depth_timestamp, color_timestamp > depth_timestamp);
    } else {
      LOGE("CameraInterface MyElasticFusion: Could not find a valid relative "
           "pose at %f time for color and depth cameras :%f ",
           color_timestamp, depth_timestamp);
      if (err == TANGO_INVALID) {
        LOGE("CameraInterface MyElasticFusion TANGO_INVALID");
      }
      if (err == TANGO_ERROR) {
        LOGE("CameraInterface MyElasticFusion TANGO_ERROR");
      }
      return;
    }
    //	  LOGI("CameraInterface 1 Position: %f, %f, %f. Orientation: %f, %f, %f,
    //%f",
    //	  		  pose_color_image_t1_T_depth_image_t0.translation[0],
    // pose_color_image_t1_T_depth_image_t0.translation[1],
    // pose_color_image_t1_T_depth_image_t0.translation[2],
    //	            pose_color_image_t1_T_depth_image_t0.orientation[0],
    // pose_color_image_t1_T_depth_image_t0.orientation[1],
    // pose_color_image_t1_T_depth_image_t0.orientation[2],
    //	            pose_color_image_t1_T_depth_image_t0.orientation[3]);
    //	  LOGI("CameraInterface 1 status code: %d",
    // pose_color_image_t1_T_depth_image_t0.status_code);
    //	  LOGI("CameraInterface 1 accuracy : %f",
    // pose_color_image_t1_T_depth_image_t0.accuracy); 	LOGI("CameraInterface 1
    // confidence: %d",   pose_color_image_t1_T_depth_image_t0.confidence);
    //	return;
    if (std::isnan(pose_color_image_t1_T_depth_image_t0.translation[0])) {
      LOGI("CameraInterface MyElasticFusion Position: is Nan");
      return;
    }
    //	  double x =  pose_color_image_t1_T_depth_image_t0.orientation[0];
    //	  double y =  pose_color_image_t1_T_depth_image_t0.orientation[1];
    //	  double z =  pose_color_image_t1_T_depth_image_t0.orientation[2];
    //	  double w =  pose_color_image_t1_T_depth_image_t0.orientation[3];
    //	  pose_color_image_t1_T_depth_image_t0.orientation[0] = w;
    //	  pose_color_image_t1_T_depth_image_t0.orientation[1] = x;
    //	  pose_color_image_t1_T_depth_image_t0.orientation[2] = y;
    //	  pose_color_image_t1_T_depth_image_t0.orientation[3] = z;
    // The Color Camera frame at timestamp t0 with respect to Depth
    // Camera frame at timestamp t1.
    glm::mat4 color_image_t1_T_depth_image_t0 =
        GetMatrixFromPose(&pose_color_image_t1_T_depth_image_t0);
    //  if (gpu_upsample_) {
    //    depth_image_.RenderDepthToTexture(color_image_t1_T_depth_image_t0,
    //                                      pointcloud_buffer, new_points);
    //  } else {
    std::vector<unsigned short> depth_map_buffer_;
    //        	  UpdateAndUpsampleDepth(color_image_t1_T_depth_image_t0,
    //        	                                        pointcloud_buffer,
    //        depth_map_buffer_);
    UpdateAndUpsampleDepth(color_image_t1_T_depth_image_t0,
                           frameBuffers[bufferIndex].pointCloudPoints,
                           depth_map_buffer_, num_points);
    //  TangoCameraIntrinsics rgb_camera_intrinsics_ =
    //  CameraInterface::TangoGetIntrinsics();
    //    int depth_image_width = rgb_camera_intrinsics_.width;
    //    int depth_image_height = rgb_camera_intrinsics_.height;
    //    int depth_image_size = depth_image_width * depth_image_height;
    //	    if (rgbd_callback_) {
    unsigned short *depthForEF = &depth_map_buffer_[0];
    //	          (*rgbd_callback_)(frame.get(), depth, color_timestamp);
    //	     }
    LOGI("MyElasticFusion thread rgbdCallback Processing done ");
    unsigned long long int timeStampleForEF =
        frameBuffers[bufferIndex].m_lastTimestamp;
    unsigned char *rgbImageForEF = frameBuffers[bufferIndex].image;
    TangoPoseData pose = frameBuffers[bufferIndex].pose;
    // Eigen::Matrix4f * currentPose = 0;
    Eigen::Quaternion<float> quaternion = Eigen::Quaternion<float>(
        (float)pose.orientation[3], (float)pose.orientation[0],
        (float)pose.orientation[1], (float)pose.orientation[2]);
    Eigen::Matrix3f rot = Eigen::Quaternionf((float)pose.orientation[3],
                                             (float)pose.orientation[0],
                                             (float)pose.orientation[1],
                                             (float)pose.orientation[2])
                              .toRotationMatrix();
    Eigen::Matrix4f *currentPose = 0;
    currentPose = new Eigen::Matrix4f;
    currentPose->setIdentity();
    currentPose->block(0, 0, 3, 3) = rot;
    Eigen::Vector4f trans((float)pose.translation[0],
                          (float)pose.translation[1],
                          (float)pose.translation[2], 1.00);
    currentPose->rightCols<1>() = trans;

    Eigen::IOFormat CleanFmt1(4, 0, ", ", "\n", "[", "]");
    std::stringstream ss1;
    ss1 << currentPose->format(CleanFmt1);
    std::string str1(ss1.str());
    LOGI("input pose is : %s", str1.c_str());
    Eigen::Matrix4f *incrementalTrans = 0;
    incrementalTrans = new Eigen::Matrix4f;
    incrementalTrans->setIdentity();
    *incrementalTrans = previousPose->inverse() * (*currentPose);
    // float x = (float)pose.translation[0];
    // float y = (float)pose.translation[1];
    // float z =(float) pose.translation[2];
    // float qx = (float)pose.orientation[0];
    // float qy = (float)pose.orientation[1];
    // float qz = (float)pose.orientation[2];
    // float qw = (float)pose.orientation[3];
    //三位重建
    // for (int file_ptr = file_start; file_ptr <= file_end; file_ptr = file_ptr
    // +1) { LOGI("Processing frame : "<<file_ptr << " ..."); FRAME f =
    // readFrame(file_ptr, filedir); unsigned char * rgb = f.rgb; unsigned short
    // * dep = f.dep; Eigen::Matrix4f * currentPose;//当前的位姿
    //  currentPose = new Eigen::Matrix4f;
    //  currentPose->setIdentity();
    //  *currentPose = groundTruthOdometry->getTransformation(timestamp); }
    LOGI("MyElasticFusion Processing frames ready.");
    check_gl_errorEF();
    eFusion.processFrame(rgbImageForEF, depthForEF, timeStampleForEF,
                         incrementalTrans);
    *previousPose = *currentPose;
    delete &trans;
    delete incrementalTrans;
    delete currentPose;
    // eFusion.processFrame(rgb, dep, timestamp);
    check_gl_errorEF();
    LOGI("MyElasticFusion Processing frames ready done.");
    check_gl_errorEF();
    Eigen::Matrix4f currPose = eFusion.getCurrPose();
    posesEigen.push_back(currPose);
    Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
    std::stringstream ss;
    ss << currPose.format(CleanFmt);
    std::string str(ss.str());
    LOGI("output pose is : %s", str.c_str());
    LOGI("MyElasticFusion Processing frames done.");
    LOGI("MyElasticFusion Log processing result start.");
    //查看处理的结果
    delete &currPose;
    int time = eFusion.getTick(); //查看此时eFusion的系统时间
    LOGI("MyElasticFusion Log processing result time : %d ", time);

    //  Ferns keyfern = eFusion.getFerns();//关键帧dataset

    int ld_num = eFusion.getDeforms(); //局部deformations的数量
    // Deformation ld = eFusion.getLocalDeformation(); //局部deformation图

    int gd_num = eFusion.getFernDeforms(); //全局deformations的数量
    // GlobalModel gm = eFusion.getGlobalModel(); //全局deformation model:

    int CloudPoint_num = eFusion.getGlobalModel().lastCount(); //点云的点数量
    LOGI("MyElasticFusion Log processing result "
         "eFusion.globalModel.lastCount(): "
         "totalPoints :%d ",
         CloudPoint_num);

    int totalNodes = eFusion.getLocalDeformation().getGraph().size();
    LOGI("MyElasticFusion Log processing result "
         "eFusion.getLocalDeformation().getGraph().size(): totalNodes : %d",
         totalNodes);

    int totalFerns = eFusion.getFerns().frames.size();
    LOGI("MyElasticFusion Log processing result "
         "eFusion.getLocalDeformation().getGraph().size(): totalNodes : %d",
         totalNodes);

    int totalDefs = eFusion.getDeforms();
    LOGI("MyElasticFusion Log processing result "
         "eFusion.getDeforms(): totalDefs :%d ",
         totalDefs);

    int totalFernDefs = eFusion.getFernDeforms();
    LOGI("MyElasticFusion Log processing result "
         "eFusion.getFernDeforms(): totalFernDefs :%d ",
         totalFernDefs);

    // Eigen::Vector4f *mapData = gm.downloadMap(); //点云图
    // int validCount = 0;
    // for (unsigned int i = 0; i < CloudPoint_num; i++) {
    //   Eigen::Vector4f pos = mapData[(i * 3) + 0];
    //   if (pos[3] > confidence) {
    //     validCount++; //这是个有效的顶点,
    //   }
    // } //调用点云中的每一个点
    // LOGI("MyElasticFusion Log processing result "
    //      "eFusion.globalModel.downloadMap() valid count : %d",
    //      validCount);

    // RGBDOdometry modelToModel = eFusion.getModelToModel();
    float lastICPError = eFusion.getModelToModel().lastICPError;
    float lastICPCount = eFusion.getModelToModel().lastICPCount;
    LOGI("MyElasticFusion Log processing result lastICPError: %f, "
         "icpErrThresh: %f",
         lastICPError, 5e-05);
    LOGI("MyElasticFusion Log processing result lastICPCount: %f, "
         "icpCountThresh: %f",
         lastICPCount, 35000.00);
    //
    // LOGI("saving cloud points..." );
    // eFusion.savePly();			 //保存当前的点云图至ply
    // LOGI("cloud point has saved to " << savefilename << ".ply" );

    LOGI("MyElasticFusion Log processing result done.");

    if (shouldSavePly.getValueWait()) {
      LOGI("ElasticFusion start to save frame.");
      Eigen::Vector4f *mapData;

      float confidenceThreshold = eFusion.getConfidenceThreshold();
      LOGI("ElasticFusion start to save frame. 0");
      unsigned int lastCount = eFusion.savePly(mapData);
      LOGI("ElasticFusion start to save frame 1. lastCount: %d, "
           "confidenceThreshold: %f",
           lastCount, confidenceThreshold);
      std::string plyFilename("/sdcard/ElasticFusionPly_" +
                              current_date_time());
      plyFilename.append(".ply");
      LOGI("ElasticFusion start to save frame 1 1");
      // Open file
      std::ofstream fs;
      fs.open(plyFilename.c_str());
      // File* plyFile = fopen(plyFilename.c_str(),"wb+");
      if (fs == NULL) {
        LOGE("There was a problem opening the ply file:%s",
             plyFilename.c_str());
      }
      LOGI("ElasticFusion start to save frame. 2");
      int validCount = 0;

      for (unsigned int i = 0; i < lastCount; i++) {
        Eigen::Vector4f pos = mapData[(i * 3) + 0];

        if (pos[3] > confidenceThreshold) {
          validCount++;
        }
      }
      LOGI("ElasticFusion start to save frame. 3");
      // Write header
      fs << "ply";
      fs << "\nformat "
         << "binary_little_endian"
         << " 1.0";

      // Vertices
      fs << "\nelement vertex " << validCount;
      fs << "\nproperty float x"
            "\nproperty float y"
            "\nproperty float z";

      fs << "\nproperty uchar red"
            "\nproperty uchar green"
            "\nproperty uchar blue";

      fs << "\nproperty float nx"
            "\nproperty float ny"
            "\nproperty float nz";

      fs << "\nproperty float radius";

      fs << "\nend_header\n";

      // Close the file
      fs.close();
      // delete[] mapData;
      LOGI("ElasticFusion start to save frame. 4");
      // Open file in binary appendable
      std::ofstream fpout(plyFilename.c_str(),
                          std::ios::app | std::ios::binary);
      LOGI("ElasticFusion start to save frame. 5");
      for (unsigned int i = 0; i < lastCount; i++) {
        Eigen::Vector4f pos = mapData[(i * 3) + 0];

        if (pos[3] > confidenceThreshold) {
          Eigen::Vector4f col = mapData[(i * 3) + 1];
          Eigen::Vector4f nor = mapData[(i * 3) + 2];

          nor[0] *= -1;
          nor[1] *= -1;
          nor[2] *= -1;

          float value;
          memcpy(&value, &pos[0], sizeof(float));
          fpout.write(reinterpret_cast<const char *>(&value), sizeof(float));

          memcpy(&value, &pos[1], sizeof(float));
          fpout.write(reinterpret_cast<const char *>(&value), sizeof(float));

          memcpy(&value, &pos[2], sizeof(float));
          fpout.write(reinterpret_cast<const char *>(&value), sizeof(float));

          unsigned char r = int(col[0]) >> 16 & 0xFF;
          unsigned char g = int(col[0]) >> 8 & 0xFF;
          unsigned char b = int(col[0]) & 0xFF;

          fpout.write(reinterpret_cast<const char *>(&r),
                      sizeof(unsigned char));
          fpout.write(reinterpret_cast<const char *>(&g),
                      sizeof(unsigned char));
          fpout.write(reinterpret_cast<const char *>(&b),
                      sizeof(unsigned char));

          memcpy(&value, &nor[0], sizeof(float));
          fpout.write(reinterpret_cast<const char *>(&value), sizeof(float));

          memcpy(&value, &nor[1], sizeof(float));
          fpout.write(reinterpret_cast<const char *>(&value), sizeof(float));

          memcpy(&value, &nor[2], sizeof(float));
          fpout.write(reinterpret_cast<const char *>(&value), sizeof(float));

          memcpy(&value, &nor[3], sizeof(float));
          fpout.write(reinterpret_cast<const char *>(&value), sizeof(float));
        }
      }
      LOGI("ElasticFusion start to save frame 6.");
      // Close file
      fs.close();
      LOGI("ElasticFusion start to save frame. 7");
      delete[] mapData;
      shouldSavePly.assignValue(false);
      LOGI("MyElasticFusion save frame done.");
    }
    // LOGI("Saving Elastic-Fusion model...");
    // LOGI("Saving Elastic-Fusion model done");
    //==============================================
    //     unsigned long compressed_size = depth_compress_buf_size;
    //     boost::thread_group threads;
    //     threads.add_thread(new boost::thread(compress2,
    //                                              depth_compress_buf,
    //                                              &compressed_size,
    // //                                             (const
    // Bytef*)frameBuffers[bufferIndex].first.first,
    //                                              (const Bytef*)depth,
    //                                              depth_image_width *
    //                                              depth_image_height *
    //                                              sizeof(short),
    //                                              Z_BEST_SPEED));
    //========== not compress
    //        depthSize = width * height * sizeof(short);
    //                  rgbSize = width * height * sizeof(unsigned char) * 3;
    //
    //                  depthData = (unsigned char
    //                  *)openNI2Interface->frameBuffers[bufferIndex].first.first;
    //                  rgbData = (unsigned char
    //                  *)openNI2Interface->frameBuffers[bufferIndex].first.second;
    //===============
    // threads.add_thread(new
    // boost::thread(boost::bind(&MyElasticFusion::encodeJpeg,
    //                                                  this,
    //                                                  (cv::Vec<unsigned char,
    //                                                  3>
    //                                                  *)frameBuffers[bufferIndex].image)));
    //                                                         (cv::Vec<unsigned
    //                                                         char, 3>
    //                                                         *)frameBuffers[bufferIndex].first.second)));
    // threads.join_all();
    // LOGI("logger threads.join_all(); done ");
    // int32_t depthSize = compressed_size;
    // int32_t imageSize = encodedImage->width;
    //        int32_t imageSize = myImageSize * sizeof(unsigned char) * 3;
    //        unsigned char * rgbData = (unsigned char
    //        *)frameBuffers[bufferIndex].first.second;
    /**
     * Format is:
     * int64_t: timestamp
     * int32_t: depthSize
     * int32_t: imageSize
     * depthSize * unsigned char: depth_compress_buf
     * imageSize * unsigned char: encodedImage->data.ptr
     */
    //         size_t result =
    //         fwrite(&frameBuffers[bufferIndex].m_lastTimestamp,
    //         sizeof(int64_t), 1, RGBlog_file_); LOGI("Logger fwrite timestamp:
    //         %d", result); result =  fwrite(&depthSize, sizeof(int32_t), 1,
    //         RGBlog_file_); LOGI("Logger fwrite: depthSize : %d", result);
    //         result = fwrite(&imageSize, sizeof(int32_t), 1, RGBlog_file_);
    //         LOGI("Logger fwrite imageSize : %d", imageSize);
    //         result = fwrite(depth_compress_buf, depthSize, 1, RGBlog_file_);
    //         LOGI("Logger fwrite:depth_compress_buf : %d", result);
    //         result = fwrite(encodedImage->data.ptr, imageSize, 1,
    //         RGBlog_file_);
    // //        result = fwrite(rgbData, imageSize, 1, log_file_);
    // //        int cols = encodedImage->cols;
    // //        LOGI("encodedImage cols %d ", cols);
    //         LOGI("Logger fwrite rgbData: %d", result);
    //         LOGI("Logger 2 timestamp: %lld, depthSize : %lld,  imageSize:
    //         %lld ", (long long)(frameBuffers[bufferIndex].m_lastTimestamp),
    //         (long long)depthSize,(long long)imageSize);
    //
    //         TangoPoseData pose = frameBuffers[bufferIndex].pose;
    //         unsigned long long int utime =
    //         frameBuffers[bufferIndex].m_lastTimestamp; float x =
    //         (float)pose.translation[0]; float y = (float)pose.translation[1];
    //         float z =(float) pose.translation[2];
    //         float qx = (float)pose.orientation[0];
    //         float qy = (float)pose.orientation[1];
    //         float qz = (float)pose.orientation[2];
    //         float qw = (float)pose.orientation[3];
    //         char buffer [100];
    //         int n;
    //         n = sprintf(buffer, "%llu,%f,%f,%f,%f,%f,%f,%f", utime, x, y, z,
    //         qx, qy, qz, qw); LOGI("depth log: n: %d, s: %s", n, buffer);
    // //        printf ("[%s] is a string %d chars long\n",buffer,n);
    // //        int n = sscanf(line.c_str(), "%llu,%f,%f,%f,%f,%f,%f,%f", );
    //         result = fwrite(&buffer, sizeof(char), n, Depthlog_file_);
    //         LOGI("Logger fwrite depth1: %d", result);
    //         result = fwrite("\n", sizeof(char), 1, Depthlog_file_);
    //         LOGI("Logger fwrite depth2: %d", result);
    LOGI("MyElasticFusion: processing++");
    numFrames++;
    lastProcessed = bufferIndex;
    LOGI("MyElasticFusion: processed one frame, total: %d", numFrames);
    // break;
  }
  // fseek(RGBlog_file_, 0, SEEK_SET);
  // fwrite(&numFrames, sizeof(int32_t), 1, RGBlog_file_);
  // LOGI("Logger flush: numFrames: %d ", numFrames);
  // fflush(RGBlog_file_);
  // fflush(Depthlog_file_);
  // LOGI("Logger close:");
  // fclose(RGBlog_file_);
  // fclose(Depthlog_file_);
delete previousPose;
  LOGI("MyElasticFusion deleting");
  // delete &eFusion;
  // eFusion = NULL;
  LOGI("ElasticFusion done: done");
}
}
