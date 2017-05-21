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

#include "FeedbackBuffer.h"

const std::string FeedbackBuffer::RAW = "RAW";
const std::string FeedbackBuffer::FILTERED = "FILTERED";

static const char *glErrorStringFeedbackBuffer(GLenum err) {
  switch (err) {
  case GL_INVALID_ENUM:
    return "Invalid Enum";
  case GL_INVALID_VALUE:
    return "Invalid Value";
  case GL_INVALID_OPERATION:
    return "Invalid Operation";
  // case GL_STACK_OVERFLOW: return "Stack Overflow";
  // case GL_STACK_UNDERFLOW: return "Stack Underflow";
  case GL_OUT_OF_MEMORY:
    return "error Out of Memory";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
  default:
    return "Unknown Error";
  }
}

inline const char *glCheckFramebufferStatusFeedbackBuffer() {
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status == GL_FRAMEBUFFER_COMPLETE) {
    return "MY elasitcfusion  GL_FRAMEBUFFER_COMPLETE";
  } else if (status == GL_FRAMEBUFFER_UNDEFINED) {
    return "MY elasitcfusion   GL_FRAMEBUFFER_UNDEFINED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
    return "MY elasitcfusion  "
           "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
    return "MY elasitcfusion  "
           "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
  } else if (status == GL_FRAMEBUFFER_UNSUPPORTED) {
    return "MY elasitcfusion   GL_FRAMEBUFFER_UNSUPPORTED";
  } else if (status == GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
    return "MY elasitcfusion   "
           "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
  } else if (status == GL_INVALID_ENUM) {
    return "MY elasitcfusion  GL_INVALID_ENUM";
  } else {
    char integer_string[32];
    int integer = status;
    sprintf(integer_string, "%d", status);
    char other_string[64] = "MY elasitcfusion  else: ";
    strcat(other_string, integer_string);
    return other_string;
  }
}

inline void check_gl_errorFeedbackBuffer() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_error FeedbackBuffer cpp My elastic-fusion CheckGlDieOnError after: %s, "
         "%s() glError (0x%x)\n", glCheckFramebufferStatusFeedbackBuffer(),
         glErrorStringFeedbackBuffer(error), error);
  }
}

FeedbackBuffer::FeedbackBuffer(std::shared_ptr<Shader> program)
    : program(program), drawProgram(loadProgram(draw_feedbackvert_tuple,
                                                draw_feedbackfrag_tuple)),
      bufferSize(Resolution::getInstance().numPixels() * Vertex::SIZE),
      count(0) {
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init start 1 ");
  float *vertices = new float[bufferSize];

  memset(&vertices[0], 0, bufferSize);

  glGenTransformFeedbacks(1, &fid);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 2 ");
  glGenBuffers(1, &vbo);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 3 ");
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 4 ");
  glBufferData(GL_ARRAY_BUFFER, bufferSize, &vertices[0], GL_STREAM_DRAW);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 5 ");
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 6");

  delete[] vertices;

  std::vector<Eigen::Vector2f> uv;

  for (int i = 0; i < Resolution::getInstance().width(); i++) {
    for (int j = 0; j < Resolution::getInstance().height(); j++) {
      uv.push_back(Eigen::Vector2f(
          ((float)i / (float)Resolution::getInstance().width()) +
              1.0 / (2 * (float)Resolution::getInstance().width()),
          ((float)j / (float)Resolution::getInstance().height()) +
              1.0 / (2 * (float)Resolution::getInstance().height())));
    }
  }
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 7 ");
  glGenBuffers(1, &uvo);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 8 ");
  glBindBuffer(GL_ARRAY_BUFFER, uvo);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 9 ");
  glBufferData(GL_ARRAY_BUFFER, uv.size() * sizeof(Eigen::Vector2f), &uv[0],
               GL_STATIC_DRAW);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 10 ");
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 11 ");
  program->Bind();
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 12 ");
  int loc[3] = {
      glGetVaryingLocationNV(program->programId(), "vPosition0"),
      glGetVaryingLocationNV(program->programId(), "vColor0"),
      glGetVaryingLocationNV(program->programId(), "vNormRad0"),
  };
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 13 ");
  GLchar *vars[3] = {"vPosition0", "vColor0", "vNormRad0"};
  glTransformFeedbackVaryings(program->programId(), 3, vars,
                              GL_INTERLEAVED_ATTRIBS);
  // glTransformFeedbackVaryingsNV(program->programId(), 3, loc,
  // INTERLEAVED_ATTRIBS_NV);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 14 ");
  program->Unbind();
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 15 ");
  glGenQueries(1, &countQuery);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct init 16 done ");
}

FeedbackBuffer::~FeedbackBuffer() {
  glDeleteTransformFeedbacks(1, &fid);
  glDeleteBuffers(1, &vbo);
  glDeleteBuffers(1, &uvo);
  glDeleteQueries(1, &countQuery);
}

// void FeedbackBuffer::compute(pangolin::GlTexture * color, pangolin::GlTexture
// * depth,
void FeedbackBuffer::compute(GlTexture *color, GlTexture *depth,
                             const int &time, const float depthCutoff) {
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute start 1 ");
  program->Bind();
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 2");
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
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 3 ");
  glEnableVertexAttribArray(0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 4");
  glBindBuffer(GL_ARRAY_BUFFER, uvo);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 5 ");
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 6");
  glEnable(GL_RASTERIZER_DISCARD);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 7 glBindTransformFeedback fid: %d", fid);
  glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fid);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 8 glBindTransformFeedback fid: %d", fid);
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 9 ");
  glBeginTransformFeedback(GL_POINTS);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 10 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 11 ");
  glBindTexture(GL_TEXTURE_2D, depth->tid);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 12 ");
  glActiveTexture(GL_TEXTURE1);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 13 ");
  glBindTexture(GL_TEXTURE_2D, color->tid);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 14 ");
  glDrawArrays(GL_POINTS, 0, Resolution::getInstance().numPixels());
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 15 ");
  glBindTexture(GL_TEXTURE_2D, 0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 16 ");
  glActiveTexture(GL_TEXTURE0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 17 ");
  glEndTransformFeedback();
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 18 ");
  glDisable(GL_RASTERIZER_DISCARD);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 19 ");
  glDisableVertexAttribArray(0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 19 1");
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 19 2 glBindTransformFeedback");
  glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 20 glBindTransformFeedback");
  program->Unbind();
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute 21 ");
  glFinish();
  check_gl_errorFeedbackBuffer();
  LOGI("MY elasitcfusion FeedbackBuffer struct compute done ");
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
//     drawProgram->setUniform(Uniform("colorType", (drawNormals ? 1 :
//     drawColors ? 2 : 0)));
//
//     glBindBuffer(GL_ARRAY_BUFFER, vbo);
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
//     glDrawTransformFeedback(GL_POINTS, fid);
//
//     glDisableVertexAttribArray(0);
//     glDisableVertexAttribArray(1);
//     glDisableVertexAttribArray(2);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//
//     drawProgram->Unbind();
// }
