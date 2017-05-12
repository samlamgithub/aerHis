#include <tango_interface/camera_interface.hpp>
#include <tango_interface/gui_interface.hpp>
#include <tango_interface/imu_interface.hpp>
#include <tango_interface/logger.hpp>
#include <tango_interface/util.hpp>
#include <memory>
#include "aer.hpp"


// #include <tango_interface/mylogger.hpp>
//myElasticFusion
#include "tango_interface/myElasticFusion.hpp"

#include <elasticfusion/ElasticFusion.h>

namespace aer {

// 1042.110000 1042.370000 637.475000 358.318000 1280 720
const int tangoCamWidth = 1280;
const int tangoCamHeight = 720;
const float tangoCamFx = 1042.110000;
const float tangoCamFy = 1042.370000;
const float tangoCamCx = 637.475000;
const float tangoCamCy = 358.318000;

Aer::Aer() {
//  if (tango_interface::ImuInterface::initialise()) {
//    tango_interface::ImuInterface::register_accelerometer_callback(
//      std::bind(&Aer::accelerometer_callback, this, std::placeholders::_1));
//    tango_interface::ImuInterface::register_gyroscope_callback(
//      std::bind(&Aer::gyroscope_callback, this, std::placeholders::_1));
//  }
//  tango_interface::CameraInterface::register_raw_frame_callback(
//    std::bind(&Aer::raw_frame_callback, this, std::placeholders::_1));
//  tango_interface::CameraInterface::register_marker_callback(
//    std::bind(&Aer::marker_callback, this, std::placeholders::_1));

  tango_interface::CameraInterface::register_loggerWidthHeight_callback(
  std::bind(&Aer::setLoggerWidthHeight_callback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
		  std::placeholders::_5, std::placeholders::_6,  std::placeholders::_7));
  tango_interface::CameraInterface::register_rgbd_callback(
      std::bind(&Aer::rgbdCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
  // tango_interface::CameraInterface::register_writing_callback(
  //   std::bind(&Aer::writing_callback, this));
//    tango_interface::CameraInterface::register_elasticfusion_callback(
//      std::bind(&Aer::elasticfusion_callback, this));

  //  tango_interface::Mylogger logger;
  //  tango_interface::CameraInterface::setLogger(&logger);
  //  tango_interface::Logger file_logger;
  //  file_logger.log(tango_interface::kInfo,"My first default informational log %.02f %i",0.05,42);
}

Aer::~Aer() {

}

//
//void Aer::accelerometer_callback(const tango_interface::AccelerometerEvent& event) {
//  // TODO: do something with the values...
//}
//
//void Aer::gyroscope_callback(const tango_interface::GyroscopeEvent& event) {
//  // TODO: do something with the values...
//}
//
//void Aer::marker_callback(const tango_interface::MarkerEvent& event) {
//  if (!event.marker_data.empty()) {
//    LOGI("Markers detected: %lld", event.timestamp_nanoseconds);
//    tango_interface::GuiInterface::display_marker_data(event.marker_data);
//  }
//  // TODO later: do something...
//}


void Aer::elasticFusion() {
//  	LOGI("ElasticFusion Initializing ...");
//  	Resolution::getInstance(tangoCamWidth, tangoCamHeight);
//  	Intrinsics::getInstance(tangoCamFx, tangoCamFy, tangoCamCx, tangoCamCy);
//  	LOGI("ElasticFusion Initializing done ...");
//
//  	LOGI("ElasticFusion Setting parameters...");
//  	float confidence = 10.0f; //fusion的confidence阈值
//  	float depth = 3.0f; //去掉depth大于某个阈值的帧
//  	float icp = 10.0f; //icp的阈值
//  	float icpErrThresh = 5e-05; //icp错误阈值
//  	float covThresh = 1e-05;
//  	float photoThresh = 115;
//  	float fernThresh = 0.3095f; //新fern的阈值
//  	int timeDelta = 200;
//  	int icpCountThresh = 35000;
//  	//int start = 1;
//  	//int end = std::numeric_limits<unsigned short>::max(); //Funny bound, since we predict times in this format really!
//  	// bool openLoop = 0; //open loop模式：不开启
//    bool openLoop = 1; //open loop模式：开启
//  	bool iclnuim = 0; //使用icl dataset:不使用
//  	bool reloc = 0; //重定位模式：先做重建，不开启重定位
//  	// bool fastOdom = 0; //Fast odometry (single level pyramid) mode :不开启
//    bool fastOdom = 1; //Fast odometry (single level pyramid) mode :开启
//  	// bool so3 = 1; //SO(3) pre-alignment in tracking：开启
//    bool so3 = 0; //SO(3) pre-alignment in tracking：不开启
//  	bool frameToFrameRGB = 0; //只做rgb图像的tracking：不开启
//  	int timestamp = 0;
//  	std::string savefilename = "testElasticFusion";
//  	LOGI("ElasticFusion Setting parameters done.");
//
//  	LOGI("ElasticFusion Building eFusion..." );
//  	// pangolin::Params windowParams;
//  	// windowParams.Set("SAMPLE_BUFFERS", 0);
//  	// windowParams.Set("SAMPLES", 0);
//  	// pangolin::CreateWindowAndBind("Main", width, height, windowParams);
//  	// glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//  	// glPixelStorei(GL_PACK_ALIGNMENT, 1);
//  	ElasticFusion eFusion(
//  			openLoop ? std::numeric_limits<int>::max() / 2 : timeDelta,
//  			icpCountThresh, icpErrThresh, covThresh, !openLoop, iclnuim, reloc,
//  			photoThresh, confidence, depth, icp, fastOdom, fernThresh, so3,
//  			frameToFrameRGB, savefilename);
//  	LOGI("ElasticFusion Building eFusion done" );
//
//  	//待处理文件的位置和下标
//  	// std::string filedir = "../pic/";
//  	// int file_start = 1;
//  	// int file_end = 782;
//
////  	vector<Eigen::Matrix4f> pose;
//  		//三位重建
//  		// for (int file_ptr = file_start; file_ptr <= file_end; file_ptr = file_ptr +1) {
//  			// LOGI("Processing frame : "<<file_ptr << " ...");
//  			// FRAME f = readFrame(file_ptr, filedir);
//  			// unsigned char * rgb = f.rgb;
//  			// unsigned short * dep = f.dep;
//        Eigen::Matrix4f * currentPose;//当前的位姿
//           currentPose = new Eigen::Matrix4f;
//           currentPose->setIdentity();
//          //  *currentPose = groundTruthOdometry->getTransformation(timestamp); }
//           eFusion.processFrame(rgb, depth, timestamp, currentPose);
//           delete currentPose;
//  			// eFusion.processFrame(rgb, dep, timestamp);
//  			Eigen::Matrix4f currPose = eFusion.getCurrPose();
//  			pose.push_back(currPose);
//  			LOGI("current pose is : " <<currPose);
//  		// }
  		LOGI("Processing frames done." );
  		LOGI("Saving Elastic-Fusion model...");
  		LOGI("Saving Elastic-Fusion model done");

  	/*
  	 //查看处理的结果

  	 int time = eFusion.getTick();//查看此时eFusion的系统时间
  	 LOGI("time : "<<time);

  	 Ferns keyfern = eFusion.getFerns();//关键帧dataset

  	 int ld_num = eFusion.getDeforms();//局部deformations的数量
  	 Deformation ld = eFusion.getLocalDeformation();//局部deformation图

  	 int gd_num = eFusion.getFernDeforms();//全局deformations的数量
  	GlobalModel gm = eFusion.getGlobalModel();//全局deformation model:

  	 int CloudPoint_num = eFusion.globalModel.lastCount(); //点云的点数量
  	 Eigen::Vector4f * mapData = eFusion.globalModel.downloadMap(); //点云图
  	 for (unsigned int i = 0; i < CloudPoint_num; i++) {
  	 Eigen::Vector4f pos = mapData[(i * 3) + 0];
  	 if (pos[3] > eFusion.confidenceThreshold) {
  	 //这是个有效的顶点,validCount++;
  	 }
  	 }//调用点云中的每一个点
  	 */
    //
  	// LOGI("saving cloud points..." );
  	// eFusion.savePly();			 //保存当前的点云图至ply
  	// LOGI("cloud point has saved to " << savefilename << ".ply" );
}

////=========================================
//#include <ElasticFusion.h>
//初始化
//Resolution::getInstance(640, 480); #图像大小
//Intrinsics::getInstance(528, 528, 320, 240); #相机参数
//pangolin::Params windowParams; #GUI窗口
//windowParams.Set("SAMPLE_BUFFERS", 0);
//windowParams.Set("SAMPLES", 0);
//pangolin::CreateWindowAndBind("Main", width, height, windowParams);
//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//glPixelStorei(GL_PACK_ALIGNMENT, 1);
//
//使用封装好的ElasticFusion类，可以精确定义每一个参数
//默认的构造函数
//
//ElasticFusion eFusion;
//精确定义每一个参数后调用构建函数
//
//float confidence = 10.0f; //fusion的confidence阈值
//float depth = 3.0f; //去掉depth大于某个阈值的帧
//float icp = 10.0f; //icp的阈值
//float icpErrThresh = 5e-05; //icp错误阈值
//float covThresh = 1e-05;
//float photoThresh = 115;
//float fernThresh = 0.3095f; //新fern的阈值
//int timeDelta = 200;
//int icpCountThresh = 35000;
//bool openLoop = 0; //open loop模式：不开启
//bool iclnuim = 0; //使用icl dataset:不使用
//bool reloc = 0; //重定位模式：不开启
//bool fastOdom = 0; //Fast odometry (single level pyramid) mode :不开启
//bool so3 = 1; //SO(3) pre-alignment in tracking：开启
//bool frameToFrameRGB = 0; //只做rgb图像的tracking：不开启
//int timestamp = 0;
//std::string savefilename = "test";
//
//ElasticFusion eFusion(openLoop ? std::numeric_limits<int>::max() / 2 : timeDelta,icpCountThresh, icpErrThresh, covThresh, !openLoop, iclnuim, reloc,photoThresh, confidence, depth, icp, fastOdom, fernThresh, so3,frameToFrameRGB, savefilename);
//
//处理输入帧：
//
//eFusion.processFrame(rgb, depth, timestamp, currentPose, weightMultiplier);
//
//其中rgb为uchar*，depth为ushor*，timestamp为时间戳（不重要），剩下两个为可选参数
//可选参数1:currentPose——先验的current pose,比如传入了标定好的ground truth三维坐标数据,则可以用上一时刻和这一时刻的三位坐标来估计相机姿态
//
//     Eigen::Matrix4f * currentPose;//当前的位姿
//     currentPose = new Eigen::Matrix4f;
//     currentPose->setIdentity();
//     *currentPose = groundTruthOdometry->getTransformation(timestamp); }
//     eFusion.processFrame(rgb, depth, timestamp, currentPose, weightMultiplier);
//     delete currentPose;
//
//可选参数2:weightMultiplier (full frame fusion weight)——默认为1.f  可以自己调整,比如针对前面跳过的帧数对参数进行调整
//
//float weightMultiplier = framesToSkip + 1;
//eFusion.processFrame(rgb, depth, timestamp, currentPose, weightMultiplier);
//
//一个简单的使用CoreAPI的程序：
//
//#include "myfusion_base.h"
//
//int main() {
//
//	//图像的尺寸以及相机参数
//	int width = 640;
//	int height = 480;
//	int camera_fx = 528;
//	int camera_fy = 528;
//	int camera_cx = 320;
//	int camera_cy = 240;
//
//	LOGI("Initializing ..." );
//	Resolution::getInstance(width, height);
//	Intrinsics::getInstance(camera_fx, camera_fy, camera_cx, camera_cy);
//	LOGI("Initializing done." );
//
//	LOGI("Setting parameters..." );
//	float confidence = 10.0f; //fusion的confidence阈值
//	float depth = 3.0f; //去掉depth大于某个阈值的帧
//	float icp = 10.0f; //icp的阈值
//	float icpErrThresh = 5e-05; //icp错误阈值
//	float covThresh = 1e-05;
//	float photoThresh = 115;
//	float fernThresh = 0.3095f; //新fern的阈值
//	int timeDelta = 200;
//	int icpCountThresh = 35000;
//	//int start = 1;
//	//int end = std::numeric_limits<unsigned short>::max(); //Funny bound, since we predict times in this format really!
//	bool openLoop = 0; //open loop模式：不开启
//	bool iclnuim = 0; //使用icl dataset:不使用
//	bool reloc = 0; //重定位模式：先做重建，不开启重定位
//	bool fastOdom = 0; //Fast odometry (single level pyramid) mode :不开启
//	bool so3 = 1; //SO(3) pre-alignment in tracking：开启
//	bool frameToFrameRGB = 0; //只做rgb图像的tracking：不开启
//	int timestamp = 0;
//	std::string savefilename = "test";
//	LOGI("Setting parameters done." );
//
//	LOGI("Building eFusion..." );
//	pangolin::Params windowParams;
//	windowParams.Set("SAMPLE_BUFFERS", 0);
//	windowParams.Set("SAMPLES", 0);
//	pangolin::CreateWindowAndBind("Main", width, height, windowParams);
//	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//	glPixelStorei(GL_PACK_ALIGNMENT, 1);
//	ElasticFusion eFusion(
//			openLoop ? std::numeric_limits<int>::max() / 2 : timeDelta,
//			icpCountThresh, icpErrThresh, covThresh, !openLoop, iclnuim, reloc,
//			photoThresh, confidence, depth, icp, fastOdom, fernThresh, so3,
//			frameToFrameRGB, savefilename);
//	LOGI("Building eFusion done" );
//
//	//待处理文件的位置和下标
//	std::string filedir = "../pic/";
//	int file_start = 1;
//	int file_end = 782;
//
//
//		vector<Eigen::Matrix4f> pose;
//		//三位重建
//		for (int file_ptr = file_start; file_ptr <= file_end; file_ptr = file_ptr +1) {
//			LOGI("Processing frame : "<<file_ptr << " ...");
//			FRAME f = readFrame(file_ptr, filedir);
//			unsigned char * rgb = f.rgb;
//			unsigned short * dep = f.dep;
//			eFusion.processFrame(rgb, dep, timestamp);
//			Eigen::Matrix4f currPose = eFusion.getCurrPose();
//			pose.push_back(currPose);
//			LOGI("current pose is : " )<<currPose );
//		}
//		LOGI("Processing frames done." ));
//		LOGI("Saving Elastic-Fusion model...");
//		LOGI("Saving Elastic-Fusion model done"));
//
//	/*
//	 //查看处理的结果
//
//	 int time = eFusion.getTick();//查看此时eFusion的系统时间
//	 LOGI("time : "<<time);
//
//	 Ferns keyfern = eFusion.getFerns();//关键帧dataset
//
//	 int ld_num = eFusion.getDeforms();//局部deformations的数量
//	 Deformation ld = eFusion.getLocalDeformation();//局部deformation图
//
//	 int gd_num = eFusion.getFernDeforms();//全局deformations的数量
//	GlobalModel gm = eFusion.getGlobalModel();//全局deformation model:
//
//	 int CloudPoint_num = eFusion.globalModel.lastCount(); //点云的点数量
//	 Eigen::Vector4f * mapData = eFusion.globalModel.downloadMap(); //点云图
//	 for (unsigned int i = 0; i < CloudPoint_num; i++) {
//	 Eigen::Vector4f pos = mapData[(i * 3) + 0];
//	 if (pos[3] > eFusion.confidenceThreshold) {
//	 //这是个有效的顶点,validCount++;
//	 }
//	 }//调用点云中的每一个点
//	 */
//
//	LOGI("saving cloud points..." );
//	eFusion.savePly();			 //保存当前的点云图至ply
//	LOGI("cloud point has saved to " << savefilename << ".ply" );
//
//	return 0;
//}
//
//FRAME readFrame(int index, std::string filedir) {
//    FRAME f;
//    // 文件目录
//    string rgbDir = filedir + "rgb/";
//    string depthDir = filedir + "depth/";
//    // 文件后缀
//    string rgbExt = ".png";
//    string depthExt = ".png";
//    stringstream ss;
//    // 读rgb图
//    ss << rgbDir << index << rgbExt;
//    string filename;
//    ss >> filename;
//    f.rgb_mat = cv::imread(filename);
//    f.rgb = f.rgb_mat.data;
//    // 读depth图
//    ss.clear();
//    filename.clear();
//    ss << depthDir << index << depthExt;
//    ss >> filename;
//    f.dep_mat = cv::imread(filename, -1);
//    f.dep = (unsigned short*) f.dep_mat.data;
//    f.frameID = index;
//
//    return f;
//}

//============================================

//void Aer::setLoggerWidthHeight_callback(const int width, const int height,const double fx,const double fy,const double cx,const double cy) {
//	mylogger.setCamWidthAndheight(width, height, fx, fy, cx, cy);
//}

void Aer::setLoggerWidthHeight_callback(const int width, const int height, const double fx,const double fy,const double cx,const double cy, const int max_vertex_count) {
  // mylogger.setCamWidthAndheight(width, height, fx, fy, cx, cy, max_vertex_count);
	LOGI("AER Elasticfusion setting width height callback");
myElasticFusion.setCamWidthAndheight(width, height, fx, fy, cx, cy, max_vertex_count);
}

void Aer::rgbdCallback(unsigned char* image, TangoPointCloud* pointcloud_buffer, double cameraTime, TangoPoseData pose) {
	// mylogger.rgbdCallback(image, pointcloud_buffer, cameraTime, pose);
myElasticFusion.rgbdCallback(image, pointcloud_buffer, cameraTime, pose);
}
//
// void Aer::writing_callback() {
// 	 LOGI("logger start writing callback");
// }

//void Aer::rgbd_callback(const tango_interface::DepthEvent& event) {
////  LOGI("raw depth(t): %lld", event.timestamp_nanoseconds);
//}
//
//void Aer::raw_frame_callback(const tango_interface::RawFrameEvent& event) {
//  LOGI("raw frame(t): %lld", event.timestamp_nanoseconds);
//}

//Save elasticFusion Ply file
void Aer::savePlyFile() {
	LOGI("AER Elasticfusion setting width height callback");
  myElasticFusion.savePly();
}

//Run ElasticFusion
void Aer::aerStartElasticFusion(bool startElasticFusion) {
    // TODO: make use of startElasticFusion to stop
    // elasticFusion_thread = std::thread(&Aer::elasticFusion, this);
    LOGI("AER ElasticFusion start called: %d", startElasticFusion);
  	if (startElasticFusion) {
  	   LOGI("AER ElasticFusion starting");
  	   myElasticFusion.startElasticFusion();
  	} else {
  		 LOGI("AER ElasticFusion stopping");
  		 myElasticFusion.stopElasticFusion();
  	}
}

void Aer::aerStartWriting(bool startWriting) {
	LOGI("Aer hello start writing");
	if (startWriting) {
		 LOGI("aer logger start writing in aerStartWriting");
			//  mylogger.startWriting();
	} else {
		  LOGI("aer logger stop weriting in aerStartWriting");
			// mylogger.stopWriting();
	}
}

} // namespace aer
