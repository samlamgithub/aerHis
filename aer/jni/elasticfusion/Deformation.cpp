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

#include "Deformation.h"

static const char *glErrorStringDeformation(GLenum err) {
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
    return "Out of Memory";
  case GL_INVALID_FRAMEBUFFER_OPERATION:
    return "GL_INVALID_FRAMEBUFFER_OPERATION";
  //  case GL_TABLE_TOO_LARGE: return "Table too Large";
  default:
    return "Unknown Error";
  }
}

inline void check_gl_errorDeformation() {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("check_gl_errorGlobalModel My elastic-fusion CheckGlDieOnError after "
         "%s() glError (0x%x)\n",
         glErrorStringDeformation(error), error);
  }
}

Deformation::Deformation()
    : def(4, &pointPool), originalPointPool(0), firstGraphNode(0),
      sampleProgram(loadProgramGeom(samplevert_tuple, samplegeom_tuple)),
      bufferSize(1024), // max nodes basically
      count(0), vertices(new Eigen::Vector4f[bufferSize]),
      graphPosePoints(new std::vector<Eigen::Vector3f>), lastDeformTime(0) {
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 1 ");

  // x, y, z and init time
  memset(&vertices[0], 0, bufferSize);

  glGenTransformFeedbacks(1, &fid);
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 2");
  glGenBuffers(1, &vbo);
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 3 ");
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 4 ");
  glBufferData(GL_ARRAY_BUFFER, bufferSize * sizeof(Eigen::Vector4f),
               &vertices[0], GL_STREAM_DRAW);
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 5 ");
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 6 ");
  sampleProgram->Bind();
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 7 ");
  int loc[1] = {
      glGetVaryingLocationNV(sampleProgram->programId(), "vData"),
  };
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 8 ");
  GLchar *vars[1] = {"vData"};
  glTransformFeedbackVaryings(sampleProgram->programId(), 1, vars,
                              GL_INTERLEAVED_ATTRIBS);
  // glTransformFeedbackVaryingsNV(sampleProgram->programId(), 1, loc,
  // INTERLEAVED_ATTRIBS_NV);
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 9 ");
  sampleProgram->Unbind();
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 10 ");
  glGenQueries(1, &countQuery);
  check_gl_errorDeformation();
  LOGI("MY elasitcfusion Deformation struct init 11 ");
}

Deformation::~Deformation() {
  delete[] vertices;
  glDeleteTransformFeedbacks(1, &fid);
  glDeleteBuffers(1, &vbo);
  glDeleteQueries(1, &countQuery);
  delete graphPosePoints;
}

std::vector<GraphNode *> &Deformation::getGraph() { return def.getGraph(); }

void Deformation::addConstraint(const Constraint &constraint) {
  constraints.push_back(constraint);
}

void Deformation::addConstraint(const Eigen::Vector4f &src,
                                const Eigen::Vector4f &target,
                                const uint64_t &srcTime,
                                const uint64_t &targetTime,
                                const bool pinConstraints) {
  // Add the new constraint
  constraints.push_back(
      Constraint(src.head(3), target.head(3), srcTime, targetTime, false));
  // pin constrain 需要保持 target 点不动，也就是保持 inactive 点不动

  if (pinConstraints) {
    constraints.push_back(Constraint(target.head(3), target.head(3), targetTime,
                                     targetTime, false, true));
  }
}

