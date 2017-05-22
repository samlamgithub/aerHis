/*
 * This file is part of RunDatasetEF.
 *
 * Copyright (C) 2015 Imperial College London
 *
 * The use of the code within this file and all code within files that
 * make up the software that is RunDatasetEF is permitted for
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

#include "runDataset.hpp"

static const char *glErrorStringDS(GLenum err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "Invalid Enum";
  case GL_INVALID_VALUE:
    return "Invalid Value";
  case GL_INVALID_OPERATION:
    return "Invalid Operation";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  case GL_OUT_OF_MEMORY:
    return "error Out of Memory";
  default:
    return "Unknown Error";
  }
}

inline const char *glCheckFramebufferStatusDS() {
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return "run dataset GL_FRAMEBUFFER_COMPLETE";
  } else if (status == GL_FRAMEBUFFER_UNDEFINED) {
    return "run dataset  GL_FRAMEBUFFER_UNDEFINED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    return "run dataset GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    return "run dataset GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    return "run dataset GL_FRAMEBUFFER_UNSUPPORTED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    return "run dataset GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
  } else if (status == GL_INVALID_ENUM) {
    return "run dataset GL_INVALID_ENUM";
  } else {
    char integer_string[32];
    int integer = status;
    sprintf(integer_string, "%d", status);
    char other_string[64] = "run dataset else: ";
    strcat(other_string, integer_string);
    return other_string;
  }
}

inline void check_gl_errorDS() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_error run dataset CheckGlDieOnError %s after "
         "%s() glError (0x%x)\n",
         glCheckFramebufferStatusDS(), glErrorStringDS(error), error);
  }
}

namespace tango_interface {

const int myWidth = 640;
const int myHeight = 380;

RunDatasetEF::RunDatasetEF()
    : RunDatasetEFThread(0), numFrames(0), timestamp(0), depth(0), rgb(0),
      currentFrame(0), decompressionBufferDepth(0), decompressionBufferImage(0),
      file("file"), width(myWidth), height(myHeight),
      numPixels(myWidth * myHeight) {

  runningRunDatasetEF.assignValue(false);
  shouldSavePly.assignValue(false);
}

RunDatasetEF::~RunDatasetEF() {
  delete[] depthReadBuffer;
  delete[] imageReadBuffer;
  delete[] decompressionBufferDepth;
  delete[] decompressionBufferImage;

  fclose(fp);

  runningRunDatasetEF.assignValue(false);
  shouldSavePly.assignValue(false);
  RunDatasetEFThread->join();
}

void RunDatasetEF::startRunDatasetEFDataSet() {
  assert(!RunDatasetEFThread && !runningRunDatasetEF.getValue());
  LOGI("RunDatasetEF startRunDatasetEF running");
  runningRunDatasetEF.assignValue(true);
  RunDatasetEFThread =
      new boost::thread(boost::bind(&RunDatasetEF::runEF, this));
  LOGI("RunDatasetEF startRunDatasetEF running done");
}

void RunDatasetEF::stopRunDatasetEFDataSet() {
  assert(RunDatasetEFThread && runningRunDatasetEF.getValue());
  LOGI("RunDatasetEF stopRunDatasetEF running");
  runningRunDatasetEF.assignValue(false);
  RunDatasetEFThread->join();
  delete[] depthReadBuffer;
  delete[] imageReadBuffer;
  delete[] decompressionBufferDepth;
  delete[] decompressionBufferImage;

  fclose(fp);

  runningRunDatasetEF.assignValue(false);
  shouldSavePly.assignValue(false);
  RunDatasetEFThread = 0;
  LOGI("RunDatasetEF stopRunDatasetEF running done");
}

void RunDatasetEF::savePly() {
  assert(RunDatasetEFThread && runningRunDatasetEF.getValue());
  LOGI("RunDatasetEF is running, should save ply");
  shouldSavePly.assignValue(true);
  LOGI("RunDatasetEF is running, should save ply done");
}

bool RunDatasetEF::file_exists(const std::string &filename) const {
  struct stat st;
  int result = stat(filename.c_str(), &st);
  return (result == 0);
}

std::string RunDatasetEF::to_string(int value) const {
  char buf[128];
  snprintf(buf, 128, "%d", value);
  return std::string(buf);
}

std::string RunDatasetEF::current_date_time() const {
  time_t now = time(0);
  struct tm tstruct;
  char buf[80];
  tstruct = *localtime(&now);
  strftime(buf, sizeof(buf), "%Y-%m-%d_%H-%M-%S", &tstruct);
  return std::string(buf);
}

void RunDatasetEF::runEF() {
  LOGI("RunDatasetEF egl context setup start ...");
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
    LOGI("RunDatasetEF egl  eglGetDisplay failed");
  }

  EGLint major, minor;
  if (!eglInitialize(display, &major, &minor) || eglGetError() != EGL_SUCCESS) {
    LOGI("RunDatasetEF egl  eglInitialize failed");
  }
  LOGI("RunDatasetEF  runEF EGL init with version %d.%d", major, minor);
  EGLint numConfigs;
  EGLConfig config;
  if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs) ||
      eglGetError() != EGL_SUCCESS) {
    LOGI("RunDatasetEF egl eglChooseConfig failed");
  }

  EGLSurface surface;
  const EGLint surfaceAttr[] = {EGL_WIDTH, 1280, EGL_HEIGHT, 720, EGL_NONE};

  // PixmapSurface和PBufferSurface
  surface = eglCreatePbufferSurface(display, config, surfaceAttr);
  //	    surface = eglCreateWindowSurface(display, config, mWindow, NULL);
  if (surface == EGL_NO_SURFACE || eglGetError() != EGL_SUCCESS) {
    LOGI("RunDatasetEF egl eglCreatePbufferSurface failed");
  }

  EGLContext context;

  // EGL_NO_CONTEXT
  context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
  if (context == EGL_NO_CONTEXT || eglGetError() != EGL_SUCCESS) {
    LOGI("RunDatasetEF egl eglCreateContext failed");
  }

  if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE ||
      eglGetError() != EGL_SUCCESS) {
    LOGI("RunDatasetEF egl eglMakeCurrent failed");
  }
  if (eglGetError() != EGL_SUCCESS) {
    LOGI("RunDatasetEF egl may failed");
  }
  LOGI("RunDatasetEF egl context setup done ...");
  //==================================
  bool success = LoadOpenGLExtensionsManually();
  if (!success) {
    LOGE("RunDatasetEF LoadOpenGLExtensionsManually failed");
  } else {
    LOGE("RunDatasetEF LoadOpenGLExtensionsManually success");
  }
  //============================
  LOGI("RunDatasetEF RunDatasetEF Initialising ...");

  Resolution::getInstance(myWidth, myHeight);
  Intrinsics::getInstance(528, 528, 320, 240);

  file = "/sdcard/dataset.klg";
  struct stat st;
  int result = stat(file.c_str(), &st);
  assert(result == 0);
  // assert(pangolin::FileExists(file.c_str()));

  fp = fopen(file.c_str(), "rb");

  currentFrame = 0;

  assert(fread(&numFrames, sizeof(int32_t), 1, fp));
  LOGI("RunDatasetEF this dataset has num frame: %d", numFrames)
      : depthReadBuffer = new unsigned char[numPixels * 2];
  imageReadBuffer = new unsigned char[numPixels * 3];
  decompressionBufferDepth =
      new Bytef[Resolution::getInstance().numPixels() * 2];
  decompressionBufferImage =
      new Bytef[Resolution::getInstance().numPixels() * 3];
  LOGI("RunDatasetEF RunDatasetEF Initialising done ...");
  LOGI("RunDatasetEF RunDatasetEF Setting parameters...");
  float confidence = 10.0f;   // fusion的confidence阈值
  float depth = 3.0f;         //去掉depth大于某个阈值的帧
  float icp = 10.0f;          // icp的阈值
  float icpErrThresh = 5e-05; // icp错误阈值
  float covThresh = 1e-05;
  float photoThresh = 115;
  float fernThresh = 0.3095f; //新fern的阈值
  int timeDelta = 200;
  int icpCountThresh = 35000;
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
  LOGI("RunDatasetEF RunDatasetEF Setting parameters done.");
  LOGI("RunDatasetEF RunDatasetEF Building eFusion...");
  // pangolin::Params windowParams;
  // windowParams.Set("SAMPLE_BUFFERS", 0);
  // windowParams.Set("SAMPLES", 0);
  // pangolin::CreateWindowAndBind("Main", width, height, windowParams);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_PACK_ALIGNMENT, 1);
  ElasticFusion eFusion(openLoop ? std::numeric_limits<int>::max() / 2
                                 : timeDelta,
                        icpCountThresh, icpErrThresh, covThresh, !openLoop,
                        iclnuim, reloc, photoThresh, confidence, depth, icp,
                        fastOdom, fernThresh, so3, frameToFrameRGB);
  LOGI("RunDatasetEF RunDatasetEF Building eFusion done");
  check_gl_errorEF();
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
  check_gl_errorEF();
  LOGI("RunDatasetEF RunDatasetEF: good");

  // start of while loop
  while (runningRunDatasetEF.getValueWait(1)) {
    check_gl_errorEF();
    LOGI("RunDatasetEF RunDatasetEF: outter while loop in");

    while (hasMore()) {
      check_gl_errorEF();
      LOGI("RunDatasetEF RunDatasetEF: hasMore while loop in");
      getNext();
      // Eigen::Matrix4f *currentPose = 0;
      eFusion->processFrame(rgb, depth, timestamp);
      // if (currentPose) {
      //   delete currentPose;
      // }

      check_gl_errorEF();
      LOGI("RunDatasetEF Processing frames ready done.");
      check_gl_errorEF();
      Eigen::Matrix4f currPose = eFusion.getCurrPose();
      posesEigen.push_back(currPose);
      Eigen::IOFormat CleanFmt(4, 0, ", ", "\n", "[", "]");
      std::stringstream ss;
      ss << currPose.format(CleanFmt);
      std::string str(ss.str());
      LOGI("current pose is : %s", str.c_str());
      LOGI("RunDatasetEF Processing frames done.");
      LOGI("RunDatasetEF Log processing result start.");
      //查看处理的结果
      delete &currPose;
      int time = eFusion.getTick(); //查看此时eFusion的系统时间
      LOGI("RunDatasetEF Log processing result time : %d ", time);

      //  Ferns keyfern = eFusion.getFerns();//关键帧dataset

      int ld_num = eFusion.getDeforms(); //局部deformations的数量
      // Deformation ld = eFusion.getLocalDeformation(); //局部deformation图

      int gd_num = eFusion.getFernDeforms(); //全局deformations的数量
      // GlobalModel gm = eFusion.getGlobalModel(); //全局deformation model:

      int CloudPoint_num = eFusion.getGlobalModel().lastCount(); //点云的点数量
      LOGI("RunDatasetEF Log processing result "
           "eFusion.globalModel.lastCount(): "
           "totalPoints :%d ",
           CloudPoint_num);

      int totalNodes = eFusion.getLocalDeformation().getGraph().size();
      LOGI("RunDatasetEF Log processing result "
           "eFusion.getLocalDeformation().getGraph().size(): totalNodes : %d",
           totalNodes);

      int totalFerns = eFusion.getFerns().frames.size();
      LOGI("RunDatasetEF Log processing result "
           "eFusion.getLocalDeformation().getGraph().size(): totalNodes : %d",
           totalNodes);

      int totalDefs = eFusion.getDeforms();
      LOGI("RunDatasetEF Log processing result "
           "eFusion.getDeforms(): totalDefs :%d ",
           totalDefs);

      int totalFernDefs = eFusion.getFernDeforms();
      LOGI("RunDatasetEF Log processing result "
           "eFusion.getFernDeforms(): totalFernDefs :%d ",
           totalFernDefs);
      float lastICPError = eFusion.getModelToModel().lastICPError;
      float lastICPCount = eFusion.getModelToModel().lastICPCount;
      LOGI("RunDatasetEF Log processing result lastICPError: %f, "
           "icpErrThresh: %f",
           lastICPError, 5e-05);
      LOGI("RunDatasetEF Log processing result lastICPCount: %f, "
           "icpCountThresh: %f",
           lastICPCount, 35000.00);
      LOGI("RunDatasetEF Log processing result done.");

      if (shouldSavePly.getValueWait()) {
        LOGI("RunDatasetEF start to save frame.");
        Eigen::Vector4f *mapData;

        float confidenceThreshold = eFusion.getConfidenceThreshold();
        LOGI("RunDatasetEF start to save frame. 0");
        unsigned int lastCount = eFusion.savePly(mapData);
        LOGI("RunDatasetEF start to save frame 1. lastCount: %d, "
             "confidenceThreshold: %f",
             lastCount, confidenceThreshold);
        std::string plyFilename("/sdcard/RunDatasetEFPly_" +
                                current_date_time());
        plyFilename.append(".ply");
        LOGI("RunDatasetEF start to save frame 1 1");
        // Open file
        std::ofstream fs;
        fs.open(plyFilename.c_str());
        // File* plyFile = fopen(plyFilename.c_str(),"wb+");
        if (fs == NULL) {
          LOGE("There was a problem opening the ply file:%s",
               plyFilename.c_str());
        }
        LOGI("RunDatasetEF start to save frame. 2");
        int validCount = 0;

        for (unsigned int i = 0; i < lastCount; i++) {
          Eigen::Vector4f pos = mapData[(i * 3) + 0];

          if (pos[3] > confidenceThreshold) {
            validCount++;
          }
        }
        LOGI("RunDatasetEF start to save frame. 3");
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
        delete[] mapData;
        LOGI("RunDatasetEF start to save frame. 4");
        // Open file in binary appendable
        std::ofstream fpout(plyFilename.c_str(),
                            std::ios::app | std::ios::binary);
        LOGI("RunDatasetEF start to save frame. 5");
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
          }
        }
        LOGI("RunDatasetEF start to save frame 6.");
        // Close file
        fs.close();
        LOGI("RunDatasetEF start to save frame. 7");
        delete[] mapData;
        shouldSavePly.assignValue(false);
        LOGI("RunDatasetEF save frame done.");
      }
      LOGI("RunDatasetEF: processing++");
      processedFrameCount++;
      LOGI("RunDatasetEF: processed one frame : %d", processedFrameCount);
    }

    delete[] depthReadBuffer;
    delete[] imageReadBuffer;
    delete[] decompressionBufferDepth;
    delete[] decompressionBufferImage;

    fclose(fp);
    LOGI("RunDatasetEF deleting");
    // delete &eFusion;
    // eFusion = NULL;
    LOGI("RunDatasetEF done: done");
    break;
  }
  // end of while loop

  LOGI("RunDatasetEF run Dataset done: done");
}

void RunDatasetEF::getBack() {
  assert(filePointers.size() > 0);

  fseek(fp, filePointers.top(), SEEK_SET);

  filePointers.pop();

  getCore();
}

void RunDatasetEF::getNext() {
  filePointers.push(ftell(fp));

  getCore();
}

void RunDatasetEF::getCore() {
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

  if (imageSize == numPixels * 3) {
    memcpy(&decompressionBufferImage[0], imageReadBuffer, numPixels * 3);
  } else if (imageSize > 0) {
    // jpeg.readData(imageReadBuffer, imageSize,
    // (unsigned char *)&decompressionBufferImage[0]);
    // int    nSize = ...       // Size of buffer
    // uchar* pcBuffer = ...    // Raw buffer data
    //
    //
    // // Create a Size(1, nSize) Mat object of 8-bit, single-byte elements
    cv::Mat rawData = cv::Mat(1, numPixels * 3, CV_8UC1, imageReadBuffer);
    //
    cv::Mat decodedImage = cv::imdecode(rawData /*, flags */);
    if (decodedImage.data == NULL) {
      LOGI("RunDatasetEF decodedImage.data == NULL");
      memset(&decompressionBufferImage[0], 0, numPixels * 3);
      // Error reading raw image data
    } else {
      &decompressionBufferImage[0] = (unsigned char *)decodedImage.data;
    }
  } else {
    memset(&decompressionBufferImage[0], 0, numPixels * 3);
  }

  depth = (unsigned short *)decompressionBufferDepth;
  rgb = (unsigned char *)&decompressionBufferImage[0];

  // if (flipColors) {
  //   for (int i = 0; i < Resolution::getInstance().numPixels() * 3; i += 3) {
  //     std::swap(rgb[i + 0], rgb[i + 2]);
  //   }
  // }

  currentFrame++;
}

void RunDatasetEF::fastForward(int frame) {
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

int RunDatasetEF::getNumFrames() { return numFrames; }

bool RunDatasetEF::hasMore() { return currentFrame + 1 < numFrames; }

bool RunDatasetEF::rewound() {
  if (filePointers.size() == 0) {
    fclose(fp);
    fp = fopen(file.c_str(), "rb");

    assert(fread(&numFrames, sizeof(int32_t), 1, fp));

    currentFrame = 0;

    return true;
  }

  return false;
}

const std::string RunDatasetEF::getFile() { return file; }

void RunDatasetEF::setAuto(bool value) {}
}
