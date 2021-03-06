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

#ifndef FEEDBACKBUFFER_H_
#define FEEDBACKBUFFER_H_

#include "../Utils/Intrinsics.h"
#include "../Utils/Resolution.h"
#include "Shaders.h"
#include "Uniform.h"
#include "Vertex.h"
#include <glTexture.h>

#include "../../tango_interface/util.hpp"

class FeedbackBuffer {
public:
  FeedbackBuffer(std::shared_ptr<Shader> program);
  virtual ~FeedbackBuffer();

  std::shared_ptr<Shader> program;

  void compute(GlTexture *color, GlTexture *depth, const int &time,
               const float depthCutoff);

  static const std::string RAW, FILTERED;

  GLuint vbo;
  GLuint fid;

private:
  std::shared_ptr<Shader> drawProgram;
  GLuint uvo;
  GLuint countQuery;
  const int bufferSize;
  unsigned int count;
};

#endif /* FEEDBACKBUFFER_H_ */
