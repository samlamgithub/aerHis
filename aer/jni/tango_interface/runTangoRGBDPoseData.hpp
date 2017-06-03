/*
 * This file is part of ElasticFusion.
 *
 * Copyright (C) 2015 Imperial College London
 *
 * The use of the code within this file and all code within files that
 * make up the software that is ElasticFusion is permitted for
 * non-commercial purposes only.  The full terms and conditions that
 * apply to the code within this file are detailed within the LICENSE.txt
 * file and at
 * <http://www.imperial.ac.uk/dyson-robotics-lab/downloads/elastic-fusion/elastic-fusion-license/>
 * unless explicitly stated.  By downloading this file you agree to
 * comply with these terms.
 *
 * If you wish to use any of this code for commercial purposes then
 * please email researchcontracts.engineering@imperial.ac.uk.
 *
 */

#ifndef RUNTANGORGBDPOSEDATA_HPP_
#define RUNTANGORGBDPOSEDATA_HPP_

#include <../elasticfusion/ElasticFusion.h>
#include <../elasticfusion/Utils/Img.h>
#include <../elasticfusion/Utils/Resolution.h>
#include <../elasticfusion/Utils/Stopwatch.h>
#include <tango_interface/camera_interface.hpp>

#include "ThreadMutexObject.hpp"
#include "util.hpp"
#include <GLES3/gl3.h>
#include <cassert>
#include <cstdarg>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <jni.h>
#include <limits>
#include <memory>
#include <opencv2/opencv.hpp>
#include <poll.h>
#include <stack>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <vector>
#include <zlib.h>
#define __gl2_h_
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>
// #include "JPEGLoader.hpp"

#include "GroundTruthOdometry.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace tango_interface {

class RunTangoRGBDPoseData {
public:
  RunTangoRGBDPoseData();

  virtual ~RunTangoRGBDPoseData();

  void getNext();

  void getBack();

  int getNumFrames();

  bool hasMore();

  bool rewound();

  bool file_exists(const std::string &filename) const;
  std::string to_string(int) const;
  std::string current_date_time() const;

  void fastForward(int frame);

  const std::string getFile();

  void setAuto(bool value);

  void savePly();

  void startRunTangoRGBDPoseData(bool hasPose);
  void stopRunTangoRGBDPoseData();

  boost::thread *RunTangoRGBDPoseDataThread;

  ThreadMutexObject<bool> runningTangoRGBDPoseData;
  ThreadMutexObject<bool> shouldSavePly;

  void runEF(bool hasPose);

  std::stack<int> filePointers;

  // bool flipColors;
  int64_t timestamp;

  unsigned short *depth;
  unsigned char *rgb;
  int currentFrame;

private:
  Bytef *decompressionBufferDepth;
  Bytef *decompressionBufferImage;
  unsigned char *depthReadBuffer;
  unsigned char *imageReadBuffer;
  int32_t depthSize;
  int32_t imageSize;

  GroundTruthOdometry *groundTruthOdometry;

  std::string file;
  FILE *fp;
  int32_t numFrames;
  int width;
  int height;
  int numPixels;

  // JPEGLoader jpeg;
  void getCore();
};
}

#endif /* RUNTANGORGBDPOSEDATA_HPP_ */
