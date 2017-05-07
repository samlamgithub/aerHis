#ifndef __GL_EXTENSIONS__
#define __GL_EXTENSIONS__

#include <GLES3/gl3.h>
#define __gl2_h_                 // what the f***
#include <GLES2/gl2ext.h>
#include <GLES3/gl3platform.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#ifndef GL_API_CALL
	#ifdef ANDROID
		#define GL_API_CALL
	#endif
	// #ifdef WINDOWS
	// 	#define GL_API_CALL __stdcall
	// #endif
#endif

#ifndef GL_API_CALL_P
	#define GL_API_CALL_P GL_API_CALL *
#endif

#ifdef ANDROID
	typedef void (GL_API_CALL *GLDEBUGPROCKHR)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
#endif

bool LoadOpenGLExtensionsManually();

// GL_KHR_debug (not EGL_KHR_debug)
//https://www.khronos.org/registry/egl/extensions/KHR/EGL_KHR_debug.txt
//https://www.opengl.org/registry/specs/KHR/debug.txt
#ifdef LOAD__GL_KHR_debug
	#define GL_DEBUG_OUTPUT                                     0x92E0
	#define GL_DEBUG_OUTPUT_SYNCHRONOUS                         0x8242

	#define GL_DEBUG_SEVERITY_HIGH                              0x9146
	#define GL_DEBUG_SEVERITY_MEDIUM                            0x9147
	#define GL_DEBUG_SEVERITY_LOW                               0x9148
	#define GL_DEBUG_SEVERITY_NOTIFICATION                      0x826B

	#define GL_DEBUG_SOURCE_API                                 0x8246
	#define GL_DEBUG_SOURCE_WINDOW_SYSTEM                       0x8247
	#define GL_DEBUG_SOURCE_SHADER_COMPILER                     0x8248
	#define GL_DEBUG_SOURCE_THIRD_PARTY                         0x8249
	#define GL_DEBUG_SOURCE_APPLICATION                         0x824A
	#define GL_DEBUG_SOURCE_OTHER                               0x824B

	#define GL_DEBUG_TYPE_ERROR                                 0x824C
	#define GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR                   0x824D
	#define GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR                    0x824E
	#define GL_DEBUG_TYPE_PORTABILITY                           0x824F
	#define GL_DEBUG_TYPE_PERFORMANCE                           0x8250
	#define GL_DEBUG_TYPE_OTHER                                 0x8251
	#define GL_DEBUG_TYPE_MARKER                                0x8268


	#ifndef GL_BUFFER
		#define GL_BUFFER                                       0x82E0
	#endif
	#ifndef GL_SHADER
		#define GL_SHADER                                       0x82E1
	#endif
	#ifndef GL_PROGRAM
		#define GL_PROGRAM                                      0x82E2
	#endif
	// GL_VERTEX_ARRAY
	#ifndef QUERY
		#define QUERY                                           0x82E3
	#endif
	#ifndef GL_PROGRAM_PIPELINE
		#define GL_PROGRAM_PIPELINE                             0x82E4
	#endif
	//TRANSFORM_FEEDBACK
	#ifndef GL_SAMPLER
		#define GL_SAMPLER                                      0x82E6
	#endif
	//TEXTURE
	//RENDERBUFFER
	//FRAMEBUFFER



	typedef void (GL_API_CALL_P PFNGLDEBUGMESSAGECONTROLPROCKHR)(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled);
	extern PFNGLDEBUGMESSAGECONTROLPROCKHR glDebugMessageControl;

	typedef void (GL_API_CALL_P PFNGLDEBUGMESSAGECALLBACKPROCKHR)(GLDEBUGPROCKHR callback, const void* userParam);
	extern PFNGLDEBUGMESSAGECALLBACKPROCKHR glDebugMessageCallback;

	typedef void (GL_API_CALL_P PFNGLPUSHDEBUGGROUPPROCKHR)(GLenum source, GLuint id, GLsizei length, const GLchar* message);
	extern PFNGLPUSHDEBUGGROUPPROCKHR glPushDebugGroup;

	typedef void(GL_API_CALL_P PFNGLPOPDEBUGGROUPPROCKHR)(void);
	extern PFNGLPOPDEBUGGROUPPROCKHR glPopDebugGroup;

	typedef void (GL_API_CALL_P PFNGLOBJECTLABELPROCKHR)(GLenum identifier, GLuint name, GLsizei length, const GLchar* label);
	extern PFNGLOBJECTLABELPROCKHR glObjectLabel;

