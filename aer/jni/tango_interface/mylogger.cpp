#include "mylogger.hpp"

#include <tango_interface/util.hpp>
#include "logger.hpp"
#include "util.hpp"
#include <ctime>
#include <cstdarg>
#include <jni.h>
#include <tango_client_api.h>
#include <sys/stat.h>
#include <memory>
//#include <opencv2/opencv.hpp>
//#include <opencv2/core/core.hpp>
//#include <opencv2/core/core_c.h>
//#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/imgproc/imgproc_c.h>
//#include <opencv2/imgproc/types_c.h>
//#include <opencv2/highgui/highgui_c.h>

//using namespace cv;

namespace tango_interface {

Mylogger::Mylogger()
 : lastWritten(-1),
   writeThread(0),
   depth_image_width(0),
   depth_image_height(0)
{
//    std::string deviceId = "#1";


//    setupDevice(deviceId);
}

Mylogger::~Mylogger()
{
    free(depth_compress_buf);

    writing.assignValue(false);

    writeThread->join();

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    for(int i = 0; i < 10; i++)
    {
        free(imageBuffers[i].first);
    }

    for(int i = 0; i < 10; i++)
    {
        free(frameBuffers[i].first.first);
        free(frameBuffers[i].first.second);
    }
}


void Mylogger::setCamWidthAndheight(int width, int height, double fx, double fy, double cx, double cy) {
	depth_image_width = width,
    depth_image_height = height;
	myImageSize = width * height;
	myFx = fx;
	myFy = fy;
	myCx = cx;
	myCy = cy;
	int imageSize = width * height;

	    depth_compress_buf_size = imageSize * sizeof(int16_t) * 4;
	    depth_compress_buf = (uint8_t*)malloc(depth_compress_buf_size);

	    encodedImage = 0;

	    writing.assignValue(false);

	    latestDepthIndex.assignValue(-1);
	    latestImageIndex.assignValue(-1);

	    for(int i = 0; i < 10; i++)
	    {
	        uint8_t * newImage = (uint8_t *)calloc(imageSize * 3, sizeof(uint8_t));
	        imageBuffers[i] = std::pair<uint8_t *, int64_t>(newImage, 0);
	    }

	    for(int i = 0; i < 10; i++)
	    {
	        uint8_t * newDepth = (uint8_t *)calloc(imageSize * 2, sizeof(uint8_t));
	        uint8_t * newImage = (uint8_t *)calloc(imageSize * 3, sizeof(uint8_t));
	        frameBuffers[i] = std::pair<std::pair<uint8_t *, uint8_t *>, int64_t>(std::pair<uint8_t *, uint8_t *>(newDepth, newImage), 0);
	    }
}

void Mylogger::encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data)
{
	LOGI("Logger Encoding start");
    cv::Mat3b rgb(depth_image_height, depth_image_width, rgb_data, depth_image_width*3);

    IplImage * img = new IplImage(rgb);

    int jpeg_params[] = {CV_IMWRITE_JPEG_QUALITY, 90, 0};

    if(encodedImage != 0)
    {
        cvReleaseMat(&encodedImage);
    }

    encodedImage = cvEncodeImage(".jpg", img, jpeg_params);

    delete img;
    LOGI("Logger Encoding done");
}