bool Deformation::constrain(
    std::vector<Ferns::Frame *> &ferns, std::vector<float> &rawGraph, int time,
    const bool fernMatch,
    std::vector<std::pair<unsigned long long int, Eigen::Matrix4f>> &poseGraph,
    const bool relaxGraph, std::vector<Constraint> *newRelativeCons) {
  if (def.isInit()) {
    std::vector<unsigned long long int> times;
    std::vector<Eigen::Matrix4f> poses;
    std::vector<Eigen::Matrix4f *> rawPoses;

    // Deform the set of ferns
    for (size_t i = 0; i < ferns.size(); i++) {
      times.push_back(ferns.at(i)->srcTime);
      poses.push_back(ferns.at(i)->pose);
      rawPoses.push_back(&ferns.at(i)->pose);
    }

    if (fernMatch) {
      // Also apply to the current full pose graph (this might be silly :D)
      for (size_t i = 0; i < poseGraph.size(); i++) {
        times.push_back(poseGraph.at(i).first);
        poses.push_back(poseGraph.at(i).second);
        rawPoses.push_back(&poseGraph.at(i).second);
      }
    }
    // Ferns 和 nodes 建立连接，nodes 中的参数优化后，作用于 Ferns 的 pose

    def.setPosesSeq(&times, poses);
    //上一次添加好的约束点连接图的标签

    int originalPointPool = pointPool.size();

    // Target to source
    // Target to source 给之前建立的 constrain 点重新赋值 ID，constrain 点赋值的
    // ID 是继 pointpool 点的 ID 之后的赋值的，每次 pointpool
    // 中的点数不同，constrain 的 ID 也也不同

    for (size_t i = 0; i < constraints.size(); i++) {
      pointPool.push_back(constraints.at(i).src);
      vertexTimes.push_back(constraints.at(i).srcTime);
      constraints.at(i).srcPointPoolId = pointPool.size() - 1;

      if (constraints.at(i).relative) {
        pointPool.push_back(constraints.at(i).target);
        vertexTimes.push_back(constraints.at(i).targetTime);
        constraints.at(i).tarPointPoolId = pointPool.size() - 1;
      }
    }
    //重建的点，约束点 和 nodes 之间建立连接
    def.appendVertices(&vertexTimes, originalPointPool);
    def.clearConstraints();

    for (size_t i = 0; i < constraints.size(); i++) {
      if (constraints.at(i).relative) {
        def.addRelativeConstraint(constraints.at(i).srcPointPoolId,
                                  constraints.at(i).tarPointPoolId);
      } else {
        Eigen::Vector3f targetPoint = constraints.at(i).target;
        def.addConstraint(constraints.at(i).srcPointPoolId, targetPoint);
      }
    }

    float error = 0;
    float meanConsError = 0;
    bool optimised =
        def.optimiseGraphSparse(error, meanConsError, fernMatch,
                                (fernMatch || relaxGraph) ? 0 : lastDeformTime);

    bool poseUpdated = false;

    if (!fernMatch ||
        (fernMatch && optimised && meanConsError < 0.0003 && error < 0.12)) {
      // 将优化之后的参数 R 和 t 作用于 Fern 的 pose
      def.applyGraphToPoses(rawPoses);

      def.applyGraphToVertices();

      if (!fernMatch && newRelativeCons) {
        newRelativeCons->clear();

        // Target to source
        for (size_t i = 0; i < constraints.size(); i++) {
          if (!constraints.at(i).relative && !constraints.at(i).pin) {
            // Relative constraints (previous loop closures)
            // Relative constrain 在此处产生，localdeform
            // 在非第一次的时候，产生的 constrain 都是 Relative constrain

            newRelativeCons->push_back(
                Constraint(pointPool.at(constraints.at(i).srcPointPoolId),
                           constraints.at(i).target, constraints.at(i).srcTime,
                           constraints.at(i).targetTime, true));
          }
        }
      }

      std::vector<GraphNode *> &graphNodes = def.getGraph();
      std::vector<unsigned long long int> graphTimes = def.getGraphTimes();

      // 16 floats per node...
      rawGraph.resize(graphNodes.size() * 16);

      for (size_t i = 0; i < graphNodes.size(); i++) {
        memcpy(&rawGraph.at(i * 16), graphNodes.at(i)->position.data(),
               sizeof(float) * 3);
        memcpy(&rawGraph.at(i * 16 + 3), graphNodes.at(i)->rotation.data(),
               sizeof(float) * 9);
        memcpy(&rawGraph.at(i * 16 + 12), graphNodes.at(i)->translation.data(),
               sizeof(float) * 3);
        rawGraph.at(i * 16 + 15) = (float)graphTimes.at(i);
      }

      if (!fernMatch && !relaxGraph) {
        lastDeformTime = time;
      }

      poseUpdated = true;
    }

    vertexTimes.resize(originalPointPool);
    pointPool.resize(originalPointPool);

    constraints.clear();

    return poseUpdated;
  }

  return false;
}

void Deformation::sampleGraphFrom(Deformation &other) {
  Eigen::Vector4f *otherVerts = other.getVertices();

  int sampleRate = 5;

  if (other.getCount() / sampleRate > def.k) {
    for (int i = 0; i < other.getCount(); i += sampleRate) {
      Eigen::Vector3f newPoint = otherVerts[i].head<3>();

      graphPosePoints->push_back(newPoint);

      if (i > 0 && otherVerts[i](3) < graphPoseTimes.back()) {
        assert(false && "Assumption failed");
      }

      graphPoseTimes.push_back(otherVerts[i](3));
    }

    def.initialiseGraph(graphPosePoints, &graphPoseTimes);

    graphPoseTimes.clear();
    graphPosePoints->clear();
  }
}

// sample.vert sample.geom
// localDeformation 每 5000 个点采一个点，总共设置了 1024 个 node
void Deformation::sampleGraphModel(const std::pair<GLuint, GLuint> &model) {
  sampleProgram->Bind();

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

  glEnable(GL_RASTERIZER_DISCARD);

  glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, fid);

  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, vbo);

  glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, countQuery);

  glBeginTransformFeedback(GL_POINTS);

  glDrawTransformFeedback(GL_POINTS, model.second);

  glEndTransformFeedback();

  glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

  glGetQueryObjectuiv(countQuery, GL_QUERY_RESULT, &count);

  glDisable(GL_RASTERIZER_DISCARD);

  glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  sampleProgram->Unbind();

  glFinish();
  //采样点数必须大于设置的 neighbours
  if ((int)count > def.k) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glGetBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector4f) * count,
                       vertices);
    // Eigen::Vector4f * ver = (Eigen::Vector4f *
    // )glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(Eigen::Vector4f) * count,
    // GL_MAP_READ_BIT); vertices = ver; glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    for (size_t i = 0; i < count; i++) {
      Eigen::Vector3f newPoint = vertices[i].head<3>();

      graphPosePoints->push_back(newPoint);
      //模型中的点是按照时间顺序添加并且存储的，在采样的时候也是后面采的点的时间要比前面的要完，
      //后面做的 deform 优化必须保持时间先后顺序
      if (i > 0 && vertices[i](3) < graphPoseTimes.back()) {
        assert(false && "Assumption failed");
      }
      //时间存进 vector
      graphPoseTimes.push_back(vertices[i](3));
    }

    def.initialiseGraph(graphPosePoints, &graphPoseTimes);

    graphPoseTimes.clear();
    graphPosePoints->clear();
  }
}
