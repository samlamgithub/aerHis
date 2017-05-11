#include "myElasticFusion.hpp"

#include <tango_interface/util.hpp>
// #include "logger.hpp"
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
#include <../elasticfusion/ElasticFusion.h>
#include <../elasticfusion/Shaders/GLExtensions.h>

namespace tango_interface {

MyElasticFusion::MyElasticFusion() :
		lastProcessed(-1), elasticFusionThread(0), depth_image_width(0), depth_image_height(
				0) {
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
		double fy, double cx, double cy, int maxVerCount) {
	LOGI("setCamWidthAndheight start");
	depth_image_width = width, depth_image_height = height;
	myImageSize = width * height;
	myFx = fx;
	myFy = fy;
	myCx = cx;
	myCy = cy;
	LOGI(
			"setCamWidthAndheight intrinsic: %d, %d, %f, %f, %f, %f , myImageSize: %d",
			depth_image_width, depth_image_height, fx, fy, cx, cy, myImageSize);
	// int imageSize = width * height;
	depth_compress_buf_size = myImageSize * sizeof(int16_t) * 4;
	depth_compress_buf = (unsigned char*) malloc(depth_compress_buf_size);
	// encodedImage = 0;
	runningElasticFusion.assignValue(false);
	shouldSavePly.assignValue(false);
//	    latestDepthIndex.assignValue(-1);
	latestBufferIndex.assignValue(-1);
//	    for(int i = 0; i < 10; i++)
//	    {
//	        unsigned char * newImage = (unsigned char *)calloc(imageSize * 3, sizeof(unsigned char));
//	        imageBuffers[i] = std::pair<unsigned char *, int64_t>(newImage, 0);
//	    }
//	    for(int i = 0; i < 10; i++) {
	for (int i = 0; i < 50; i++) {
//	        unsigned char * newDepth = (unsigned char *)calloc(myImageSize * 2, sizeof(unsigned char));
		float * newDepth = (float *) calloc(4 * maxVerCount, sizeof(float));
		unsigned char * newImage = (unsigned char *) calloc(myImageSize * 3,
				sizeof(unsigned char));
//	        frameBuffers[i] = std::pair<std::pair<unsigned char *, unsigned char *>, int64_t>(std::pair<unsigned char *, unsigned char *>(newDepth, newImage), 0);
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
	LOGI("setCamWidthAndheight done");
}
//
// void MyElasticFusion::encodeJpeg(cv::Vec<unsigned char, 3> * rgb_data) {
// 	LOGI("Logger Encoding start: %d, %d", depth_image_height, depth_image_width);
// 	int step = depth_image_width*3*sizeof(unsigned char);
// //	LOGI("step: %d", step);
//     cv::Mat3b rgb(depth_image_height, depth_image_width, rgb_data, step);
// //    LOGI("depth: %d, channels: %d, row: %d, cols : %d", rgb.depth(),  rgb.channels(),rgb.rows, rgb.cols);
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

void MyElasticFusion::rgbdCallback(unsigned char* image,
		TangoPointCloud* pointcloud_buffer, double color_timestamp,
		TangoPoseData pose) {
	LOGI("Writing thread rgbdCallback start ");
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
	frameBuffers[bufferIndex].pointCloudTimestamp =
			pointcloud_buffer->timestamp;
	frameBuffers[bufferIndex].pointCloudNumpoints =
			pointcloud_buffer->num_points;
	memcpy(frameBuffers[bufferIndex].pointCloudPoints,
			pointcloud_buffer->points,
			(pointcloud_buffer->num_points) * 4 * sizeof(float));
	int rgbPixeldatacount = myImageSize * 3;
	memcpy(frameBuffers[bufferIndex].image, image, rgbPixeldatacount);
	frameBuffers[bufferIndex].colorTimeStamp = color_timestamp;
	frameBuffers[bufferIndex].m_lastTimestamp = m_lastFrameTime;
	frameBuffers[bufferIndex].pose = pose;
//    memcpy(frameBuffers[bufferIndex].first.first, reinterpret_cast<unsigned char*>(depth), myImageSize * 2);
//    int rgbPixeldatacount = myImageSize * 3;
//    LOGI("rgbPixeldatacount %d", rgbPixeldatacount);
//    memcpy(frameBuffers[bufferIndex].first.second, reinterpret_cast<unsigned char*>(image), rgbPixeldatacount);
//    frameBuffers[bufferIndex].second = m_lastFrameTime;
	latestBufferIndex++;
	LOGI("myElasticFusion thread rgbdCallback done ");
}

glm::mat4 MyElasticFusion::GetMatrixFromPose(const TangoPoseData* pose_data) {
	glm::vec3 translation = glm::vec3(pose_data->translation[0],
			pose_data->translation[1], pose_data->translation[2]);
	glm::quat rotation = glm::quat(pose_data->orientation[3],
			pose_data->orientation[0], pose_data->orientation[1],
			pose_data->orientation[2]);
	return glm::translate(glm::mat4(1.0f), translation)
			* glm::mat4_cast(rotation);
}

//void MyElasticFusion::UpdateAndUpsampleDepth(const glm::mat4& color_t1_T_depth_t0,
//		const TangoPointCloud* render_point_cloud_buffer,
//		std::vector<unsigned short> &depth_map_buffer_) {
void MyElasticFusion::UpdateAndUpsampleDepth(
		const glm::mat4& color_t1_T_depth_t0,
		const float* render_point_cloud_buffer,
		std::vector<unsigned short> &depth_map_buffer_, int point_cloud_size) {
//  TangoCameraIntrinsics rgb_camera_intrinsics_ = CameraInterface::TangoGetIntrinsics();
//  int depth_image_width = rgb_camera_intrinsics_.width;
//  int depth_image_height = rgb_camera_intrinsics_.height;
//  int depth_image_size = myImageHeight * myImageWidth;
////  std::vector<float> depth_map_buffer_;
//	LOGI("UpdateAndUpsampleDepth intrinsic: %d, %d, %d,  %f, %f, %f, %f ",depth_image_width , depth_image_height,
//			myImageSize, myFx, myFy, myCx, myCy);
	depth_map_buffer_.resize(myImageSize);
//  grayscale_display_buffer_.resize(depth_image_size);
	std::fill(depth_map_buffer_.begin(), depth_map_buffer_.end(), 0);
//  std::fill(grayscale_display_buffer_.begin(), grayscale_display_buffer_.end(),
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
//    LOGI("UpdateAndUpsampleDepth 2: %f, %f ,%f ",color_t1_point.x, color_t1_point.y, color_t1_point.z);
//    LOGI("UpdateAndUpsampleDepth 2: %f, %f ,%f, | %f, %f ,%f ", x, y, z, color_t1_point.x, color_t1_point.y, color_t1_point.z);
		// get the coordinate on image plane.
		pixel_x = static_cast<int>((myFx)
				* (color_t1_point.x / color_t1_point.z) + myCx);
		pixel_y = static_cast<int>((myFy)
				* (color_t1_point.y / color_t1_point.z) + myCy);
//    LOGI("UpdateAndUpsampleDepth 3: %d, %d ",pixel_x,pixel_y);
		// Color value is the GL_LUMINANCE value used for displaying the depth
		// image.
		// We can query for depth value in mm from grayscale image buffer by
		// getting a `pixel_value` at (pixel_x,pixel_y) and calculating
		// pixel_value * (kMaxDepthDistance / USHRT_MAX)
		float depth_value = color_t1_point.z;
//    unsigned char grayscale_value =
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

void MyElasticFusion::UpSampleDepthAroundPoint(float depth_value, int pixel_x,
		int pixel_y, std::vector<unsigned short>* depth_map_buffer) {
//  int image_width = rgb_camera_intrinsics_.width;
//  int image_height = rgb_camera_intrinsics_.height;
//  int image_size = myImageHeight * myImageWidth;
	// Set the neighbour pixels to same color.
//    LOGI("UpSampleDepthAroundPoint ===: %d, %d ,%d , %d, %d, %f", pixel_x, pixel_y, depth_image_width, depth_image_height,
//    		(unsigned short)round(depth_value * 1000), depth_value);
	for (int a = -kWindowSize; a <= kWindowSize; ++a) {
		for (int b = -kWindowSize; b <= kWindowSize; ++b) {
			if (pixel_x > depth_image_width || pixel_y > depth_image_height
					|| pixel_x < 0 || pixel_y < 0) {
//    	  LOGI("UpSampleDepthAroundPoint failed");
//    	  LOGI("UpSampleDepthAroundPoint failed: %d, %d ,%d , %d, %f", pixel_x, pixel_y, depth_image_width, depth_image_height, depth_value);
				continue;
			}
			int pixel_num = (pixel_x + a) + (pixel_y + b) * depth_image_width;
			if (pixel_num > 0 && pixel_num < myImageSize) {
				(*depth_map_buffer)[pixel_num] = (unsigned short) round(
						depth_value * 1000);
//        LOGI("UpSampleDepthAroundPoint success");
//        LOGI("UpSampleDepthAroundPoint success: %d, %d ,%d , %d, %d, %f", pixel_x, pixel_y, depth_image_width, depth_image_height,
//        		(*depth_map_buffer)[pixel_num], depth_value);
			}
		}
	}
}

void MyElasticFusion::startElasticFusion() {
	assert(!elasticFusionThread && !runningElasticFusion.getValue());
	LOGI("MyElasticFusion start running");
	bool success = LoadOpenGLExtensionsManually();
	if (!success) {
		LOGE("MyElasticFusion LoadOpenGLExtensionsManually failed");
	}
//    this->filename = filename;
	runningElasticFusion.assignValue(true);
	elasticFusionThread = new boost::thread(
			boost::bind(&MyElasticFusion::runEF, this));
}

void MyElasticFusion::stopElasticFusion() {
	assert(elasticFusionThread && runningElasticFusion.getValue());
	LOGI("MyElasticFusion stop running");
	runningElasticFusion.assignValue(false);
	elasticFusionThread->join();
	elasticFusionThread = 0;
	// LOGI("logger stop logging3");
}

void MyElasticFusion::savePly() {
	assert(elasticFusionThread && runningElasticFusion.getValue());
	LOGI("MyElasticFusion is running, should save ply");
	shouldSavePly.assignValue(true);
}

bool MyElasticFusion::file_exists(const std::string& filename) const {
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
	// std::string RGBfilename("/sdcard/mymy_imperial_tango_RGB" + current_date_time());
	//  int RGBversion = 0;
	//   std::string RGBversion_suffix(".klg");
	//   while (file_exists(RGBfilename + RGBversion_suffix)) {
	//     RGBversion_suffix = "_" + to_string(++RGBversion) + ".klg";
	//   }
	//   // Finish opening the file
	//   RGBfilename += RGBversion_suffix;
	//   RGBlog_file_ = fopen(RGBfilename.c_str(),"wb+");
	//   if (RGBlog_file_ == NULL) {
	//     LOGE("Logger: There was a problem opening the RGB log file:%s",RGBfilename.c_str());
	//   }
	//
	//   std::string Depthfilename("/sdcard/mymy_imperial_tango_Depth" + current_date_time());
	//   	 int Depthversion = 0;
	//   	  std::string Depthversion_suffix(".klg");
	//   	  while (file_exists(Depthfilename + Depthversion_suffix)) {
	//   		Depthversion_suffix = "_" + to_string(++Depthversion) + ".klg";
	//   	  }
	//   	  // Finish opening the file
	//   	Depthfilename += Depthversion_suffix;
	//   	Depthlog_file_ = fopen(Depthfilename.c_str(),"wb+");
	//   	  if (Depthlog_file_ == NULL) {
	//   	    LOGE("Logger: There was a problem opening the Depth log file:%s",Depthfilename.c_str());
	//   	  }
	LOGI("ElasticFusion Initializing ...");
	Resolution::getInstance(depth_image_width, depth_image_height);
	Intrinsics::getInstance(myFx, myFy, myCx, myCy);
	LOGI("ElasticFusion Initializing done ...");
	LOGI("ElasticFusion Setting parameters...");
	float confidence = 10.0f; //fusion的confidence阈值
	float depth = 3.0f; //去掉depth大于某个阈值的帧
	float icp = 10.0f; //icp的阈值
	float icpErrThresh = 5e-05; //icp错误阈值
	float covThresh = 1e-05;
	float photoThresh = 115;
	float fernThresh = 0.3095f; //新fern的阈值
	int timeDelta = 200;
	int icpCountThresh = 35000;
	//int start = 1;
	//int end = std::numeric_limits<unsigned short>::max(); //Funny bound, since we predict times in this format really!
	// bool openLoop = 0; //open loop模式：不开启
	bool openLoop = 1; //open loop模式：开启
	bool iclnuim = 0; //使用icl dataset:不使用
	bool reloc = 0; //重定位模式：先做重建，不开启重定位
	// bool fastOdom = 0; //Fast odometry (single level pyramid) mode :不开启
	bool fastOdom = 1; //Fast odometry (single level pyramid) mode :开启
	// bool so3 = 1; //SO(3) pre-alignment in tracking：开启
	bool so3 = 0; //SO(3) pre-alignment in tracking：不开启
	bool frameToFrameRGB = 0; //只做rgb图像的tracking：不开启
	int timestamp = 0;
	std::string savefilename = "testElasticFusion";
	LOGI("ElasticFusion Setting parameters done.");
	LOGI("ElasticFusion Building eFusion...");
	// pangolin::Params windowParams;
	// windowParams.Set("SAMPLE_BUFFERS", 0);
	// windowParams.Set("SAMPLES", 0);
	// pangolin::CreateWindowAndBind("Main", width, height, windowParams);
	// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// glPixelStorei(GL_PACK_ALIGNMENT, 1);
	ElasticFusion eFusion(
			openLoop ? std::numeric_limits<int>::max() / 2 : timeDelta,
			icpCountThresh, icpErrThresh, covThresh, !openLoop, iclnuim, reloc,
			photoThresh, confidence, depth, icp, fastOdom, fernThresh, so3,
			frameToFrameRGB, savefilename);
	LOGI("ElasticFusion Building eFusion done");
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
	// LOGI("Logger: good");
	while (runningElasticFusion.getValueWait(1)) {
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
		LOGI("ElasticFusion thread Processing start ");
		// double depth_timestamp = 0.0;
		//  depth_timestamp = pointcloud_buffer->timestamp;
		double depth_timestamp = frameBuffers[bufferIndex].pointCloudTimestamp;
		// uint32_t num_points = pointcloud_buffer->num_points;
		uint32_t num_points = frameBuffers[bufferIndex].pointCloudNumpoints;
		LOGI("ElasticFusion depth_timestamp: %f , num_points:  %d",
				depth_timestamp, num_points);
		double color_timestamp = frameBuffers[bufferIndex].colorTimeStamp;
		// In the following code, we define t0 as the depth timestamp and t1 as the
		// color camera timestamp.
		// Calculate the relative pose between color camera frame at timestamp
		// color_timestamp t1 and depth camera frame at depth_timestamp t0.
		TangoPoseData pose_color_image_t1_T_depth_image_t0;
		TangoErrorType err = TangoSupport_calculateRelativePose(color_timestamp,
				TANGO_COORDINATE_FRAME_CAMERA_COLOR, depth_timestamp,
				TANGO_COORDINATE_FRAME_CAMERA_DEPTH,
				&pose_color_image_t1_T_depth_image_t0);
		if (err == TANGO_SUCCESS) {
			LOGI(
					"CameraInterface ElasticFusion: success get valid relative pose at %f time for color and depth cameras :%f , color > depth: %d",
					color_timestamp, depth_timestamp,
					color_timestamp > depth_timestamp);
		} else {
			LOGE(
					"CameraInterface ElasticFusion: Could not find a valid relative pose at %f time for color and depth cameras :%f ",
					color_timestamp, depth_timestamp);
			if (err == TANGO_INVALID) {
				LOGE("CameraInterface ElasticFusion TANGO_INVALID");
			}
			if (err == TANGO_ERROR) {
				LOGE("CameraInterface ElasticFusion TANGO_ERROR");
			}
			return;
		}
		//	  LOGI("CameraInterface 1 Position: %f, %f, %f. Orientation: %f, %f, %f, %f",
		//	  		  pose_color_image_t1_T_depth_image_t0.translation[0], pose_color_image_t1_T_depth_image_t0.translation[1], pose_color_image_t1_T_depth_image_t0.translation[2],
		//	            pose_color_image_t1_T_depth_image_t0.orientation[0], pose_color_image_t1_T_depth_image_t0.orientation[1], pose_color_image_t1_T_depth_image_t0.orientation[2],
		//	            pose_color_image_t1_T_depth_image_t0.orientation[3]);
		//	  LOGI("CameraInterface 1 status code: %d", pose_color_image_t1_T_depth_image_t0.status_code);
		//	  LOGI("CameraInterface 1 accuracy : %f",   pose_color_image_t1_T_depth_image_t0.accuracy);
		//	LOGI("CameraInterface 1 confidence: %d",   pose_color_image_t1_T_depth_image_t0.confidence);
		//	return;
		if (std::isnan(pose_color_image_t1_T_depth_image_t0.translation[0])) {
			LOGI("CameraInterface ElasticFusion Position: is Nan");
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
		glm::mat4 color_image_t1_T_depth_image_t0 = GetMatrixFromPose(
				&pose_color_image_t1_T_depth_image_t0);
		//  if (gpu_upsample_) {
		//    depth_image_.RenderDepthToTexture(color_image_t1_T_depth_image_t0,
		//                                      pointcloud_buffer, new_points);
		//  } else {
		std::vector<unsigned short> depth_map_buffer_;
//        	  UpdateAndUpsampleDepth(color_image_t1_T_depth_image_t0,
//        	                                        pointcloud_buffer, depth_map_buffer_);
		UpdateAndUpsampleDepth(color_image_t1_T_depth_image_t0,
				frameBuffers[bufferIndex].pointCloudPoints, depth_map_buffer_,
				num_points);
		//  TangoCameraIntrinsics rgb_camera_intrinsics_ = CameraInterface::TangoGetIntrinsics();
		//    int depth_image_width = rgb_camera_intrinsics_.width;
		//    int depth_image_height = rgb_camera_intrinsics_.height;
		//    int depth_image_size = depth_image_width * depth_image_height;
		//	    if (rgbd_callback_) {
		unsigned short* depthForEF = &depth_map_buffer_[0];
		//	          (*rgbd_callback_)(frame.get(), depth, color_timestamp);
		//	     }
		LOGI("ElasticFusion thread rgbdCallback Processing done ");
		unsigned long long int timeStampleForEF =
				frameBuffers[bufferIndex].m_lastTimestamp;
		unsigned char *rgbImageForEF = frameBuffers[bufferIndex].image;
		TangoPoseData pose = frameBuffers[bufferIndex].pose;
		// Eigen::Matrix4f * currentPose = 0;
		Eigen::Quaternion<float> quaternion = Eigen::Quaternion<float>(
				(float) pose.orientation[3], (float) pose.orientation[0],
				(float) pose.orientation[1], (float) pose.orientation[2]);
		Eigen::Matrix3f rot = Eigen::Quaternionf((float) pose.orientation[3],
				(float) pose.orientation[0], (float) pose.orientation[1],
				(float) pose.orientation[2]).toRotationMatrix();
		Eigen::Matrix4f *currentPose = 0;
		currentPose = new Eigen::Matrix4f;
		currentPose->setIdentity();
		currentPose->block(0, 0, 3, 3) = rot;
		Eigen::Vector4f trans((float) pose.translation[0],
				(float) pose.translation[1], (float) pose.translation[2], 1.00);
		currentPose->rightCols<1>() = trans;
		// float x = (float)pose.translation[0];
		// float y = (float)pose.translation[1];
		// float z =(float) pose.translation[2];
		// float qx = (float)pose.orientation[0];
		// float qy = (float)pose.orientation[1];
		// float qz = (float)pose.orientation[2];
		// float qw = (float)pose.orientation[3];
		//三位重建
		// for (int file_ptr = file_start; file_ptr <= file_end; file_ptr = file_ptr +1) {
		// LOGI("Processing frame : "<<file_ptr << " ...");
		// FRAME f = readFrame(file_ptr, filedir);
		// unsigned char * rgb = f.rgb;
		// unsigned short * dep = f.dep;
		// Eigen::Matrix4f * currentPose;//当前的位姿
		//  currentPose = new Eigen::Matrix4f;
		//  currentPose->setIdentity();
		//  *currentPose = groundTruthOdometry->getTransformation(timestamp); }
		eFusion.processFrame(rgbImageForEF, depthForEF, timeStampleForEF,
				currentPose);
		delete currentPose;
		// eFusion.processFrame(rgb, dep, timestamp);
		Eigen::Matrix4f currPose = eFusion.getCurrPose();
		posesEigen.push_back(currPose);
//        LOGI("current pose is : " <<currPose);
		LOGI("Processing frames done.");

		if (shouldSavePly.getValueWait()) {

			LOGI("ElasticFusion start to save frame.");
			Eigen::Vector4f * mapData;
			unsigned int lastCount;
			float confidenceThreshold;
			eFusion.savePly(*mapData, lastCount, confidenceThreshold);

			std::string plyFilename(
					"/sdcard/ElasticFusionPly_" + current_date_time());
			plyFilename.append(".ply");

			// Open file
			std::ofstream fs;
			fs.open(plyFilename.c_str());
			// File* plyFile = fopen(plyFilename.c_str(),"wb+");
			if (fs == NULL) {
				LOGE("There was a problem opening the ply file:%s",
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
			fs << "\nformat " << "binary_little_endian" << " 1.0";

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
					fpout.write(reinterpret_cast<const char*>(&value),
							sizeof(float));

					memcpy(&value, &pos[1], sizeof(float));
					fpout.write(reinterpret_cast<const char*>(&value),
							sizeof(float));

					memcpy(&value, &pos[2], sizeof(float));
					fpout.write(reinterpret_cast<const char*>(&value),
							sizeof(float));

					unsigned char r = int(col[0]) >> 16 & 0xFF;
					unsigned char g = int(col[0]) >> 8 & 0xFF;
					unsigned char b = int(col[0]) & 0xFF;

					fpout.write(reinterpret_cast<const char*>(&r),
							sizeof(unsigned char));
					fpout.write(reinterpret_cast<const char*>(&g),
							sizeof(unsigned char));
					fpout.write(reinterpret_cast<const char*>(&b),
							sizeof(unsigned char));

					memcpy(&value, &nor[0], sizeof(float));
					fpout.write(reinterpret_cast<const char*>(&value),
							sizeof(float));

					memcpy(&value, &nor[1], sizeof(float));
					fpout.write(reinterpret_cast<const char*>(&value),
							sizeof(float));

					memcpy(&value, &nor[2], sizeof(float));
					fpout.write(reinterpret_cast<const char*>(&value),
							sizeof(float));

					memcpy(&value, &nor[3], sizeof(float));
					fpout.write(reinterpret_cast<const char*>(&value),
							sizeof(float));
				}
			}

			// Close file
			fs.close();

			delete[] mapData;
			shouldSavePly.assignValue(false);
			LOGI("ElasticFusion save frame done.");
		}
		// LOGI("Saving Elastic-Fusion model...");
		// LOGI("Saving Elastic-Fusion model done");
		//==============================================
//     unsigned long compressed_size = depth_compress_buf_size;
//     boost::thread_group threads;
//     threads.add_thread(new boost::thread(compress2,
//                                              depth_compress_buf,
//                                              &compressed_size,
// //                                             (const Bytef*)frameBuffers[bufferIndex].first.first,
//                                              (const Bytef*)depth,
//                                              depth_image_width * depth_image_height * sizeof(short),
//                                              Z_BEST_SPEED));
		//========== not compress
//        depthSize = width * height * sizeof(short);
//                  rgbSize = width * height * sizeof(unsigned char) * 3;
//
//                  depthData = (unsigned char *)openNI2Interface->frameBuffers[bufferIndex].first.first;
//                  rgbData = (unsigned char *)openNI2Interface->frameBuffers[bufferIndex].first.second;
		//===============
		// threads.add_thread(new boost::thread(boost::bind(&MyElasticFusion::encodeJpeg,
		//                                                  this,
		//                                                  (cv::Vec<unsigned char, 3> *)frameBuffers[bufferIndex].image)));
//                                                         (cv::Vec<unsigned char, 3> *)frameBuffers[bufferIndex].first.second)));
		// threads.join_all();
		// LOGI("logger threads.join_all(); done ");
		// int32_t depthSize = compressed_size;
		// int32_t imageSize = encodedImage->width;
//        int32_t imageSize = myImageSize * sizeof(unsigned char) * 3;
//        unsigned char * rgbData = (unsigned char *)frameBuffers[bufferIndex].first.second;
		/**
		 * Format is:
		 * int64_t: timestamp
		 * int32_t: depthSize
		 * int32_t: imageSize
		 * depthSize * unsigned char: depth_compress_buf
		 * imageSize * unsigned char: encodedImage->data.ptr
		 */
//         size_t result = fwrite(&frameBuffers[bufferIndex].m_lastTimestamp, sizeof(int64_t), 1, RGBlog_file_);
//         LOGI("Logger fwrite timestamp: %d", result);
//         result =  fwrite(&depthSize, sizeof(int32_t), 1, RGBlog_file_);
//         LOGI("Logger fwrite: depthSize : %d", result);
//         result = fwrite(&imageSize, sizeof(int32_t), 1, RGBlog_file_);
//         LOGI("Logger fwrite imageSize : %d", imageSize);
//         result = fwrite(depth_compress_buf, depthSize, 1, RGBlog_file_);
//         LOGI("Logger fwrite:depth_compress_buf : %d", result);
//         result = fwrite(encodedImage->data.ptr, imageSize, 1, RGBlog_file_);
// //        result = fwrite(rgbData, imageSize, 1, log_file_);
// //        int cols = encodedImage->cols;
// //        LOGI("encodedImage cols %d ", cols);
//         LOGI("Logger fwrite rgbData: %d", result);
//         LOGI("Logger 2 timestamp: %lld, depthSize : %lld,  imageSize: %lld ", (long long)(frameBuffers[bufferIndex].m_lastTimestamp), (long long)depthSize,(long long)imageSize);
//
//         TangoPoseData pose = frameBuffers[bufferIndex].pose;
//         unsigned long long int utime = frameBuffers[bufferIndex].m_lastTimestamp;
//         float x = (float)pose.translation[0];
//         float y = (float)pose.translation[1];
//         float z =(float) pose.translation[2];
//         float qx = (float)pose.orientation[0];
//         float qy = (float)pose.orientation[1];
//         float qz = (float)pose.orientation[2];
//         float qw = (float)pose.orientation[3];
//         char buffer [100];
//         int n;
//         n = sprintf(buffer, "%llu,%f,%f,%f,%f,%f,%f,%f", utime, x, y, z, qx, qy, qz, qw);
//         LOGI("depth log: n: %d, s: %s", n, buffer);
// //        printf ("[%s] is a string %d chars long\n",buffer,n);
// //        int n = sscanf(line.c_str(), "%llu,%f,%f,%f,%f,%f,%f,%f", );
//         result = fwrite(&buffer, sizeof(char), n, Depthlog_file_);
//         LOGI("Logger fwrite depth1: %d", result);
//         result = fwrite("\n", sizeof(char), 1, Depthlog_file_);
//         LOGI("Logger fwrite depth2: %d", result);
		LOGI("ElasticFusion: processing");
		numFrames++;
		lastProcessed = bufferIndex;
		LOGI("ElasticFusion: processed one frame, total: %d", numFrames);
	}
	// fseek(RGBlog_file_, 0, SEEK_SET);
	// fwrite(&numFrames, sizeof(int32_t), 1, RGBlog_file_);
	// LOGI("Logger flush: numFrames: %d ", numFrames);
	// fflush(RGBlog_file_);
	// fflush(Depthlog_file_);
	// LOGI("Logger close:");
	// fclose(RGBlog_file_);
	// fclose(Depthlog_file_);
	LOGI("ElasticFusion done:");
}
}