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

#include "FillIn.h"

inline const char *glCheckFramebufferStatusFillin() {
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
    char other_string[64] = "MY elasitcfusion glCheckFramebufferStatus else: ";
    strcat(other_string, integer_string);
    return other_string;
  }
}

static const char *glErrorStringFillIn(GLenum err) {
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

inline void check_gl_errorFillIn() {
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      LOGI("frame buffer error: %s", glCheckFramebufferStatusFillin());
  }
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_error FillIn cpp My elastic-fusion CheckGlDieOnError after: %s, %s() glError (0x%x)\n",glCheckFramebufferStatusFillin(),
         glErrorStringFillIn(error), error);
  }
}

FillIn::FillIn()
    : imageTexture(Resolution::getInstance().width(),
                   Resolution::getInstance().height(),
// GL_RGBA, GL_RGB, GL_UNSIGNED_BYTE, false, true),
GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, true),
      vertexTexture(Resolution::getInstance().width(),
                    Resolution::getInstance().height(),
 // GL_RGBA32F,GL_LUMINANCE, GL_FLOAT, false, true),
 GL_RGBA32F, GL_RED, GL_FLOAT, false, true),
      normalTexture(Resolution::getInstance().width(),
                    Resolution::getInstance().height(),
 // GL_RGBA32F,GL_LUMINANCE, GL_FLOAT, false, true),
 GL_RGBA32F, GL_RED, GL_FLOAT, false, true),
      imageProgram(
          loadProgram(emptyvert_tuple, fill_rgbfrag_tuple, quadgeom_tuple)),
      imageRenderBuffer(Resolution::getInstance().width(),
                        Resolution::getInstance().height()),
      vertexProgram(
          loadProgram(emptyvert_tuple, fill_vertexfrag_tuple, quadgeom_tuple)),
      vertexRenderBuffer(Resolution::getInstance().width(),
                         Resolution::getInstance().height()),
      normalProgram(
          loadProgram(emptyvert_tuple, fill_normalfrag_tuple, quadgeom_tuple)),
      normalRenderBuffer(Resolution::getInstance().width(),
                         Resolution::getInstance().height()) {
  LOGI("MY elasitcfusion FillIn struct init start 1 ");
  LOGI("MY elasitcfusion FillIn struct FillIn "
       "AttachColour(*imageTexture.texture); start ");
  imageFrameBuffer.AttachColour(*imageTexture.texture);
  LOGI("MY elasitcfusion FillIn struct FillIn "
       "AttachColour(*imageTexture.texture); done ");
  LOGI("MY elasitcfusion FillIn struct FillIn AttachDepth(imageRenderBuffer); "
       "start ");
  imageFrameBuffer.AttachDepth(imageRenderBuffer);
  LOGI("MY elasitcfusion FillIn struct FillIn AttachDepth(imageRenderBuffer); "
       "done ");
  LOGI("MY elasitcfusion FillIn struct FillIn "
       "AttachColour(*vertexTexture.texture); start ");
  vertexFrameBuffer.AttachColour(*vertexTexture.texture);
  LOGI("MY elasitcfusion FillIn struct FillIn "
       "AttachColour(*vertexTexture.texture); done ");
  LOGI("MY elasitcfusion FillIn struct FillIn AttachDepth(vertexRenderBuffer); "
       "start ");
  vertexFrameBuffer.AttachDepth(vertexRenderBuffer);
  LOGI("MY elasitcfusion FillIn struct FillIn "
       "AttachDepth(*imageTexture.texture); done ");
  LOGI("MY elasitcfusion FillIn struct FillIn "
       "AttachColour(*normalTexture.texture); start ");
  normalFrameBuffer.AttachColour(*normalTexture.texture);
  LOGI("MY elasitcfusion FillIn struct FillIn "
       "AttachColour(*normalTexture.texture); done ");
  LOGI("MY elasitcfusion FillIn struct FillIn AttachDepth(normalRenderBuffer); "
       "start ");
  normalFrameBuffer.AttachDepth(normalRenderBuffer);
  LOGI("MY elasitcfusion FillIn struct FillIn AttachDepth(normalRenderBuffer); "
       "done ");
  LOGI("MY elasitcfusion FillIn struct init done 2 ");
}

FillIn::~FillIn() {}

