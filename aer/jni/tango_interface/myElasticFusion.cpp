#include "myElasticFusion.hpp"

#include <tango_interface/util.hpp>
#include "util.hpp"
#include <cstdarg>
#include <ctime>
#include <jni.h>
#include <memory>
#include <sys/stat.h>
#include <tango_client_api.h>

namespace tango_interface {

MyElasticFusion::MyElasticFusion()
    : lastProcessed(-1), elasticFusionThread(0), depth_image_width(0),
      depth_image_height(0) {}

MyElasticFusion::~MyElasticFusion() {
  free(depth_compress_buf);
  runningElasticFusion.assignValue(false);
  shouldSavePly.assignValue(false);
  elasticFusionThread->join();

  for (int i = 0; i < 50; i++) {
    free(frameBuffers[i].image);
    free(frameBuffers[i].pointCloudPoints);
  }
}

void MyElasticFusion::setCamWidthAndheight(int width, int height, double fx,
                                           double fy, double cx, double cy,
                                           int maxVerCount) {
  depth_image_width = width, depth_image_height = height;
  myImageSize = width * height;
  myFx = fx;
  myFy = fy;
  myCx = cx;
  myCy = cy;
  depth_compress_buf_size = myImageSize * sizeof(int16_t) * 4;
  depth_compress_buf = (unsigned char *)malloc(depth_compress_buf_size);
  runningElasticFusion.assignValue(false);
  shouldSavePly.assignValue(false);
  latestBufferIndex.assignValue(-1);

  for (int i = 0; i < 50; i++) {
    float *newDepth = (float *)calloc(4 * maxVerCount, sizeof(float));
    unsigned char *newImage =
        (unsigned char *)calloc(myImageSize * 4, sizeof(unsigned char));

    struct ElasticFusionData rgbdData;
    rgbdData.colorTimeStamp = 0.0;
    rgbdData.image = newImage;
    rgbdData.pointCloudTimestamp = 0.0;
    rgbdData.pointCloudNumpoints = 0;
    rgbdData.pointCloudPoints = newDepth;
    rgbdData.m_lastTimestamp = 0;
    frameBuffers[i] = rgbdData;
  }
}

void MyElasticFusion::rgbdCallback(unsigned char *image,
                                   TangoPointCloud *pointcloud_buffer,
                                   double color_timestamp, TangoPoseData pose) {
  boost::posix_time::ptime time =
      boost::posix_time::microsec_clock::local_time();
  boost::posix_time::time_duration duration(time.time_of_day());
  m_lastFrameTime = duration.total_microseconds();

  int bufferIndex = (latestBufferIndex.getValue() + 1) % 50;

  frameBuffers[bufferIndex].pointCloudTimestamp = pointcloud_buffer->timestamp;
  frameBuffers[bufferIndex].pointCloudNumpoints = pointcloud_buffer->num_points;
  memcpy(frameBuffers[bufferIndex].pointCloudPoints, pointcloud_buffer->points,
         (pointcloud_buffer->num_points) * 4 * sizeof(float));
  int rgbPixeldatacount = myImageSize * 4;
  memcpy(frameBuffers[bufferIndex].image, image, rgbPixeldatacount);
  frameBuffers[bufferIndex].colorTimeStamp = color_timestamp;
  frameBuffers[bufferIndex].m_lastTimestamp = m_lastFrameTime;
  frameBuffers[bufferIndex].pose = pose;

  latestBufferIndex++;
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

void MyElasticFusion::UpdateAndUpsampleDepth(
    const glm::mat4 &color_t1_T_depth_t0,
    const float *render_point_cloud_buffer,
    std::vector<unsigned short> &depth_map_buffer_, int point_cloud_size) {

  depth_map_buffer_.resize(myImageSize);

  std::fill(depth_map_buffer_.begin(), depth_map_buffer_.end(), 0);

  for (int i = 0; i < point_cloud_size; ++i) {

    float x = render_point_cloud_buffer[3 * i];
    float y = render_point_cloud_buffer[3 * i + 1];
    float z = render_point_cloud_buffer[3 * i + 2];

    glm::vec4 depth_t0_point = glm::vec4(x, y, z, 1.0);
    // color_t1_point is the point in camera frame on timestamp t1.
    // (color image timestamp).
    glm::vec4 color_t1_point = color_t1_T_depth_t0 * depth_t0_point;
    int pixel_x, pixel_y;
    // get the coordinate on image plane.
    pixel_x =
        static_cast<int>((myFx) * (color_t1_point.x / color_t1_point.z) + myCx);
    pixel_y =
        static_cast<int>((myFy) * (color_t1_point.y / color_t1_point.z) + myCy);

    float depth_value = color_t1_point.z;
    UpSampleDepthAroundPoint(depth_value, pixel_x, pixel_y, &depth_map_buffer_);
  }
}

// Window size for splatter upsample
static const int kWindowSize = 7;

void MyElasticFusion::UpSampleDepthAroundPoint(
    float depth_value, int pixel_x, int pixel_y,
    std::vector<unsigned short> *depth_map_buffer) {
  // Set the neighbour pixels to same color.
  for (int a = -kWindowSize; a <= kWindowSize; ++a) {
    for (int b = -kWindowSize; b <= kWindowSize; ++b) {
      if (pixel_x > depth_image_width || pixel_y > depth_image_height ||
          pixel_x < 0 || pixel_y < 0) {
        continue;
      }
      int pixel_num = (pixel_x + a) + (pixel_y + b) * depth_image_width;
      if (pixel_num > 0 && pixel_num < myImageSize) {
        (*depth_map_buffer)[pixel_num] =
            (unsigned short)round(depth_value * 1000);
      }
    }
  }
}

void MyElasticFusion::startElasticFusion() {
  assert(!elasticFusionThread && !runningElasticFusion.getValue());
  runningElasticFusion.assignValue(true);
  elasticFusionThread =
      new boost::thread(boost::bind(&MyElasticFusion::runEF, this));
}

void MyElasticFusion::stopElasticFusion() {
  assert(elasticFusionThread && runningElasticFusion.getValue());
  runningElasticFusion.assignValue(false);
  elasticFusionThread->join();
  elasticFusionThread = 0;
}

void MyElasticFusion::savePly() {
  assert(elasticFusionThread && runningElasticFusion.getValue());
  shouldSavePly.assignValue(true);
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
    LOGE("eglGetDisplay failed");
  }

  EGLint major, minor;
  if (!eglInitialize(display, &major, &minor) || eglGetError() != EGL_SUCCESS) {
    LOGE("eglInitialize failed");
  }

  EGLint numConfigs;
  EGLConfig config;
  if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) ||
      eglGetError() != EGL_SUCCESS) {
    LOGE("eglChooseConfig failed");
  }

  EGLSurface surface;
  const EGLint surfaceAttr[] = {EGL_WIDTH, 1280, EGL_HEIGHT, 720, EGL_NONE};
  EGLContext context;

  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS) {
    LOGE("eglCreateContext failed");
  }

  surface = eglCreatePbufferSurface(display, config, surfaceAttr);
  if (surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS) {
    LOGE("eglCreatePbufferSurface  failed");
  }

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE ||
      eglGetError() != EGL_SUCCESS) {
    LOGE("eglMakeCurrent failed");
  }

  if (eglGetError() != EGL_SUCCESS) {
    LOGE("egl error");
  }

  bool success = LoadOpenGLExtensionsManually();
  if (!success) {
    LOGE("LoadOpenGLExtensionsManually failed");
  } else {
    LOGI("LoadOpenGLExtensionsManually success");
  }

  Resolution::getInstance(depth_image_width, depth_image_height);
  Intrinsics::getInstance(myFx, myFy, myCx, myCy);

  float confidence = 0.4f;
  float depth = 12.0f;
  float icp = 10.0f;
  float icpErrThresh = 5e-05;
  float covThresh = 1e-05;
  float photoThresh = 115;
  float fernThresh = 0.3095f;
  int timeDelta = 200;
  int icpCountThresh = 35000;
  bool bootstrap = false;
  bool openLoop = 1;
  bool iclnuim = 0;
  bool reloc = 0;
  bool fastOdom = 1; // Fast odometry (single level pyramid) mode :
  bool so3 = 0;             // SO(3) pre-alignment in tracking：
  bool frameToFrameRGB = 0; // rgb only tracking：
  int timestamp = 0;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);

  ElasticFusion eFusion(openLoop ? std::numeric_limits<int>::max() / 2
                                 : timeDelta,
                        icpCountThresh, icpErrThresh, covThresh, !openLoop,
                        iclnuim, reloc, photoThresh, confidence, depth, icp,
                        fastOdom, fernThresh, so3, frameToFrameRGB);

  std::vector<Eigen::Matrix4f> posesEigen;
  int32_t numFrames = 0;

  Eigen::Matrix4f *previousPose = 0;
  previousPose = new Eigen::Matrix4f;
  previousPose->setIdentity();

  while (runningElasticFusion.getValueWait(1)) {
    int bufferIndex = latestBufferIndex.getValue();
    if (bufferIndex == -1) {
      continue;
    }

    bufferIndex = bufferIndex % 50;
    if (bufferIndex == lastProcessed) {
      continue;
    }

    double depth_timestamp = frameBuffers[bufferIndex].pointCloudTimestamp;
    uint32_t num_points = frameBuffers[bufferIndex].pointCloudNumpoints;
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
    } else {
      if (err == TANGO_INVALID) {
        LOGE("TANGO_INVALID");
      }
      if (err == TANGO_ERROR) {
        LOGE("TANGO_ERROR");
      }
      continue;
    }

    if (std::isnan(pose_color_image_t1_T_depth_image_t0.translation[0])) {
      continue;
    }

    glm::mat4 color_image_t1_T_depth_image_t0 =
        GetMatrixFromPose(&pose_color_image_t1_T_depth_image_t0);

    std::vector<unsigned short> depth_map_buffer_;

    UpdateAndUpsampleDepth(color_image_t1_T_depth_image_t0,
                           frameBuffers[bufferIndex].pointCloudPoints,
                           depth_map_buffer_, num_points);

    unsigned short *depthForEF = &depth_map_buffer_[0];

    unsigned long long int timeStampleForEF =
        frameBuffers[bufferIndex].m_lastTimestamp;
    unsigned char *rgbImageForEF = frameBuffers[bufferIndex].image;
    TangoPoseData pose = frameBuffers[bufferIndex].pose;

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

    eFusion.processFrame(rgbImageForEF, depthForEF, timeStampleForEF,
                         currentPose, bootstrap);

    *previousPose = *currentPose;

    delete currentPose;

    Eigen::Matrix4f currPose = eFusion.getCurrPose();

    posesEigen.push_back(currPose);

    delete &currPose;
    int time = eFusion.getTick();

    int ld_num = eFusion.getDeforms();

    int gd_num = eFusion.getFernDeforms();

    bool isLost = eFusion.getLost();

    int CloudPoint_num = eFusion.getGlobalModel().lastCount();

    int totalNodes = eFusion.getLocalDeformation().getGraph().size();

    int totalFerns = eFusion.getFerns().frames.size();

    int totalDefs = eFusion.getDeforms();

    int totalFernDefs = eFusion.getFernDeforms();

    float lastICPError = eFusion.getModelToModel().lastICPError;
    float lastICPCount = eFusion.getModelToModel().lastICPCount;

    if (shouldSavePly.getValueWait()) {
      float confidenceThreshold = eFusion.getConfidenceThreshold();

      unsigned int lastCount = CloudPoint_num;

      Eigen::Vector4f *mapData = eFusion.savePly();

      std::string plyFilename("/sdcard/ElasticFusionPly_" +
                              current_date_time());
      plyFilename.append(".ply");

      // Open file
      std::ofstream fs;
      fs.open(plyFilename.c_str());

      if (fs == NULL) {
        LOGE("There was a problem opening the ply file: %s, error",
             plyFilename.c_str());
      }

      int validCount = 0;

      for (unsigned int i = 0; i < lastCount; i++) {
        Eigen::Vector4f pos = mapData[(i * 3) + 0];
        if (pos[3] > confidenceThreshold) {
          validCount++;
        }
      }

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

      // Open file in binary appendable
      std::ofstream fpout(plyFilename.c_str(),
                          std::ios::app | std::ios::binary);

      LOGI("ElasticFusion start to save frame. 5");

      int countWriteNum = 0;
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

          countWriteNum++;
        }
      }

      // Close file
      fs.close();
      delete[] mapData;

      shouldSavePly.assignValue(false);
    }

    numFrames++;
    lastProcessed = bufferIndex;
    tango_interface::CameraInterface::incrementCounter();
  }

  delete previousPose;
}
}
