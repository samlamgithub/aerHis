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

#include "ElasticFusion.h"

static const char *glErrorStringEF(GLenum err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "Invalid Enum";
  case GL_INVALID_VALUE:
    return "Invalid Value";
  case GL_INVALID_OPERATION:
    return "Invalid Operation";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  // case GL_STACK_OVERFLOW: return "Stack Overflow";
  // case GL_STACK_UNDERFLOW: return "Stack Underflow";
  case GL_OUT_OF_MEMORY:
    return "error Out of Memory";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
  default:
    return "Unknown Error";
  }
}

inline const char *glCheckFramebufferStatusEFCPP() {
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return "MY elasitcfusion  GL_FRAMEBUFFER_COMPLETE";
  } else if (status == GL_FRAMEBUFFER_UNDEFINED) {
    return "MY elasitcfusion   GL_FRAMEBUFFER_UNDEFINED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    return "MY elasitcfusion  "
           "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    return "MY elasitcfusion  "
           "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    return "MY elasitcfusion   GL_FRAMEBUFFER_UNSUPPORTED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    return "MY elasitcfusion   "
           "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
  } else if (status == GL_INVALID_ENUM) {
    return "MY elasitcfusion  GL_INVALID_ENUM";
  } else {
      LOGI("glCheckFramebufferStatus else: %d", status);
    char integer_string[32];
    int integer = status;
    sprintf(integer_string, "%d", status);
    char other_string[64] = "MY elasitcfusion glCheckFramebufferStatus else: ";
    strcat(other_string, integer_string);
    return other_string;
  }
}

inline void check_gl_errorElasticFusion() {
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      LOGI("frame buffer error: %s", glCheckFramebufferStatusEFCPP());
  }
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_error ElasticFusion.cpp My elastic-fusion CheckGlDieOnError "
         "after "
         ": %s, %s() glError (0x%x)\n",
         glCheckFramebufferStatusEFCPP(), glErrorStringEF(error), error);
  }
}

ElasticFusion::ElasticFusion(const int timeDelta, const int countThresh,
                             const float errThresh, const float covThresh,
                             const bool closeLoops, const bool iclnuim,
                             const bool reloc, const float photoThresh,
                             const float confidence, const float depthCut,
                             const float icpThresh, const bool fastOdom,
                             const float fernThresh, const bool so3,
                             const bool frameToFrameRGB)
    : frameToModel(
          Resolution::getInstance().width(), Resolution::getInstance().height(),
          Intrinsics::getInstance().cx(), Intrinsics::getInstance().cy(),
          Intrinsics::getInstance().fx(), Intrinsics::getInstance().fy()),
      modelToModel(
          Resolution::getInstance().width(), Resolution::getInstance().height(),
          Intrinsics::getInstance().cx(), Intrinsics::getInstance().cy(),
          Intrinsics::getInstance().fx(), Intrinsics::getInstance().fy()),
      ferns(500, depthCut * 1000, photoThresh),
      currPose(Eigen::Matrix4f::Identity()), tick(1), timeDelta(timeDelta),
      icpCountThresh(countThresh), icpErrThresh(errThresh),
      covThresh(covThresh), deforms(0), fernDeforms(0), consSample(20),
      resize(Resolution::getInstance().width(),
             Resolution::getInstance().height(),
             Resolution::getInstance().width() / consSample,
             Resolution::getInstance().height() / consSample),
      imageBuff(Resolution::getInstance().rows() / consSample,
                Resolution::getInstance().cols() / consSample),
      consBuff(Resolution::getInstance().rows() / consSample,
               Resolution::getInstance().cols() / consSample),
      timesBuff(Resolution::getInstance().rows() / consSample,
                Resolution::getInstance().cols() / consSample),
      closeLoops(closeLoops), iclnuim(iclnuim), reloc(reloc), lost(false),
      lastFrameRecovery(false), trackingCount(0), maxDepthProcessed(20.0f),
      rgbOnly(false), icpWeight(icpThresh), pyramid(true), fastOdom(fastOdom),
      confidenceThreshold(confidence), fernThresh(fernThresh), so3(so3),
      frameToFrameRGB(frameToFrameRGB), depthCutoff(depthCut) {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct init start 1 ");
  createTextures();
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct init 2 ");
  createCompute();
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct init 3 ");
  createFeedbackBuffers();
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct init 4 ");

  //    std::string filename = fileName;
  //    filename.append(".freiburg");
  //
  //    std::ofstream file;
  //    file.open(filename.c_str(), std::fstream::out);
  //    file.close();

  Stopwatch::getInstance().setCustomSignature(12431231);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct init 5 done");
}

