// #include "stdafx.h"
#include "GLExtensions.h"

#ifdef LOAD__GL_KHR_debug
	PFNGLDEBUGMESSAGECONTROLPROCKHR glDebugMessageControl;
	PFNGLDEBUGMESSAGECALLBACKPROCKHR glDebugMessageCallback;
	PFNGLPUSHDEBUGGROUPPROCKHR glPushDebugGroup;
	PFNGLPOPDEBUGGROUPPROCKHR glPopDebugGroup;
	PFNGLOBJECTLABELPROCKHR glObjectLabel;
#endif

#ifdef LOAD__OES_texture_buffer
	PFNGLTEXBUFFEROESPROC glTexBufferOES;
	PFNGLTEXBUFFERRANGEOESPROC glTexBufferRangOES;
#endif

#ifdef LOAD__EXT_texture_buffer
	PFNGLTEXBUFFEREXTPROC glTexBufferEXT;
	PFNGLTEXBUFFERRANGEEXTPROC glTexBufferRangeEXT;
#endif

#ifdef LOAD__UNKNOWN_COMPUTE_SHADER_EXTENSION
	PFNGLDISPATCHCOMPUTEPROC glDispatchCompute30;
	PFNGLBINDIMAGETEXTUREPROC glBindImageTexture30;
	PFNGLMEMORYBARRIERPROC glMemoryBarrier30;
#endif

#if defined(LOAD__MISSING_API_FUNCTIONS)
	PFNGLFRAMEBUFFERTEXTUREPROC glFramebufferTexture;
	PFNGLGETTEXLEVELPARAMETERIVPROC glGetTexLevelParameteriv30;
#endif

#if defined(LOAD__EXT_disjoint_timer_query)
	PFNGLQUERYCOUNTERPROC glQueryCounter;
	PFNGLGETQUERYOBJECTI64VPROC glGetQueryObjecti64vEXT;
	PFNGLGETQUERYOBJECTUI64VPROC glGetQueryObjectui64vEXT;
#endif

#if defined(LOAD__EXT_texture_view)
	PFNGLTEXTUREVIEWPROC glTextureView;
#endif

PFNGLGETVARYINGLOCATIONNVPROC glGetVaryingLocationNV;

PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC glTransformFeedbackVaryingsNV;

PFNGLDRAWTRANSFORMFEEDBACKNVPROC glDrawTransformFeedback;

PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData;

PFNGLPUSHATTRIBPROC glPushAttrib;

PFNGLPOPATTRIBPROC glPopAttrib;

void* Load(const char* extension, bool& success)
{
	void* ptr = nullptr;
#ifdef ANDROID
	ptr = (void*)eglGetProcAddress(extension);
#endif
// #ifdef WINDOWS
// 	ptr = glfwGetProcAddress(extension);
// #endif

	char buffer[255];
	sprintf(buffer, "Loaded '%s' %p", extension, ptr);
	// Log::Verbose("[GLExtensions]", buffer);
LOGI("GLExtensions %s", buffer);

	success &= true;
	return ptr;
}

bool LoadOpenGLExtensionsManually() {
	bool success = true;

#ifdef LOAD__GL_KHR_debug
	glDebugMessageControl = (PFNGLDEBUGMESSAGECONTROLPROCKHR)Load("glDebugMessageControlKHR", success);
	glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROCKHR)Load("glDebugMessageCallbackKHR", success);
	glPushDebugGroup = (PFNGLPUSHDEBUGGROUPPROCKHR)Load("glPushDebugGroupKHR", success);
	glPopDebugGroup = (PFNGLPOPDEBUGGROUPPROCKHR)Load("glPopDebugGroupKHR", success);
	glObjectLabel = (PFNGLOBJECTLABELPROCKHR)Load("glObjectLabelKHR", success);
#endif

#ifdef LOAD__OES_texture_buffer
	glTexBufferOES = (PFNGLTEXBUFFEROESPROC)Load("glTexBufferOES", success);
	glTexBufferRangeOES = (PFNGLTEXBUFFERRANGEOESPROC)Load("glTexBufferRangeOES", success);
#endif

#ifdef LOAD__EXT_texture_buffer
	glTexBufferEXT = (PFNGLTEXBUFFEREXTPROC)Load("glTexBufferEXT", success);
	glTexBufferRangeEXT = (PFNGLTEXBUFFERRANGEEXTPROC)Load("glTexBufferRangeEXT", success);
#endif

#ifdef LOAD__UNKNOWN_COMPUTE_SHADER_EXTENSION
	glDispatchCompute30 = (PFNGLDISPATCHCOMPUTEPROC)Load("glDispatchCompute", success);
	glBindImageTexture30 = (PFNGLBINDIMAGETEXTUREPROC)Load("glBindImageTexture", success);
	glMemoryBarrier30 = (PFNGLMEMORYBARRIERPROC)Load("glMemoryBarrier", success);
#endif

#if defined(LOAD__MISSING_API_FUNCTIONS)
	glFramebufferTexture = (PFNGLFRAMEBUFFERTEXTUREPROC)Load("glFramebufferTexture", success);
	glGetTexLevelParameteriv30 = (PFNGLGETTEXLEVELPARAMETERIVPROC)Load("glGetTexLevelParameteriv", success);
#endif

#if defined(LOAD__EXT_disjoint_timer_query)
	glQueryCounter = (PFNGLQUERYCOUNTERPROC)Load("glQueryCounter", success);
	glGetQueryObjecti64vEXT = (PFNGLGETQUERYOBJECTI64VPROC)Load("glGetQueryObjecti64vEXT", success);
	glGetQueryObjectui64vEXT = (PFNGLGETQUERYOBJECTUI64VPROC)Load("glGetQueryObjectui64vEXT", success);
#endif

#if defined(LOAD__EXT_texture_view)
	glTextureView = (PFNGLTEXTUREVIEWPROC)Load("glTextureView", success);
#endif

glGetVaryingLocationNV = (PFNGLGETVARYINGLOCATIONNVPROC)Load("glGetVaryingLocationNV", success);
glTransformFeedbackVaryingsNV = (PFNGLTRANSFORMFEEDBACKVARYINGSNVPROC)Load("glTransformFeedbackVaryingsNV", success);
glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKNVPROC)Load("glDrawTransformFeedback", success);
glGetBufferSubData = (PFNGLGETBUFFERSUBDATAPROC)Load("glGetBufferSubData", success);
glPushAttrib = (PFNGLPUSHATTRIBPROC)Load("glPushAttrib", success);
glPopAttrib = (PFNGLPOPATTRIBPROC)Load("glPopAttrib", success);

	return success;
}
