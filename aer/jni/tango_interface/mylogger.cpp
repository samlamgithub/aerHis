#include "mylogger.hpp"

#include <tango_interface/util.hpp>
#include "util.hpp"
#include <cstdarg>
#include <ctime>
#include <jni.h>
#include <memory>
#include <sys/stat.h>
#include <tango_client_api.h>

namespace tango_interface {

Mylogger::Mylogger()
    : lastWritten(-1), writeThread(0), depth_image_width(0),
      depth_image_height(0) {}

Mylogger::~Mylogger() {
  free(depth_compress_buf);

  writing.assignValue(false);

  writeThread->join();

  if (encodedImage != 0) {
    cvReleaseMat(&encodedImage);
  }

  for (int i = 0; i < 50; i++) {
    free(frameBuffers[i].image);
    free(frameBuffers[i].pointCloudPoints);
  }
}

void Mylogger::setCamWidthAndheight(int width, int height, double fx, double fy,
                                    double cx, double cy, int maxVerCount) {
  depth_image_width = width, depth_image_height = height;
  myImageSize = width * height;
  myFx = fx;
  myFy = fy;
  myCx = cx;
  myCy = cy;
  depth_compress_buf_size = myImageSize * sizeof(int16_t) * 4;
  depth_compress_buf = (uint8_t *)malloc(depth_compress_buf_size);

  encodedImage = 0;

  writing.assignValue(false);
  latestBufferIndex.assignValue(-1);

  for (int i = 0; i < 50; i++) {
    float *newDepth = (float *)calloc(4 * maxVerCount, sizeof(float));
    uint8_t *newImage = (uint8_t *)calloc(myImageSize * 4, sizeof(uint8_t));
    struct RGBDdata rgbdData;
    rgbdData.colorTimeStamp = 0.0;
    rgbdData.image = newImage;
    rgbdData.pointCloudTimestamp = 0.0;
    rgbdData.pointCloudNumpoints = 0;
    rgbdData.pointCloudPoints = newDepth;
    rgbdData.m_lastTimestamp = 0;
    frameBuffers[i] = rgbdData;
  }
}

void Mylogger::encodeJpeg(cv::Vec<unsigned char, 4> *rgb_data) {
  int step = depth_image_width * 4 * sizeof(unsigned char);
  cv::Mat4b rgb(depth_image_height, depth_image_width, rgb_data, step);
  IplImage *img = new IplImage(rgb);

  int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

  if (encodedImage != 0) {
    cvReleaseMat(&encodedImage);
  }

  encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

  delete img;
}

void Mylogger::rgbdCallback(unsigned char *image,
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
  memcpy(frameBuffers[bufferIndex].image, reinterpret_cast<uint8_t *>(image),
         rgbPixeldatacount);
  frameBuffers[bufferIndex].colorTimeStamp = color_timestamp;
  frameBuffers[bufferIndex].m_lastTimestamp = m_lastFrameTime;
  frameBuffers[bufferIndex].pose = pose;

  latestBufferIndex++;
}

glm::mat4 Mylogger::GetMatrixFromPose(const TangoPoseData *pose_data) {
  glm::vec3 translation =
      glm::vec3(pose_data->translation[0], pose_data->translation[1],
                pose_data->translation[2]);
  glm::quat rotation =
      glm::quat(pose_data->orientation[3], pose_data->orientation[0],
                pose_data->orientation[1], pose_data->orientation[2]);
  return glm::translate(glm::mat4(1.0f), translation) *
         glm::mat4_cast(rotation);
}

void Mylogger::UpdateAndUpsampleDepth(
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

    glm::vec4 color_t1_point = color_t1_T_depth_t0 * depth_t0_point;
    int pixel_x, pixel_y;

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

void Mylogger::UpSampleDepthAroundPoint(
    float depth_value, int pixel_x, int pixel_y,
    std::vector<unsigned short> *depth_map_buffer) {

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

void Mylogger::startWriting() {
  assert(!writeThread && !writing.getValue());
  writing.assignValue(true);
  writeThread = new boost::thread(boost::bind(&Mylogger::writeData, this));
}

void Mylogger::stopWriting() {
  assert(writeThread && writing.getValue());

  writing.assignValue(false);

  writeThread->join();

  writeThread = 0;
}

bool Mylogger::file_exists(const std::string &filename) const {
  struct stat st;
  int result = stat(filename.c_str(), &st);
  return (result == 0);
}

std::string Mylogger::to_string(int value) const {
  char buf[128];
  snprintf(buf, 128, "%d", value);
  return std::string(buf);
}

std::string Mylogger::current_date_time() const {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
  return std::string(buf);
}

void Mylogger::writeData() {
  /**
   * int32_t at file beginning for frame count
   */
  std::string RGBDfilename("/sdcard/tango_RGBD_logdata_" +
                          current_date_time());
  int RGBversion = 0;
  std::string RGBDversion_suffix(".klg");
  while (file_exists(RGBDfilename + RGBDversion_suffix)) {
    RGBDversion_suffix = "_" + to_string(++RGBversion) + ".klg";
  }
  // Finish opening the file
  RGBDfilename += RGBDversion_suffix;
  RGBDlog_file_ = fopen(RGBDfilename.c_str(), "wb+");
  if (RGBDlog_file_ == NULL) {
    return;
  }

  std::string PoseDataFilename("/sdcard/tango_pose_logdata_" +
                            current_date_time());
  int PoseFileVersion = 0;
  std::string PoseFileVersion_suffix(".klg");
  while (file_exists(PoseDataFilename + PoseFileVersion_suffix)) {
    PoseFileVersion_suffix = "_" + to_string(++PoseFileVersion) + ".klg";
  }
  // Finish opening the file
  PoseDataFilename += PoseFileVersion_suffix;
  PoseDataLofFile = fopen(PoseDataFilename.c_str(), "wb+");
  if (PoseDataLofFile == NULL) {
    return;
  }

  int32_t numFrames = 0;

  size_t result1 = fwrite(&numFrames, sizeof(int32_t), 1, RGBDlog_file_);

  while (writing.getValueWait(1)) {
    int bufferIndex = latestBufferIndex.getValue();
    if (bufferIndex == -1) {
      continue;
    }

    //   bufferIndex = bufferIndex % 10;
    bufferIndex = bufferIndex % 50;
    if (bufferIndex == lastWritten) {
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
        continue;
    } else {
      if (err == TANGO_INVALID) {
        continue;
      }
      if (err == TANGO_ERROR) {
        continue;
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

    unsigned short *depth = &depth_map_buffer_[0];

    unsigned long compressed_size = depth_compress_buf_size;
    boost::thread_group threads;
    threads.add_thread(new boost::thread(
        compress2, depth_compress_buf, &compressed_size,
        (const Bytef *)depth,
        depth_image_width * depth_image_height * sizeof(short), Z_BEST_SPEED));

    threads.add_thread(new boost::thread(boost::bind(
        &Mylogger::encodeJpeg, this,
        (cv::Vec<unsigned char, 4> *)frameBuffers[bufferIndex].image)));

    threads.join_all();

    int32_t depthSize = compressed_size;
    int32_t imageSize = encodedImage->width;

    /**
     * Format is:
     * int64_t: timestamp
     * int32_t: depthSize
     * int32_t: imageSize
     * depthSize * unsigned char: depth_compress_buf
     * imageSize * unsigned char: encodedImage->data.ptr
     */
    size_t result = fwrite(&frameBuffers[bufferIndex].m_lastTimestamp,
                           sizeof(int64_t), 1, RGBDlog_file_);
    result = fwrite(&depthSize, sizeof(int32_t), 1, RGBDlog_file_);
    result = fwrite(&imageSize, sizeof(int32_t), 1, RGBDlog_file_);
    result = fwrite(depth_compress_buf, depthSize, 1, RGBDlog_file_);
    result = fwrite(encodedImage->data.ptr, imageSize, 1, RGBDlog_file_);

    TangoPoseData pose = frameBuffers[bufferIndex].pose;
    unsigned long long int utime = frameBuffers[bufferIndex].m_lastTimestamp;
    float x = (float)pose.translation[0];
    float y = (float)pose.translation[1];
    float z = (float)pose.translation[2];
    float qx = (float)pose.orientation[0];
    float qy = (float)pose.orientation[1];
    float qz = (float)pose.orientation[2];
    float qw = (float)pose.orientation[3];

    result = fwrite(&buffer, sizeof(char), n, PoseDataLofFile);
    result = fwrite("\n", sizeof(char), 1, PoseDataLofFile);

    numFrames++;
    lastWritten = bufferIndex;
    tango_interface::CameraInterface::incrementCounter();
  }
  fseek(RGBDlog_file_, 0, SEEK_SET);
  fwrite(&numFrames, sizeof(int32_t), 1, RGBDlog_file_);
  fflush(RGBDlog_file_);
  fflush(PoseDataLofFile);
  fclose(RGBDlog_file_);
  fclose(PoseDataLofFile);
}
}