void Mylogger::rgbdCallback(unsigned char* image, TangoPointCloud* pointcloud_buffer, double color_timestamp)
{
	LOGI("Writing thread rgbdCallback start ");
	LOGI("Writing thread rgbdCallback Processing start ");
	double depth_timestamp = 0.0;
	depth_timestamp = pointcloud_buffer->timestamp;
	uint32_t num_points = pointcloud_buffer->num_points;
	LOGI( "depth_timestamp: %f , num_points:  %d", depth_timestamp, num_points);
	  // In the following code, we define t0 as the depth timestamp and t1 as the
	  // color camera timestamp.
	  // Calculate the relative pose between color camera frame at timestamp
	  // color_timestamp t1 and depth camera frame at depth_timestamp t0.
	  TangoPoseData pose_color_image_t1_T_depth_image_t0;
	 TangoErrorType err = TangoSupport_calculateRelativePose(
			  color_timestamp, TANGO_COORDINATE_FRAME_CAMERA_COLOR, depth_timestamp,
	          TANGO_COORDINATE_FRAME_CAMERA_DEPTH, &pose_color_image_t1_T_depth_image_t0);
	  if (err == TANGO_SUCCESS)  {
		 LOGI( "CameraInterface: success get valid relative pose at %f time for color and depth cameras :%f ", color_timestamp, depth_timestamp);
	  } else {
	    LOGE( "CameraInterface: Could not find a valid relative pose at %f time for color and depth cameras :%f ", color_timestamp, depth_timestamp);
	    if (err == TANGO_INVALID) {
	    	LOGE( "CameraInterface TANGO_INVALID");
	    }
	    if (err == TANGO_ERROR) {
	        LOGE( "CameraInterface TANGO_ERROR");
	    }
	    return;
	  }
	  LOGI("CameraInterface Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
	  		  pose_color_image_t1_T_depth_image_t0.translation[0], pose_color_image_t1_T_depth_image_t0.translation[1], pose_color_image_t1_T_depth_image_t0.translation[2],
	            pose_color_image_t1_T_depth_image_t0.orientation[0], pose_color_image_t1_T_depth_image_t0.orientation[1], pose_color_image_t1_T_depth_image_t0.orientation[2],
	            pose_color_image_t1_T_depth_image_t0.orientation[3]);
	  if (std::isnan(pose_color_image_t1_T_depth_image_t0.translation[0])) {
		  LOGI("CameraInterface Position: is Nan");
		  return;
	  }
	  // The Color Camera frame at timestamp t0 with respect to Depth
	  // Camera frame at timestamp t1.
	  glm::mat4 color_image_t1_T_depth_image_t0 = GetMatrixFromPose(&pose_color_image_t1_T_depth_image_t0);
	//  if (gpu_upsample_) {
	//    depth_image_.RenderDepthToTexture(color_image_t1_T_depth_image_t0,
	//                                      pointcloud_buffer, new_points);
	//  } else {
	  std::vector<float> depth_map_buffer_;
	  UpdateAndUpsampleDepth(color_image_t1_T_depth_image_t0,
	                                        pointcloud_buffer, depth_map_buffer_);
	//  TangoCameraIntrinsics rgb_camera_intrinsics_ = CameraInterface::TangoGetIntrinsics();
	//    int depth_image_width = rgb_camera_intrinsics_.width;
	//    int depth_image_height = rgb_camera_intrinsics_.height;
	//    int depth_image_size = depth_image_width * depth_image_height;
//	    if (rgbd_callback_) {
	float* depth = &depth_map_buffer_[0];
//	          (*rgbd_callback_)(frame.get(), depth, color_timestamp);
//	     }
	LOGI("Writing thread rgbdCallback Processing done ");
	boost::posix_time::ptime time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration duration(time.time_of_day());
	m_lastImageTime = duration.total_microseconds();
    int bufferIndex = (latestImageIndex.getValue() + 1) % 10;

    memcpy(imageBuffers[bufferIndex].first, reinterpret_cast<uint8_t*>(image), depth_image_width * depth_image_height * 3);
    imageBuffers[bufferIndex].second = m_lastImageTime;

    latestImageIndex++;

	m_lastDepthTime = m_lastImageTime;

	bufferIndex = (latestDepthIndex.getValue() + 1) % 10;

    memcpy(frameBuffers[bufferIndex].first.first, reinterpret_cast<uint8_t*>(depth), depth_image_width * depth_image_height * 2);
    frameBuffers[bufferIndex].second = m_lastDepthTime;
    int lastImageVal = latestImageIndex.getValue();
    if(lastImageVal == -1)
    {
        return;
    }

    lastImageVal %= 10;

    memcpy(frameBuffers[bufferIndex].first.second, imageBuffers[lastImageVal].first, depth_image_width * depth_image_height * 3);

    latestDepthIndex++;
    LOGI("rgbdCallback done ");
}