#endif

typedef GLint (GL_API_CALL_P PFNGLGETVARYINGLOCATIONNVPROC) (GLuint program, const GLchar *name);
extern PFNGLGETVARYINGLOCATIONNVPROC glGetVaryingLocationNV;

typedef void (GL_API_CALL_P PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC) (GLuint program, GLsizei count, const GLint *locations, GLenum bufferMode);
extern PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC glTransformFeedbackVaryingsNV;

typedef void (GL_API_CALL_P PFNGLDRAWTRANSFORMFEEDBACKNVPROC) (GLenum mode, GLuint id);
extern PFNGLDRAWTRANSFORMFEEDBACKNVPROC glDrawTransformFeedback;

typedef void (GL_API_CALL_P PFNGLGETBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, void *data);
extern PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;

typedef void (GL_API_CALL_P PFNGLPUSHATTRIBPROC) (GLbitfield mask);

extern PFNGLPUSHATTRIBPROC glPushAttrib;
typedef  void (GL_API_CALL_P PFNGLPOPATTRIBPROC) (void);
extern PFNGLPOPATTRIBPROC glPopAttrib;

#define GL_LUMINANCE32F_ARB  0x8818
#define GL_LUMINANCE32UI_EXT  0x8D74
#define GL_LUMINANCE_INTEGER_EXT  0x8D9C
#define GL_LUMINANCE16UI_EXT  0x8D7A
#define GL_VIEWPORT_BIT  0x00000800
#define GL_POINT_SPRITE  0x8861
#define GL_PROGRAM_POINT_SIZE  0x8642

#if defined(GL_GLEXT_PROTOTYPES)
#	define glDebugMessageControl glDebugMessageControlKHR
#	define glDebugMessageCallback glDebugMessageCallbackKHR
#	define glPushDebugGroup glPushDebugGroupKHR
#	define glPopDebugGroup glPopDebugGroupKHR
#	define glObjectLabel glObjectLabelKHR
#endif
// OES_texture_buffer
// https://www.khronos.org/registry/gles/extensions/OES/OES_texture_buffer.txt
#ifdef LOAD__OES_texture_buffer
	#define GL_TEXTURE_BUFFER_OES							   0x8C2A
	#define GL_TEXTURE_BUFFER_BINDING_OES                      0x8C2A
	#define GL_MAX_TEXTURE_BUFFER_SIZE_OES                     0x8C2B
	#define GL_TEXTURE_BINDING_BUFFER_OES                      0x8C2C
	#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_OES             0x919F
	#define GL_SAMPLER_BUFFER_OES                              0x8DC2
	#define GL_INT_SAMPLER_BUFFER_OES                          0x8DD0
	#define GL_UNSIGNED_INT_SAMPLER_BUFFER_OES                 0x8DD8
	#define GL_IMAGE_BUFFER_OES                                0x9051
	#define GL_INT_IMAGE_BUFFER_OES                            0x905C
	#define GL_UNSIGNED_INT_IMAGE_BUFFER_OES                   0x9067
	#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_OES           0x8C2D
	#define GL_TEXTURE_BUFFER_OFFSET_OES                       0x919D
	#define GL_TEXTURE_BUFFER_SIZE_OES                         0x919E

	typedef void (GL_API_CALL_P PFNGLTEXBUFFEROESPROC)(GLenum target, GLenum internalformat, GLuint buffer);
	extern PFNGLTEXBUFFEROESPROC glTexBufferOES;

	typedef void (GL_API_CALL_P PFNGLTEXBUFFERRANGEOESPROC)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	extern PFNGLTEXBUFFERRANGEOESPROC glTexBufferRangeOES;
