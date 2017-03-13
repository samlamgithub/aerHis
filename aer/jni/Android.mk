LOCAL_PATH := $(call my-dir)
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

PROJECT_ROOT_FROM_JNI:= ../..
PROJECT_ROOT:= $(call my-dir)/../..
 
include $(CLEAR_VARS)
OPENCV_LIB_TYPE:=STATIC
OPENCV_INSTALL_MODULES:=on
OPENCV_CAMERA_MODULES:=on
include $(OPENCV_MK_PATH)
include /Users/jiahaolin/Downloads/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk 


include $(CLEAR_VARS)
LOCAL_MODULE           := aer

LOCAL_SHARED_LIBRARIES += tango_client_api \
                          tango_support_api \
                          tango_3d_reconstruction \
                          ar_toolkit
                          
LOCAL_STATIC_LIBRARIES += boost_date_time_static \
						  boost_filesystem_static \
						  boost_thread_static \
						   opencv_calib3d opencv_features2d opencv_highgui \
						    opencv_imgproc opencv_flann \
						    opencv_core\
						      IlmImf  libjasper  libjpeg  libpng  libtiff libtbb \
						  png
						                           
LOCAL_CFLAGS           += -std=c++11 -O0 -g -D__ANDROID__

LOCAL_SRC_FILES        += jni_interface.cpp \
                          tango_interface/logger.cpp \
                          tango_interface/mylogger.cpp \
                          tango_interface/gui_interface.cpp \
                          tango_interface/imu_interface.cpp \
                          tango_interface/camera_interface.cpp \
                          aer/aer.cpp \
                          tango_interface/gl_util.cpp 
        
LOCAL_C_INCLUDES += $(PROJECT_ROOT)/third_party/glm/ \
            $(PROJECT_ROOT)/third_party/boost/1.59.0/include \
     /Users/jiahaolin/Downloads/OpenCV-android-sdk/sdk/native/jni/include   \
       $(LOCAL_PATH)/include \
		 $(LOCAL_PATH)
                    
LOCAL_LDLIBS           += -lstdc++ -L$(SYSROOT)/usr/lib -llog -lm -lc -ldl -landroid -lGLESv3 -lz \
               -L/Users/jiahaolin/Downloads/OpenCV-android-sdk/sdk/native/jni/3rdparty/libs/armeabi-v7a \
               -L/Users/jiahaolin/Desktop/mylogger/aer_practicals/projects/third_party/boost/1.59.0/libs/armeabi-v7a/gnu-4.9 -lboost_system -lboost_thread
               
LOCAL_CC	             := /usr/bin/gcc-4.8

include $(BUILD_SHARED_LIBRARY)


################BOOST#####################部分
#include $(CLEAR_VARS)
#BOOST_VERSION      := 1_49
#BOOST_INCLUDE_PATH := $(PROJECT_ROOT)/third_party/Boost/include/boost-1_49
#BOOST_LIB_PATH     := $(PROJECT_ROOT)/third_party/Boost/lib
#LOCAL_C_INCLUDES += $(BOOST_INCLUDE_PATH) 
#LOCAL_LDLIBS    := -llog
# The order of these libraries is often important.
#LOCAL_LDLIBS += -L$(BOOST_LIB_PATH)     
#LOCAL_LDLIBS    +=-lz -llog -landroid -lEGL -lGLESv1_CM
#LOCAL_CPPFLAGS := -std=c++11 -pthread -frtti -fexceptions
#LOCAL_CPPFLAGS += -D__cplusplus=201103L

#LOCAL_EXPORT_C_INCLUDES += $(BOOST_INCLUDE_PATH)
#LOCAL_EXPORT_C_INCLUDES += /Users/jiahaolin/Downloads/OpenCV-android-sdk/sdk/native/jni/include

#include $(BUILD_SHARED_LIBRARY)
###############################################################



include $(CLEAR_VARS)
LOCAL_MODULE           := libIlmImf
LOCAL_SRC_FILES  := 3rdparty/libs/armeabi-v7a/liblibIlmImf.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := libjasper
LOCAL_SRC_FILES  := 3rdparty/libs/armeabi-v7a/liblibjasper.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := libjpeg
LOCAL_SRC_FILES  := 3rdparty/libs/armeabi-v7a/liblibjpeg.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := libpng
LOCAL_SRC_FILES  := 3rdparty/libs/armeabi-v7a/liblibpng.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := libtiff
LOCAL_SRC_FILES  := 3rdparty/libs/armeabi-v7a/liblibtiff.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := libbb
LOCAL_SRC_FILES  := 3rdparty/libs/armeabi-v7a/libtbb.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := opencv_calib3d
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_calib3d.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := opencv_features2d
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_features2d.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := opencv_highgui
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_highgui.a
include $(PREBUILD_STATIC_LIBRARY)
   
include $(CLEAR_VARS)
LOCAL_MODULE           := opencv_imgproc
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_imgproc.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := opencv_flann
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_flann.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE           := opencv_core
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_core.a

include $(PREBUILD_STATIC_LIBRARY)

$(call import-add-path, $(LOCAL_PATH)/../..)
$(call import-add-path, $(PROJECT_ROOT))
$(call import-add-path, $(PROJECT_ROOT)/third_party)
$(call import-module, tango_client_api)
$(call import-module,tango_3d_reconstruction)
$(call import-module, tango_support_api)
$(call import-module, ar_toolkit)