glm::mat4 Mylogger::GetMatrixFromPose(const TangoPoseData* pose_data) {
  glm::vec3 translation =
      glm::vec3(pose_data->translation[0], pose_data->translation[1],
                pose_data->translation[2]);
  glm::quat rotation =
      glm::quat(pose_data->orientation[3], pose_data->orientation[0],
                pose_data->orientation[1], pose_data->orientation[2]);
  return glm::translate(glm::mat4(1.0f), translation) *
         glm::mat4_cast(rotation);
}


void Mylogger::UpdateAndUpsampleDepth(const glm::mat4& color_t1_T_depth_t0, const TangoPointCloud* render_point_cloud_buffer, std::vector<float> &depth_map_buffer_) {
//  TangoCameraIntrinsics rgb_camera_intrinsics_ = CameraInterface::TangoGetIntrinsics();
//  int depth_image_width = rgb_camera_intrinsics_.width;
//  int depth_image_height = rgb_camera_intrinsics_.height;
//  int depth_image_size = myImageHeight * myImageWidth;
//  std::vector<float> depth_map_buffer_;
  depth_map_buffer_.resize(myImageSize);
//  grayscale_display_buffer_.resize(depth_image_size);
  std::fill(depth_map_buffer_.begin(), depth_map_buffer_.end(), 0);
//  std::fill(grayscale_display_buffer_.begin(), grayscale_display_buffer_.end(),
//            0);
  int point_cloud_size = render_point_cloud_buffer->num_points;
  for (int i = 0; i < point_cloud_size; ++i) {
    float x = render_point_cloud_buffer->points[i][0];
    float y = render_point_cloud_buffer->points[i][1];
    float z = render_point_cloud_buffer->points[i][2];
    // depth_t0_point is the point in depth camera frame on timestamp t0.
    // (depth image timestamp).
    glm::vec4 depth_t0_point = glm::vec4(x, y, z, 1.0);
    // color_t1_point is the point in camera frame on timestamp t1.
    // (color image timestamp).
    glm::vec4 color_t1_point = color_t1_T_depth_t0 * depth_t0_point;
    int pixel_x, pixel_y;
    // get the coordinate on image plane.
    pixel_x = static_cast<int>((myFx) *(color_t1_point.x / color_t1_point.z) + myCx);
    pixel_y = static_cast<int>((myFy) *(color_t1_point.y / color_t1_point.z) + myCy);
    // Color value is the GL_LUMINANCE value used for displaying the depth
    // image.
    // We can query for depth value in mm from grayscale image buffer by
    // getting a `pixel_value` at (pixel_x,pixel_y) and calculating
    // pixel_value * (kMaxDepthDistance / USHRT_MAX)
    float depth_value = color_t1_point.z;
//    uint8_t grayscale_value =
//        (color_t1_point.z * kMeterToMillimeter) * UCHAR_MAX / kMaxDepthDistance;
    UpSampleDepthAroundPoint(depth_value, pixel_x, pixel_y,
                           &depth_map_buffer_);
  }
//
//  this->CreateOrBindCPUTexture();
//  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, depth_image_width, depth_image_height,
//                  GL_LUMINANCE, GL_UNSIGNED_BYTE,
//                  grayscale_display_buffer_.data());
//  tango_gl::util::CheckGlError("DepthImage glTexSubImage2D");
//  glBindTexture(GL_TEXTURE_2D, 0);
//
//  texture_id_ = cpu_texture_id_;
}

// Window size for splatter upsample
static const int kWindowSize = 7;

void Mylogger::UpSampleDepthAroundPoint(
    float depth_value, int pixel_x, int pixel_y,
    std::vector<float>* depth_map_buffer) {
//  int image_width = rgb_camera_intrinsics_.width;
//  int image_height = rgb_camera_intrinsics_.height;
//  int image_size = myImageHeight * myImageWidth;
  // Set the neighbour pixels to same color.
  for (int a = -kWindowSize; a <= kWindowSize; ++a) {
    for (int b = -kWindowSize; b <= kWindowSize; ++b) {
      if (pixel_x > depth_image_width || pixel_y > depth_image_height || pixel_x < 0 ||
          pixel_y < 0) {
        continue;
      }
      int pixel_num = (pixel_x + a) + (pixel_y + b) * depth_image_width;

      if (pixel_num > 0 && pixel_num < myImageSize) {
        (*depth_map_buffer)[pixel_num] = depth_value;
      }
    }
  }
}

