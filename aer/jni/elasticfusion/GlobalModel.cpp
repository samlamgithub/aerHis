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

#include "GlobalModel.h"

//const int GlobalModel::TEXTURE_DIMENSION = 3072;
const int GlobalModel::TEXTURE_DIMENSION = 772;
const int GlobalModel::MAX_VERTICES = GlobalModel::TEXTURE_DIMENSION * GlobalModel::TEXTURE_DIMENSION;
const int GlobalModel::NODE_TEXTURE_DIMENSION = 16384;
const int GlobalModel::MAX_NODES = GlobalModel::NODE_TEXTURE_DIMENSION / 16; //16 floats per node


static const char* glErrorStringGlobalModel(GLenum err) {
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

inline void check_gl_errorGlobalModel() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_errorGlobalModel My elastic-fusion CheckGlDieOnError after %s() glError (0x%x)\n", glErrorStringGlobalModel(error), error);
  }
}

GlobalModel::GlobalModel()
 : target(0),
   renderSource(1),
   bufferSize(MAX_VERTICES * Vertex::SIZE),
   count(0),
   initProgram(loadProgram(init_unstablevert_tuple)),
   drawProgram(loadProgram(draw_feedbackvert_tuple, draw_feedbackfrag_tuple)),
   drawSurfelProgram(loadProgram(draw_global_surfacevert_tuple, draw_global_surfacefrag_tuple, draw_global_surfacegeom_tuple)),
   dataProgram(loadProgram(datavert_tuple, datafrag_tuple, datageom_tuple)),
   updateProgram(loadProgram(updatevert_tuple)),
   unstableProgram(loadProgramGeom(copy_unstablevert_tuple, copy_unstablegeom_tuple)),
   renderBuffer(TEXTURE_DIMENSION, TEXTURE_DIMENSION),
   updateMapVertsConfs(TEXTURE_DIMENSION, TEXTURE_DIMENSION, GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
   updateMapColorsTime(TEXTURE_DIMENSION, TEXTURE_DIMENSION, GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
   updateMapNormsRadii(TEXTURE_DIMENSION, TEXTURE_DIMENSION, GL_RGBA32F, GL_LUMINANCE, GL_FLOAT),
    deformationNodes(NODE_TEXTURE_DIMENSION, 1, GL_LUMINANCE32F_ARB, GL_LUMINANCE, GL_FLOAT)
//   deformationNodes(NODE_TEXTURE_DIMENSION, 1, GL_LUMINANCE32F_EXT, GL_LUMINANCE, GL_FLOAT)
{
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 ");
    vbos = new std::pair<GLuint, GLuint>[2];
    LOGI("MY elasitcfusion GlobalModel struct init 1 1 1 1 : %d", bufferSize);
    float * vertices = new float[bufferSize];
    LOGI("MY elasitcfusion GlobalModel struct init 1 1 1 2");
    memset(&vertices[0], 0, bufferSize);
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 1 1");
    glGenTransformFeedbacks(1, &vbos[0].second);
check_gl_errorGlobalModel();
    LOGI("MY elasitcfusion GlobalModel struct init 1 1 2");
    glGenBuffers(1, &vbos[0].first);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0].first);
check_gl_errorGlobalModel();
    LOGI("MY elasitcfusion GlobalModel struct init 1 1 3");
    glBufferData(GL_ARRAY_BUFFER, bufferSize, &vertices[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 1 ");
    glGenTransformFeedbacks(1, &vbos[1].second);
    glGenBuffers(1, &vbos[1].first);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1].first);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, &vertices[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete [] vertices;
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 2");
    vertices = new float[Resolution::getInstance().numPixels() * Vertex::SIZE];

    memset(&vertices[0], 0, Resolution::getInstance().numPixels() * Vertex::SIZE);

    glGenTransformFeedbacks(1, &newUnstableFid);
    glGenBuffers(1, &newUnstableVbo);
    glBindBuffer(GL_ARRAY_BUFFER, newUnstableVbo);
    glBufferData(GL_ARRAY_BUFFER, Resolution::getInstance().numPixels() * Vertex::SIZE, &vertices[0], GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    delete [] vertices;
check_gl_errorGlobalModel();
    LOGI("MY elasitcfusion GlobalModel struct init 1 3 ");

    std::vector<Eigen::Vector2f> uv;

    for(int i = 0; i < Resolution::getInstance().width(); i++)
    {
        for(int j = 0; j < Resolution::getInstance().height(); j++)
        {
            uv.push_back(Eigen::Vector2f(((float)i / (float)Resolution::getInstance().width()) + 1.0 / (2 * (float)Resolution::getInstance().width()),
                                   ((float)j / (float)Resolution::getInstance().height()) + 1.0 / (2 * (float)Resolution::getInstance().height())));
        }
    }

    uvSize = uv.size();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 4 ");
    glGenBuffers(1, &uvo);
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    glBufferData(GL_ARRAY_BUFFER, uvSize * sizeof(Eigen::Vector2f), &uv[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    frameBuffer.AttachColour(*updateMapVertsConfs.texture);
    frameBuffer.AttachColour(*updateMapColorsTime.texture);
    frameBuffer.AttachColour(*updateMapNormsRadii.texture);
    frameBuffer.AttachDepth(renderBuffer);

check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 5 ");
    updateProgram->Bind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 1 ");
    int locUpdate[3] =
    {
        glGetVaryingLocationNV(updateProgram->programId(), "vPosition0"),
        glGetVaryingLocationNV(updateProgram->programId(), "vColor0"),
        glGetVaryingLocationNV(updateProgram->programId(), "vNormRad0"),
    };
check_gl_errorGlobalModel();
GLchar *vars[3] = {"vPosition0", "vColor0",  "vNormRad0"};
LOGI("MY elasitcfusion GlobalModel struct init check 2: %d, %d, %d: ", locUpdate[0], ,locUpdate[1], locUpdate[2]);
    glTransformFeedbackVaryings(updateProgram->programId(), 3, vars, GL_INTERLEAVED_ATTRIBS);
// glTransformFeedbackVaryingsNV(updateProgram->programId(), 3, locUpdate, INTERLEAVED_ATTRIBS_NV);

check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 3 ");
LOGI("MY elasitcfusion GlobalModel struct init 1 6 ");
    updateProgram->Unbind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 4 ");
    dataProgram->Bind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 5 ");
    int dataUpdate[3] =
    {
        glGetVaryingLocationNV(dataProgram->programId(), "vPosition0"),
        glGetVaryingLocationNV(dataProgram->programId(), "vColor0"),
        glGetVaryingLocationNV(dataProgram->programId(), "vNormRad0"),
    };
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6 ");
    glTransformFeedbackVaryingsNV(dataProgram->programId(), 3, vars, GL_INTERLEAVED_ATTRIBS);
// glTransformFeedbackVaryingsNV(dataProgram->programId(), 3, dataUpdate, INTERLEAVED_ATTRIBS_NV);
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  1");
    dataProgram->Unbind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  2");
    unstableProgram->Bind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  3");
    int unstableUpdate[3] =
    {
        glGetVaryingLocationNV(unstableProgram->programId(), "vPosition0"),
        glGetVaryingLocationNV(unstableProgram->programId(), "vColor0"),
        glGetVaryingLocationNV(unstableProgram->programId(), "vNormRad0"),
    };
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  4");
   glTransformFeedbackVaryingsNV(unstableProgram->programId(), 3, vars, GL_INTERLEAVED_ATTRIBS);
  // glTransformFeedbackVaryingsNV(unstableProgram->programId(), 3, unstableUpdate, INTERLEAVED_ATTRIBS_NV);
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  5");
    unstableProgram->Unbind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  6");
    initProgram->Bind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  7");
    int locInit[3] =
     {
         glGetVaryingLocationNV(initProgram->programId(), "vPosition0"),
         glGetVaryingLocationNV(initProgram->programId(), "vColor0"),
         glGetVaryingLocationNV(initProgram->programId(), "vNormRad0"),
     };
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init check 6  8");
    glTransformFeedbackVaryingsNV(initProgram->programId(), 3, vars, GL_INTERLEAVED_ATTRIBS);
// glTransformFeedbackVaryingsNV(initProgram->programId(), 3, locInit, INTERLEAVED_ATTRIBS_NV);
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 7 ");
    glGenQueries(1, &countQuery);
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 8 ");
    //Empty both transform feedbacks
    glEnable(GL_RASTERIZER_DISCARD);
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 1 9 ");
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, vbos[0].second);
check_gl_errorGlobalModel();
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbos[0].first);
check_gl_errorGlobalModel();
    glBeginTransformFeedback(GL_POINTS);
check_gl_errorGlobalModel();
    glDrawArrays(GL_POINTS, 0, 0);
check_gl_errorGlobalModel();
    glEndTransformFeedback();
check_gl_errorGlobalModel();
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
check_gl_errorGlobalModel();
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, vbos[1].second);
check_gl_errorGlobalModel();
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbos[1].first);
check_gl_errorGlobalModel();
    glBeginTransformFeedback(GL_POINTS);
check_gl_errorGlobalModel();
    glDrawArrays(GL_POINTS, 0, 0);
check_gl_errorGlobalModel();
    glEndTransformFeedback();
check_gl_errorGlobalModel();
    glDisable(GL_RASTERIZER_DISCARD);
check_gl_errorGlobalModel();
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
check_gl_errorGlobalModel();
    initProgram->Unbind();
check_gl_errorGlobalModel();
LOGI("MY elasitcfusion GlobalModel struct init 2");
}

