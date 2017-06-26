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

#include "ComputePack.h"

const std::string ComputePack::NORM = "NORM";
const std::string ComputePack::FILTER = "FILTER";
const std::string ComputePack::METRIC = "METRIC";
const std::string ComputePack::METRIC_FILTERED = "METRIC_FILTERED";

#ifndef GL_CHECK_ERROR
#define GL_CHECK_ERROR

static const char *glErrorString(GLenum err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "Invalid Enum";
  case GL_INVALID_VALUE:
    return "Invalid Value";
  case GL_INVALID_OPERATION:
    return "Invalid Operation";
  case GL_OUT_OF_MEMORY:
    return "error Out of Memory";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  default:
    return "Unknown Error";
  }
}

inline const char * glCheckFramebufferStatusFunc() {
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return "GL_FRAMEBUFFER_COMPLETE";
  } else if (status == GL_FRAMEBUFFER_UNDEFINED) {
    return "GL_FRAMEBUFFER_UNDEFINED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    return " GL_FRAMEBUFFER_UNSUPPORTED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
  } else if (status == GL_INVALID_ENUM) {
    return "GL_INVALID_ENUM";
  } else {
	  char integer_string[32];
	  int integer = status;
	  sprintf(integer_string, "%d", status);
	  char other_string[64] = "else: ";
	  strcat(other_string, integer_string);
	  return other_string;
  }
}

inline void check_gl_errorFunc() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    if (error != GL_NO_ERROR) {
      LOGI("CheckGlDieOnError after %s, %s() "
           "glError (0x%x)\n", glCheckFramebufferStatusFunc(),
           glErrorString(error), error);
    }
  }
}

#endif /* GL_CHECK_ERROR */

ComputePack::ComputePack(std::shared_ptr<Shader> program, GlTexture *target)
    : program(program), renderBuffer(Resolution::getInstance().width(),
                                     Resolution::getInstance().height()),
      target(target) {
  frameBuffer.AttachColour(*target);
  frameBuffer.AttachDepth(renderBuffer);
}

ComputePack::~ComputePack() {}

void ComputePack::compute(GlTexture *input,
                          const std::vector<Uniform> *const uniforms) {
  input->Bind();
  frameBuffer.Bind();
  glViewport(0, 0, renderBuffer.width, renderBuffer.height);
  glClearColor(0, 0, 0, 0);
  glClear(GL_COLOR_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT);
  program->Bind();
  if (uniforms) {
    for (size_t i = 0; i < uniforms->size(); i++) {
      program->setUniform(uniforms->at(i));
    }
  }
  glDrawArrays(GL_POINTS, 0, 1);
  frameBuffer.Unbind();
  program->Unbind();
  glFinish();
}
