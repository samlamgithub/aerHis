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

#include "Resize.h"

inline const char *glCheckFramebufferStatusResize() {
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
    char integer_string[32];
    int integer = status;
    sprintf(integer_string, "%d", status);
    char other_string[64] = "MY elasitcfusion  else: ";
    strcat(other_string, integer_string);
    return other_string;
  }
}

static const char *glErrorStringResize(GLenum err) {
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

inline void check_gl_errorResize() {
  glCheckFramebufferStatusResize();
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_error GlobalModel My elastic-fusion CheckGlDieOnError after "
         ":%s ,%s() glError (0x%x)\n",  glCheckFramebufferStatusResize(),
         glErrorStringResize(error), error);
  }
}

Resize::Resize(int srcWidth, int srcHeight, int destWidth, int destHeight):
// imageTexture(destWidth, destHeight, GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE, false, true),
imageTexture(destWidth, destHeight, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, true),
// vertexTexture(destWidth, destHeight, GL_RGBA32F, GL_LUMINANCE, GL_FLOAT,  false, true),
  vertexTexture(destWidth, destHeight,  GL_RGBA32F, GL_RGBA, GL_FLOAT,  false, true),
// timeTexture(destWidth, destHeight, GL_LUMINANCE16UI_EXT,  GL_LUMINANCE_INTEGER_EXT,  GL_UNSIGNED_SHORT, false, true),
timeTexture(destWidth, destHeight, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, true),
    imageProgram(
          loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
      imageRenderBuffer(destWidth, destHeight),
      vertexProgram(
          loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
      vertexRenderBuffer(destWidth, destHeight),
      timeProgram(
          loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
      timeRenderBuffer(destWidth, destHeight) {
  LOGI("MY elasitcfusion resize struct init start 1 ");
  LOGI("MY elasitcfusion Resize struct Resize "
       "AttachColour(*imageTexture.texture); start ");
  imageFrameBuffer.AttachColour(*imageTexture.texture);
  LOGI("MY elasitcfusion Resize struct Resize "
       "AttachColour(*imageTexture.texture); done ");
  LOGI("MY elasitcfusion Resize struct Resize AttachDepth(imageRenderBuffer); "
       "start ");
  imageFrameBuffer.AttachDepth(imageRenderBuffer);
  LOGI("MY elasitcfusion Resize struct Resize AttachDepth(imageRenderBuffer); "
       "done ");
  LOGI("MY elasitcfusion Resize struct Resize "
       "AttachColour(*vertexTexture.texture); start ");
  vertexFrameBuffer.AttachColour(*vertexTexture.texture);
  LOGI("MY elasitcfusion Resize struct Resize "
       "AttachColour(*vertexTexture.texture); done ");
  LOGI("MY elasitcfusion Resize struct Resize AttachDepth(vertexRenderBuffer); "
       "start ");
  vertexFrameBuffer.AttachDepth(vertexRenderBuffer);
  LOGI("MY elasitcfusion Resize struct Resize AttachDepth(vertexRenderBuffer); "
       "done ");
  LOGI("MY elasitcfusion Resize struct Resize "
       "AttachColour(*timeTexture.texture); start ");
  timeFrameBuffer.AttachColour(*timeTexture.texture);
  LOGI("MY elasitcfusion Resize struct Resize "
       "AttachColour(*timeTexture.texture); done ");
  LOGI("MY elasitcfusion Resize struct Resize AttachDepth(timeRenderBuffer); "
       "start ");
  timeFrameBuffer.AttachDepth(timeRenderBuffer);
  LOGI("MY elasitcfusion Resize struct Resize AttachDepth(timeRenderBuffer); "
       "done ");
  LOGI("MY elasitcfusion resize struct init done 2 ");
}

Resize::~Resize() {}

void Resize::image(GPUTexture *source,
                   Img<Eigen::Matrix<unsigned char, 3, 1>> &dest) {
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 1 start");
  imageFrameBuffer.Bind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 2 ");
  // glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 3 ");
  glViewport(0, 0, imageRenderBuffer.width, imageRenderBuffer.height);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 4 ");
  glClearColor(0, 0, 0, 0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 5 ");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // keyi
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 6 ");
  imageProgram->Bind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 7 ");
  imageProgram->setUniform(Uniform("eSampler", 0));
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 8 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 9 ");
  glBindTexture(GL_TEXTURE_2D, source->texture->tid);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 10 ");
  glDrawArrays(GL_POINTS, 0, 1);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 11 glReadPixels");
// glReadPixels(0, 0, imageRenderBuffer.width, imageRenderBuffer.height, GL_RGB,
  glReadPixels(0, 0, imageRenderBuffer.width, imageRenderBuffer.height, GL_RGBA,
               GL_UNSIGNED_BYTE, dest.data); // here Invalid Operation()
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 12 glReadPixels");
  imageFrameBuffer.Unbind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 13 ");
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 14 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 15 ");
  imageProgram->Unbind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 16 ");
  // glPopAttrib();
  glFinish();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct image 17 done");
}

void Resize::vertex(GPUTexture *source, Img<Eigen::Vector4f> &dest) {
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 1 start");
  vertexFrameBuffer.Bind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 2 ");
  // glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 3 ");
  glViewport(0, 0, vertexRenderBuffer.width, vertexRenderBuffer.height);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 4");
  glClearColor(0, 0, 0, 0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 4 1");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // keyi
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 5");
  vertexProgram->Bind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 6");
  vertexProgram->setUniform(Uniform("eSampler", 0));
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 7");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 8");
  glBindTexture(GL_TEXTURE_2D, source->texture->tid);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 9");
  glDrawArrays(GL_POINTS, 0, 1);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 10 glReadPixels");
  glReadPixels(0, 0, vertexRenderBuffer.width, vertexRenderBuffer.height,
               GL_RGBA, GL_FLOAT, dest.data); // no
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 11 glReadPixels");
  vertexFrameBuffer.Unbind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 12");
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 13");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 14");
  vertexProgram->Unbind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex 15");
  // glPopAttrib();
  glFinish();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct vertex done");
}

void Resize::time(GPUTexture *source, Img<unsigned short> &dest) {
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 1 start");
  timeFrameBuffer.Bind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 2");
  // glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 3");
  glViewport(0, 0, timeRenderBuffer.width, timeRenderBuffer.height);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 4");
  glClearColor(0, 0, 0, 0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 5");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // no log
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 6");
  timeProgram->Bind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 7");
  timeProgram->setUniform(Uniform("eSampler", 0));
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 8");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 9");
  glBindTexture(GL_TEXTURE_2D, source->texture->tid);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 10");
  glDrawArrays(GL_POINTS, 0, 1);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 11 glReadPixels");
  glReadPixels(0, 0, timeRenderBuffer.width, timeRenderBuffer.height,
               GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_SHORT, dest.data); // no
  // glReadPixels(0, 0, timeRenderBuffer.width, timeRenderBuffer.height,
  // GL_LUMINANCE, GL_UNSIGNED_SHORT, dest.data);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 12 glReadPixels");
  timeFrameBuffer.Unbind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 13");
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 14");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 15");
  timeProgram->Unbind();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 16");
  // glPopAttrib();
  glFinish();
  check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time done");
}
