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

#ifndef RUNDATASET_HPP_
#define RUNDATASET_HPP_

#include <../elasticfusion/ElasticFusion.h>
#include <../elasticfusion/Utils/Img.h>
#include <../elasticfusion/Utils/Resolution.h>
#include <../elasticfusion/Utils/Stopwatch.h>

#include <opencv2/opencv.hpp>
#include "ThreadMutexObject.hpp"
#include "util.hpp"
#include <GLES3/gl3.h>
#include <cassert>
#include <iostream>
#include <poll.h>
#include <stack>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <zlib.h>
#define __gl2_h_ // what the f***
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition_variable.hpp>

// #include "JPEGLoader.hpp"

#include <EGL/egl.h>
#include <EGL/eglext.h>

namespace tango_interface {

class RunDatasetEF {
public:
  RunDatasetEF(std::string file);

  virtual ~RunDatasetEF();

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
  void startElasticFusionDataSet();
  void stopElasticFusionDataSet();
  boost::thread *elasticFusionThread;
  ThreadMutexObject<bool> runningElasticFusion;
  ThreadMutexObject<bool> shouldSavePly;
  void runEF();

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

  const std::string file;
  FILE *fp;
  int32_t numFrames;
  int width;
  int height;
  int numPixels;

  // JPEGLoader jpeg;
  void getCore();
};
}

#endif /* RUNDATASET_HPP_ */
