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

#include "IndexMap.h"

const int IndexMap::FACTOR = 1;

IndexMap::IndexMap()
    : indexProgram(loadProgram(index_mapvert_tuple, index_mapfrag_tuple)),
      indexRenderBuffer(Resolution::getInstance().width() * IndexMap::FACTOR,
                        Resolution::getInstance().height() * IndexMap::FACTOR),
      indexTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                   Resolution::getInstance().height() * IndexMap::FACTOR,
                   GL_R32UI, GL_RED_INTEGER, GL_UNSIGNED_INT),
      vertConfTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                      Resolution::getInstance().height() * IndexMap::FACTOR,
                      GL_RGBA32F, GL_RGBA, GL_FLOAT),
      colorTimeTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                       Resolution::getInstance().height() * IndexMap::FACTOR,
                       GL_RGBA32F, GL_RGBA, GL_FLOAT),
      normalRadTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                       Resolution::getInstance().height() * IndexMap::FACTOR,
                       GL_RGBA32F, GL_RGBA, GL_FLOAT),
      drawDepthProgram(loadProgram(
          emptyvert_tuple, visualise_texturesfrag_tuple, quadgeom_tuple)),
      drawRenderBuffer(Resolution::getInstance().width(),
                       Resolution::getInstance().height()),
      drawTexture(Resolution::getInstance().width(),
                  Resolution::getInstance().height(), GL_RGBA, GL_RGBA,
                  GL_UNSIGNED_BYTE, false),
      depthProgram(loadProgram(splatvert_tuple, depth_splatfrag_tuple)),
      depthRenderBuffer(Resolution::getInstance().width(),
                        Resolution::getInstance().height()),
      depthTexture(Resolution::getInstance().width(),
                   Resolution::getInstance().height(),
                   GL_R32F, GL_RED, GL_FLOAT, false, true),
      combinedProgram(loadProgram(splatvert_tuple, combo_splatfrag_tuple)),
      combinedRenderBuffer(Resolution::getInstance().width(),
                           Resolution::getInstance().height()),
      imageTexture(Resolution::getInstance().width(),
                   Resolution::getInstance().height(),
                   GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, true),
      vertexTexture(Resolution::getInstance().width(),
                    Resolution::getInstance().height(),
                    GL_RGBA32F, GL_RGBA, GL_FLOAT, false, true),
      normalTexture(Resolution::getInstance().width(),
                    Resolution::getInstance().height(),
                    GL_R32F, GL_RED, GL_FLOAT, false, true),
      timeTexture(Resolution::getInstance().width(),
                  Resolution::getInstance().height(), GL_R16UI, GL_RED_INTEGER,
                  GL_UNSIGNED_SHORT, false, true),
      oldRenderBuffer(Resolution::getInstance().width(),
                      Resolution::getInstance().height()),
      oldImageTexture(Resolution::getInstance().width(),
                      Resolution::getInstance().height(),
                      GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, false, true),
      oldVertexTexture(Resolution::getInstance().width(),
                       Resolution::getInstance().height(),
                       GL_RGBA32F, GL_RGBA, GL_FLOAT, false, true),
      oldNormalTexture(Resolution::getInstance().width(),
                       Resolution::getInstance().height(),
                       GL_RGBA32F, GL_RGBA, GL_FLOAT, false, true),
      oldTimeTexture(Resolution::getInstance().width(),
                     Resolution::getInstance().height(),
                     GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT),
      infoRenderBuffer(Resolution::getInstance().width(),
                       Resolution::getInstance().height()),
      colorInfoTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                       Resolution::getInstance().height() * IndexMap::FACTOR,
                       GL_RGBA32F, GL_RGBA, GL_FLOAT),
      vertexInfoTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                        Resolution::getInstance().height() * IndexMap::FACTOR,
                        GL_RGBA32F, GL_RGBA, GL_FLOAT),
      normalInfoTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                        Resolution::getInstance().height() * IndexMap::FACTOR,
                        GL_RGBA32F, GL_RGBA, GL_FLOAT) {
  indexFrameBuffer.AttachColour(*indexTexture.texture);

  indexFrameBuffer.AttachColour(*vertConfTexture.texture);

  indexFrameBuffer.AttachColour(*colorTimeTexture.texture);

  indexFrameBuffer.AttachColour(*normalRadTexture.texture);

  indexFrameBuffer.AttachDepth(indexRenderBuffer);

  drawFrameBuffer.AttachColour(*drawTexture.texture);

  drawFrameBuffer.AttachDepth(drawRenderBuffer);

  depthFrameBuffer.AttachColour(*depthTexture.texture);

  depthFrameBuffer.AttachDepth(depthRenderBuffer);

  combinedFrameBuffer.AttachColour(*imageTexture.texture);

  combinedFrameBuffer.AttachColour(*vertexTexture.texture);

  combinedFrameBuffer.AttachColour(*normalTexture.texture);

  combinedFrameBuffer.AttachColour(*timeTexture.texture);

  combinedFrameBuffer.AttachDepth(combinedRenderBuffer);

  oldFrameBuffer.AttachDepth(oldRenderBuffer);

  oldFrameBuffer.AttachColour(*oldImageTexture.texture);

  oldFrameBuffer.AttachColour(*oldVertexTexture.texture);

  oldFrameBuffer.AttachColour(*oldNormalTexture.texture);

  oldFrameBuffer.AttachColour(*oldTimeTexture.texture);

  infoFrameBuffer.AttachColour(*colorInfoTexture.texture);

  infoFrameBuffer.AttachColour(*vertexInfoTexture.texture);

  infoFrameBuffer.AttachColour(*normalInfoTexture.texture);

  infoFrameBuffer.AttachDepth(infoRenderBuffer);
}

