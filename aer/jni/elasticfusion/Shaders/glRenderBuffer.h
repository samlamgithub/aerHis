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

#include <GLES3/gl3.h>
#define __gl2_h_                 // what the f***
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

struct GlRenderBuffer {

   GlRenderBuffer() : width(0), height(0), rbid(0) {}

   GlRenderBuffer(GLint width, GLint height, GLint internal_format = GL_DEPTH_COMPONENT24 )
    : width(0), height(0), rbid(0) {
        Reinitialise(width,height,internal_format);
  }

   ~GlRenderBuffer() {
       // We have no GL context whilst exiting.
       if( width!=0  ) {
           glDeleteTextures(1, &rbid);
       }
   }

   void Reinitialise(GLint width, GLint height, GLint internal_format = GL_DEPTH_COMPONENT24) {
	   if( width!=0 ) {
	             glDeleteTextures(1, &rbid);
	         }

	         // Use a texture instead...
	         glGenTextures(1, &rbid);
	         glBindTexture(GL_TEXTURE_2D, rbid);

	         glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
	                 width, height,
	                 0, internal_format, GL_UNSIGNED_SHORT, NULL);

	         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }

      //! Move Constructor
    GlRenderBuffer(GlRenderBuffer&& tex)
        : width(tex.width), height(tex.height), rbid(tex.rbid) {
        tex.rbid = tex.width = tex.height = 0;
    }

    GLint width;
    GLint height;
    GLuint rbid;

  private:
    // Private copy constructor
        GlRenderBuffer(const GlRenderBuffer&) {}

};

#endif /* GLRENDERBUFFER_H_ */
