/*
 * This file is part of RunTangoRGBDPoseData.
 *
 * Copyright (C) 2015 Imperial College London
 *
 * The use of the code within this file and all code within files that
 * make up the software that is RunTangoRGBDPoseData is permitted for
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

static const char *glErrorStringTANGODATA(GLenum err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "error Invalid Enum";
  case GL_INVALID_VALUE:
    return "error Invalid Value";
  case GL_INVALID_OPERATION:
    return "error Invalid Operation";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "error GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "error Out of Memory";
  default:
    return "error runTangoRGBDPoseData Unknown GL Error";
  }
}

inline const char *glCheckFramebufferStatusTANGODATA() {
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return "tangoRGBDData GL_FRAMEBUFFER_COMPLETE";
  } else if (status == GL_FRAMEBUFFER_UNDEFINED) {
    return "tangoRGBDData  GL_FRAMEBUFFER_UNDEFINED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    return "tangoRGBDData GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    return "tangoRGBDData GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    return "tangoRGBDData GL_FRAMEBUFFER_UNSUPPORTED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    return "tangoRGBDData GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
  } else if (status == GL_INVALID_ENUM) {
    return "tangoRGBDData GL_INVALID_ENUM";
  } else {
    LOGI("glCheckFramebufferStatus else: %d", status);
    char integer_string[32];
    int integer = status;
    sprintf(integer_string, "%d", status);
    char other_string[64] = "tangoRGBDData glCheckFramebufferStatus else: ";
    strcat(other_string, integer_string);
    return other_string;
  }
}

inline void check_gl_errorTANGODATA() {
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      LOGI("tangoRGBDData frame buffer error: %s", glCheckFramebufferStatusTANGODATA());
  }
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_error tangoRGBDData CheckGlDieOnError %s after "
         "%s() glError (0x%x)\n",
         glCheckFramebufferStatusTANGODATA(), glErrorStringTANGODATA(error), error);
  }
}

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
  // RunTangoRGBDPoseDataThread->join();
}

void RunTangoRGBDPoseData::startRunTangoRGBDPoseData(bool hasPose) {
  assert(!RunTangoRGBDPoseDataThread && !runningTangoRGBDPoseData.getValue());
  LOGI("RunTangoRGBDPoseData startRunTangoRGBDPoseData running hasPose: %d", hasPose);
  runningTangoRGBDPoseData.assignValue(true);
  RunTangoRGBDPoseDataThread =
      new boost::thread(boost::bind(&RunTangoRGBDPoseData::runEF,this, hasPose));
  LOGI("RunTangoRGBDPoseData startRunTangoRGBDPoseData running done");
}

void RunTangoRGBDPoseData::stopRunTangoRGBDPoseData() {
  assert(RunTangoRGBDPoseDataThread && runningTangoRGBDPoseData.getValue());
  LOGI("RunTangoRGBDPoseData stopRunTangoRGBDPoseData running");
  runningTangoRGBDPoseData.assignValue(false);
  RunTangoRGBDPoseDataThread->join();
  // delete[] depthReadBuffer;
  // delete[] imageReadBuffer;
  // delete[] decompressionBufferDepth;
  // delete[] decompressionBufferImage;

  fclose(fp);

  runningTangoRGBDPoseData.assignValue(false);
  shouldSavePly.assignValue(false);
  RunTangoRGBDPoseDataThread = 0;
  LOGI("RunTangoRGBDPoseData stopRunTangoRGBDPoseData running done");
}

void RunTangoRGBDPoseData::savePly() {
  assert(RunTangoRGBDPoseDataThread && runningTangoRGBDPoseData.getValue());
  LOGI("RunTangoRGBDPoseData is running, should save ply");
  shouldSavePly.assignValue(true);
  LOGI("RunTangoRGBDPoseData is running, should save ply done");
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
  LOGI("RunTangoRGBDPoseData egl context setup start ...");
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
    LOGI("RunTangoRGBDPoseData egl  eglGetDisplay failed");
  }

  EGLint major, minor;
  if (!eglInitialize(display, &major, &minor) || eglGetError() != EGL_SUCCESS) {
    LOGI("RunTangoRGBDPoseData egl  eglInitialize failed error");
  }
  LOGI("RunTangoRGBDPoseData runEF EGL init with version %d.%d", major, minor);

  EGLint numConfigs;
  EGLConfig config;

  if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) ||
      eglGetError() != EGL_SUCCESS) {
    LOGI("RunTangoRGBDPoseData egl eglChooseConfig failed");
  }

  EGLSurface surface;
  const EGLint surfaceAttr[] = {EGL_WIDTH, 1280, EGL_HEIGHT, 720, EGL_NONE};

  EGLContext context;

  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS) {
    LOGI("RunTangoRGBDPoseData egl eglCreateContext failed");
  }

  // PixmapSurface和PBufferSurface
  surface = eglCreatePbufferSurface(display, config, surfaceAttr);
  //	    surface = eglCreateWindowSurface(display, config, mWindow, NULL);
  if (surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS) {
    LOGI("RunTangoRGBDPoseData egl eglCreatePbufferSurface failed");
  }

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE ||
      eglGetError() != EGL_SUCCESS) {
    LOGI("RunTangoRGBDPoseData egl eglMakeCurrent failed");
  }

  if (eglGetError() != EGL_SUCCESS) {
    LOGI("RunTangoRGBDPoseData egl may failed");
  }

  LOGI("RunTangoRGBDPoseData egl context setup done ...");
  //==================================
  bool success = LoadOpenGLExtensionsManually();
  if (!success) {
    LOGE("RunTangoRGBDPoseData LoadOpenGLExtensionsManually failed");
  } else {
    LOGE("RunTangoRGBDPoseData LoadOpenGLExtensionsManually success");
  }
  //============================
  LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData Initialising ...");

  Resolution::getInstance(myWidth, myHeight);
  Intrinsics::getInstance(1042.110000, 1042.370000, 637.475000, 358.318000);

  file = "/sdcard/tangoDataRGBD.klg";
  struct stat st;
  int result = stat(file.c_str(), &st);
  if (result != 0) {
    LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData file not exist");
  }
  assert(result == 0);
  // assert(pangolin::FileExists(file.c_str()));

  fp = fopen(file.c_str(), "rb");

  if (hasPose) {
    LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData hasPose file");
    std::string poseFile = "/sdcard/tangoDataPose.klg";
    struct stat st2;
    int result2 = stat(poseFile.c_str(), &st2);
    if (result2 != 0) {
      LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData poseFile not exist");
    }
    assert(result2 == 0);
    groundTruthOdometry = new GroundTruthOdometry(poseFile);
  }

  currentFrame = 0;

  assert(fread(&numFrames, sizeof(int32_t), 1, fp));
  LOGI("RunTangoRGBDPoseData this dataset has num frame: %d", numFrames);

  depthReadBuffer = new unsigned char[numPixels * 2];
  imageReadBuffer = new unsigned char[numPixels * 4];
  decompressionBufferDepth =
      new Bytef[Resolution::getInstance().numPixels() * 2];
  decompressionBufferImage =
      new Bytef[Resolution::getInstance().numPixels() * 4];

  LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData Initialising done ...");
  LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData Setting parameters...");

  float confidence = 0.40f;   // fusion的confidence阈值
  float depthThre = 12.0f;     //去掉depth大于某个阈值的帧
  float icp = 10.0f;          // icp的阈值
  float icpErrThresh = 5e-05; // icp错误阈值
  float covThresh = 1e-05;
  float photoThresh = 115;
  float fernThresh = 0.3095f; //新fern的阈值
  int timeDelta = 200;
  int icpCountThresh = 35000;

  bool bootstrap = false;

  // int start = 1;
  // int end = std::numeric_limits<unsigned short>::max(); //Funny bound, since
  // we predict times in this format really!
  // bool openLoop = 0; //open loop模式：不开启
  bool openLoop = 1; // open loop模式：开启
  bool iclnuim = 0;  //使用icl dataset:不使用
  bool reloc = 0;    //重定位模式：先做重建，不开启重定位
  // bool fastOdom = 0; //Fast odometry (single level pyramid) mode :不开启
  bool fastOdom = 1; // Fast odometry (single level pyramid) mode :开启
  // bool so3 = 1; //SO(3) pre-alignment in tracking：开启
  bool so3 = 0;             // SO(3) pre-alignment in tracking：不开启
  bool frameToFrameRGB = 0; //只做rgb图像的tracking：不开启
  // int timestamp = 0;

  LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData Setting parameters done.");
  LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData Building eFusion...");
  // pangolin::Params windowParams;
  // windowParams.Set("SAMPLE_BUFFERS", 0);
  // windowParams.Set("SAMPLES", 0);
  // pangolin::CreateWindowAndBind("Main", width, height, windowParams);

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
  LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData Building eFusion done");
  check_gl_errorTANGODATA();
  //待处理文件的位置和下标
  // std::string filedir = "../pic/";
  // int file_start = 1;
  // int file_end = 782;
  std::vector<Eigen::Matrix4f> posesEigen;
  int32_t processedFrameCount = 0;
  // size_t result1 = fwrite(&numFrames, sizeof(int32_t), 1, RGBlog_file_);
  // LOGI("Logger fwrite: %d", result1);
  //    int result = fputs("\n:testest     \n", log_file_);
  //    LOGI("Logger puts: %d", result);
  check_gl_errorTANGODATA();
  LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData: good");

  // start of while loop
  while (runningTangoRGBDPoseData.getValueWait(1)) {
    check_gl_errorTANGODATA();
    LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData: outter while loop in");

    while (hasMore() && runningTangoRGBDPoseData.getValueWait(1)) {
      check_gl_errorTANGODATA();
      LOGI("RunTangoRGBDPoseData RunTangoRGBDPoseData: hasMore while loop in");
      getNext();
      check_gl_errorTANGODATA();
      LOGI("RunTangoRGBDPoseData Processing frames ready done.");
      // Eigen::Matrix4f *currentPose = 0;
      if (hasPose) {
        LOGI("RunTangoRGBDPoseData processFrame hasPose");
        Eigen::Matrix4f *inputPose = 0;
        inputPose = new Eigen::Matrix4f;
        inputPose->setIdentity();
        *inputPose = groundTruthOdometry->getTransformation(timestamp);

        Eigen::IOFormat CleanFmt1(4, 0, ", ", "\n", "[", "]");
        std::stringstream ss;
        ss << inputPose->format(CleanFmt1);
        std::string poseStr(ss.str());
        LOGI("RunTangoRGBDPoseDatainput pose is : %s", poseStr.c_str());

        eFusion.processFrame(rgb, depth, timestamp, inputPose, bootstrap);

        check_gl_errorTANGODATA();
        LOGI("RunTangoRGBDPoseData Processing frames hasPose done.");
        if (inputPose) {
          delete inputPose;
        }
      } else {
        LOGI("RunTangoRGBDPoseData processFrame has no pose");
        eFusion.processFrame(rgb, depth, timestamp);
        check_gl_errorTANGODATA();
        LOGI("RunTangoRGBDPoseData Processing frames  has no pose done.");
      }

      // if (currentPose) {
      //   delete currentPose;
      // }
      check_gl_errorTANGODATA();
      LOGI("RunTangoRGBDPoseData Processing frames total done.");

      Eigen::Matrix4f currPose = eFusion.getCurrPose();

      check_gl_errorTANGODATA();
      LOGI("RunTangoRGBDPoseData eFusion.getCurrPose()");

      posesEigen.push_back(currPose);

      Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
      std::stringstream ss1;
      ss1 << currPose.format(CleanFmt);
      std::string str(ss1.str());

      LOGI("output pose is : %s", str.c_str());
      LOGI("RunTangoRGBDPoseData Log processing result start.");
      //查看处理的结果
      delete &currPose;
      int time = eFusion.getTick(); //查看此时eFusion的系统时间
      LOGI("RunTangoRGBDPoseData Log processing result time : %d ", time);

      //  Ferns keyfern = eFusion.getFerns();//关键帧dataset

      int ld_num = eFusion.getDeforms(); //局部deformations的数量
LOGI("RunTangoRGBDPoseData Log processing result eFusion.getDeforms(): %d ", ld_num);
      // Deformation ld = eFusion.getLocalDeformation(); //局部deformation图

      int gd_num = eFusion.getFernDeforms(); //全局deformations的数量
  LOGI("RunTangoRGBDPoseData Log processing result eFusion.getFernDeforms(): %d ", gd_num);
      // GlobalModel gm = eFusion.getGlobalModel(); //全局deformation model:

      bool isLost = eFusion.getLost();
      if (isLost) {
        LOGI("RunTangoRGBDPoseData Log processing result is lost");
      } else {
        LOGI("RunTangoRGBDPoseData Log processing result is not lost");
      }

      int CloudPoint_num = eFusion.getGlobalModel().lastCount(); //点云的点数量
      LOGI("RunTangoRGBDPoseData Log processing result "
           "eFusion.globalModel.lastCount(): "
           "totalPoints :%d ",
           CloudPoint_num);

      int totalNodes = eFusion.getLocalDeformation().getGraph().size();
      LOGI("RunTangoRGBDPoseData Log processing result "
           "eFusion.getLocalDeformation().getGraph().size(): totalNodes : %d",
           totalNodes);

           int totalFerns = eFusion.getFerns().frames.size();
           LOGI("RunTangoRGBDPoseData Log processing result "
                "eFusion.getFerns().frames.size(): totalFerns : %d",
                totalFerns);

      int totalDefs = eFusion.getDeforms();
      LOGI("RunTangoRGBDPoseData Log processing result "
           "eFusion.getDeforms(): totalDefs :%d ",
           totalDefs);

      int totalFernDefs = eFusion.getFernDeforms();
      LOGI("RunTangoRGBDPoseData Log processing result "
           "eFusion.getFernDeforms(): totalFernDefs :%d ",
           totalFernDefs);

      float lastICPError = eFusion.getModelToModel().lastICPError;
      float lastICPCount = eFusion.getModelToModel().lastICPCount;
      LOGI("RunTangoRGBDPoseData Log processing result lastICPError: %f, "
           "icpErrThresh: %f",
           lastICPError, 5e-05);
      LOGI("RunTangoRGBDPoseData Log processing result lastICPCount: %f, "
           "icpCountThresh: %f",
           lastICPCount, 35000.00);
      LOGI("RunTangoRGBDPoseData Log processing result done.");
      // LOGI("saving cloud points..." );
      // eFusion.savePly();			 //保存当前的点云图至ply
      // LOGI("cloud point has saved to " << savefilename << ".ply" );
      LOGI("RunTangoRGBDPoseData Log processing result done.");

      if (shouldSavePly.getValueWait()) {
        LOGI("RunTangoRGBDPoseData start to save frame.");

        float confidenceThreshold = eFusion.getConfidenceThreshold();

        LOGI("RunTangoRGBDPoseData start to save frame. 0 confidenceThreshold: %f", confidenceThreshold);

        unsigned int lastCount = CloudPoint_num;

        Eigen::Vector4f *mapData = eFusion.savePly();

        LOGI("RunTangoRGBDPoseData start to save frame 1. lastCount: %d, "
             "confidenceThreshold: %f",
             lastCount, confidenceThreshold);

        std::string plyFilename("/sdcard/RunTangoRGBDPoseDataPly_" +
                                current_date_time());
        plyFilename.append(".ply");

        LOGI("RunTangoRGBDPoseData start to save frame 1 1");
        // Open file
        std::ofstream fs;
        fs.open(plyFilename.c_str());
        // File* plyFile = fopen(plyFilename.c_str(),"wb+");
        if (fs == NULL) {
          LOGE("RunTangoRGBDPoseData There was a problem opening the ply file: %s, error",
               plyFilename.c_str());
        }

        LOGI("RunTangoRGBDPoseData start to save frame. 2");
        int validCount = 0;

        for (unsigned int i = 0; i < lastCount; i++) {
          Eigen::Vector4f pos = mapData[(i * 3) + 0];

    // LOGI("RunTangoRGBDPoseData save frame: pos[3]: %f", pos[3]);
          if (pos[3] > confidenceThreshold) {
          // LOGI("RunTangoRGBDPoseData save frame: pos[3]: %f", pos[3]);
            validCount++;
          }
        }

        LOGI("RunTangoRGBDPoseData start to save frame. 3 validCount: %d", validCount);

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
        // delete[] mapData;
        LOGI("RunTangoRGBDPoseData start to save frame. 4");
        // Open file in binary appendable
        std::ofstream fpout(plyFilename.c_str(),
                            std::ios::app | std::ios::binary);

        LOGI("RunTangoRGBDPoseData start to save frame. 5");

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
        LOGI("RunTangoRGBDPoseData start to save frame 6. countWriteNum: %d", countWriteNum);
        // Close file
        fs.close();
        LOGI("RunTangoRGBDPoseData start to save frame. 7");
        delete[] mapData;

        shouldSavePly.assignValue(false);
        LOGI("RunTangoRGBDPoseData save frame done.");
      }

      LOGI("RunTangoRGBDPoseData: processing++");
      processedFrameCount++;
      LOGI("RunTangoRGBDPoseData: processed one frame : %d", processedFrameCount);
      tango_interface::CameraInterface::incrementCounter();
    }

    delete[] depthReadBuffer;
    delete[] imageReadBuffer;
    delete[] decompressionBufferDepth;
    delete[] decompressionBufferImage;

    fclose(fp);
    LOGI("RunTangoRGBDPoseData deleting");
    // delete &eFusion;
    // eFusion = NULL;
    LOGI("RunTangoRGBDPoseData done: done");
    break;
  }
  // end of while loop

  LOGI("RunTangoRGBDPoseData tangoRGBDData done: done");
}

void RunTangoRGBDPoseData::getBack() {
  assert(filePointers.size() > 0);

  fseek(fp, filePointers.top(), SEEK_SET);

  filePointers.pop();

  getCore();
}

void RunTangoRGBDPoseData::getNext() {
  LOGI("RunTangoRGBDPoseData tangoRGBDData getNext: start");
  filePointers.push(ftell(fp));

  getCore();
  LOGI("RunTangoRGBDPoseData tangoRGBDData getNext: done");
}

void RunTangoRGBDPoseData::getCore() {
  LOGI("RunTangoRGBDPoseData tangoRGBDData getCore: start");
  assert(fread(&timestamp, sizeof(int64_t), 1, fp));

  assert(fread(&depthSize, sizeof(int32_t), 1, fp));
  assert(fread(&imageSize, sizeof(int32_t), 1, fp));

  assert(fread(depthReadBuffer, depthSize, 1, fp));

  if (imageSize > 0) {
    assert(fread(imageReadBuffer, imageSize, 1, fp));
  }

  LOGI("RunTangoRGBDPoseData tangoRGBDData imageSize: %d, depthSize: %d",
       imageSize, depthSize);

  if (depthSize == numPixels * 2) {
    LOGI("RunTangoRGBDPoseData tangoRGBDData depthSize no need to compress");
    memcpy(&decompressionBufferDepth[0], depthReadBuffer, numPixels * 2);
  } else {
    LOGI("RunTangoRGBDPoseData tangoRGBDData depthSize compressed");
    unsigned long decompLength = numPixels * 2;
    uncompress(&decompressionBufferDepth[0], (unsigned long *)&decompLength,
               (const Bytef *)depthReadBuffer, depthSize);
  }

  if (imageSize == numPixels * 4) {
    LOGI("RunTangoRGBDPoseData tangoRGBDData getCore: rgb not compressed: imageSize == numPixels * 4");
    memcpy(&decompressionBufferImage[0], imageReadBuffer, numPixels * 4);
  } else if (imageSize == numPixels * 3) {
    LOGI("RunTangoRGBDPoseData tangoRGBDData getCore: rgb not compressed: imageSize == numPixels * 3");
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
    // memcpy(&decompressionBufferImage[0], imageReadBuffer, numPixels * 4);
  } else if (imageSize > 0) {
    LOGI("RunTangoRGBDPoseData tangoRGBDData getCore: rgb  compressed");

    // jpeg.readData(imageReadBuffer, imageSize, (unsigned char *)&decompressionBufferImage[0]);
    // LOGI("RunTangoRGBDPoseData tangoRGBDData getCore: rgb uncompress: "
    //       "numPixels * 4: %d, imageSize: %d", numPixels * 4, imageSize);

    // int    nSize = ...       // Size of buffer
    // uchar* pcBuffer = ...    // Raw buffer data
    //
    //
    // // Create a Size(1, nSize) Mat object of 8-bit, single-byte elements

    cv::Mat rawData = cv::Mat(1, numPixels * 3, CV_8UC1, imageReadBuffer);
    //
    cv::Mat decodedImage = cv::imdecode(rawData, 1 /*, flags */);
    if (decodedImage.data == NULL) {
      LOGI("RunTangoRGBDPoseData decodedImage.data == NULL");

      memset(&decompressionBufferImage[0], 0, numPixels * 4);
      // Error reading raw image data
    } else {
      LOGI("RunTangoRGBDPoseData tangoRGBDData getCore: rgb  uncompress: "
           "numPixels * 3: "
           "%d, imageSize: %d, cv size: %d",
           numPixels * 3, imageSize,
           decodedImage.total() * decodedImage.elemSize());
      // decompressionBufferImage = (Bytef *)decodedImage.data;

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

      // memcpy(&decompressionBufferImage[0], decodedImage.data, numPixels * 3);
      //  memcpy(&decompressionBufferImage[0], decodedImage.data,
      //         imageSize);
    }
  } else {
    LOGI("RunTangoRGBDPoseData tangoRGBDData getCore: rgb  failed");
    memset(&decompressionBufferImage[0], 0, numPixels * 4);
  }

  depth = (unsigned short *)decompressionBufferDepth;

  // LOGI("-------------------------------------------------");
  // for (int j= 0; j < 640*480; j++) {
  //   if (depth[j] != 0) {
  //        LOGI("depth ==== %d: %d", j, depth[j]);
  //   }
  // }
  //   for (int i = 0; i < 480*2; i++) {
  //     LOGI("depth ========= %u, %u, %u, %u, %u, %u, %u, %u,  %u, %u, %u, %u,
  //     %u, %u, %u, %u, ",
  //  depth[16*i],
  // depth[16*i+1],
  // depth[16*i+2],
  // depth[16*i+3],
  // depth[16*i+4],
  // depth[16*i+5],
  // depth[16*i+6],
  // depth[16*i+7],
  // depth[16*i+8],
  // depth[16*i+9],
  // depth[16*i+9],
  // depth[16*i+10],
  // depth[16*i+11],
  // depth[16*i+12],
  // depth[16*i+13],
  // depth[16*i+14]);
  //   }
  // LOGI("==================================================");

  rgb = (unsigned char *)&decompressionBufferImage[0];

  // if (flipColors) {
  //   for (int i = 0; i < Resolution::getInstance().numPixels() * 3; i += 3) {
  //     std::swap(rgb[i + 0], rgb[i + 2]);
  //   }
  // }

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