void Mylogger::startWriting()
{
    assert(!writeThread && !writing.getValue());

    LOGI("logger start logging");
//    this->filename = filename;

    writing.assignValue(true);

    writeThread = new boost::thread(boost::bind(&Mylogger::writeData,
                                               this));
}

void Mylogger::stopWriting()
{
    assert(writeThread && writing.getValue());

    LOGI("logger stop logging2");
    writing.assignValue(false);

    writeThread->join();

    writeThread = 0;
    LOGI("logger stop logging3");
}


bool Mylogger::file_exists(const std::string& filename) const {
  struct stat st;
  int result = stat(filename.c_str(), &st);
  return (result == 0);
}


std::string Mylogger::to_string(int value) const {
  char buf[128];
  snprintf(buf,128,"%d",value);
  return std::string(buf);
}

std::string Mylogger::current_date_time() const {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
    return std::string(buf);
}


void Mylogger::sayHello() {
//   LOGI("I am saying Hello");
}

void Mylogger::writeData()
{
    /**
     * int32_t at file beginning for frame count
     */
	std::string filename("/sdcard/mymy_imperial_tango_" + current_date_time());
	 int version = 0;
	  std::string version_suffix(".log");
	  while (file_exists(filename + version_suffix)) {
	    version_suffix = "_" + to_string(++version) + ".log";
	  }
	  // Finish opening the file
	  filename += version_suffix;
	  log_file_ = fopen(filename.c_str(),"wb+");
	  if (log_file_ == NULL) {
	    LOGE("Logger: There was a problem opening the log file:%s",filename.c_str());
	  }

    int32_t numFrames = 0;

    size_t result1 = fwrite(&numFrames, sizeof(int32_t), 1, log_file_);
    LOGI("Logger fwrite: %d", result1);
//    int result = fputs("\n:testest     \n", log_file_);
//    LOGI("Logger puts: %d", result);
    LOGI("Logger: good");

    while(writing.getValueWait(1))
    {
        int lastDepth = latestDepthIndex.getValue();

        if(lastDepth == -1)
        {
            continue;
        }

        int bufferIndex = lastDepth % 10;

        if(bufferIndex == lastWritten)
        {
            continue;
        }

        unsigned long compressed_size = depth_compress_buf_size;
        boost::thread_group threads;

        threads.add_thread(new boost::thread(compress2,
                                             depth_compress_buf,
                                             &compressed_size,
                                             (const Bytef*)frameBuffers[bufferIndex].first.first,
                                             depth_image_width * depth_image_height * sizeof(short),
                                             Z_BEST_SPEED));

        threads.add_thread(new boost::thread(boost::bind(&Mylogger::encodeJpeg,
                                                         this,
                                                         (cv::Vec<unsigned char, 3> *)frameBuffers[bufferIndex].first.second)));

        threads.join_all();
        LOGI("logger threads.join_all(); done ");
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

        size_t result = fwrite(&frameBuffers[bufferIndex].second, sizeof(int64_t), 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result =  fwrite(&depthSize, sizeof(int32_t), 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result = fwrite(&imageSize, sizeof(int32_t), 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result = fwrite(depth_compress_buf, depthSize, 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        result = fwrite(encodedImage->data.ptr, imageSize, 1, log_file_);
        LOGI("Logger fwrite: %d", result);
        LOGI("Logger: logging");
        numFrames++;

        lastWritten = bufferIndex;
    }

    fseek(log_file_, 0, SEEK_SET);
    fwrite(&numFrames, sizeof(int32_t), 1, log_file_);
    LOGI("Logger flush: numFrames: %d ", numFrames);
    fflush(log_file_);
    LOGI("Logger close:");
    fclose(log_file_);
    LOGI("Logger close done:");
}
}
