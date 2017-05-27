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

#include "GPUTexture.h"

const std::string GPUTexture::RGB = "RGB";
const std::string GPUTexture::DEPTH_RAW = "DEPTH";
const std::string GPUTexture::DEPTH_FILTERED = "DEPTH_FILTERED";
const std::string GPUTexture::DEPTH_METRIC = "DEPTH_METRIC";
const std::string GPUTexture::DEPTH_METRIC_FILTERED = "DEPTH_METRIC_FILTERED";
const std::string GPUTexture::DEPTH_NORM = "DEPTH_NORM";

static const char *glErrorStringGT(GLenum err) {
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
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
  default:
    return "Unknown Error";
  }
}

inline const char *glCheckFramebufferStatusGT() {
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

inline void CheckGlDieOnErrorGT() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("GPUTexture.cpp CheckGlDieOnError after : %s, %s: glError (0x%x)\n",
         glCheckFramebufferStatusGT(), glErrorStringGT(error), error);
  }
}

// : texture(new pangolin::GlTexture(width, height, internalFormat, draw, 0,
// format, dataType)),
GPUTexture::GPUTexture(const int width, const int height,
                       const GLenum internalFormat, const GLenum format,
                       const GLenum dataType, const bool draw, const bool cuda)
    : texture(new GlTexture(width, height, internalFormat, draw, 0, format,
                            dataType)),
      draw(draw), width(width), height(height), internalFormat(internalFormat),
      format(format), dataType(dataType) {
  CheckGlDieOnErrorGT();
  LOGI(
      "MY elasitcfusion GPUTexture struct start init 1 : %d, %d,  %d,  %d,  %d",
      width, height, internalFormat, format, dataType);
  if (cuda) {
    LOGI("MY elasitcfusion GPUTexture is cuda: %d", texture->tid);
    cudaError_t err =
        cudaGraphicsGLRegisterImage(&cudaRes, texture->tid, GL_TEXTURE_2D,
                                    cudaGraphicsRegisterFlagsReadOnly);
    if (cudaSuccess != err) {
      LOGI("elasticfusion GPU texture cudaGraphicsGLRegisterImage error: %s",
           cudaGetErrorString(err));
    } else {
      LOGI("elasticfusion GPU texture cudaGraphicsGLRegisterImage success");
    }
  } else {
    LOGI("MY elasitcfusion GPUTexture is not cuda");
    cudaRes = 0;
  }
  LOGI("MY elasitcfusion GPUTexture struct init 2 done ");
  CheckGlDieOnErrorGT();
}

GPUTexture::~GPUTexture() {
  LOGI("MY elasitcfusion GPUTexture is free start 1");
  if (texture) {
    LOGI("MY elasitcfusion GPUTexture is free texture");
    delete texture;
  }
  LOGI("MY elasitcfusion GPUTexture is free start 2");
  if (cudaRes) {
    LOGI("MY elasitcfusion GPUTexture is free cudaRes");
    cudaGraphicsUnregisterResource(cudaRes);
  }
  LOGI("MY elasitcfusion GPUTexture is free done");
}
