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

#include "FeedbackBuffer.h"

const std::string FeedbackBuffer::RAW = "RAW";
const std::string FeedbackBuffer::FILTERED = "FILTERED";

static const char* glErrorStringFeedbackBuffer(GLenum err) {
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

inline void check_gl_errorFeedbackBuffer() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_errorGlobalModel My elastic-fusion CheckGlDieOnError after %s() glError (0x%x)\n", glErrorStringFeedbackBuffer(error), error);
  }
}

FeedbackBuffer::FeedbackBuffer(std::shared_ptr<Shader> program)
 : program(program),
   drawProgram(loadProgram(draw_feedbackvert_tuple, draw_feedbackfrag_tuple)),
   bufferSize(Resolution::getInstance().numPixels() * Vertex::SIZE),
   count(0)
{
check_gl_errorFeedbackBuffer();
    LOGI("MY elasitcfusion FeedbackBuffer struct init 1 ");
    float * vertices = new float[bufferSize];

    memset(&vertices[0], 0, bufferSize);

    glGenTransformFeedbacks(1, &fid);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 1 ");
    glGenBuffers(1, &vbo);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 1 ");
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 1 ");
    glBufferData(GL_ARRAY_BUFFER, bufferSize, &vertices[0], GL_STREAM_DRAW);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 1 ");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 1 ");

    delete [] vertices;

    std::vector<Eigen::Vector2f> uv;

    for(int i = 0; i < Resolution::getInstance().width(); i++)
    {
        for(int j = 0; j < Resolution::getInstance().height(); j++)
        {
            uv.push_back(Eigen::Vector2f(((float)i / (float)Resolution::getInstance().width()) + 1.0 / (2 * (float)Resolution::getInstance().width()),
                                   ((float)j / (float)Resolution::getInstance().height()) + 1.0 / (2 * (float)Resolution::getInstance().height())));
        }
    }
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 1 ");
    glGenBuffers(1, &uvo);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 2 ");
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 3 ");
    glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(Eigen::Vector2f), &uv[0], GL_STATIC_DRAW);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 4 ");
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 5 ");
    program->Bind();
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 6 ");
    int loc[3] =
   {
       glGetVaryingLocationNV(program->programId(), "vPosition0"),
       glGetVaryingLocationNV(program->programId(), "vColor0"),
       glGetVaryingLocationNV(program->programId(), "vNormRad0"),
   };
   check_gl_errorFeedbackBuffer();
       LOGI("MY elasitcfusion FeedbackBuffer struct init 7 ");
GLchar *vars[3] = {"vPosition0", "vColor0",  "vNormRad0"};
   glTransformFeedbackVaryingsNV(program->programId(), 3, vars, GL_INTERLEAVED_ATTRIBS);
  // glTransformFeedbackVaryingsNV(program->programId(), 3, loc, INTERLEAVED_ATTRIBS_NV);
   check_gl_errorFeedbackBuffer();
       LOGI("MY elasitcfusion FeedbackBuffer struct init 8 ");
    program->Unbind();
    check_gl_errorFeedbackBuffer();
        LOGI("MY elasitcfusion FeedbackBuffer struct init 9 ");
    glGenQueries(1, &countQuery);
    check_gl_errorFeedbackBuffer();
LOGI("MY elasitcfusion FeedbackBuffer struct init 10 ");
}

FeedbackBuffer::~FeedbackBuffer()
{
    glDeleteTransformFeedbacks(1, &fid);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &uvo);
    glDeleteQueries(1, &countQuery);
}

// void FeedbackBuffer::compute(pangolin::GlTexture * color, pangolin::GlTexture * depth,
void FeedbackBuffer::compute(GlTexture * color, GlTexture * depth,
                             const int & time,
                             const float depthCutoff)
{
    program->Bind();

    Eigen::Vector4f cam(Intrinsics::getInstance().cx(),
                  Intrinsics::getInstance().cy(),
                  1.0f / Intrinsics::getInstance().fx(),
                  1.0f / Intrinsics::getInstance().fy());

    program->setUniform(Uniform("cam", cam));
    program->setUniform(Uniform("threshold", 0.0f));
    program->setUniform(Uniform("cols", (float)Resolution::getInstance().cols()));
    program->setUniform(Uniform("rows", (float)Resolution::getInstance().rows()));
    program->setUniform(Uniform("time", time));
    program->setUniform(Uniform("gSampler", 0));
    program->setUniform(Uniform("cSampler", 1));
    program->setUniform(Uniform("maxDepth", depthCutoff));

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, uvo);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnable(GL_RASTERIZER_DISCARD);

    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fid);

    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);

    glBeginTransformFeedback(GL_POINTS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth->tid);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, color->tid);

    glDrawArrays(GL_POINTS, 0, Resolution::getInstance().numPixels());

    glBindTexture(GL_TEXTURE_2D, 0);

    glActiveTexture(GL_TEXTURE0);

    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

    program->Unbind();

    glFinish();
}
//
// void FeedbackBuffer::render(pangolin::OpenGlMatrix mvp,
//                             const Eigen::Matrix4f & pose,
//                             const bool drawNormals,
//                             const bool drawColors)
// {
//     drawProgram->Bind();
//
//     drawProgram->setUniform(Uniform("MVP", mvp));
//     drawProgram->setUniform(Uniform("pose", pose));
//     drawProgram->setUniform(Uniform("colorType", (drawNormals ? 1 : drawColors ? 2 : 0)));
//
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
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
//     glDrawTransformFeedback(GL_POINTS, fid);
//
//     glDisableVertexAttribArray(0);
//     glDisableVertexAttribArray(1);
//     glDisableVertexAttribArray(2);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//     drawProgram->Unbind();
// }
