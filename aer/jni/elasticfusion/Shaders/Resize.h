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

#ifndef RESIZE_H_
#define RESIZE_H_

#include "../GPUTexture.h"
#include "../Utils/Img.h"
#include "../Utils/Intrinsics.h"
#include "../Utils/Resolution.h"
#include "Shaders.h"
#include "Uniform.h"
#include <glFrameBuffer.h>

class Resize {
public:
  Resize(int srcWidth, int srcHeight, int destWidth, int destHeight);
  virtual ~Resize();

  void image(GPUTexture *source, Img<Eigen::Matrix<unsigned char, 4, 1>> &dest);
  void vertex(GPUTexture *source, Img<Eigen::Vector4f> &dest);
  void time(GPUTexture *source, Img<unsigned short> &dest);

  GPUTexture imageTexture;
  GPUTexture vertexTexture;
  GPUTexture timeTexture;

  std::shared_ptr<Shader> imageProgram;
  GlRenderBuffer imageRenderBuffer;
  GlFramebuffer imageFrameBuffer;

  std::shared_ptr<Shader> vertexProgram;
  GlRenderBuffer vertexRenderBuffer;
  GlFramebuffer vertexFrameBuffer;

  std::shared_ptr<Shader> timeProgram;
  GlRenderBuffer timeRenderBuffer;
  GlFramebuffer timeFrameBuffer;
};

#endif /* RESIZE_H_ */
