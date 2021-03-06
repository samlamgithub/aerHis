#ifndef MYELASTICFUSION_HPP_
#define MYELASTICFUSION_HPP_

#include <cassert>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
#include <zlib.h>
#include <../elasticfusion/Shaders/GLExtensions.h>
#include <tango_interface/camera_interface.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
#include <tango_client_api.h>
#include <tango_support_api.h>
#include "ThreadMutexObject.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/matrix_decompose.hpp"
#include <../elasticfusion/ElasticFusion.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <GLES3/gl3.h>
#define __gl2_h_
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace tango_interface {

struct ElasticFusionData {
  double pointCloudTimestamp;
  uint32_t pointCloudNumpoints;
  float *pointCloudPoints;
  unsigned char *image;
  double colorTimeStamp;
  int64_t m_lastTimestamp;
  TangoPoseData pose;
};

class MyElasticFusion {
public:
  MyElasticFusion();

  virtual ~MyElasticFusion();

  void savePly();

  void startElasticFusion();
  void stopElasticFusion();

  ElasticFusionData frameBuffers[50];
  ThreadMutexObject<int> latestBufferIndex;
  void rgbdCallback(unsigned char *image, TangoPointCloud *pointcloud_buffer,
                    double color_timestamp, TangoPoseData pose);
  void setCamWidthAndheight(int width, int height, double fx, double fy,
                            double cx, double cy, int maxVerCount);
private:
  glm::mat4 GetMatrixFromPose(const TangoPoseData *pose_data);
  void UpdateAndUpsampleDepth(const glm::mat4 &color_t1_T_depth_t0,
                              const float *render_point_cloud_buffer,
                              std::vector<unsigned short> &depth_map_buffer_,
                              int point_cloud_size);
  void UpSampleDepthAroundPoint(float depth_value, int pixel_x, int pixel_y,
                                std::vector<unsigned short> *depth_map_buffer);
  int depth_image_width;
  int depth_image_height;
  int myImageSize;
  double myFx;
  double myFy;
  double myCx;
  double myCy;

  bool file_exists(const std::string &filename) const;
  std::string to_string(int) const;
  std::string current_date_time() const;
  int64_t m_lastFrameTime;
  int depth_compress_buf_size;
  unsigned char *depth_compress_buf;

  int lastProcessed;
  boost::thread *elasticFusionThread;
  ThreadMutexObject<bool> runningElasticFusion;

  ThreadMutexObject<bool> shouldSavePly;

  void runEF();
};
}
#endif /* MYELASTICFUSION_HPP_ */