GlobalModel::~GlobalModel()
{
    glDeleteBuffers(1, &vbos[0].first);
    glDeleteTransformFeedbacks(1, &vbos[0].second);

    glDeleteBuffers(1, &vbos[1].first);
    glDeleteTransformFeedbacks(1, &vbos[1].second);

    glDeleteQueries(1, &countQuery);

    glDeleteBuffers(1, &uvo);

    glDeleteTransformFeedbacks(1, &newUnstableFid);
    glDeleteBuffers(1, &newUnstableVbo);

    delete [] vbos;
}


// init_unstable.vert
//初始化模型点，颜色，法向量和半径
void GlobalModel::initialise(const FeedbackBuffer & rawFeedback,
                             const FeedbackBuffer & filteredFeedback)
{
    initProgram->Bind();

    glBindBuffer(GL_ARRAY_BUFFER, rawFeedback.vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));

    glBindBuffer(GL_ARRAY_BUFFER, filteredFeedback.vbo);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glEnable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, vbos[target].second);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbos[target].first);

    glBeginTransformFeedback(GL_POINTS);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);

    //It's ok to use either fid because both raw and filtered have the same amount of vertices
    glDrawTransformFeedback(GL_POINTS, rawFeedback.fid);

    glEndTransformFeedback();

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &count);

    glDisable(GL_RASTERIZER_DISCARD);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    initProgram->Unbind();

    glFinish();
}
//
// void GlobalModel::renderPointCloud(pangolin::OpenGlMatrix mvp,
//                                    const float threshold,
//                                    const bool drawUnstable,
//                                    const bool drawNormals,
//                                    const bool drawColors,
//                                    const bool drawPoints,
//                                    const bool drawWindow,
//                                    const bool drawTimes,
//                                    const int time,
//                                    const int timeDelta)
// {
//     std::shared_ptr<Shader> program = drawPoints ? drawProgram : drawSurfelProgram;
//
//     program->Bind();
//
//     program->setUniform(Uniform("MVP", mvp));
//
//     program->setUniform(Uniform("threshold", threshold));
//
//     program->setUniform(Uniform("colorType", (drawNormals ? 1 : drawColors ? 2 : drawTimes ? 3 : 0)));
//
//     program->setUniform(Uniform("unstable", drawUnstable));
//
//     program->setUniform(Uniform("drawWindow", drawWindow));
//
//     program->setUniform(Uniform("time", time));
//
//     program->setUniform(Uniform("timeDelta", timeDelta));
//
//     Eigen::Matrix4f pose = Eigen::Matrix4f::Identity();
//     //This is for the point shader
//     program->setUniform(Uniform("pose", pose));
//
//     glBindBuffer(GL_ARRAY_BUFFER, vbos[target].first);
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
//     glDrawTransformFeedback(GL_POINTS, vbos[target].second);
//
//     glDisableVertexAttribArray(0);
//     glDisableVertexAttribArray(1);
//     glDisableVertexAttribArray(2);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//     program->Unbind();
// }

