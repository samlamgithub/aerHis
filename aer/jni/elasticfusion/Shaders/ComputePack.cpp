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
    return "error Out of Memory";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  //  case GL_CONTEXT_LOST:
  //  return "GL_CONTEXT_LOST";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
  default:
    return "Unknown Error";
  }
}

inline const char * glCheckFramebufferStatusCC() {
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return "MY elasitcfusion ComputePack GL_FRAMEBUFFER_COMPLETE";
  } else if (status == GL_FRAMEBUFFER_UNDEFINED) {
    return "MY elasitcfusion ComputePack  GL_FRAMEBUFFER_UNDEFINED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    return "MY elasitcfusion ComputePack "
         "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    return "MY elasitcfusion ComputePack "
         "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    return "MY elasitcfusion ComputePack  GL_FRAMEBUFFER_UNSUPPORTED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    return "MY elasitcfusion ComputePack  "
         "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
  } else if (status == GL_INVALID_ENUM) {
    return "MY elasitcfusion ComputePack GL_INVALID_ENUM";
  } else {
	  char integer_string[32];
	  int integer = status;
	  sprintf(integer_string, "%d", status);
	  char other_string[64] = "MY elasitcfusion ComputePack else: ";
	  strcat(other_string, integer_string);
	  return other_string;
  }
}

inline void check_gl_errorComputePack() {
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      LOGI("frame buffer error: %s", glCheckFramebufferStatusCC());
  }
  for (GLint error = glGetError(); error; error = glGetError()) {
    if (error != GL_NO_ERROR) {
      LOGI("ComputePack My elastic-fusion  CheckGlDieOnError after %s, %s() "
           "glError (0x%x)\n", glCheckFramebufferStatusCC(),
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
LOGI("MY elasitcfusion ComputePack struct ComputePack init start");
LOGI("MY elasitcfusion ComputePack struct ComputePack AttachColour(*target); start ");
  frameBuffer.AttachColour(*target);
LOGI("MY elasitcfusion ComputePack struct ComputePack AttachColour(*target); done ");
LOGI("MY elasitcfusion ComputePack struct ComputePack AttachDepth(renderBuffer); start ");
frameBuffer.AttachDepth(renderBuffer);
LOGI("MY elasitcfusion ComputePack struct ComputePack AttachDepth(renderBuffer); done ");
LOGI("MY elasitcfusion ComputePack struct ComputePack init done");
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
  frameBuffer.Bind();  // no good
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 3");
  //glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 4");
  glViewport(0, 0, renderBuffer.width, renderBuffer.height);
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 5");
  glClearColor(0, 0, 0, 0);
  check_gl_errorComputePack();
  glCheckFramebufferStatusCC();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 6");
  glClear(GL_COLOR_BUFFER_BIT |
          GL_DEPTH_BUFFER_BIT); // here GL_INVALID_FRAMEBUFFER_OPERATION
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
  LOGI("MY elasitcfusion ComputePack compute 9 glDrawArrays 0 1 before");
  glDrawArrays(GL_POINTS, 0, 1); // here GL_INVALID_FRAMEBUFFER_OPERATION // no good
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 10 glDrawArrays 0 1 after");
  frameBuffer.Unbind();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 11");
  program->Unbind();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 12");
  //glPopAttrib();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 13");
  glFinish();
  check_gl_errorComputePack();
  LOGI("MY elasitcfusion ComputePack compute 14 done");
}
