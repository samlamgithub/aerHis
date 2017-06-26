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

#ifndef GLFRAMEBUFFER_H_
#define GLFRAMEBUFFER_H_

#include <GLES3/gl3.h>
#define __gl2_h_
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLExtensions.h>
#include <glRenderBuffer.h>
#include <glTexture.h>

struct GlFramebuffer {
  GlFramebuffer() : fbid(0), attachments(0) {}

  ~GlFramebuffer() {
    if (fbid) {
      glDeleteFramebuffers(1, &fbid);
    }
  }

  GlFramebuffer(GlTexture &colour, GlRenderBuffer &depth) : attachments(0) {
    glGenFramebuffers(1, &fbid);
    AttachColour(colour);
    AttachDepth(depth);
  }

  GlFramebuffer(GlTexture &colour0, GlTexture &colour1, GlRenderBuffer &depth)
      : attachments(0) {
    glGenFramebuffers(1, &fbid);
    AttachColour(colour0);
    AttachColour(colour1);
    AttachDepth(depth);
  }

  void Bind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, fbid);
  }

  void Reinitialise() {
    if (fbid) {
      glDeleteFramebuffers(1, &fbid);
    }
    glGenFramebuffers(1, &fbid);
  }

  void Unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  GLenum AttachColour(GlTexture &tex) {
    if (!fbid)
      Reinitialise();

    const GLenum color_attachment = GL_COLOR_ATTACHMENT0 + attachments;
    glBindFramebuffer(GL_FRAMEBUFFER, fbid);

    glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment, GL_TEXTURE_2D,
                           tex.tid, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    attachments++;

    return color_attachment;
  }

  void AttachDepth(GlRenderBuffer &rb) {
    if (!fbid)
      Reinitialise();
    glBindFramebuffer(GL_FRAMEBUFFER, fbid);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           rb.rbid, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  GLuint fbid;
  unsigned attachments;
};

#endif /* GLFRAMEBUFFER_H_ */