const std::pair<GLuint, GLuint> & GlobalModel::model()
{
    return vbos[target];
}

// dataProgram: data.vert data.frag data.geom
// updateProgram: update.vert
// fuse 以前重建的和当前重建的，分两个阶段
// 第一阶段：由模型点根据当前视角可以投影得到 indemMap,vertConfMap,colorTimeMap 等图像，通过和新输入的图像（rgb，depthRaw, depthFiltered）对比，得到需要更新的模型点
// 其中 indexMap 包括每个模型点固定唯一的编号，uvo 前面初始化，为一副图像的像素坐标点，在 GLSL 计算时通过索引每个像素点对比模型投影获取的图像和新输入的图像
// 第一阶段输出是二维的维度为 TEXTURE_DIMENSION 的图像， TEXTURE_DIMENSION 图像的像素坐标点就隐含了模型点的 ID 编号，如果对应的像素坐标处有值，说明对应的模型点需要更新
// 第二阶段：对于全体点做遍历更新，遍历模型点，通过模型点唯一的 ID 编号，可以找到在第一阶段输出的对应点，通过加权融合的方式更新点的坐标
void GlobalModel::fuse(const Eigen::Matrix4f & pose,
                       const int & time,
                       GPUTexture * rgb,
                       GPUTexture * depthRaw,
                       GPUTexture * depthFiltered,
                       GPUTexture * indexMap,
                       GPUTexture * vertConfMap,
                       GPUTexture * colorTimeMap,
                       GPUTexture * normRadMap,
                       const float depthCutoff,
                       const float confThreshold,
                       const float weighting)
{
    TICK("Fuse::Data");
check_gl_errorGlobalModel();
  LOGI("MY elasitcfusion GlobalModel fuse 1");
    //This first part does data association and computes the vertex to merge with, storing
    //in an array that sets which vertices to update by index
    frameBuffer.Bind();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 1");
    glPushAttrib(GL_VIEWPORT_BIT);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 1");
    glViewport(0, 0, renderBuffer.width, renderBuffer.height);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 1");
    glClearColor(0, 0, 0, 0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 1");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 1");
    dataProgram->Bind();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 1");
    dataProgram->setUniform(Uniform("cSampler", 0));
    dataProgram->setUniform(Uniform("drSampler", 1));
    dataProgram->setUniform(Uniform("drfSampler", 2));
    dataProgram->setUniform(Uniform("indexSampler", 3));
    dataProgram->setUniform(Uniform("vertConfSampler", 4));
    dataProgram->setUniform(Uniform("colorTimeSampler", 5));
    dataProgram->setUniform(Uniform("normRadSampler", 6));
    dataProgram->setUniform(Uniform("time", (float)time));
    dataProgram->setUniform(Uniform("weighting", weighting));

    dataProgram->setUniform(Uniform("cam", Eigen::Vector4f(Intrinsics::getInstance().cx(),
                                                     Intrinsics::getInstance().cy(),
                                                     1.0 / Intrinsics::getInstance().fx(),
                                                     1.0 / Intrinsics::getInstance().fy())));
    dataProgram->setUniform(Uniform("cols", (float)Resolution::getInstance().cols()));
    dataProgram->setUniform(Uniform("rows", (float)Resolution::getInstance().rows()));
    dataProgram->setUniform(Uniform("scale", (float)IndexMap::FACTOR));
    dataProgram->setUniform(Uniform("texDim", (float)TEXTURE_DIMENSION));
    dataProgram->setUniform(Uniform("pose", pose));
    dataProgram->setUniform(Uniform("maxDepth", depthCutoff));
    // uvo 前面初始化，为一副图像的像素坐标点，模型点投影可以得到一副图像（包括点的位置，颜色，和点的 ID 等信息），和新捕获的图像进行对比，得到哪些模型点需要更新, uvo 的维度和输入图像的维度相同
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 1");
    glEnableVertexAttribArray(0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 2");
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 3");
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 5");
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, newUnstableFid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 6");
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, newUnstableVbo);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 7");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rgb->texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 8");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, depthRaw->texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 9");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, depthFiltered->texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 10");
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, indexMap->texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 11");
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, vertConfMap->texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 12");
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, colorTimeMap->texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 13");
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, normRadMap->texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 14");
    glBeginTransformFeedback(GL_POINTS);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 15");
    glDrawArrays(GL_POINTS, 0, uvSize);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 16");
    glEndTransformFeedback();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 17");
    frameBuffer.Unbind();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 18");
    glBindTexture(GL_TEXTURE_2D, 0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 19");
    glActiveTexture(GL_TEXTURE0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 20");
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 21");
    dataProgram->Unbind();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 22");
    glPopAttrib();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 23");
    glFinish();
    TOCK("Fuse::Data");
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 24");
    TICK("Fuse::Update");
    //Next we update the vertices at the indexes stored in the update textures
    //Using a transform feedback conditional on a texture sample
    updateProgram->Bind();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 25");
    updateProgram->setUniform(Uniform("vertSamp", 0));
    updateProgram->setUniform(Uniform("colorSamp", 1));
    updateProgram->setUniform(Uniform("normSamp", 2));
    updateProgram->setUniform(Uniform("texDim", (float)TEXTURE_DIMENSION));
    updateProgram->setUniform(Uniform("time", time));
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 26");
    glBindBuffer(GL_ARRAY_BUFFER, vbos[target].first);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 27");
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 28");
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 29");
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 30");
    glEnable(GL_RASTERIZER_DISCARD);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, vbos[renderSource].second);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbos[renderSource].first);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glBeginTransformFeedback(GL_POINTS);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, updateMapVertsConfs.texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, updateMapColorsTime.texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, updateMapNormsRadii.texture->tid);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glDrawTransformFeedback(GL_POINTS, vbos[target].second);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glEndTransformFeedback();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glDisable(GL_RASTERIZER_DISCARD);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    updateProgram->Unbind();
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    std::swap(target, renderSource);
    check_gl_errorGlobalModel();
      LOGI("MY elasitcfusion GlobalModel fuse 31");
    glFinish();
    TOCK("Fuse::Update");
}