IndexMap::~IndexMap() {}

void IndexMap::predictIndices(const Eigen::Matrix4f &pose, const int &time,
                              const std::pair<GLuint, GLuint> &model,
                              const float depthCutoff, const int timeDelta) {
  indexFrameBuffer.Bind();

  glViewport(0, 0, indexRenderBuffer.width, indexRenderBuffer.height);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  indexProgram->Bind();

  Eigen::Matrix4f t_inv = pose.inverse();

  Eigen::Vector4f cam(Intrinsics::getInstance().cx() * IndexMap::FACTOR,
                      Intrinsics::getInstance().cy() * IndexMap::FACTOR,
                      Intrinsics::getInstance().fx() * IndexMap::FACTOR,
                      Intrinsics::getInstance().fy() * IndexMap::FACTOR);

  indexProgram->setUniform(Uniform("t_inv", t_inv));
  indexProgram->setUniform(Uniform("cam", cam));
  indexProgram->setUniform(Uniform("maxDepth", depthCutoff));
  indexProgram->setUniform(Uniform(
      "cols", (float)Resolution::getInstance().cols() * IndexMap::FACTOR));
  indexProgram->setUniform(Uniform(
      "rows", (float)Resolution::getInstance().rows() * IndexMap::FACTOR));
  indexProgram->setUniform(Uniform("time", time));
  indexProgram->setUniform(Uniform("timeDelta", timeDelta));

  glBindBuffer(GL_ARRAY_BUFFER, model.first);

  glEnableVertexAttribArray(0);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
                        reinterpret_cast<GLvoid *>(sizeof(Eigen::Vector4f)));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(
      2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
      reinterpret_cast<GLvoid *>(sizeof(Eigen::Vector4f) * 2));

  glDrawTransformFeedback(GL_POINTS, model.second);

  glDisableVertexAttribArray(0);

  glDisableVertexAttribArray(1);

  glDisableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  indexFrameBuffer.Unbind();

  indexProgram->Unbind();

  glFinish();
}

