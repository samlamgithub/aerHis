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

#ifndef GLRENDERBUFFER_H_
#define GLRENDERBUFFER_H_

#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>

struct GlRenderBuffer {
    GlRenderBuffer();
    GlRenderBuffer(GLint width, GLint height, GLint internal_format = GL_DEPTH_COMPONENT24);

    void Reinitialise(GLint width, GLint height, GLint internal_format = GL_DEPTH_COMPONENT24);

    //! Move Constructor
    GlRenderBuffer(GlRenderBuffer&& tex);

    ~GlRenderBuffer();

   GlRenderBuffer::GlRenderBuffer() : width(0), height(0), rbid(0) {}

   GlRenderBuffer::GlRenderBuffer(GLint width, GLint height, GLint internal_format )
    : width(0), height(0), rbid(0){
        Reinitialise(width,height,internal_format);
  }

  void GlRenderBuffer::Reinitialise(GLint width, GLint height, GLint internal_format) {
        if( this->width != 0 ) {
            glDeleteRenderbuffersEXT(1, &rbid);
        }

        this->width = width;
        this->height = height;
        glGenRenderbuffersEXT(1, &rbid);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rbid);
        glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internal_format, width, height);
        glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
    }

    GlRenderBuffer::~GlRenderBuffer()  {
        // We have no GL context whilst exiting.
        if( width!=0 && !pangolin::ShouldQuit() ) {
            glDeleteRenderbuffersEXT(1, &rbid);
        }
    }
    GLint width;
    GLint height;
    GLuint rbid;

  private:
      // Private copy constructor
    GlRenderBuffer::GlRenderBuffer(GlRenderBuffer&& tex)
        : width(tex.width), height(tex.height), rbid(tex.rbid) {
        tex.rbid = tex.width = tex.height = 0;
    }
};

#endif /* GLRENDERBUFFER_H_ */
