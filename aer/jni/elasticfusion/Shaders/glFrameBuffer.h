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

#ifndef GLFRAMEBUFFER_H_
#define GLFRAMEBUFFER_H_

#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <glTexture.h>
#include <glRenderBuffer.h>

struct GlFramebuffer {

    GlFramebuffer::GlFramebuffer(): fbid(0), attachments(0){}

    GlFramebuffer::~GlFramebuffer() {
        if(fbid) {
            glDeleteFramebuffersEXT(1, &fbid);
        }
    }

    GlFramebuffer::GlFramebuffer(GlTexture& colour, GlRenderBuffer& depth) : attachments(0) {
        glGenFramebuffersEXT(1, &fbid);
        AttachColour(colour);
        AttachDepth(depth);
        //CheckGlDieOnError();
    }

     GlFramebuffer::GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlRenderBuffer& depth)
        : attachments(0)  {
        glGenFramebuffersEXT(1, &fbid);
        AttachColour(colour0);
        AttachColour(colour1);
        AttachDepth(depth);
        //CheckGlDieOnError();
    }

    void GlFramebuffer::Bind() const {
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
        glDrawBuffers( attachments, attachment_buffers );
    }

     void GlFramebuffer::Reinitialise() {
        if(fbid) {
            glDeleteFramebuffersEXT(1, &fbid);
        }
        glGenFramebuffersEXT(1, &fbid);
    }

     void GlFramebuffer::Unbind() const {
        glDrawBuffers( 1, attachment_buffers );
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    }

     GLenum GlFramebuffer::AttachColour(GlTexture& tex ){
        if(!fbid) Reinitialise();

        const GLenum color_attachment = GL_COLOR_ATTACHMENT0_EXT + attachments;
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, color_attachment, GL_TEXTURE_2D, tex.tid, 0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        attachments++;
        //CheckGlDieOnError();
        return color_attachment;
    }

    void GlFramebuffer::AttachDepth(GlRenderBuffer& rb ) {
        if(!fbid) Reinitialise();

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, rb.rbid, 0);
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        ////CheckGlDieOnError();
    }

    GLuint fbid;
    unsigned attachments;
};

#endif /* GLFRAMEBUFFER_H_ */
