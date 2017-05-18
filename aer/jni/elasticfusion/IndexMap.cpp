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

#include "IndexMap.h"

const int IndexMap::FACTOR = 1;



static const char* glErrorStringIndexMap(GLenum err) {
  switch(err) {
    case GL_INVALID_ENUM: return "Invalid Enum";
    case GL_INVALID_VALUE: return "Invalid Value";
    case GL_INVALID_OPERATION: return "Invalid Operation";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      return "GL_INVALID_FRAMEBUFFER_OPERATION";
   // case GL_STACK_OVERFLOW: return "Stack Overflow";
   // case GL_STACK_UNDERFLOW: return "Stack Underflow";
    case GL_OUT_OF_MEMORY: return "Out of Memory";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
    default: return "Unknown Error";
  }
}

inline void check_gl_errorIndexMap() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_errorGlobalModel My elastic-fusion CheckGlDieOnError after %s() glError (0x%x)\n", glErrorStringIndexMap(error), error);
  }
}

IndexMap::IndexMap()
: indexProgram(loadProgram(index_mapvert_tuple, index_mapfrag_tuple)),
  indexRenderBuffer(Resolution::getInstance().width() * IndexMap::FACTOR, Resolution::getInstance().height() * IndexMap::FACTOR),
  indexTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
               Resolution::getInstance().height() * IndexMap::FACTOR,
               GL_LUMINANCE32UI_EXT,
               GL_LUMINANCE_INTEGER_EXT,
              // GL_LUMINANCE32F_EXT,
              // GL_LUMINANCE,
               GL_UNSIGNED_INT),
  vertConfTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                  Resolution::getInstance().height() * IndexMap::FACTOR,
                  GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
  colorTimeTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                   Resolution::getInstance().height() * IndexMap::FACTOR,
                   GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
  normalRadTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                   Resolution::getInstance().height() * IndexMap::FACTOR,
                   GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
  drawDepthProgram(loadProgram(emptyvert_tuple, visualise_texturesfrag_tuple, quadgeom_tuple)),
  drawRenderBuffer(Resolution::getInstance().width(), Resolution::getInstance().height()),
  drawTexture(Resolution::getInstance().width(),
              Resolution::getInstance().height(),
              GL_RGBA,
              GL_RGB,
              GL_UNSIGNED_BYTE,
              false),
  depthProgram(loadProgram(splatvert_tuple, depth_splatfrag_tuple)),
  depthRenderBuffer(Resolution::getInstance().width(), Resolution::getInstance().height()),
  depthTexture(Resolution::getInstance().width(),
               Resolution::getInstance().height(),
               GL_LUMINANCE32F_ARB,
              //  GL_LUMINANCE32F_EXT,
               GL_LUMINANCE,
               GL_FLOAT,
               false,
               true),
  combinedProgram(loadProgram(splatvert_tuple, combo_splatfrag_tuple)),
  combinedRenderBuffer(Resolution::getInstance().width(), Resolution::getInstance().height()),
  imageTexture(Resolution::getInstance().width(),
               Resolution::getInstance().height(),
               GL_RGBA,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               false,
               true),
  vertexTexture(Resolution::getInstance().width(),
                Resolution::getInstance().height(),
                GL_RGBA32F, GL_LUMINANCE, GL_FLOAT, false, true),
  normalTexture(Resolution::getInstance().width(),
                Resolution::getInstance().height(),
                GL_RGBA32F, GL_LUMINANCE, GL_FLOAT, false, true),
  timeTexture(Resolution::getInstance().width(),
              Resolution::getInstance().height(),
              GL_LUMINANCE16UI_EXT,
              GL_LUMINANCE_INTEGER_EXT,
              // GL_LUMINANCE16F_EXT,
              // GL_LUMINANCE,
              GL_UNSIGNED_SHORT,
              false,
              true),
  oldRenderBuffer(Resolution::getInstance().width(), Resolution::getInstance().height()),
  oldImageTexture(Resolution::getInstance().width(),
                  Resolution::getInstance().height(),
                  GL_RGBA,
                  GL_RGB,
                  GL_UNSIGNED_BYTE,
                  false,
                  true),
  oldVertexTexture(Resolution::getInstance().width(),
                   Resolution::getInstance().height(),
                   GL_RGBA32F, GL_LUMINANCE, GL_FLOAT, false, true),
  oldNormalTexture(Resolution::getInstance().width(),
                   Resolution::getInstance().height(),
                   GL_RGBA32F, GL_LUMINANCE, GL_FLOAT, false, true),
  oldTimeTexture(Resolution::getInstance().width(),
                 Resolution::getInstance().height(),
                 GL_LUMINANCE16UI_EXT,
                 GL_LUMINANCE_INTEGER_EXT,
                // GL_LUMINANCE16F_EXT,
                // GL_LUMINANCE,
                 GL_UNSIGNED_SHORT,
                 false,
                 true),
  infoRenderBuffer(Resolution::getInstance().width(), Resolution::getInstance().height()),
  colorInfoTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                   Resolution::getInstance().height() * IndexMap::FACTOR,
                   GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
  vertexInfoTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                   Resolution::getInstance().height() * IndexMap::FACTOR,
                   GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
  normalInfoTexture(Resolution::getInstance().width() * IndexMap::FACTOR,
                    Resolution::getInstance().height() * IndexMap::FACTOR,
                    GL_RGBA32F, GL_LUMINANCE, GL_FLOAT)

