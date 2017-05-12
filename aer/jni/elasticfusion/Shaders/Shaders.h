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

#ifndef SHADERS_SHADERS_H_
#define SHADERS_SHADERS_H_

// #include <pangolin/gl/glsl.h>
//#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#define __gl2_h_                 // what the f***
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <GLExtensions.h>

#include <ShaderSource.h>
#include <memory>
#include <../../tango_interface/util.hpp>
// #include "../Utils/Parse.h"
#include "Uniform.h"


void check_gl_error(const char* operation) {
  for (GLint error = glGetError(); error; error = glGetError()) {
    LOGI("My elastic-fusion shader after %s() glError (0x%x)\n", operation, error);
  }
}

class Shader //: public pangolin::GlSlProgram
{
    public:
        Shader(): linked(false), prog(0), prev_prog(0)
        {}

        GLuint programId()
        {
            return prog;
        }

        void setUniform(const Uniform & v)
        {
            GLuint loc = glGetUniformLocation(prog, v.id.c_str());

            switch(v.t)
            {
                case Uniform::INT:
                    glUniform1i(loc, v.i);
                    break;
                case Uniform::FLOAT:
                    glUniform1f(loc, v.f);
                    break;
                case Uniform::VEC2:
                    glUniform2f(loc, v.v2(0), v.v2(1));
                    break;
                case Uniform::VEC3:
                    glUniform3f(loc, v.v3(0), v.v3(1), v.v3(2));
                    break;
                case Uniform::VEC4:
                    glUniform4f(loc, v.v4(0), v.v4(1), v.v4(2), v.v4(3));
                    break;
                case Uniform::MAT4:
                    glUniformMatrix4fv(loc, 1, false, v.m4.data());
                    break;
                default:
                    assert(false && "elasitcfusion Uniform type not implemented!");
                    break;
            }
        }

   bool AddShader(GLenum shader_type, const char* source_code) {
    LOGI("MY elasitcfusion Shader AddShader  1 ");
    if(!prog) {
        prog = glCreateProgram();
        if(!prog) {
        LOGI("MY elasitcfusion Shader AddShader  1  glCreateProgram failed");
        }
    }
    // GLhandleARB shader = glCreateShader(shader_type);
    GLuint shader = glCreateShader(shader_type);
    if(!shader) {
    LOGI("MY elasitcfusion Shader AddShader  1  glCreateShader failed");
    return false;
    }
    glShaderSource(shader, 1, &source_code, NULL);
    glCompileShader(shader);
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
      GLint info_len = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
      if (info_len) {
        char* buf = (char*) malloc(info_len);
        if (buf) {
          glGetShaderInfoLog(shader, info_len, NULL, buf);
          LOGE("AddShader GlCameraFrame: Could not compile shader %d:\n%s\n", shader_type, buf);
          free(buf);
        }
        glDeleteShader(shader);
        shader = 0;
      }
      LOGI("MY elasitcfusion Shader AddShader false");
      return false;
    } else {
      glAttachShader(prog, shader);
check_gl_error("glAttachShader");
      shaders.push_back(shader);
      linked = false;
      LOGI("MY elasitcfusion Shader AddShader true");
      return true;
    }
  }

  bool Link() {
      LOGI("MY elasitcfusion Shader Link 1");
      glLinkProgram(prog);
      GLint link_status = GL_FALSE;
      glGetProgramiv(prog, GL_LINK_STATUS, &link_status);
      if (link_status != GL_TRUE) {
        GLint buf_length = 0;
        glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &buf_length);
        if (buf_length) {
          char* buf = (char*) malloc(buf_length);
          if (buf) {
            glGetProgramInfoLog(prog, buf_length, NULL, buf);
            LOGE("MY elasitcfusion Shaders: Could not link program:\n%s\n", buf);
            free(buf);
          }
        }
        glDeleteProgram(prog);
        prog = 0;
LOGI("MY elasitcfusion Shader Link false");
        return false;
    }
LOGI("MY elasitcfusion Shader Link true");
   return true;
}

 void Bind() {
LOGI("MY elasitcfusion Shader Bind");
    prev_prog = 0;
    glUseProgram(prog);
  check_gl_error("MY elasitcfusion Shader Bind:");
 }

 void Unbind() {
LOGI("MY elasitcfusion Shader Unbind");
    glUseProgram(prev_prog);
  check_gl_error("MY elasitcfusion Shader Unbind:");
 }

  protected:
    bool linked;
	  // std::vector<GLhandleARB> shaders;
    std::vector<GLuint> shaders;
    GLenum prog;
    GLint prev_prog;
};

static inline std::shared_ptr<Shader> loadProgramGeom(const char* vertex_shader_file, const char* geometry_shader_file)
{
LOGI("MY elasitcfusion Shader loadProgramGeom");
    std::shared_ptr<Shader> program = std::make_shared<Shader>();

    program->AddShader(GL_VERTEX_SHADER,  vertex_shader_file);
    program->AddShader(GL_GEOMETRY_SHADER,  geometry_shader_file);
    program->Link();

    return program;
}

static inline std::shared_ptr<Shader> loadProgram(const char* vertex_shader_file)
{
LOGI("MY elasitcfusion Shader loadProgram");
    std::shared_ptr<Shader> program = std::make_shared<Shader>();

    program->AddShader(GL_VERTEX_SHADER,  vertex_shader_file);
    program->Link();

    return program;
}

static inline std::shared_ptr<Shader> loadProgram(const char* vertex_shader_file, const char* fragment_shader_file)
{
LOGI("MY elasitcfusion Shader loadProgram 1");
    std::shared_ptr<Shader> program = std::make_shared<Shader>();

    program->AddShader(GL_VERTEX_SHADER,  vertex_shader_file);
    program->AddShader(GL_FRAGMENT_SHADER,  fragment_shader_file);
    program->Link();

    return program;
}

static inline std::shared_ptr<Shader> loadProgram(const char* vertex_shader_file, const char* fragment_shader_file, const char* geometry_shader_file)
{
LOGI("MY elasitcfusion Shader loadProgram 2");
    std::shared_ptr<Shader> program = std::make_shared<Shader>();

    program->AddShader(GL_VERTEX_SHADER,  vertex_shader_file);
    program->AddShader(GL_GEOMETRY_SHADER,  geometry_shader_file);
    program->AddShader(GL_FRAGMENT_SHADER,  fragment_shader_file);
    program->Link();

    return program;
}

#endif /* SHADERS_SHADERS_H_ */
