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

#include "Resize.h"


static const char* glErrorStringResize(GLenum err) {
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

inline void check_gl_errorResize() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_errorGlobalModel My elastic-fusion CheckGlDieOnError after %s() glError (0x%x)\n", glErrorStringResize(error), error);
  }
}

Resize::Resize(int srcWidth,
               int srcHeight,
               int destWidth,
               int destHeight)
: imageTexture(destWidth,
               destHeight,
               GL_RGBA,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               false,
               true),
  vertexTexture(destWidth,
                destHeight,
                GL_RGBA32F,
                GL_LUMINANCE,
                GL_FLOAT,
                false,
                true),
  timeTexture(destWidth,
              destHeight,
              GL_LUMINANCE16UI_EXT,
              GL_LUMINANCE_INTEGER_EXT,
              // GL_LUMINANCE16F_EXT,
            // GL_LUMINANCE,
              GL_UNSIGNED_SHORT,
              false,
              true),
  imageProgram(loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
  imageRenderBuffer(destWidth, destHeight),
  vertexProgram(loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
  vertexRenderBuffer(destWidth, destHeight),
  timeProgram(loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
  timeRenderBuffer(destWidth, destHeight)
{
LOGI("MY elasitcfusion resize struct init 1 ");
   imageFrameBuffer.AttachColour(*imageTexture.texture);
   imageFrameBuffer.AttachDepth(imageRenderBuffer);

   vertexFrameBuffer.AttachColour(*vertexTexture.texture);
   vertexFrameBuffer.AttachDepth(vertexRenderBuffer);

   timeFrameBuffer.AttachColour(*timeTexture.texture);
   timeFrameBuffer.AttachDepth(timeRenderBuffer);
LOGI("MY elasitcfusion resize struct init 2 ");
}

Resize::~Resize()
{
}

void Resize::image(GPUTexture * source, Img<Eigen::Matrix<unsigned char, 3, 1>> & dest)
{
check_gl_errorResize();
LOGI("MY elasitcfusion resize struct image 1 ");
    imageFrameBuffer.Bind();
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 2 ");
    //glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 3 ");
    glViewport(0, 0, imageRenderBuffer.width, imageRenderBuffer.height);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 4 ");
    glClearColor(0, 0, 0, 0);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 5 ");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 6 ");
    imageProgram->Bind();
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 7 ");
    imageProgram->setUniform(Uniform("eSampler", 0));
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 8 ");
    glActiveTexture(GL_TEXTURE0);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 9 ");
    glBindTexture(GL_TEXTURE_2D, source->texture->tid);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct image 10 ");
    glDrawArrays(GL_POINTS, 0, 1);

    glReadPixels(0, 0, imageRenderBuffer.width, imageRenderBuffer.height, GL_RGB, GL_UNSIGNED_BYTE, dest.data);

    imageFrameBuffer.Unbind();

    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);

    imageProgram->Unbind();

    //glPopAttrib();

    glFinish();
LOGI("MY elasitcfusion resize struct image 12 ");
}

void Resize::vertex(GPUTexture * source, Img<Eigen::Vector4f> & dest)
{
check_gl_errorResize();
LOGI("MY elasitcfusion resize struct vertex 1 ");
    vertexFrameBuffer.Bind();
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct vertex 2 ");
    //glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct vertex 3 ");
    glViewport(0, 0, vertexRenderBuffer.width, vertexRenderBuffer.height);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct vertex 4");
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct vertex 5");
    vertexProgram->Bind();
    check_gl_errorResize();
    LOGI("MY elasitcfusion resize struct vertex 6");
    vertexProgram->setUniform(Uniform("eSampler", 0));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source->texture->tid);

    glDrawArrays(GL_POINTS, 0, 1);

    glReadPixels(0, 0, vertexRenderBuffer.width, vertexRenderBuffer.height, GL_RGBA, GL_FLOAT, dest.data);

    vertexFrameBuffer.Unbind();

    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);

    vertexProgram->Unbind();

    //glPopAttrib();

    glFinish();
LOGI("MY elasitcfusion resize struct vertex 2");
}

void Resize::time(GPUTexture * source, Img<unsigned short> & dest)
{
  check_gl_errorResize();
LOGI("MY elasitcfusion resize struct time 1");
    timeFrameBuffer.Bind();
    check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 2");
    //glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 3");
    glViewport(0, 0, timeRenderBuffer.width, timeRenderBuffer.height);
    check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 4");
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_errorResize();
  LOGI("MY elasitcfusion resize struct time 5");
    timeProgram->Bind();

    timeProgram->setUniform(Uniform("eSampler", 0));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, source->texture->tid);

    glDrawArrays(GL_POINTS, 0, 1);

    glReadPixels(0, 0, timeRenderBuffer.width, timeRenderBuffer.height, GL_LUMINANCE_INTEGER_EXT, GL_UNSIGNED_SHORT, dest.data);
    // glReadPixels(0, 0, timeRenderBuffer.width, timeRenderBuffer.height, GL_LUMINANCE, GL_UNSIGNED_SHORT, dest.data);

    timeFrameBuffer.Unbind();

    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);

    timeProgram->Unbind();

    //glPopAttrib();

    glFinish();
LOGI("MY elasitcfusion resize struct time 2");
}
