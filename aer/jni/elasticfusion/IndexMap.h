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

#ifndef INDEXMAP_H_
#define INDEXMAP_H_

#include "GPUTexture.h"
#include "Shaders/Shaders.h"
#include "Shaders/Uniform.h"
#include "Shaders/Vertex.h"
#include "Utils/Intrinsics.h"
#include "Utils/Resolution.h"
#include "../tango_interface/util.hpp"
#include <Shaders/glFrameBuffer.h>

class IndexMap {
public:
  IndexMap();
  virtual ~IndexMap();

  void predictIndices(const Eigen::Matrix4f &pose, const int &time,
                      const std::pair<GLuint, GLuint> &model,
                      const float depthCutoff, const int timeDelta);

  void renderDepth(const float depthCutoff);

  enum Prediction { ACTIVE, INACTIVE };

  void combinedPredict(const Eigen::Matrix4f &pose,
                       const std::pair<GLuint, GLuint> &model,
                       const float depthCutoff, const float confThreshold,
                       const int time, const int maxTime, const int timeDelta,
                       IndexMap::Prediction predictionType);

  void synthesizeDepth(const Eigen::Matrix4f &pose,
                       const std::pair<GLuint, GLuint> &model,
                       const float depthCutoff, const float confThreshold,
                       const int time, const int maxTime, const int timeDelta);

  GPUTexture *indexTex() { return &indexTexture; }

  GPUTexture *vertConfTex() { return &vertConfTexture; }

  GPUTexture *colorTimeTex() { return &colorTimeTexture; }

  GPUTexture *normalRadTex() { return &normalRadTexture; }

  GPUTexture *drawTex() { return &drawTexture; }

  GPUTexture *depthTex() { return &depthTexture; }

  GPUTexture *imageTex() { return &imageTexture; }

  GPUTexture *vertexTex() { return &vertexTexture; }

  GPUTexture *normalTex() { return &normalTexture; }

  GPUTexture *timeTex() { return &timeTexture; }

  GPUTexture *oldImageTex() { return &oldImageTexture; }

  GPUTexture *oldVertexTex() { return &oldVertexTexture; }

  GPUTexture *oldNormalTex() { return &oldNormalTexture; }

  GPUTexture *oldTimeTex() { return &oldTimeTexture; }

  GPUTexture *colorInfoTex() { return &colorInfoTexture; }

  GPUTexture *vertexInfoTex() { return &vertexInfoTexture; }

  GPUTexture *normalInfoTex() { return &normalInfoTexture; }

  static const int FACTOR;

private:
  std::shared_ptr<Shader> indexProgram;
  GlFramebuffer indexFrameBuffer;
  GlRenderBuffer indexRenderBuffer;
  GPUTexture indexTexture;
  GPUTexture vertConfTexture;
  GPUTexture colorTimeTexture;
  GPUTexture normalRadTexture;

  std::shared_ptr<Shader> drawDepthProgram;
  GlFramebuffer drawFrameBuffer;
  GlRenderBuffer drawRenderBuffer;
  GPUTexture drawTexture;

  std::shared_ptr<Shader> depthProgram;
  GlFramebuffer depthFrameBuffer;
  GlRenderBuffer depthRenderBuffer;
  GPUTexture depthTexture;

  std::shared_ptr<Shader> combinedProgram;
  GlFramebuffer combinedFrameBuffer;
  GlRenderBuffer combinedRenderBuffer;
  GPUTexture imageTexture;
  GPUTexture vertexTexture;
  GPUTexture normalTexture;
  GPUTexture timeTexture;

  GlFramebuffer oldFrameBuffer;
  GlRenderBuffer oldRenderBuffer;
  GPUTexture oldImageTexture;
  GPUTexture oldVertexTexture;
  GPUTexture oldNormalTexture;
  GPUTexture oldTimeTexture;

  GlFramebuffer infoFrameBuffer;
  GlRenderBuffer infoRenderBuffer;
  GPUTexture colorInfoTexture;
  GPUTexture vertexInfoTexture;
  GPUTexture normalInfoTexture;
};

#endif /* INDEXMAP_H_ */