{
LOGI("MY elasitcfusion IndexMap struct init 1 ");
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
LOGI("MY elasitcfusion IndexMap struct init 2 ");
}

IndexMap::~IndexMap()
{
}

//index_map.vert index_map.frag
// 为 sliding window 做更新，投影出来模型点的位置信息，颜色，时间，法向量，半径和点的 ID
// sliding window 的方式融合新点云，将模型点投影到图像上，根据投影到图像上模型点和新捕获的图像对模型点做更新
// 投影出来的模型点仅限于 active 点（time 到 time - timeDelta），对应也只更新模型的 active 点

void IndexMap::predictIndices(const Eigen::Matrix4f & pose,
                              const int & time,
                              const std::pair<GLuint, GLuint> & model,
                              const float depthCutoff,
                              const int timeDelta)
{
  check_gl_errorIndexMap();
    LOGI("MY elasitcfusion IndexMap::predictIndices  1");
    indexFrameBuffer.Bind();
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::predictIndices 2");
    //glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::predictIndices  3");
    glViewport(0, 0, indexRenderBuffer.width, indexRenderBuffer.height);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::predictIndices  4");
    glClearColor(0, 0, 0, 0);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::predictIndices  5");
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
    indexProgram->setUniform(Uniform("cols", (float)Resolution::getInstance().cols() * IndexMap::FACTOR));
    indexProgram->setUniform(Uniform("rows", (float)Resolution::getInstance().rows() * IndexMap::FACTOR));
    indexProgram->setUniform(Uniform("time", time));
    indexProgram->setUniform(Uniform("timeDelta", timeDelta));

    glBindBuffer(GL_ARRAY_BUFFER, model.first);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glDrawTransformFeedback(GL_POINTS, model.second);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    indexFrameBuffer.Unbind();

    indexProgram->Unbind();

    //glPopAttrib();

    glFinish();
}

void IndexMap::renderDepth(const float depthCutoff)
{
  check_gl_errorIndexMap();
    LOGI("MY elasitcfusion IndexMap::renderDepth  1");
    drawFrameBuffer.Bind();
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::renderDepth  2");
    //glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::renderDepth  3");
    glViewport(0, 0, drawRenderBuffer.width, drawRenderBuffer.height);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::renderDepth  4");
    glClearColor(0, 0, 0, 0);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::renderDepth  5");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::renderDepth  6");
    drawDepthProgram->Bind();
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::renderDepth  7");
    drawDepthProgram->setUniform(Uniform("maxDepth", depthCutoff));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, vertexTexture.texture->tid);

    drawDepthProgram->setUniform(Uniform("texVerts", 0));

    glDrawArrays(GL_POINTS, 0, 1);

    drawFrameBuffer.Unbind();

    drawDepthProgram->Unbind();

    glBindTexture(GL_TEXTURE_2D, 0);

    //glPopAttrib();

    glFinish();
}

