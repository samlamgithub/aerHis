/*
 * This file is part of ElasticFusion.
 *
 * Copyright (C) 2015 Imperial College London
 *
 * The use of the code within this file and all code within files that
 * make up the software that is ElasticFusion is permitted for
 * non-commercial purposes only.  The full terms and conditions that
 * apply to the code within this file are detailed within the LICENSE.txt
 * file and at <http://www.imperial.ac.uk/dyson-robotics-lab/downloads/elastic-fusion/elastic-fusion-license/>
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


static const char* glErrorString(GLenum err) {
  switch(err) {
    case GL_INVALID_ENUM: return "Invalid Enum";
    case GL_INVALID_VALUE: return "Invalid Value";
    case GL_INVALID_OPERATION: return "Invalid Operation";
   // case GL_STACK_OVERFLOW: return "Stack Overflow";
   // case GL_STACK_UNDERFLOW: return "Stack Underflow";
    case GL_OUT_OF_MEMORY: return "Out of Memory";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
    default: return "Unknown Error";
  }
}

inline void CheckGlDieOnError() {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGI("GPUTexture.cpp CheckGlDieOnError after %s: glError (0x%x)\n", glErrorString(error), error);
    }
}

// : texture(new pangolin::GlTexture(width, height, internalFormat, draw, 0, format, dataType)),
GPUTexture::GPUTexture(const int width,
                       const int height,
                       const GLenum internalFormat,
                       const GLenum format,
                       const GLenum dataType,
                       const bool draw,
                       const bool cuda)
  : texture(new GlTexture(width, height, internalFormat, draw, 0, format, dataType)),
   draw(draw),
   width(width),
   height(height),
   internalFormat(internalFormat),
   format(format),
   dataType(dataType) {
CheckGlDieOnError();
    LOGI("MY elasitcfusion GPUTexture struct init 1 ");
    if(cuda) {
        cudaError_t err = cudaGraphicsGLRegisterImage(&cudaRes, texture->tid,
        GL_TEXTURE_2D, cudaGraphicsRegisterFlagsReadOnly);
        if(cudaSuccess != err) {
          LOGI("elasticfusion GPU texture cudaGraphicsGLRegisterImage error: %s", cudaGetErrorString(err));
        } else {
          LOGI("elasticfusion GPU texture cudaGraphicsGLRegisterImage success");
        }
    } else {
        cudaRes = 0;
    }
    LOGI("MY elasitcfusion GPUTexture struct init 2 ");
CheckGlDieOnError();
}

GPUTexture::~GPUTexture()
{
    if(texture)
    {
        delete texture;
    }

    if(cudaRes)
    {
        cudaGraphicsUnregisterResource(cudaRes);
    }
}