#endif

// EXT_texture_buffer
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_buffer.txt
#ifdef LOAD__EXT_texture_buffer
	#define GL_TEXTURE_BUFFER_EXT                              0x8C2A
	#define GL_TEXTURE_BUFFER_BINDING_EXT                      0x8C2A
	#define GL_MAX_TEXTURE_BUFFER_SIZE_EXT                     0x8C2B
	#define GL_TEXTURE_BINDING_BUFFER_EXT                      0x8C2C
	#define GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT_EXT             0x919F
	#define GL_SAMPLER_BUFFER_EXT                              0x8DC2
	#define GL_INT_SAMPLER_BUFFER_EXT                          0x8DD0
	#define GL_UNSIGNED_INT_SAMPLER_BUFFER_EXT                 0x8DD8
	#define GL_IMAGE_BUFFER_EXT                                0x9051
	#define GL_INT_IMAGE_BUFFER_EXT                            0x905C
	#define GL_UNSIGNED_INT_IMAGE_BUFFER_EXT                   0x9067
	#define GL_TEXTURE_BUFFER_DATA_STORE_BINDING_EXT           0x8C2D
	#define GL_TEXTURE_BUFFER_OFFSET_EXT                       0x919D
	#define GL_TEXTURE_BUFFER_SIZE_EXT                         0x919E

	typedef void (GL_API_CALL_P PFNGLTEXBUFFEREXTPROC)(GLenum target, GLenum internalformat, GLuint buffer);
	extern PFNGLTEXBUFFEREXTPROC glTexBufferEXT;

	typedef void (GL_API_CALL_P PFNGLTEXBUFFERRANGEEXTPROC)(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size);
	extern PFNGLTEXBUFFERRANGEEXTPROC glTexBufferRangeEXT;
#endif

// OES_EGL_image_external
// https://www.khronos.org/registry/gles/extensions/OES/OES_EGL_image_external.txt
#ifdef LOAD__OES_EGL_image_external
	#define GL_TEXTURE_EXTERNAL_OES                                0x8D65
	#define GL_SAMPLER_EXTERNAL_OES                                0x8D66
	#define GL_TEXTURE_BINDING_EXTERNAL_OES                        0x8D67
	#define GL_REQUIRED_TEXTURE_IMAGE_UNITS_OES                    0x8D68
#endif

// EXT_geometry_shader
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_geometry_shader.txt
#ifdef LOAD__EXT_geometry_shader
	#define GL_GEOMETRY_SHADER										0x8DD9
	#define GL_GEOMETRY_SHADER_BIT									0x00000004
#endif

#ifdef LOAD__EXT_texture_border_clamp
	#define GL_TEXTURE_BORDER_COLOR								0x1004
	#define GL_CLAMP_TO_BORDER									0x812D
#endif

