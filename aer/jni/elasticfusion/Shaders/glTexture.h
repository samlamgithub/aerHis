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

#ifndef GLTEXTURE_H_
#define GLTEXTURE_H_

#include <GLES3/gl3.h>
#define __gl2_h_
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "../../tango_interface/util.hpp"
#include "GLExtensions.h"
#include <string>
#include <utility>

class GlTexture {
public:
  GlTexture() : internal_format(0), tid(0), width(0), height(0) {}

  GlTexture(GLint width, GLint height, GLint internal_format = GL_RGBA8,
            bool sampling_linear = true, int border = 0,
            GLenum glformat = GL_RGBA, GLenum gltype = GL_UNSIGNED_BYTE,
            GLvoid *data = NULL)
      : internal_format(0), tid(0) {
         width, height, internal_format, border, glformat, gltype,
         data == NULL);
    Reinitialise(width, height, internal_format, sampling_linear, border,
                 glformat, gltype, data);
  }

  void operator=(GlTexture &&tex) {
    internal_format = tex.internal_format;
    tid = tex.tid;

    tex.internal_format = 0;
    tex.tid = 0;
  }

  void Delete() {
    // We have no GL context whilst exiting.
    if (internal_format != 0) {
      glDeleteTextures(1, &tid);
      internal_format = 0;
      tid = 0;
      width = 0;
      height = 0;
    }
  }

  ~GlTexture() {
    // We have no GL context whilst exiting.
    if (internal_format != 0) {
      glDeleteTextures(1, &tid);
    }
  }

  void Bind() const {
    glBindTexture(GL_TEXTURE_2D, tid);
  }

  void Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  virtual void Reinitialise(GLsizei w, GLsizei h, GLint int_format = GL_RGBA8,
                            bool sampling_linear = true, int border = 0,
                            GLenum glformat = GL_RGBA,
                            GLenum gltype = GL_UNSIGNED_BYTE,
                            GLvoid *data = NULL) {
    if (tid != 0) {
      glDeleteTextures(1, &tid);
    }

    internal_format = int_format;
    width = w;
    height = h;

    glGenTextures(1, &tid);

    Bind();

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, border,
                 glformat, gltype, data);

    if (sampling_linear) {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    } else {
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  void Upload(const void *data, GLenum data_format = GL_LUMINANCE,
              GLenum data_type = GL_FLOAT) {
    Bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, data_format,
                    data_type, data);
  }

  GLint internal_format;
  GLuint tid;
  GLint width;
  GLint height;

private:
  // Private copy constructor
  GlTexture(GlTexture &&tex) { *this = std::move(tex); }
};

#endif /* GLTEXTURE_H_ */
