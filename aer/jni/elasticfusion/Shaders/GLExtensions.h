#ifndef __GL_EXTENSIONS__
#define __GL_EXTENSIONS__

#include <stdio.h>

#include <GLES3/gl3.h>
#define __gl2_h_
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "../../tango_interface/util.hpp"

#ifndef GL_API_CALL
#ifdef ANDROID
#define GL_API_CALL
#endif
#endif

#ifndef GL_API_CALL_P
#define GL_API_CALL_P GL_API_CALL *
#endif

#ifdef ANDROID
typedef void(GL_API_CALL *GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id,
                                          GLenum severity, GLsizei length,
                                          const GLchar *message,
                                          const void *userParam);
#endif

bool LoadOpenGLExtensionsManually();

typedef void(GL_API_CALL_P PFNGLDRAWTRANSFORMFEEDBACKNVPROC)(GLenum mode,
                                                             GLuint id);
extern PFNGLDRAWTRANSFORMFEEDBACKNVPROC glDrawTransformFeedback;

// EXT_geometry_shader
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_geometry_shader.txt
#ifdef LOAD__EXT_geometry_shader
#define GL_GEOMETRY_SHADER 0x8DD9
#define GL_GEOMETRY_SHADER_BIT 0x00000004
#endif

#endif
