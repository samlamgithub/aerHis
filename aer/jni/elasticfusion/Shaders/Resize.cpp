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

#include "Resize.h"

Resize::Resize(int srcWidth, int srcHeight, int destWidth, int destHeight):
  imageTexture(destWidth, destHeight, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, true),
  vertexTexture(destWidth, destHeight,  GL_RGBA32F, GL_RGBA, GL_FLOAT,  false, true),
  timeTexture(destWidth, destHeight, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT, false, true),
    imageProgram(
          loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
      imageRenderBuffer(destWidth, destHeight),
      vertexProgram(
          loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
      vertexRenderBuffer(destWidth, destHeight),
      timeProgram(
          loadProgram(emptyvert_tuple, resizefrag_tuple, quadgeom_tuple)),
      timeRenderBuffer(destWidth, destHeight) {
  imageFrameBuffer.AttachColour(*imageTexture.texture);
  imageFrameBuffer.AttachDepth(imageRenderBuffer);
  vertexFrameBuffer.AttachColour(*vertexTexture.texture);

  vertexFrameBuffer.AttachDepth(vertexRenderBuffer);

  timeFrameBuffer.AttachColour(*timeTexture.texture);

  timeFrameBuffer.AttachDepth(timeRenderBuffer);
}

Resize::~Resize() {}

void Resize::image(GPUTexture *source,
                   Img<Eigen::Matrix<unsigned char, 4, 1>> &dest) {
  imageFrameBuffer.Bind();

  glViewport(0, 0, imageRenderBuffer.width, imageRenderBuffer.height);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  imageProgram->Bind();

  imageProgram->setUniform(Uniform("eSampler", 0));

  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, source->texture->tid);

  glDrawArrays(GL_POINTS, 0, 1);

  glReadPixels(0, 0, imageRenderBuffer.width, imageRenderBuffer.height, GL_RGBA, GL_UNSIGNED_BYTE, dest.data);

  imageFrameBuffer.Unbind();

  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);

  imageProgram->Unbind();

  glFinish();
}

void Resize::vertex(GPUTexture *source, Img<Eigen::Vector4f> &dest) {
  vertexFrameBuffer.Bind();

  glViewport(0, 0, vertexRenderBuffer.width, vertexRenderBuffer.height);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  vertexProgram->Bind();

  vertexProgram->setUniform(Uniform("eSampler", 0));

  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, source->texture->tid);

  glDrawArrays(GL_POINTS, 0, 1);

  glReadPixels(0, 0, vertexRenderBuffer.width, vertexRenderBuffer.height, GL_RGBA, GL_FLOAT, dest.data);

  vertexFrameBuffer.Unbind();

  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);

  vertexProgram->Unbind();

  glFinish();
}

void Resize::time(GPUTexture *source, Img<unsigned short> &dest) {
  timeFrameBuffer.Bind();

  glViewport(0, 0, timeRenderBuffer.width, timeRenderBuffer.height);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  timeProgram->Bind();

  timeProgram->setUniform(Uniform("eSampler", 0));

  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, source->texture->tid);

  glDrawArrays(GL_POINTS, 0, 1);

  glReadPixels(0, 0, timeRenderBuffer.width, timeRenderBuffer.height,
               GL_RED_INTEGER, GL_UNSIGNED_SHORT, dest.data);

  timeFrameBuffer.Unbind();

  glBindTexture(GL_TEXTURE_2D, 0);

  glActiveTexture(GL_TEXTURE0);

  timeProgram->Unbind();

  glFinish();
}