ElasticFusion::~ElasticFusion() {
    LOGI("MY elasitcfusion struct free start 1");
  //    if(iclnuim)
  //    {
  //        savePly();
  //    }

  // Output deformed pose graph
  //    std::string fname = saveFilename;
  //    fname.append(".freiburg");
  //
  //    std::ofstream f;
  //    f.open(fname.c_str(), std::fstream::out);

  //    for(size_t i = 0; i < poseGraph.size(); i++)
  //    {
  //        std::stringstream strs;
  //
  //        if(iclnuim)
  //        {
  //            strs << std::setprecision(6) << std::fixed <<
  //            (double)poseLogTimes.at(i) << " ";
  //        }
  //        else
  //        {
  //            strs << std::setprecision(6) << std::fixed <<
  //            (double)poseLogTimes.at(i) / 1000000.0 << " ";
  //        }
  //
  //        Eigen::Vector3f trans = poseGraph.at(i).second.topRightCorner(3, 1);
  //        Eigen::Matrix3f rot = poseGraph.at(i).second.topLeftCorner(3, 3);
  //
  //        f << strs.str() << trans(0) << " " << trans(1) << " " << trans(2) <<
  //        " ";
  //
  //        Eigen::Quaternionf currentCameraRotation(rot);
  //
  //        f << currentCameraRotation.x() << " " << currentCameraRotation.y()
  //        << " " << currentCameraRotation.z() << " " <<
  //        currentCameraRotation.w() << "\n";
  //    }
  //
  //    f.close();

  for (std::map<std::string, GPUTexture *>::iterator it = textures.begin();
       it != textures.end(); ++it) {
    delete it->second;
  }
    LOGI("MY elasitcfusion struct free  2");
  textures.clear();
  LOGI("MY elasitcfusion struct free  3");
  for (std::map<std::string, ComputePack *>::iterator it = computePacks.begin();
       it != computePacks.end(); ++it) {
    delete it->second;
  }
  LOGI("MY elasitcfusion struct free  4");
  computePacks.clear();
  LOGI("MY elasitcfusion struct free  5");
  for (std::map<std::string, FeedbackBuffer *>::iterator it =
           feedbackBuffers.begin();
       it != feedbackBuffers.end(); ++it) {
    delete it->second;
  }
  LOGI("MY elasitcfusion struct free  6");
  feedbackBuffers.clear();
  LOGI("MY elasitcfusion struct free done");
}