void IndexMap::renderDepth(const float depthCutoff) {

  drawFrameBuffer.Bind();

  glViewport(0, 0, drawRenderBuffer.width, drawRenderBuffer.height);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  drawDepthProgram->Bind();

  drawDepthProgram->setUniform(Uniform("maxDepth", depthCutoff));

  glActiveTexture(GL_TEXTURE0);

  glBindTexture(GL_TEXTURE_2D, vertexTexture.texture->tid);

  drawDepthProgram->setUniform(Uniform("texVerts", 0));

  glDrawArrays(GL_POINTS, 0, 1);

  drawFrameBuffer.Unbind();

  drawDepthProgram->Unbind();

  glBindTexture(GL_TEXTURE_2D, 0);

  glFinish();

}

// splat.vert combo_splat.frag
void IndexMap::combinedPredict(const Eigen::Matrix4f &pose,
                               const std::pair<GLuint, GLuint> &model,
                               const float depthCutoff,
                               const float confThreshold, const int time,
                               const int maxTime, const int timeDelta,
                               IndexMap::Prediction predictionType) {
  if (predictionType == IndexMap::ACTIVE) {
    combinedFrameBuffer.Bind();
  } else if (predictionType == IndexMap::INACTIVE) {
    oldFrameBuffer.Bind();
  } else {
    assert(false);
  }

  if (predictionType == IndexMap::ACTIVE) {
    glViewport(0, 0, combinedRenderBuffer.width, combinedRenderBuffer.height);
  } else if (predictionType == IndexMap::INACTIVE) {
    glViewport(0, 0, oldRenderBuffer.width, oldRenderBuffer.height);
  } else {
    assert(false);
  }

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  combinedProgram->Bind();

  Eigen::Matrix4f t_inv = pose.inverse();
  Eigen::Vector4f cam(
      Intrinsics::getInstance().cx(), Intrinsics::getInstance().cy(),
      Intrinsics::getInstance().fx(), Intrinsics::getInstance().fy());
  combinedProgram->setUniform(Uniform("t_inv", t_inv));
  combinedProgram->setUniform(Uniform("cam", cam));
  combinedProgram->setUniform(Uniform("maxDepth", depthCutoff));
  combinedProgram->setUniform(Uniform("confThreshold", confThreshold));
  combinedProgram->setUniform(
      Uniform("cols", (float)Resolution::getInstance().cols()));
  combinedProgram->setUniform(
      Uniform("rows", (float)Resolution::getInstance().rows()));
  combinedProgram->setUniform(Uniform("time", time));
  combinedProgram->setUniform(Uniform("maxTime", maxTime));
  combinedProgram->setUniform(Uniform("timeDelta", timeDelta));

  glBindBuffer(GL_ARRAY_BUFFER, model.first);

  glEnableVertexAttribArray(0);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(
      1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
      reinterpret_cast<GLvoid *>(sizeof(Eigen::Vector4f) * 1));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(
      2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
      reinterpret_cast<GLvoid *>(sizeof(Eigen::Vector4f) * 2));

  glDrawTransformFeedback(GL_POINTS, model.second);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  if (predictionType == IndexMap::ACTIVE) {
    combinedFrameBuffer.Unbind();
  } else if (predictionType == IndexMap::INACTIVE) {
    oldFrameBuffer.Unbind();
  } else {
    assert(false);
  }

  combinedProgram->Unbind();

  glFinish();
}

