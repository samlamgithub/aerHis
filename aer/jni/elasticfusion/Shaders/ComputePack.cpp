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

static const char *glErrorStringComputePack(GLenum err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "Invalid Enum";
  case GL_INVALID_VALUE:
    return "Invalid Value";
  case GL_INVALID_OPERATION:
    return "Invalid Operation";
  // case GL_STACK_OVERFLOW: return "Stack Overflow";
  // case GL_STACK_UNDERFLOW: return "Stack Underflow";
  case GL_OUT_OF_MEMORY:
    return "Out of Memory";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
//  case GL_CONTEXT_LOST:
  //  return "GL_CONTEXT_LOST";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
  default:
    return "Unknown Error";
  }
}

inline void check_gl_errorComputePack() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    if (error != GL_NO_ERROR) {
      LOGI("ComputePack My elastic-fusion  CheckGlDieOnError after %s() "
           "glError (0x%x)\n",
           glErrorStringComputePack(error), error);
    }
  }
}

// ComputePack::ComputePack(std::shared_ptr<Shader> program,
// pangolin::GlTexture * target)
ComputePack::ComputePack(std::shared_ptr<Shader> program, GlTexture *target)
    : program(program), renderBuffer(Resolution::getInstance().width(),
                                     Resolution::getInstance().height()),
      target(target) {
  frameBuffer.AttachColour(*target);
  frameBuffer.AttachDepth(renderBuffer);
}

ComputePack::~ComputePack() {}

// void ComputePack::compute(pangolin::GlTexture * input, const
// std::vector<Uniform> * const uniforms) {
void ComputePack::compute(GlTexture *input,
                          const std::vector<Uniform> *const uniforms) {
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 1 start");
  input->Bind();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 2");
  frameBuffer.Bind();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 3");
  // glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 4");
  glViewport(0, 0, renderBuffer.width, renderBuffer.height);
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 5");
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  check_gl_errorComputePack();
  GLenum status = glCheckFramebufferStatus(frameBuffer);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOGI("MY elasitcfusion ComputePack compute 5: %d", status);
    } else {
  LOGI("MY elasitcfusion ComputePack compute 5: complete");
}
  LOGI("MY elasitcfusion ComputePack compute 6");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //here
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 7");
  program->Bind();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 8");
  if (uniforms) {
    for (size_t i = 0; i < uniforms->size(); i++) {
      program->setUniform(uniforms->at(i));
    }
  }
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 9");
  glDrawArrays(GL_POINTS, 0, 1); //here
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 10");
  frameBuffer.Unbind();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 11");
  program->Unbind();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 12");
  // glPopAttrib();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 13");
  glFinish();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute  done");
}