void ElasticFusion::createTextures() {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createTextures start 1 ");
  textures[GPUTexture::RGB] = new GPUTexture(
      Resolution::getInstance().width(), Resolution::getInstance().height(),
      // GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE, true, true);
      GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, true, true);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createTextures 2 ");
  textures[GPUTexture::DEPTH_RAW] = new GPUTexture(
      Resolution::getInstance().width(), Resolution::getInstance().height(),
      // GL_LUMINANCE16UI_EXT,GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_SHORT);
      GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createTextures 3 ");
  textures[GPUTexture::DEPTH_FILTERED] = new GPUTexture(
      Resolution::getInstance().width(), Resolution::getInstance().height(),
      // GL_LUMINANCE16UI_EXT, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_SHORT,
      // false, true);
      GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, true);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createTextures 4 ");
  textures[GPUTexture::DEPTH_METRIC] = new GPUTexture(
      Resolution::getInstance().width(), Resolution::getInstance().height(),
      // GL_LUMINANCE32F_ARB, GL_LUMINANCE, GL_FLOAT);
      GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createTextures 5 ");
  textures[GPUTexture::DEPTH_METRIC_FILTERED] = new GPUTexture(
      Resolution::getInstance().width(), Resolution::getInstance().height(),
      // GL_LUMINANCE32F_ARB, GL_LUMINANCE, GL_FLOAT);
      GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createTextures 6 ");
  textures[GPUTexture::DEPTH_NORM] = new GPUTexture(
      Resolution::getInstance().width(), Resolution::getInstance().height(),
      // GL_LUMINANCE, GL_LUMINANCE, GL_FLOAT, true);
      GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, true);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createTextures done ");
}

void ElasticFusion::createCompute() {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createCompute start 1 ");
  computePacks[ComputePack::NORM] = new ComputePack(
      loadProgram(emptyvert_tuple, depth_normfrag_tuple, quadgeom_tuple),
      textures[GPUTexture::DEPTH_NORM]->texture);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createCompute 2 ");
  computePacks[ComputePack::FILTER] = new ComputePack(
      loadProgram(emptyvert_tuple, depth_bilateralfrag_tuple, quadgeom_tuple),
      textures[GPUTexture::DEPTH_FILTERED]->texture);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createCompute 3 ");
  computePacks[ComputePack::METRIC] = new ComputePack(
      loadProgram(emptyvert_tuple, depth_metricfrag_tuple, quadgeom_tuple),
      textures[GPUTexture::DEPTH_METRIC]->texture);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createCompute 4 ");
  computePacks[ComputePack::METRIC_FILTERED] = new ComputePack(
      loadProgram(emptyvert_tuple, depth_metricfrag_tuple, quadgeom_tuple),
      textures[GPUTexture::DEPTH_METRIC_FILTERED]->texture);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createCompute 5 done");
}

void ElasticFusion::createFeedbackBuffers() {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createFeedbackBuffers start 1 ");
  feedbackBuffers[FeedbackBuffer::RAW] = new FeedbackBuffer(
      loadProgramGeom(vertex_feedbackvert_tuple, vertex_feedbackgeom_tuple));
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createFeedbackBuffers 2 ");
  feedbackBuffers[FeedbackBuffer::FILTERED] = new FeedbackBuffer(
      loadProgramGeom(vertex_feedbackvert_tuple, vertex_feedbackgeom_tuple));
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct createFeedbackBuffers 3 done");
}

void ElasticFusion::computeFeedbackBuffers() {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct computeFeedbackBuffers 1 start");
  TICK("feedbackBuffers");
  feedbackBuffers[FeedbackBuffer::RAW]->compute(
      textures[GPUTexture::RGB]->texture,
      textures[GPUTexture::DEPTH_METRIC]->texture, tick, maxDepthProcessed);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct computeFeedbackBuffers 2 ");
  feedbackBuffers[FeedbackBuffer::FILTERED]->compute(
      textures[GPUTexture::RGB]->texture,
      textures[GPUTexture::DEPTH_METRIC_FILTERED]->texture, tick,
      maxDepthProcessed);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct computeFeedbackBuffers 3 done");
  TOCK("feedbackBuffers");
}

bool ElasticFusion::denseEnough(
    const Img<Eigen::Matrix<unsigned char, 3, 1>> &img) {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct denseEnough 1 start");
  int sum = 0;

  for (int i = 0; i < img.rows; i++) {
    for (int j = 0; j < img.cols; j++) {
      sum += img.at<Eigen::Matrix<unsigned char, 3, 1>>(i, j)(0) > 0 &&
             img.at<Eigen::Matrix<unsigned char, 3, 1>>(i, j)(1) > 0 &&
             img.at<Eigen::Matrix<unsigned char, 3, 1>>(i, j)(2) > 0;
    }
  }
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct denseEnough 2 done");
  return float(sum) / float(img.rows * img.cols) > 0.75f;
}

void ElasticFusion::processFrame(const unsigned char *rgb,
                                 const unsigned short *depth,
                                 const int64_t &timestamp,
                                 const Eigen::Matrix4f *inPose,
                                 const float weightMultiplier,
                                 const bool bootstrap) {
  TICK("Run");
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame Run 1 start");

  LOGI("=== timestamp: %d", timestamp);
  for (int i = 0; i < 640; i++) {
    int rgbNum = i % 3;
    if (rgbNum == 0) {
      LOGI("rgb ========= r:  %d", rgb[i]);
    } else if (rgbNum == 1) {
      LOGI("rgb ========= g:  %d", rgb[i]);
    } else if (rgbNum == 2) {
      LOGI("rgb ========= b:  %d", rgb[i]);
    } else {
      LOGI("imporssible");
    }
  }
  LOGI("-------------------------------------------------");
  for (int i = 0; i < 640; i++) {
    LOGI("depth ========= %d", depth[i]);
  }
  LOGI("==================================================");
  return;

  textures[GPUTexture::DEPTH_RAW]->texture->Upload(depth, GL_LUMINANCE,
                                                   GL_UNSIGNED_BYTE);
  // textures[GPUTexture::DEPTH_RAW]->texture->Upload(
  //     depth, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_SHORT);
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame Run 2");
  textures[GPUTexture::RGB]->texture->Upload(rgb, GL_RGBA, GL_UNSIGNED_BYTE);
//textures[GPUTexture::RGB]->texture->Upload(rgb, GL_RGB, GL_UNSIGNED_BYTE);
  check_gl_errorElasticFusion();
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame Preprocess");
  TICK("Preprocess");

  filterDepth();
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame Preprocess done 1");
  metriciseDepth();
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame Preprocess done 2");
  TOCK("Preprocess");

  // First run
  if (tick == 1) {
    LOGI(" ElasticFusion struct Process frame tick == 1");
    computeFeedbackBuffers();
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame computeFeedbackBuffers done");
    globalModel.initialise(*feedbackBuffers[FeedbackBuffer::RAW],
                           *feedbackBuffers[FeedbackBuffer::FILTERED]);
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame globalModel.initialise done");
    frameToModel.initFirstRGB(textures[GPUTexture::RGB]);
    check_gl_errorElasticFusion();
    LOGI("ElasticFusion struct Process frame tick 1 done");
  } else {
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 1");
    Eigen::Matrix4f lastPose = currPose;

    bool trackingOk = true;

    if (bootstrap || !inPose) {
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 2");
      TICK("autoFill");
      resize.image(indexMap.imageTex(), imageBuff);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 3");
      bool shouldFillIn = !denseEnough(imageBuff);
      TOCK("autoFill");
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 4");
      TICK("odomInit");
      // WARNING initICP* must be called before initRGB*
      frameToModel.initICPModel(
          shouldFillIn ? &fillIn.vertexTexture : indexMap.vertexTex(),
          shouldFillIn ? &fillIn.normalTexture : indexMap.normalTex(),
          maxDepthProcessed, currPose);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 5");
      frameToModel.initRGBModel((shouldFillIn || frameToFrameRGB)
                                    ? &fillIn.imageTexture
                                    : indexMap.imageTex());
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 6");
      frameToModel.initICP(textures[GPUTexture::DEPTH_FILTERED],
                           maxDepthProcessed);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 7");
      frameToModel.initRGB(textures[GPUTexture::RGB]);
      TOCK("odomInit");
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 8 1 ");
      if (bootstrap) {
        LOGI(" ElasticFusion struct Process frame else 8 2");
        assert(inPose);
        currPose = currPose * (*inPose);
      }
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 8 3");
      Eigen::Vector3f trans = currPose.topRightCorner(3, 1);
      Eigen::Matrix<float, 3, 3, Eigen::RowMajor> rot =
          currPose.topLeftCorner(3, 3);
      // rgbOnly：只利用 RGB 信息进行跟踪，icpWeight：计算位姿时 icp
      // 点云配准占的比重
      // pyramid：配准使用金字塔模式，fastOdom：使用快速跟踪算法
      TICK("odom");
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 9");
      frameToModel.getIncrementalTransformation(trans, rot, rgbOnly, icpWeight,
                                                pyramid, fastOdom, so3);
      TOCK("odom");
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 10");
      trackingOk = !reloc || frameToModel.lastICPError < 1e-04;

      if (reloc) {
        if (!lost) {
          check_gl_errorElasticFusion();
          LOGI(" ElasticFusion struct Process frame else 11");
          Eigen::MatrixXd covariance = frameToModel.getCovariance();

          for (int i = 0; i < 6; i++) {
            if (covariance(i, i) > 1e-04) {
              trackingOk = false;
              break;
            }
          }

          if (!trackingOk) {
            trackingCount++;

            if (trackingCount > 10) {
              lost = true;
            }
          } else {
            trackingCount = 0;
          }
          check_gl_errorElasticFusion();
          LOGI(" ElasticFusion struct Process frame else 12");
        } else if (lastFrameRecovery) {
          check_gl_errorElasticFusion();
          LOGI(" ElasticFusion struct Process frame else 13");
          Eigen::MatrixXd covariance = frameToModel.getCovariance();

          for (int i = 0; i < 6; i++) {
            if (covariance(i, i) > 1e-04) {
              trackingOk = false;
              break;
            }
          }

          if (trackingOk) {
            lost = false;
            trackingCount = 0;
          }

          lastFrameRecovery = false;
        }
        check_gl_errorElasticFusion();
        LOGI(" ElasticFusion struct Process frame else 14");
      }
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 15");
      currPose.topRightCorner(3, 1) = trans;
      currPose.topLeftCorner(3, 3) = rot;
    } else {
      LOGI(" ElasticFusion struct Process frame else 15 2");
      currPose = *inPose;
    }
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 16");
    Eigen::Matrix4f diff = currPose.inverse() * lastPose;

    Eigen::Vector3f diffTrans = diff.topRightCorner(3, 1);
    Eigen::Matrix3f diffRot = diff.topLeftCorner(3, 3);

    // Weight by velocity
    float weighting = std::max(diffTrans.norm(), rodrigues2(diffRot).norm());
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 17");
    float largest = 0.01;
    float minWeight = 0.5;

    if (weighting > largest) {
      weighting = largest;
    }

    weighting =
        std::max(1.0f - (weighting / largest), minWeight) * weightMultiplier;
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 18");
    std::vector<Ferns::SurfaceConstraint> constraints;
    check_gl_errorElasticFusion();
    LOGI("ElasticFusion struct Process frame predict 1");
    predict();
    check_gl_errorElasticFusion();
    LOGI("ElasticFusion struct Process frame predict 2 ");

    //----------------------------------------------------------global loop
    // closure--------------------------------------------------------
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 20");
    Eigen::Matrix4f recoveryPose = currPose;

    if (closeLoops) {
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 21");
      lastFrameRecovery = false;

      TICK("Ferns::findFrame");
      recoveryPose = ferns.findFrame(
          constraints, currPose, &fillIn.vertexTexture, &fillIn.normalTexture,
          &fillIn.imageTexture, tick, lost);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 22");
      TOCK("Ferns::findFrame");
    }

    std::vector<float> rawGraph;

    bool fernAccepted = false;
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 23");
    if (closeLoops && ferns.lastClosest != -1) {
      if (lost) {
        currPose = recoveryPose;
        lastFrameRecovery = true;
      } else {
        for (size_t i = 0; i < constraints.size(); i++) {
          globalDeformation.addConstraint(
              constraints.at(i).sourcePoint, constraints.at(i).targetPoint,
              tick, ferns.frames.at(ferns.lastClosest)->srcTime, true);
        }
        check_gl_errorElasticFusion();
        LOGI(" ElasticFusion struct Process frame else 24");
        for (size_t i = 0; i < relativeCons.size(); i++) {
          globalDeformation.addConstraint(relativeCons.at(i));
        }
        check_gl_errorElasticFusion();
        LOGI(" ElasticFusion struct Process frame else 25");
        // rawGraph 保存了控制点坐标和优化之后的 R和t
        if (globalDeformation.constrain(ferns.frames, rawGraph, tick, true,
                                        poseGraph, true)) {
          currPose = recoveryPose;

          poseMatches.push_back(
              PoseMatch(ferns.lastClosest, ferns.frames.size(),
                        ferns.frames.at(ferns.lastClosest)->pose, currPose,
                        constraints, true));

          fernDeforms += rawGraph.size() > 0;

          fernAccepted = true;
        }
      }
    }
    //----------------------------------------------------------local loop
    // closure--------------------------------------------------------
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 26");
    // If we didn't match to a fern
    if (!lost && closeLoops && rawGraph.size() == 0) {
      // Only predict old view, since we just predicted the current view for the
      // ferns (which failed!)
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 27");
      TICK("IndexMap::INACTIVE");
      indexMap.combinedPredict(currPose, globalModel.model(), maxDepthProcessed,
                               confidenceThreshold, 0, tick - timeDelta,
                               timeDelta, IndexMap::INACTIVE);
      TOCK("IndexMap::INACTIVE");
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 28");
      // WARNING initICP* must be called before initRGB*
      modelToModel.initICPModel(indexMap.oldVertexTex(),
                                indexMap.oldNormalTex(), maxDepthProcessed,
                                currPose);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 29");
      modelToModel.initRGBModel(indexMap.oldImageTex());
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 30");
      modelToModel.initICP(indexMap.vertexTex(), indexMap.normalTex(),
                           maxDepthProcessed);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 31");
      modelToModel.initRGB(indexMap.imageTex());
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 32");
      Eigen::Vector3f trans = currPose.topRightCorner(3, 1);
      Eigen::Matrix<float, 3, 3, Eigen::RowMajor> rot =
          currPose.topLeftCorner(3, 3);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 33");
      modelToModel.getIncrementalTransformation(trans, rot, false, 10, pyramid,
                                                fastOdom, false);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 34");
      Eigen::MatrixXd covar = modelToModel.getCovariance();
      bool covOk = true;

      for (int i = 0; i < 6; i++) {
        if (covar(i, i) > covThresh) {
          covOk = false;
          break;
        }
      }

      Eigen::Matrix4f estPose = Eigen::Matrix4f::Identity();

      estPose.topRightCorner(3, 1) = trans;
      estPose.topLeftCorner(3, 3) = rot;

      if (covOk && modelToModel.lastICPCount > icpCountThresh &&
          modelToModel.lastICPError < icpErrThresh) {
        check_gl_errorElasticFusion();
        LOGI(" ElasticFusion struct Process frame else 35");
        resize.vertex(indexMap.vertexTex(), consBuff);
        resize.time(indexMap.oldTimeTex(), timesBuff);
        check_gl_errorElasticFusion();
        LOGI(" ElasticFusion struct Process frame else 36");
        for (int i = 0; i < consBuff.cols; i++) {
          for (int j = 0; j < consBuff.rows; j++) {
            if (consBuff.at<Eigen::Vector4f>(j, i)(2) > 0 &&
                consBuff.at<Eigen::Vector4f>(j, i)(2) < maxDepthProcessed &&
                timesBuff.at<unsigned short>(j, i) > 0) {
              Eigen::Vector4f worldRawPoint =
                  currPose *
                  Eigen::Vector4f(consBuff.at<Eigen::Vector4f>(j, i)(0),
                                  consBuff.at<Eigen::Vector4f>(j, i)(1),
                                  consBuff.at<Eigen::Vector4f>(j, i)(2), 1.0f);
              check_gl_errorElasticFusion();
              LOGI(" ElasticFusion struct Process frame else 37");
              Eigen::Vector4f worldModelPoint =
                  estPose *
                  Eigen::Vector4f(consBuff.at<Eigen::Vector4f>(j, i)(0),
                                  consBuff.at<Eigen::Vector4f>(j, i)(1),
                                  consBuff.at<Eigen::Vector4f>(j, i)(2), 1.0f);
              check_gl_errorElasticFusion();
              LOGI(" ElasticFusion struct Process frame else 38");
              constraints.push_back(
                  Ferns::SurfaceConstraint(worldRawPoint, worldModelPoint));
              check_gl_errorElasticFusion();
              LOGI(" ElasticFusion struct Process frame else 39");
              localDeformation.addConstraint(
                  worldRawPoint, worldModelPoint, tick,
                  timesBuff.at<unsigned short>(j, i), deforms == 0);
            }
          }
        }
        check_gl_errorElasticFusion();
        LOGI(" ElasticFusion struct Process frame else 40");
        std::vector<Deformation::Constraint> newRelativeCons;
        // newRelativeCons 只有在 localDeformation 时才会产生
        if (localDeformation.constrain(ferns.frames, rawGraph, tick, false,
                                       poseGraph, false, &newRelativeCons)) {
          poseMatches.push_back(PoseMatch(ferns.frames.size() - 1,
                                          ferns.frames.size(), estPose,
                                          currPose, constraints, false));

          deforms += rawGraph.size() > 0;

          currPose = estPose;

          for (size_t i = 0; i < newRelativeCons.size();
               i += newRelativeCons.size() / 3) {
            relativeCons.push_back(newRelativeCons.at(i));
          }
        }
      }
    }
    //----------------------------------------------------------fuse new
    // data--------------------------------------------------------
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame else 41");
    if (!rgbOnly && trackingOk && !lost) {
      TICK("indexMap");
      indexMap.predictIndices(currPose, tick, globalModel.model(),
                              maxDepthProcessed, timeDelta);
      TOCK("indexMap");
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 42");
      globalModel.fuse(currPose, tick, textures[GPUTexture::RGB],
                       textures[GPUTexture::DEPTH_METRIC],
                       textures[GPUTexture::DEPTH_METRIC_FILTERED],
                       indexMap.indexTex(), indexMap.vertConfTex(),
                       indexMap.colorTimeTex(), indexMap.normalRadTex(),
                       maxDepthProcessed, confidenceThreshold, weighting);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 43");
      TICK("indexMap");
      indexMap.predictIndices(currPose, tick, globalModel.model(),
                              maxDepthProcessed, timeDelta);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 44");
      TOCK("indexMap");

      // If we're deforming we need to predict the depth again to figure out
      // which  points to update the timestamp's of, since a deformation means a
      // second pose update  this loop
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 45");
      if (rawGraph.size() > 0 && !fernAccepted) {
        indexMap.synthesizeDepth(currPose, globalModel.model(),
                                 maxDepthProcessed, confidenceThreshold, tick,
                                 tick - timeDelta,
                                 std::numeric_limits<unsigned short>::max());
      }
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 46");
      globalModel.clean(currPose, tick, indexMap.indexTex(),
                        indexMap.vertConfTex(), indexMap.colorTimeTex(),
                        indexMap.normalRadTex(), indexMap.depthTex(),
                        confidenceThreshold, rawGraph, timeDelta,
                        maxDepthProcessed, fernAccepted);
      check_gl_errorElasticFusion();
      LOGI(" ElasticFusion struct Process frame else 47");
    }
  }
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame tick tick  done");

  poseGraph.push_back(
      std::pair<unsigned long long int, Eigen::Matrix4f>(tick, currPose));
  poseLogTimes.push_back(timestamp);
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame  sampleGraph");
  TICK("sampleGraph");
  //在这里初始化 DeformationGraph

  localDeformation.sampleGraphModel(globalModel.model());

  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame  sampleGraph 1");
  globalDeformation.sampleGraphFrom(localDeformation);
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame  sampleGraph done");
  TOCK("sampleGraph");

  predict();
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame     predict done");
  //如果没跟丢则检测是否将当前帧添加为关键帧
  if (!lost) {
    processFerns();
    check_gl_errorElasticFusion();
    LOGI(" ElasticFusion struct Process frame   processFerns");
    tick++;
  }
  check_gl_errorElasticFusion();
  LOGI(" ElasticFusion struct Process frame  run finally done");
  TOCK("Run");
}

void ElasticFusion::processFerns() {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct processFerns 1 start ");
  TICK("Ferns::addFrame");
  ferns.addFrame(&fillIn.imageTexture, &fillIn.vertexTexture,
                 &fillIn.normalTexture, currPose, tick, fernThresh);
  TOCK("Ferns::addFrame");
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct processFerns 2 done");
}

void ElasticFusion::predict() {
  TICK("IndexMap::ACTIVE");
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct predict 1  start 1111");
  if (lastFrameRecovery) {
    LOGI("MY elasitcfusion struct predict 1   1111 a start");
    indexMap.combinedPredict(currPose, globalModel.model(), maxDepthProcessed,
                             confidenceThreshold, 0, tick, timeDelta,
                             IndexMap::ACTIVE);
    check_gl_errorElasticFusion();
    LOGI("MY elasitcfusion struct predict 1   1111 a done");
  } else {
    LOGI("MY elasitcfusion struct predict 1   1111 b start");
    indexMap.combinedPredict(currPose, globalModel.model(), maxDepthProcessed,
                             confidenceThreshold, tick, tick, timeDelta,
                             IndexMap::ACTIVE);
    check_gl_errorElasticFusion();
    LOGI("MY elasitcfusion struct predict 1   1111 b done");
  }
  //当跟丢的时候，使用当前帧的信息为下一帧配准，passthrough 传递的参数为 lsot
  //当参数没有跟丢时，使用模型投影获取的图像对下一帧图像配准，当模型投影有残缺值时，使用当前帧的图像做补丁
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct predict combinedPredict done 0");
  TICK("FillIn");
  fillIn.vertex(indexMap.vertexTex(), textures[GPUTexture::DEPTH_FILTERED],
                lost);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct predict combinedPredict done 1");
  fillIn.normal(indexMap.normalTex(), textures[GPUTexture::DEPTH_FILTERED],
                lost);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct predict combinedPredict done 2");
  fillIn.image(indexMap.imageTex(), textures[GPUTexture::RGB],
               lost || frameToFrameRGB);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct predict combinedPredict done 3");
  TOCK("FillIn");

  TOCK("IndexMap::ACTIVE");
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct predict 2 doen done ");
}

void ElasticFusion::metriciseDepth() {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct metriciseDepth 1 ");
  std::vector<Uniform> uniforms;
  uniforms.push_back(Uniform("maxD", depthCutoff));
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct metriciseDepth 2 ");
  computePacks[ComputePack::METRIC]->compute(
      textures[GPUTexture::DEPTH_RAW]->texture, &uniforms);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct metriciseDepth 3");
  computePacks[ComputePack::METRIC_FILTERED]->compute(
      textures[GPUTexture::DEPTH_FILTERED]->texture, &uniforms);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct metriciseDepth 4");
}

void ElasticFusion::filterDepth() {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct filterDepth 1 start ");
  std::vector<Uniform> uniforms;

  uniforms.push_back(Uniform("cols", (float)Resolution::getInstance().cols()));
  uniforms.push_back(Uniform("rows", (float)Resolution::getInstance().rows()));
  uniforms.push_back(Uniform("maxD", depthCutoff));
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct filterDepth 2 ");
  computePacks[ComputePack::FILTER]->compute(
      textures[GPUTexture::DEPTH_RAW]->texture, &uniforms);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct filterDepth 3 done ");
}

void ElasticFusion::normaliseDepth(const float &minVal, const float &maxVal) {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct normaliseDepth 1 start");
  std::vector<Uniform> uniforms;

  uniforms.push_back(Uniform("maxVal", maxVal * 1000.f));
  uniforms.push_back(Uniform("minVal", minVal * 1000.f));
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct normaliseDepth 2");
  computePacks[ComputePack::NORM]->compute(
      textures[GPUTexture::DEPTH_RAW]->texture, &uniforms);
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct normaliseDepth 3 done");
}

unsigned int ElasticFusion::savePly(Eigen::Vector4f *myMapData) {
  // TODO: save PLY
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct savePly 1 start confidenceThreshold: %f",
       confidenceThreshold);
  unsigned int count = globalModel.lastCount();
  myMapData = globalModel.downloadMap();
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct savePly 1 done, count: %d, confidenceThreshold: %f",
       count, confidenceThreshold);
  return count;
  //===========
  // std::string filename = saveFilename;
  // filename.append(".ply");
  //
  // // Open file
  // std::ofstream fs;
  // fs.open (filename.c_str ());
  //
  // Eigen::Vector4f * mapData = globalModel.downloadMap();
  //
  // int validCount = 0;
  //
  // for(unsigned int i = 0; i < globalModel.lastCount(); i++)
  // {
  //     Eigen::Vector4f pos = mapData[(i * 3) + 0];
  //
  //     if(pos[3] > confidenceThreshold)
  //     {
  //         validCount++;
  //     }
  // }
  //
  // // Write header
  // fs << "ply";
  // fs << "\nformat " << "binary_little_endian" << " 1.0";
  //
  // // Vertices
  // fs << "\nelement vertex "<< validCount;
  // fs << "\nproperty float x"
  //       "\nproperty float y"
  //       "\nproperty float z";
  //
  // fs << "\nproperty uchar red"
  //       "\nproperty uchar green"
  //       "\nproperty uchar blue";
  //
  // fs << "\nproperty float nx"
  //       "\nproperty float ny"
  //       "\nproperty float nz";
  //
  // fs << "\nproperty float radius";
  //
  // fs << "\nend_header\n";
  //
  // // Close the file
  // fs.close ();
  //
  // // Open file in binary appendable
  // std::ofstream fpout (filename.c_str (), std::ios::app | std::ios::binary);
  //
  // for(unsigned int i = 0; i < globalModel.lastCount(); i++)
  // {
  //     Eigen::Vector4f pos = mapData[(i * 3) + 0];
  //
  //     if(pos[3] > confidenceThreshold)
  //     {
  //         Eigen::Vector4f col = mapData[(i * 3) + 1];
  //         Eigen::Vector4f nor = mapData[(i * 3) + 2];
  //
  //         nor[0] *= -1;
  //         nor[1] *= -1;
  //         nor[2] *= -1;
  //
  //         float value;
  //         memcpy (&value, &pos[0], sizeof (float));
  //         fpout.write (reinterpret_cast<const char*> (&value), sizeof
  //         (float));
  //
  //         memcpy (&value, &pos[1], sizeof (float));
  //         fpout.write (reinterpret_cast<const char*> (&value), sizeof
  //         (float));
  //
  //         memcpy (&value, &pos[2], sizeof (float));
  //         fpout.write (reinterpret_cast<const char*> (&value), sizeof
  //         (float));
  //
  //         unsigned char r = int(col[0]) >> 16 & 0xFF;
  //         unsigned char g = int(col[0]) >> 8 & 0xFF;
  //         unsigned char b = int(col[0]) & 0xFF;
  //
  //         fpout.write (reinterpret_cast<const char*> (&r), sizeof (unsigned
  //         char)); fpout.write (reinterpret_cast<const char*> (&g), sizeof
  //         (unsigned char)); fpout.write (reinterpret_cast<const char*> (&b),
  //         sizeof (unsigned char));
  //
  //         memcpy (&value, &nor[0], sizeof (float));
  //         fpout.write (reinterpret_cast<const char*> (&value), sizeof
  //         (float));
  //
  //         memcpy (&value, &nor[1], sizeof (float));
  //         fpout.write (reinterpret_cast<const char*> (&value), sizeof
  //         (float));
  //
  //         memcpy (&value, &nor[2], sizeof (float));
  //         fpout.write (reinterpret_cast<const char*> (&value), sizeof
  //         (float));
  //
  //         memcpy (&value, &nor[3], sizeof (float));
  //         fpout.write (reinterpret_cast<const char*> (&value), sizeof
  //         (float));
  //     }
  // }
  //
  // // Close file
  // fs.close ();
  //
  // delete [] mapData;
}

Eigen::Vector3f ElasticFusion::rodrigues2(const Eigen::Matrix3f &matrix) {
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct rodrigues2 start 1");
  Eigen::JacobiSVD<Eigen::Matrix3f> svd(matrix, Eigen::ComputeFullV |
                                                    Eigen::ComputeFullU);
  Eigen::Matrix3f R = svd.matrixU() * svd.matrixV().transpose();

  double rx = R(2, 1) - R(1, 2);
  double ry = R(0, 2) - R(2, 0);
  double rz = R(1, 0) - R(0, 1);

  double s = sqrt((rx * rx + ry * ry + rz * rz) * 0.25);
  double c = (R.trace() - 1) * 0.5;
  c = c > 1. ? 1. : c < -1. ? -1. : c;

  double theta = acos(c);

  if (s < 1e-5) {
    double t;

    if (c > 0)
      rx = ry = rz = 0;
    else {
      t = (R(0, 0) + 1) * 0.5;
      rx = sqrt(std::max(t, 0.0));
      t = (R(1, 1) + 1) * 0.5;
      ry = sqrt(std::max(t, 0.0)) * (R(0, 1) < 0 ? -1.0 : 1.0);
      t = (R(2, 2) + 1) * 0.5;
      rz = sqrt(std::max(t, 0.0)) * (R(0, 2) < 0 ? -1.0 : 1.0);

      if (fabs(rx) < fabs(ry) && fabs(rx) < fabs(rz) &&
          (R(1, 2) > 0) != (ry * rz > 0))
        rz = -rz;
      theta /= sqrt(rx * rx + ry * ry + rz * rz);
      rx *= theta;
      ry *= theta;
      rz *= theta;
    }
  } else {
    double vth = 1 / (2 * s);
    vth *= theta;
    rx *= vth;
    ry *= vth;
    rz *= vth;
  }
  return Eigen::Vector3d(rx, ry, rz).cast<float>();
  check_gl_errorElasticFusion();
  LOGI("MY elasitcfusion struct rodrigues2 done 1");
}

// Sad times ahead
IndexMap &ElasticFusion::getIndexMap() { return indexMap; }

GlobalModel &ElasticFusion::getGlobalModel() { return globalModel; }

Ferns &ElasticFusion::getFerns() { return ferns; }

Deformation &ElasticFusion::getLocalDeformation() { return localDeformation; }

std::map<std::string, GPUTexture *> &ElasticFusion::getTextures() {
  return textures;
}

const std::vector<PoseMatch> &ElasticFusion::getPoseMatches() {
  return poseMatches;
}

const RGBDOdometry &ElasticFusion::getModelToModel() { return modelToModel; }

const float &ElasticFusion::getConfidenceThreshold() {
  return confidenceThreshold;
}

void ElasticFusion::setRgbOnly(const bool &val) { rgbOnly = val; }

void ElasticFusion::setIcpWeight(const float &val) { icpWeight = val; }

void ElasticFusion::setPyramid(const bool &val) { pyramid = val; }

void ElasticFusion::setFastOdom(const bool &val) { fastOdom = val; }

void ElasticFusion::setSo3(const bool &val) { so3 = val; }

void ElasticFusion::setFrameToFrameRGB(const bool &val) {
  frameToFrameRGB = val;
}

void ElasticFusion::setConfidenceThreshold(const float &val) {
  confidenceThreshold = val;
}

void ElasticFusion::setFernThresh(const float &val) { fernThresh = val; }

void ElasticFusion::setDepthCutoff(const float &val) { depthCutoff = val; }

const bool &ElasticFusion::getLost() // lel
{
  return lost;
}

const int &ElasticFusion::getTick() { return tick; }

const int &ElasticFusion::getTimeDelta() { return timeDelta; }

void ElasticFusion::setTick(const int &val) { tick = val; }

const float &ElasticFusion::getMaxDepthProcessed() { return maxDepthProcessed; }

const Eigen::Matrix4f &ElasticFusion::getCurrPose() { return currPose; }

const int &ElasticFusion::getDeforms() { return deforms; }

const int &ElasticFusion::getFernDeforms() { return fernDeforms; }

std::map<std::string, FeedbackBuffer *> &ElasticFusion::getFeedbackBuffers() {
  return feedbackBuffers;
}
