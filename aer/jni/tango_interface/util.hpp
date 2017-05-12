#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <android/log.h>
// This is for logging to a file via the class 'Logger'
//#include "logger.hpp"

#define LOG_TAG "JNI Log"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG,__VA_ARGS__)

#endif // __UTIL_HPP__