#ifdef LOAD__UNKNOWN_COMPUTE_SHADER_EXTENSION
	#define GL_MAX_COMPUTE_UNIFORM_BLOCKS                      0x91BB
	#define GL_MAX_COMPUTE_TEXTURE_IMAGE_UNITS                 0x91BC
	#define GL_MAX_COMPUTE_IMAGE_UNIFORMS                      0x91BD
	#define GL_MAX_COMPUTE_SHARED_MEMORY_SIZE                  0x8262
	#define GL_MAX_COMPUTE_UNIFORM_COMPONENTS                  0x8263
	#define GL_MAX_COMPUTE_ATOMIC_COUNTER_BUFFERS              0x8264
	#define GL_MAX_COMPUTE_ATOMIC_COUNTERS                     0x8265
	#define GL_MAX_COMBINED_COMPUTE_UNIFORM_COMPONENTS         0x8266
	#define GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS              0x90EB

	#define GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT					0x00000001
	#define GL_ELEMENT_ARRAY_BARRIER_BIT						0x00000002
	#define GL_UNIFORM_BARRIER_BIT								0x00000004
	#define GL_TEXTURE_FETCH_BARRIER_BIT						0x00000008
	#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT					0x00000020
	#define GL_COMMAND_BARRIER_BIT								0x00000040
	#define GL_PIXEL_BUFFER_BARRIER_BIT							0x00000080
	#define GL_TEXTURE_UPDATE_BARRIER_BIT						0x00000100
	#define GL_BUFFER_UPDATE_BARRIER_BIT						0x00000200
	#define GL_FRAMEBUFFER_BARRIER_BIT							0x00000400
	#define GL_TRANSFORM_FEEDBACK_BARRIER_BIT					0x00000800
	#define GL_ATOMIC_COUNTER_BARRIER_BIT						0x00001000
	#define GL_ALL_BARRIER_BITS									0xFFFFFFFF

	typedef void (GL_API_CALL_P PFNGLDISPATCHCOMPUTEPROC)(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
	extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute30;

	typedef void (GL_API_CALL_P PFNGLBINDIMAGETEXTUREPROC)(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
	extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture30;

	typedef void (GL_API_CALL_P PFNGLMEMORYBARRIERPROC)(GLbitfield barriers);
	extern PFNGLMEMORYBARRIERPROC glMemoryBarrier30;
#endif


// EXT_texture_cube_map_array
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_cube_map_array.txt
#ifdef LOAD__EXT_texture_cube_map_array
	#define GL_TEXTURE_CUBE_MAP_ARRAY								0x9009
#endif

// OpenGL ES 3.2 functions that are not available at compile time
#if defined(LOAD__MISSING_API_FUNCTIONS)
	typedef void (GL_API_CALL_P PFNGLFRAMEBUFFERTEXTUREPROC)(GLenum target, GLenum attachment, GLuint texture, GLint level);
	extern PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;

	typedef void (GL_API_CALL_P PFNGLGETTEXLEVELPARAMETERIVPROC)(GLenum target, GLint level, GLenum pname, GLint* params);
	extern PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv30;

#endif

// EXT_disjoint_timer_query
// https://www.opengl.org/registry/specs/ARB/timer_query.txt
// https://www.opengl.org/registry/specs/EXT/timer_query.txt
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_disjoint_timer_query.txt
#if defined(LOAD__EXT_disjoint_timer_query)
	#define GL_TIME_ELAPSED									0x88BF
	#define GL_TIMESTAMP									0x8E28
	#define GL_GPU_DISJOINT									0x8FBB

	typedef void(GL_API_CALL_P PFNGLQUERYCOUNTERPROC)(GLuint id, GLenum target);
	extern PFNGLQUERYCOUNTERPROC glQueryCounter;

	typedef void (GL_API_CALL_P PFNGLGETQUERYOBJECTI64VPROC)(GLuint id, GLenum target, GLint64 *params);
	extern PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64vEXT;

	typedef void (GL_API_CALL_P PFNGLGETQUERYOBJECTUI64VPROC)(GLuint id, GLenum target, GLuint64 *params);
	extern PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64vEXT;
#endif

// EXT_texture_view
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_view.txt
#if defined(LOAD__EXT_texture_view)

	typedef void(GL_API_CALL_P PFNGLTEXTUREVIEWPROC)(GLuint texture, GLenum target, GLuint origtexture,	GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers);
	extern PFNGLTEXTUREVIEWPROC glTextureView;
#endif

// EXT_tessellation_shader
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_tessellation_shader.txt
#if defined(LOAD__EXT_tessellation_shader)
	#define GL_PATCHES											0xE
	#define GL_TESS_EVALUATION_SHADER							0x8E87
	#define GL_TESS_CONTROL_SHADER								0x8E88
#endif


// EXT_texture_border_clamp
// https://www.khronos.org/registry/gles/extensions/EXT/EXT_texture_border_clamp.txt
#if defined(LOAD__EXT_texture_border_clamp)
	#define GL_TEXTURE_BORDER_COLOR								0x1004
	#define GL_CLAMP_TO_BORDER									0x812D
#endif



#endif
