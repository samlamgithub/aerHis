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

#ifndef GLRENDERBUFFER_H_
#define GLRENDERBUFFER_H_

#include <GLES3/gl3.h>
#define __gl2_h_ // what the f***
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#include "GLExtensions.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

static const char *glErrorStringRB(GLenum err) {
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

inline const char *glCheckFramebufferStatusGlRenderBuffer() {
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

inline void CheckGlDieOnErrorRB() {
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      LOGI("frame buffer error: %s", glCheckFramebufferStatusGlRenderBuffer());
  }
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("GlRenderBuffer.h CheckGlDieOnError after: %s, %s: glError (0x%x)\n", glCheckFramebufferStatusGlRenderBuffer(),
         glErrorStringRB(error), error);
  }
}

struct GlRenderBuffer {
  GlRenderBuffer() : width(0), height(0), rbid(0) {
    LOGI("GlRenderBuffer init with nothing");
  }

  GlRenderBuffer(GLint width, GLint height,
                 GLint internal_format = GL_DEPTH_COMPONENT24)
      : width(0), height(0), rbid(0) {
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer init start %d, %d, %d, ", width, height,
         internal_format);
    Reinitialise(width, height, internal_format);
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer init done");
  }

  ~GlRenderBuffer() {
    // We have no GL context whilst exiting.
    if (width != 0) {
      glDeleteTextures(1, &rbid);
    }
  }

  void Reinitialise(GLint width, GLint height,
                    GLint internal_format = GL_DEPTH_COMPONENT24) {
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer Reinitialise start");
    if (width != 0) {
        LOGI("GlRenderBuffer Reinitialise start glDeleteTextures");
      glDeleteTextures(1, &rbid);
    }
    // Use a texture instead...
    GLint format = internal_format;
    GLint type = GL_UNSIGNED_SHORT;
    if (internal_format == GL_DEPTH_COMPONENT24) {
  LOGI("GlRenderBuffer Reinitialise change");
      format = GL_DEPTH_COMPONENT;
      type = GL_UNSIGNED_INT;
    }
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer Reinitialise 1");
    glGenTextures(1, &rbid);
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer Reinitialise 1 2");
    glBindTexture(GL_TEXTURE_2D, rbid);
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer Reinitialise 2");
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format,
                 type, NULL);
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer Reinitialise 3");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    CheckGlDieOnErrorRB();
    LOGI("GlRenderBuffer Reinitialise done");
  }

  //! Move Constructor
  GlRenderBuffer(GlRenderBuffer &&tex)
      : width(tex.width), height(tex.height), rbid(tex.rbid) {
    tex.rbid = tex.width = tex.height = 0;
  }

  GLint width;
  GLint height;
  GLuint rbid;
  // private:
  // Private copy constructor
  GlRenderBuffer(const GlRenderBuffer &) {}
};

#endif /* GLRENDERBUFFER_H_ */
