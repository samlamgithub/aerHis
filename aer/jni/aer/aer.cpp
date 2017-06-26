#include "aer.hpp"
#include <memory>
#include <tango_interface/camera_interface.hpp>
#include <tango_interface/gui_interface.hpp>
#include <tango_interface/imu_interface.hpp>
#include <tango_interface/logger.hpp>
#include <tango_interface/util.hpp>

namespace aer {

const int tangoCamWidth = 1280;
const int tangoCamHeight = 720;
const float tangoCamFx = 1042.110000;
const float tangoCamFy = 1042.370000;
const float tangoCamCx = 637.475000;
const float tangoCamCy = 358.318000;

Aer::Aer() {
  userMode.assignValue(-1);
  tango_interface::CameraInterface::register_loggerWidthHeight_callback(
      std::bind(
          &Aer::setLoggerWidthHeight_callback, this, std::placeholders::_1,
          std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
          std::placeholders::_5, std::placeholders::_6, std::placeholders::_7));
  tango_interface::CameraInterface::register_rgbd_callback(std::bind(
      &Aer::rgbdCallback, this, std::placeholders::_1, std::placeholders::_2,
      std::placeholders::_3, std::placeholders::_4));
}

Aer::~Aer() {}

void Aer::setLoggerWidthHeight_callback(const int width, const int height,
                                        const double fx, const double fy,
                                        const double cx, const double cy,
                                        const int max_vertex_count) {
  mylogger.setCamWidthAndheight(width, height, fx, fy, cx, cy,
                                max_vertex_count);
  myElasticFusion.setCamWidthAndheight(width, height, fx, fy, cx, cy,
                                       max_vertex_count);
}

void Aer::rgbdCallback(unsigned char *image, TangoPointCloud *pointcloud_buffer,
                       double cameraTime, TangoPoseData pose) {
  if (userMode.getValue() == aer::EFMODE) {
    myElasticFusion.rgbdCallback(image, pointcloud_buffer, cameraTime, pose);
  } else if (userMode.getValue() == aer::WRITEDATAMODE) {
    mylogger.rgbdCallback(image, pointcloud_buffer, cameraTime, pose);
  }
}

bool Aer::aerStartWriting(bool startWriting) {
  if (startWriting && (userMode.getValue() == -1)) {
    userMode.assignValue(aer::WRITEDATAMODE);
    mylogger.startWriting();
    return true;
  } else if (!startWriting && (userMode.getValue() == aer::WRITEDATAMODE)) {
    userMode.assignValue(-1);
    mylogger.stopWriting();
    tango_interface::CameraInterface::countFrame.assignValue(-1);
    return true;
  } else {
    return false;
  }
}

// Run ElasticFusion
bool Aer::aerStartElasticFusion(bool startElasticFusion) {
  if (startElasticFusion && (userMode.getValue() == -1)) {
    userMode.assignValue(aer::EFMODE);
    myElasticFusion.startElasticFusion();
    return true;
  } else if (!startElasticFusion && (userMode.getValue() == aer::EFMODE)) {
    userMode.assignValue(-1);
    myElasticFusion.stopElasticFusion();
    tango_interface::CameraInterface::countFrame.assignValue(-1);
    return true;
  } else {
    return false;
  }
}

bool Aer::aerStartRundataset(bool startRundataset) {
  if (startRundataset && (userMode.getValue() == -1)) {
    userMode.assignValue(aer::DATASETMODE);
    runDatasetEF.startRunDatasetEFDataSet();
    return true;
  } else if (!startRundataset && (userMode.getValue() == aer::DATASETMODE)) {
    runDatasetEF.stopRunDatasetEFDataSet();
    userMode.assignValue(-1);
    tango_interface::CameraInterface::countFrame.assignValue(-1);
    return true;
  } else {
    return false;
  }
}

bool Aer::aerStartRuntangoRGBDData(bool startRuntangoRGBDData) {
  if (startRuntangoRGBDData && (userMode.getValue() == -1)) {
    userMode.assignValue(aer::TANGORGBDDATAMODE);
    tangoRGBDData.startRunTangoRGBDPoseData(false);
    return true;
  } else if (!startRuntangoRGBDData && (userMode.getValue() == aer::TANGORGBDDATAMODE)) {
    tangoRGBDData.stopRunTangoRGBDPoseData();
    userMode.assignValue(-1);
    tango_interface::CameraInterface::countFrame.assignValue(-1);
    return true;
  } else {
    return false;
  }
}

bool Aer::aerRuntangoRGBDPoseData(bool startRuntangoRGBDPoseData) {
  if (startRuntangoRGBDPoseData && (userMode.getValue() == -1)) {
    userMode.assignValue(aer::TANGORGBDPOSEDATAMODE);
    tangoRGBDData.startRunTangoRGBDPoseData(true);
    return true;
  } else if (!startRuntangoRGBDPoseData && (userMode.getValue() == aer::TANGORGBDPOSEDATAMODE)) {
    tangoRGBDData.stopRunTangoRGBDPoseData();
    userMode.assignValue(-1);
    tango_interface::CameraInterface::countFrame.assignValue(-1);
    return true;
  } else {
    return false;
  }
}

// Save elasticFusion Ply file
bool Aer::savePlyFile() {
  if (userMode.getValue() == aer::DATASETMODE) {
    runDatasetEF.savePly();
    return true;
  } else if (userMode.getValue() == aer::EFMODE) {
    myElasticFusion.savePly();
    return true;
  } else if (userMode.getValue() == aer::TANGORGBDDATAMODE || userMode.getValue() == aer::TANGORGBDPOSEDATAMODE) {
    tangoRGBDData.savePly();
    return true;
  } else {
    return false;
  }
}

} // namespace aer