void FillIn::image(GPUTexture *existingRgb, GPUTexture *rawRgb,
                   bool passthrough) {
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image start 1 ");
  imageFrameBuffer.Bind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 2 ");
  //glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 3 ");
  glViewport(0, 0, imageRenderBuffer.width, imageRenderBuffer.height);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 4 ");
  glClearColor(0, 0, 0, 0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 5 ");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // keyi
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 6 ");
  imageProgram->Bind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 7 ");
  imageProgram->setUniform(Uniform("eSampler", 0));
  imageProgram->setUniform(Uniform("rSampler", 1));
  imageProgram->setUniform(Uniform("passthrough", (int)passthrough));
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 8 ");
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, existingRgb->texture->tid);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 9 ");
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, rawRgb->texture->tid);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 10 ");
  glDrawArrays(GL_POINTS, 0, 1);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 11 ");
  imageFrameBuffer.Unbind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 12 ");
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 13 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 14 ");
  imageProgram->Unbind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 15 ");
  //glPopAttrib();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 16 ");
  glFinish();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn image 17 done");
}

void FillIn::vertex(GPUTexture *existingVertex, GPUTexture *rawDepth,
                    bool passthrough) {
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 1 start");
  vertexFrameBuffer.Bind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 2");
  //glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 3 ");
  glViewport(0, 0, vertexRenderBuffer.width, vertexRenderBuffer.height);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 4 ");
  glClearColor(0, 0, 0, 0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 5 ");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // keyi
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 6 ");
  vertexProgram->Bind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 7 ");
  vertexProgram->setUniform(Uniform("eSampler", 0));
  vertexProgram->setUniform(Uniform("rSampler", 1));
  vertexProgram->setUniform(Uniform("passthrough", (int)passthrough));
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 8 ");
  Eigen::Vector4f cam(Intrinsics::getInstance().cx(),
                      Intrinsics::getInstance().cy(),
                      1.0f / Intrinsics::getInstance().fx(),
                      1.0f / Intrinsics::getInstance().fy());
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 9 ");
  vertexProgram->setUniform(Uniform("cam", cam));
  vertexProgram->setUniform(
      Uniform("cols", (float)Resolution::getInstance().cols()));
  vertexProgram->setUniform(
      Uniform("rows", (float)Resolution::getInstance().rows()));
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 10 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 11 ");
  glBindTexture(GL_TEXTURE_2D, existingVertex->texture->tid);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 12");
  glActiveTexture(GL_TEXTURE1);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 13 ");
  glBindTexture(GL_TEXTURE_2D, rawDepth->texture->tid);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 14 ");
  glDrawArrays(GL_POINTS, 0, 1);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 15 ");
  vertexFrameBuffer.Unbind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 16 ");
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 17 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 18 ");
  vertexProgram->Unbind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 19 ");
  //glPopAttrib();
  glFinish();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn vertex 20 done ");
}

void FillIn::normal(GPUTexture *existingNormal, GPUTexture *rawDepth,
                    bool passthrough) {
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 1 start ");
  normalFrameBuffer.Bind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 2 ");
  //glPushAttrib(GL_VIEWPORT_BIT);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 3 ");
  glViewport(0, 0, normalRenderBuffer.width, normalRenderBuffer.height);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 4 ");
  glClearColor(0, 0, 0, 0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 5 ");
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // keyi
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 6");
  normalProgram->Bind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 7 ");
  normalProgram->setUniform(Uniform("eSampler", 0));
  normalProgram->setUniform(Uniform("rSampler", 1));
  normalProgram->setUniform(Uniform("passthrough", (int)passthrough));

  Eigen::Vector4f cam(Intrinsics::getInstance().cx(),
                      Intrinsics::getInstance().cy(),
                      1.0f / Intrinsics::getInstance().fx(),
                      1.0f / Intrinsics::getInstance().fy());

  normalProgram->setUniform(Uniform("cam", cam));
  normalProgram->setUniform(
      Uniform("cols", (float)Resolution::getInstance().cols()));
  normalProgram->setUniform(
      Uniform("rows", (float)Resolution::getInstance().rows()));
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 8 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 9 ");
  glBindTexture(GL_TEXTURE_2D, existingNormal->texture->tid);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 10 ");
  glActiveTexture(GL_TEXTURE1);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 11");
  glBindTexture(GL_TEXTURE_2D, rawDepth->texture->tid);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 12 ");
  glDrawArrays(GL_POINTS, 0, 1);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 13 ");
  normalFrameBuffer.Unbind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 14");
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 15 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 16 ");
  normalProgram->Unbind();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 17 ");
  //glPopAttrib();
  glFinish();
  check_gl_errorFillIn();
  LOGI("MY elasitcfusion FillIn normal 18 done");
}