//copy_unstable.vert copy_unstable.geom
// 分两个阶段更新，第一个阶段模型所有的点都做更新，更新后 target 和 redener swap，第一阶段更新时实际是对模型点用 deformation graph 更新点的位置
// 第二阶段更新输入是 fuse 阶段的 newUnstableVbo，newUnstableVbo 保存了需要融合的 unstable 点的信息，第二阶段对这些点用 deformation graph 更新点的位置并且融合进模型点中
// isFern 当不是全局的回环时，更新能够投影到相平面的点的时间戳，将投影到相平面的时间戳赋值为 time
void GlobalModel::clean(const Eigen::Matrix4f & pose,
                        const int & time,
                        GPUTexture * indexMap,
                        GPUTexture * vertConfMap,
                        GPUTexture * colorTimeMap,
                        GPUTexture * normRadMap,
                        GPUTexture * depthMap,
                        const float confThreshold,
                        std::vector<float> & graph,
                        const int timeDelta,
                        const float maxDepth,
                        const bool isFern)
{
    assert(graph.size() / 16 < MAX_NODES);

    if(graph.size() > 0)
    {
        //Can be optimised by only uploading new nodes with offset
        glBindTexture(GL_TEXTURE_2D, deformationNodes.texture->tid);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, graph.size(), 1, GL_LUMINANCE, GL_FLOAT, graph.data());
    }

    TICK("Fuse::Copy");
    //Next we copy the new unstable vertices from the newUnstableFid transform feedback into the global map
    unstableProgram->Bind();
    unstableProgram->setUniform(Uniform("time", time));
    unstableProgram->setUniform(Uniform("confThreshold", confThreshold));
    unstableProgram->setUniform(Uniform("scale", (float)IndexMap::FACTOR));
    unstableProgram->setUniform(Uniform("indexSampler", 0));
    unstableProgram->setUniform(Uniform("vertConfSampler", 1));
    unstableProgram->setUniform(Uniform("colorTimeSampler", 2));
    unstableProgram->setUniform(Uniform("normRadSampler", 3));
    unstableProgram->setUniform(Uniform("nodeSampler", 4));
    unstableProgram->setUniform(Uniform("depthSampler", 5));
    unstableProgram->setUniform(Uniform("nodes", (float)(graph.size() / 16)));
    unstableProgram->setUniform(Uniform("nodeCols", (float)NODE_TEXTURE_DIMENSION));
    unstableProgram->setUniform(Uniform("timeDelta", timeDelta));
    unstableProgram->setUniform(Uniform("maxDepth", maxDepth));
    unstableProgram->setUniform(Uniform("isFern", (int)isFern));

    Eigen::Matrix4f t_inv = pose.inverse();
    unstableProgram->setUniform(Uniform("t_inv", t_inv));

    unstableProgram->setUniform(Uniform("cam", Eigen::Vector4f(Intrinsics::getInstance().cx(),
                                                         Intrinsics::getInstance().cy(),
                                                         Intrinsics::getInstance().fx(),
                                                         Intrinsics::getInstance().fy())));
    unstableProgram->setUniform(Uniform("cols", (float)Resolution::getInstance().cols()));
    unstableProgram->setUniform(Uniform("rows", (float)Resolution::getInstance().rows()));

    glBindBuffer(GL_ARRAY_BUFFER, vbos[target].first);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glEnable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, vbos[renderSource].second);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbos[renderSource].first);

    glBeginTransformFeedback(GL_POINTS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, indexMap->texture->tid);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, vertConfMap->texture->tid);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, colorTimeMap->texture->tid);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, normRadMap->texture->tid);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, deformationNodes.texture->tid);

    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, depthMap->texture->tid);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);

    glDrawTransformFeedback(GL_POINTS, vbos[target].second);

    glBindBuffer(GL_ARRAY_BUFFER, newUnstableVbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, 0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f)));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, Vertex::SIZE, reinterpret_cast<GLvoid*>(sizeof(Eigen::Vector4f) * 2));

    glDrawTransformFeedback(GL_POINTS, newUnstableFid);

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &count);

    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    unstableProgram->Unbind();

    std::swap(target, renderSource);

    glFinish();
    TOCK("Fuse::Copy");
}

unsigned int GlobalModel::lastCount()
{
    return count;
}

Eigen::Vector4f * GlobalModel::downloadMap()
{
    glFinish();

    Eigen::Vector4f * vertices = new Eigen::Vector4f[count * 3];

    memset(&vertices[0], 0, count * Vertex::SIZE);

    glBindBuffer(GL_ARRAY_BUFFER, vbos[renderSource].first);

    glGetBufferSubData(GL_ARRAY_BUFFER, 0, count * Vertex::SIZE, vertices);
    // Eigen::Vector4f * ver = glMapBufferRange(GL_ARRAY_BUFFER, 0, count * Vertex::SIZE, GL_MAP_READ_BIT);
    // vertices = ver;
    // glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glFinish();

    return vertices;
}
