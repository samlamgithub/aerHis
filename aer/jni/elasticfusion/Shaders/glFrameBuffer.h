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

#include <GLES3/gl3.h>
#define __gl2_h_                 // what the f***
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <glTexture.h>
#include <glRenderBuffer.h>
#include <GLExtensions.h>

static const char* glErrorStringFB(GLenum err) {
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

inline void CheckGlDieOnErrorFB() {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGI("GlFramebuffer.h CheckGlDieOnError after %s: glError (0x%x)\n", glErrorStringFB(error), error);
    }
}

struct GlFramebuffer {

    GlFramebuffer(): fbid(0), attachments(0){  LOGI("GlFramebuffer init 1");}

    ~GlFramebuffer() {
        if(fbid) {
            // glDeleteFramebuffersEXT(1, &fbid);
            glDeleteFramebuffers(1, &fbid);
        }
    }

    GlFramebuffer(GlTexture& colour, GlRenderBuffer& depth) : attachments(0) {
        // glGenFramebuffersEXT(1, &fbid);
        LOGI("GlFramebuffer init 2 start");
        glGenFramebuffers(1, &fbid);
        AttachColour(colour);
        AttachDepth(depth);
        CheckGlDieOnErrorFB();
        LOGI("GlFramebuffer init 2 done");
    }

    GlFramebuffer(GlTexture& colour0, GlTexture& colour1, GlRenderBuffer& depth) : attachments(0)  {
        // glGenFramebuffersEXT(1, &fbid);
        LOGI("GlFramebuffer init 3 start");
        glGenFramebuffers(1, &fbid);
        AttachColour(colour0);
        AttachColour(colour1);
        AttachDepth(depth);
        CheckGlDieOnErrorFB();
        LOGI("GlFramebuffer init 3 start");
    }

    void Bind() const {
        // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
        glBindFramebuffer(GL_FRAMEBUFFER, fbid);
        // glDrawBuffers( attachments, attachment_buffers );
    }

    void Reinitialise() {
        if(fbid) {
            // glDeleteFramebuffersEXT(1, &fbid);
            glDeleteFramebuffers(1, &fbid);
        }
        // glGenFramebuffersEXT(1, &fbid);
        glGenFramebuffers(1, &fbid);
    }

     void Unbind() const {
        // glDrawBuffers( 1, attachment_buffers );
        // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

     GLenum AttachColour(GlTexture& tex ){
       LOGI("GlFramebuffer AttachColour start");
        if(!fbid) Reinitialise();
        // const GLenum color_attachment = GL_COLOR_ATTACHMENT0_EXT + attachments;
        // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
        // glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, color_attachment, GL_TEXTURE_2D, tex.tid, 0);
        // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
        const GLenum color_attachment = GL_COLOR_ATTACHMENT0 + attachments;
        glBindFramebuffer(GL_FRAMEBUFFER, fbid);
        glFramebufferTexture2D(GL_FRAMEBUFFER, color_attachment, GL_TEXTURE_2D, tex.tid, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        attachments++;
        CheckGlDieOnErrorFB();
 LOGI("GlFramebuffer AttachColour done");
        return color_attachment;
    }

    void AttachDepth(GlRenderBuffer& rb ) {
 LOGI("GlFramebuffer AttachDepth start");
        if(!fbid) Reinitialise();

        // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbid);
        // glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, rb.rbid, 0);
        // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, fbid);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, rb.rbid, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        CheckGlDieOnErrorFB();
 LOGI("GlFramebuffer AttachDepth done");
    }

    GLuint fbid;
    unsigned attachments;
};

#endif /* GLFRAMEBUFFER_H_ */
