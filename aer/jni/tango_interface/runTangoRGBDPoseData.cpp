/*
 * This file is part of RunTangoRGBDPoseData.
 *
 * Copyright (C) 2015 Imperial College London
 *
 * The use of the code within this file and all code within files that
 * make up the software that is is permitted for
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

#include "runTangoRGBDPoseData.hpp"

namespace tango_interface {

const int myWidth = 1280;
const int myHeight = 720;

RunTangoRGBDPoseData::RunTangoRGBDPoseData()
    : RunTangoRGBDPoseDataThread(0), numFrames(0), timestamp(0), depth(0), rgb(0),
      currentFrame(0), decompressionBufferDepth(0), decompressionBufferImage(0),
      file("file"), width(myWidth), height(myHeight),
      numPixels(myWidth * myHeight) {
  runningTangoRGBDPoseData.assignValue(false);
  shouldSavePly.assignValue(false);
}

RunTangoRGBDPoseData::~RunTangoRGBDPoseData() {
  delete[] depthReadBuffer;
  delete[] imageReadBuffer;
  delete[] decompressionBufferDepth;
  delete[] decompressionBufferImage;

  fclose(fp);

  if (groundTruthOdometry) {
    delete groundTruthOdometry;
  }

  runningTangoRGBDPoseData.assignValue(false);
  shouldSavePly.assignValue(false);
}

void RunTangoRGBDPoseData::startRunTangoRGBDPoseData(bool hasPose) {
  assert(!RunTangoRGBDPoseDataThread && !runningTangoRGBDPoseData.getValue());
  LOGI("startrunning hasPose: %d", hasPose);
  runningTangoRGBDPoseData.assignValue(true);
  RunTangoRGBDPoseDataThread =
      new boost::thread(boost::bind(&RunTangoRGBDPoseData::runEF,this, hasPose));
  LOGI("startrunning done");
}

void RunTangoRGBDPoseData::stopRunTangoRGBDPoseData() {
  assert(RunTangoRGBDPoseDataThread && runningTangoRGBDPoseData.getValue());
  runningTangoRGBDPoseData.assignValue(false);
  RunTangoRGBDPoseDataThread->join();

  fclose(fp);

  runningTangoRGBDPoseData.assignValue(false);
  shouldSavePly.assignValue(false);
  RunTangoRGBDPoseDataThread = 0;
}

void RunTangoRGBDPoseData::savePly() {
  assert(RunTangoRGBDPoseDataThread && runningTangoRGBDPoseData.getValue());
  shouldSavePly.assignValue(true);
}

bool RunTangoRGBDPoseData::file_exists(const std::string &filename) const {
  struct stat st;
  int result = stat(filename.c_str(), &st);
  return (result == 0);
}

std::string RunTangoRGBDPoseData::to_string(int value) const {
  char buf[128];
  snprintf(buf, 128, "%d", value);
  return std::string(buf);
}

std::string RunTangoRGBDPoseData::current_date_time() const {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
  return std::string(buf);
}

void RunTangoRGBDPoseData::runEF(bool hasPose) {
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

  // PixmapSurface和PBufferSurface
  surface = eglCreatePbufferSurface(display, config, surfaceAttr);
  //	    surface = eglCreateWindowSurface(display, config, mWindow, NULL);
  if (surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS) {
    LOGI("eglCreatePbufferSurface failed");
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

  Resolution::getInstance(myWidth, myHeight);
  Intrinsics::getInstance(1042.110000, 1042.370000, 637.475000, 358.318000);

  file = "/sdcard/tangoDataRGBD.klg";
  struct stat st;
  int result = stat(file.c_str(), &st);
  if (result != 0) {
    LOGE("dataset file not exist");
  }
  assert(result == 0);

  fp = fopen(file.c_str(), "rb");

  if (hasPose) {
    std::string poseFile = "/sdcard/tangoDataPose.klg";
    struct stat st2;
    int result2 = stat(poseFile.c_str(), &st2);
    if (result2 != 0) {
      LOGI("dataset poseFile does not exist");
    }
    assert(result2 == 0);
    groundTruthOdometry = new GroundTruthOdometry(poseFile);
  }

  currentFrame = 0;

  assert(fread(&numFrames, sizeof(int32_t), 1, fp));

  depthReadBuffer = new unsigned char[numPixels * 2];
  imageReadBuffer = new unsigned char[numPixels * 4];
  decompressionBufferDepth =
      new Bytef[Resolution::getInstance().numPixels() * 2];
  decompressionBufferImage =
      new Bytef[Resolution::getInstance().numPixels() * 4];

  float confidence = 0.40f;
  float depthThre = 12.0f;
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
  bool fastOdom = 1;
  bool so3 = 0;
  bool frameToFrameRGB = 0;

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);

  glEnable(GL_DEPTH_TEST);
  glDepthMask(GL_TRUE);
  glDepthFunc(GL_LESS);

  ElasticFusion eFusion(openLoop ? std::numeric_limits<int>::max() / 2
                                 : timeDelta,
                        icpCountThresh, icpErrThresh, covThresh, !openLoop,
                        iclnuim, reloc, photoThresh, confidence, depthThre, icp,
                        fastOdom, fernThresh, so3, frameToFrameRGB);

  std::vector<Eigen::Matrix4f> posesEigen;
  int32_t processedFrameCount = 0;

  // start of while loop
  while (runningTangoRGBDPoseData.getValueWait(1)) {
    while (hasMore() && runningTangoRGBDPoseData.getValueWait(1)) {
      getNext();

      if (hasPose) {
        Eigen::Matrix4f *inputPose = 0;
        inputPose = new Eigen::Matrix4f;
        inputPose->setIdentity();
        *inputPose = groundTruthOdometry->getTransformation(timestamp);

        eFusion.processFrame(rgb, depth, timestamp, inputPose, bootstrap);

        if (inputPose) {
          delete inputPose;
        }
      } else {
        eFusion.processFrame(rgb, depth, timestamp);
      }

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

        std::string plyFilename("/sdcard/RunTangoRGBDPoseDataPly_" +
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

      processedFrameCount++;
      tango_interface::CameraInterface::incrementCounter();
    }

    delete[] depthReadBuffer;
    delete[] imageReadBuffer;
    delete[] decompressionBufferDepth;
    delete[] decompressionBufferImage;

    fclose(fp);
    break;
  }
  // end of while loop

}

void RunTangoRGBDPoseData::getBack() {
  assert(filePointers.size() > 0);

  fseek(fp, filePointers.top(), SEEK_SET);

  filePointers.pop();

  getCore();
}

void RunTangoRGBDPoseData::getNext() {
  filePointers.push(ftell(fp));
  getCore();
}

void RunTangoRGBDPoseData::getCore() {
  assert(fread(&timestamp, sizeof(int64_t), 1, fp));

  assert(fread(&depthSize, sizeof(int32_t), 1, fp));
  assert(fread(&imageSize, sizeof(int32_t), 1, fp));

  assert(fread(depthReadBuffer, depthSize, 1, fp));

  if (imageSize > 0) {
    assert(fread(imageReadBuffer, imageSize, 1, fp));
  }

  if (depthSize == numPixels * 2) {
    memcpy(&decompressionBufferDepth[0], depthReadBuffer, numPixels * 2);
  } else {
    unsigned long decompLength = numPixels * 2;
    uncompress(&decompressionBufferDepth[0], (unsigned long *)&decompLength,
               (const Bytef *)depthReadBuffer, depthSize);
  }

  if (imageSize == numPixels * 4) {
    memcpy(&decompressionBufferImage[0], imageReadBuffer, numPixels * 4);
  } else if (imageSize == numPixels * 3) {
    unsigned char * decomBuffer = (unsigned char *)&decompressionBufferImage[0];
    unsigned int rgbaCount = 0;
    for (int h = 0; h < height; h++) {
      for (int w = 0; w < width; w+=3) {
        int basePosition = h * width + w;
        unsigned char * bgr = imageReadBuffer+ basePosition;
        decomBuffer[rgbaCount] = bgr[2];
        decomBuffer[rgbaCount + 1] = bgr[1];
        decomBuffer[rgbaCount + 2] = bgr[0];
        decomBuffer[rgbaCount + 3] = (unsigned char)255;
        rgbaCount += 4;
      }
    }
  } else if (imageSize > 0) {
    cv::Mat rawData = cv::Mat(1, numPixels * 3, CV_8UC1, imageReadBuffer);

    cv::Mat decodedImage = cv::imdecode(rawData, 1 /*, flags */);
    if (decodedImage.data == NULL) {
      memset(&decompressionBufferImage[0], 0, numPixels * 4);
      // Error reading raw image data
    } else {
      unsigned char * decomBuffer = (unsigned char *)&decompressionBufferImage[0];

      unsigned int rgbaCount = 0;
      for (int h = 0; h < height; h++) {
        for (int w = 0; w < width; w+=3) {
          int basePosition = h * width + w;
          unsigned char * bgr = (decodedImage.data)+ basePosition;
          decomBuffer[rgbaCount] = bgr[2];
          decomBuffer[rgbaCount + 1] = bgr[1];
          decomBuffer[rgbaCount + 2] = bgr[0];
          decomBuffer[rgbaCount + 3] = (unsigned char)255;
          rgbaCount += 4;
        }
      }
    }
  } else {
    memset(&decompressionBufferImage[0], 0, numPixels * 4);
  }

  depth = (unsigned short *)decompressionBufferDepth;

  rgb = (unsigned char *)&decompressionBufferImage[0];

  currentFrame++;
}

void RunTangoRGBDPoseData::fastForward(int frame) {
  while (currentFrame < frame && hasMore()) {
    filePointers.push(ftell(fp));

    assert(fread(&timestamp, sizeof(int64_t), 1, fp));

    assert(fread(&depthSize, sizeof(int32_t), 1, fp));
    assert(fread(&imageSize, sizeof(int32_t), 1, fp));

    assert(fread(depthReadBuffer, depthSize, 1, fp));

    if (imageSize > 0) {
      assert(fread(imageReadBuffer, imageSize, 1, fp));
    }

    currentFrame++;
  }
}

int RunTangoRGBDPoseData::getNumFrames() { return numFrames; }

bool RunTangoRGBDPoseData::hasMore() { return currentFrame + 1 < numFrames; }

bool RunTangoRGBDPoseData::rewound() {
  if (filePointers.size() == 0) {
    fclose(fp);
    fp = fopen(file.c_str(), "rb");

    assert(fread(&numFrames, sizeof(int32_t), 1, fp));

    currentFrame = 0;

    return true;
  }

  return false;
}

const std::string RunTangoRGBDPoseData::getFile() { return file; }

void RunTangoRGBDPoseData::setAuto(bool value) {}
}