// splat.vert combo_splat.frag
//从模型到图像做投影，投影得到的图像用于配准求位姿，combinedFrameBuffer 用于 ACTIVE，oldFrameBuffer 用于 INACTIVE
// time 当前时间戳，maxTime 允许点的最大时间戳，timeDelta 在 time 到 time - timeDelta 内的点投影到图像上
// 投影出来的图像是 active（time 到　time - timeDelta）的点，所以下一帧图像配准也是和只是和 active 点配准
// 当重定位时 time = 0,不限制 active，所有点都向图像做投影
void IndexMap::combinedPredict(const Eigen::Matrix4f & pose,
                               const std::pair<GLuint, GLuint> & model,
                               const float depthCutoff,
                               const float confThreshold,
                               const int time,
                               const int maxTime,
                               const int timeDelta,
                               IndexMap::Prediction predictionType)
{
  check_gl_errorIndexMap();
    LOGI("MY elasitcfusion IndexMap::combinedPredict  1");
    glEnable(GL_PROGRAM_POINT_SIZE);  //here
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict  2");
    glEnable(GL_POINT_SPRITE);  //here
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict  3");
    if(predictionType == IndexMap::ACTIVE)
    {
        combinedFrameBuffer.Bind();
    }
    else if(predictionType == IndexMap::INACTIVE)
    {
        oldFrameBuffer.Bind();
    }
    else
    {
        assert(false);
    }
check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 4");

    //glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 5");
    if(predictionType == IndexMap::ACTIVE)
    {
        glViewport(0, 0, combinedRenderBuffer.width, combinedRenderBuffer.height);
    }
    else if(predictionType == IndexMap::INACTIVE)
    {
        glViewport(0, 0, oldRenderBuffer.width, oldRenderBuffer.height);
    }
    else
    {
        assert(false);
    }
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 6");
    glClearColor(0, 0, 0, 0);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 7");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 8");
    combinedProgram->Bind(); //here
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 9");
    Eigen::Matrix4f t_inv = pose.inverse();

    Eigen::Vector4f cam(Intrinsics::getInstance().cx(),
                  Intrinsics::getInstance().cy(),
                  Intrinsics::getInstance().fx(),
                  Intrinsics::getInstance().fy());

    combinedProgram->setUniform(Uniform("t_inv", t_inv));
    combinedProgram->setUniform(Uniform("cam", cam));
    combinedProgram->setUniform(Uniform("maxDepth", depthCutoff));
    combinedProgram->setUniform(Uniform("confThreshold", confThreshold));
    combinedProgram->setUniform(Uniform("cols", (float)Resolution::getInstance().cols()));
    combinedProgram->setUniform(Uniform("rows", (float)Resolution::getInstance().rows()));
    combinedProgram->setUniform(Uniform("time", time));
    combinedProgram->setUniform(Uniform("maxTime", maxTime));
    combinedProgram->setUniform(Uniform("timeDelta", timeDelta));
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 10");
    glBindBuffer(GL_ARRAY_BUFFER, model.first);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 11");
    glEnableVertexAttribArray(0);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 12");
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 13");
    glEnableVertexAttribArray(1);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 14");
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 15");
    glEnableVertexAttribArray(2);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 16");
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 17");
    glDrawTransformFeedback(GL_POINTS, model.second);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 18");
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 19");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 20");
    if(predictionType == IndexMap::ACTIVE)
    {
        combinedFrameBuffer.Unbind();
    }
    else if(predictionType == IndexMap::INACTIVE)
    {
        oldFrameBuffer.Unbind();
    }
    else
    {
        assert(false);
    }
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 21");
    combinedProgram->Unbind();
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 22");
    glDisable(GL_PROGRAM_POINT_SIZE);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 23");
    glDisable(GL_POINT_SPRITE);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 24");
    //glPopAttrib();

    glFinish();
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::combinedPredict 25");
}

