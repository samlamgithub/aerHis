#include "GLExtensions.h"

PFNGLDRAWTRANSFORMFEEDBACKNVPROC glDrawTransformFeedback;

void *Load(const char *extension, bool &success) {
  void *ptr = nullptr;
#ifdef ANDROID
  ptr = (void *)eglGetProcAddress(extension);
#endif
  char buffer[255];
  sprintf(buffer, "Loaded '%s' %p", extension, ptr);
  LOGI("GLExtensions %s", buffer);

  success &= true;
  return ptr;
}

bool LoadOpenGLExtensionsManually() {
  bool success = true;

  glDrawTransformFeedback = (PFNGLDRAWTRANSFORMFEEDBACKNVPROC)Load(
      "glDrawTransformFeedback", success);

  return success;
}