// splat.vert depth_splat.frag
void IndexMap::synthesizeDepth(const Eigen::Matrix4f &pose,
                               const std::pair<GLuint, GLuint> &model,
                               const float depthCutoff,
                               const float confThreshold, const int time,
                               const int maxTime, const int timeDelta) {
  depthFrameBuffer.Bind();

  glViewport(0, 0, depthRenderBuffer.width, depthRenderBuffer.height);

  glClearColor(0, 0, 0, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  depthProgram->Bind();

  Eigen::Matrix4f t_inv = pose.inverse();

  Eigen::Vector4f cam(
      Intrinsics::getInstance().cx(), Intrinsics::getInstance().cy(),
      Intrinsics::getInstance().fx(), Intrinsics::getInstance().fy());

  depthProgram->setUniform(Uniform("t_inv", t_inv));
  depthProgram->setUniform(Uniform("cam", cam));
  depthProgram->setUniform(Uniform("maxDepth", depthCutoff));
  depthProgram->setUniform(Uniform("confThreshold", confThreshold));
  depthProgram->setUniform(
      Uniform("cols", (float)Resolution::getInstance().cols()));
  depthProgram->setUniform(
      Uniform("rows", (float)Resolution::getInstance().rows()));
  depthProgram->setUniform(Uniform("time", time));
  depthProgram->setUniform(Uniform("maxTime", maxTime));
  depthProgram->setUniform(Uniform("timeDelta", timeDelta));

  glBindBuffer(GL_ARRAY_BUFFER, model.first);

  glEnableVertexAttribArray(0);

  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

  glEnableVertexAttribArray(1);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
      reinterpret_cast<GLvoid *>(sizeof(Eigen::Vector4f) * 1));

  glEnableVertexAttribArray(2);

  glVertexAttribPointer(
      2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
      reinterpret_cast<GLvoid *>(sizeof(Eigen::Vector4f) * 2));

  glDrawTransformFeedback(GL_POINTS, model.second);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  depthFrameBuffer.Unbind();

  depthProgram->Unbind();

  glFinish();
}
//
// void IndexMap::synthesizeInfo(const Eigen::Matrix4f & pose,
//                               const std::pair<GLuint, GLuint> & model,
//                               const float depthCutoff,
//                               const float confThreshold)
// {
//
//     LOGI("MY elasitcfusion IndexMap::synthesizeInfo  1");
//     glEnable(GL_PROGRAM_POINT_SIZE);
//
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  2");
//     glEnable(GL_POINT_SPRITE);
//
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  3");
//     infoFrameBuffer.Bind();
//
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  4");
//     //
//
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  5");
//     glViewport(0, 0, infoRenderBuffer.width, infoRenderBuffer.height);
//
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  6");
//     glClearColor(0, 0, 0, 0);
//
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  7");
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  8");
//     combinedProgram->Bind();
//
//     Eigen::Matrix4f t_inv = pose.inverse();
//
//     Eigen::Vector4f cam(Intrinsics::getInstance().cx(),
//                   Intrinsics::getInstance().cy(),
//                   Intrinsics::getInstance().fx(),
//                   Intrinsics::getInstance().fy());
//
//     combinedProgram->setUniform(Uniform("t_inv", t_inv));
//     combinedProgram->setUniform(Uniform("cam", cam));
//     combinedProgram->setUniform(Uniform("maxDepth", depthCutoff));
//     combinedProgram->setUniform(Uniform("confThreshold", confThreshold));
//     combinedProgram->setUniform(Uniform("cols",
//     (float)Resolution::getInstance().cols()));
//     combinedProgram->setUniform(Uniform("rows",
//     (float)Resolution::getInstance().rows()));
//     combinedProgram->setUniform(Uniform("time", 0));
//     combinedProgram->setUniform(Uniform("maxTime",
//     std::numeric_limits<int>::max()));
//     combinedProgram->setUniform(Uniform("timeDelta",
//     std::numeric_limits<int>::max()));
//
//     glBindBuffer(GL_ARRAY_BUFFER, model.first);
//
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);
//
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
//     reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
//
//     glEnableVertexAttribArray(2);
//     glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE,
//     reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));
//
//     glDrawTransformFeedback(GL_POINTS, model.second);
//
//     glDisableVertexAttribArray(0);
//     glDisableVertexAttribArray(1);
//     glDisableVertexAttribArray(2);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//     infoFrameBuffer.Unbind();
//
//     combinedProgram->Unbind();
//
//     glDisable(GL_PROGRAM_POINT_SIZE);
//     glDisable(GL_POINT_SPRITE);
//
//     //
//
//     glFinish();
// }