// splat.vert depth_splat.frag
// 合成深度图用于显示
void IndexMap::synthesizeDepth(const Eigen::Matrix4f & pose,
                               const std::pair<GLuint, GLuint> & model,
                               const float depthCutoff,
                               const float confThreshold,
                               const int time,
                               const int maxTime,
                               const int timeDelta)
{
  check_gl_errorIndexMap();
    LOGI("MY elasitcfusion IndexMap::synthesizeDepth  1");
    glEnable(GL_PROGRAM_POINT_SIZE);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth  2");
    glEnable(GL_POINT_SPRITE);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth  3");
    depthFrameBuffer.Bind();
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth  4");
    //glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth  5");
    glViewport(0, 0, depthRenderBuffer.width, depthRenderBuffer.height);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth  6");
    glClearColor(0, 0, 0, 0);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth  7");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth 8");
    depthProgram->Bind();
    check_gl_errorIndexMap();
      LOGI("MY elasitcfusion IndexMap::synthesizeDepth  9");
    Eigen::Matrix4f t_inv = pose.inverse();

    Eigen::Vector4f cam(Intrinsics::getInstance().cx(),
                  Intrinsics::getInstance().cy(),
                  Intrinsics::getInstance().fx(),
                  Intrinsics::getInstance().fy());

    depthProgram->setUniform(Uniform("t_inv", t_inv));
    depthProgram->setUniform(Uniform("cam", cam));
    depthProgram->setUniform(Uniform("maxDepth", depthCutoff));
    depthProgram->setUniform(Uniform("confThreshold", confThreshold));
    depthProgram->setUniform(Uniform("cols", (float)Resolution::getInstance().cols()));
    depthProgram->setUniform(Uniform("rows", (float)Resolution::getInstance().rows()));
    depthProgram->setUniform(Uniform("time", time));
    depthProgram->setUniform(Uniform("maxTime", maxTime));
    depthProgram->setUniform(Uniform("timeDelta", timeDelta));

    glBindBuffer(GL_ARRAY_BUFFER, model.first);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glDrawTransformFeedback(GL_POINTS, model.second);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    depthFrameBuffer.Unbind();

    depthProgram->Unbind();

    glDisable(GL_PROGRAM_POINT_SIZE);
    glDisable(GL_POINT_SPRITE);

    //glPopAttrib();

    glFinish();
}
//
// void IndexMap::synthesizeInfo(const Eigen::Matrix4f & pose,
//                               const std::pair<GLuint, GLuint> & model,
//                               const float depthCutoff,
//                               const float confThreshold)
// {
//   check_gl_errorIndexMap();
//     LOGI("MY elasitcfusion IndexMap::synthesizeInfo  1");
//     glEnable(GL_PROGRAM_POINT_SIZE);
//     check_gl_errorIndexMap();
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  2");
//     glEnable(GL_POINT_SPRITE);
//     check_gl_errorIndexMap();
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  3");
//     infoFrameBuffer.Bind();
//     check_gl_errorIndexMap();
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  4");
//     //glPushAttrib(GL_VIEWPORT_BIT);
//     check_gl_errorIndexMap();
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  5");
//     glViewport(0, 0, infoRenderBuffer.width, infoRenderBuffer.height);
//     check_gl_errorIndexMap();
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  6");
//     glClearColor(0, 0, 0, 0);
//     check_gl_errorIndexMap();
//       LOGI("MY elasitcfusion IndexMap::synthesizeInfo  7");
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//     check_gl_errorIndexMap();
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
//     combinedProgram->setUniform(Uniform("cols", (float)Resolution::getInstance().cols()));
//     combinedProgram->setUniform(Uniform("rows", (float)Resolution::getInstance().rows()));
//     combinedProgram->setUniform(Uniform("time", 0));
//     combinedProgram->setUniform(Uniform("maxTime", std::numeric_limits<int>::max()));
//     combinedProgram->setUniform(Uniform("timeDelta", std::numeric_limits<int>::max()));
//
//     glBindBuffer(GL_ARRAY_BUFFER, model.first);
//
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);
//
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 1));
//
//     glEnableVertexAttribArray(2);
//     glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));
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
//     //glPopAttrib();
//
//     glFinish();
// }
