LOCAL_PATH := $(call my-dir)
LOCAL_ALLOW_UNDEFINED_SYMBOLS := true

PROJECT_ROOT_FROM_JNI:= ../..
PROJECT_ROOT:= $(call my-dir)/../..

include $(CLEAR_VARS)
OPENCV_LIB_TYPE:=STATIC
OPENCV_INSTALL_MODULES:=on
OPENCV_CAMERA_MODULES:=off
include $(OPENCV_MK_PATH)
include /home/sam/Downloads/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

include $(CLEAR_VARS)
LOCAL_MODULE := libcudart_static
LOCAL_SRC_FILES  := $(CUDA_TOOLKIT_ROOT)/targets/armv7-linux-androideabi/lib/libcudart_static.a
LOCAL_EXPORT_C_INCLUDES := $(CUDA_TOOLKIT_ROOT)/targets/armv7-linux-androideabi/include
include $(PREBUILT_STATIC_LIBRARY)
#
# include $(CLEAR_VARS)
# LOCAL_MODULE := pangolin
# LOCAL_SRC_FILES := /home/sam/pangolin_android/build/libs/armeabi-v7a/libpangolin.so
# LOCAL_EXPORT_C_INCLUDES := /home/sam/pangolin_android/Pangolin/include
# LOCAL_EXPORT_C_INCLUDES += /home/sam/pangolin_android/build/src/include
# include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libsuitesparse_config
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config/obj/local/armeabi-v7a/libsuitesparse_config.a
LOCAL_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config
LOCAL_EXPORT_C_INCLUDES := thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libblas
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/clapack/BLAS/obj/local/armeabi-v7a/libblas.a
LOCAL_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libf2c
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/clapack/F2CLIBS/obj/local/armeabi-v7a/libf2c.a
LOCAL_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
include $(PREBUILT_STATIC_LIBRARY)

# LOCAL_C_INCLUDES +=  ../../clapack/F2CLIBS/libf2c
# LOCAL_EXPORT_C_INCLUDES +=  ../../clapack/F2CLIBS/libf2c

include $(CLEAR_VARS)
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/clapack/obj/local/armeabi-v7a/libclapack.a
LOCAL_MODULE:= liblapack
LOCAL_STATIC_LIBRARIES += libblas libf2c
LOCAL_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_EXPORT_LDLIBS += $(LOCAL_LDLIBS)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libamd
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/AMD/obj/local/armeabi-v7a/libamd.a
LOCAL_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/AMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/AMD/Include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcamd
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/CAMD/obj/local/armeabi-v7a/libcamd.a
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CAMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CAMD/Include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libccolamd
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/CCOLAMD/obj/local/armeabi-v7a/libccolamd.a
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CCOLAMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CCOLAMD/Include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcolamd
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/COLAMD/obj/local/armeabi-v7a/libcolamd.a
LOCAL_C_INCLUDES  += thirdparty/SuiteSparse_v4.5.5/COLAMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/COLAMD/Include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libcxsparse
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/CXSparse/obj/local/armeabi-v7a/libcxsparse.a
LOCAL_C_INCLUDES  += thirdparty/SuiteSparse_v4.5.5/CXSparse/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CXSparse/Include
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libmetis
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/obj/local/armeabi-v7a/libmetis.a
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/Lib
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/programs
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/GKlib
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/Lib
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/programs
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/GKlib
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/include
include $(PREBUILT_STATIC_LIBRARY)

#LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/cholmod
#LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/AMD

include $(CLEAR_VARS)
LOCAL_MODULE := libcholmod
LOCAL_SRC_FILES := thirdparty/SuiteSparse_v4.5.5/CHOLMOD/obj/local/armeabi-v7a/libcholmod.a
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/cholmod
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/AMD
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/CAMD
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/CCOLAMD
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/COLAMD
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/CXSparse
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/CXSparse
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include/metis
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CHOLMOD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/AMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CAMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CCOLAMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/COLAMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CXSparse/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/Lib
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/GKlib
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CHOLMOD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config
LOCAL_EXPORT_C_INCLUDES +=  thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/AMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CAMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CCOLAMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/COLAMD/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CXSparse/Include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/Lib
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/GKlib
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/include
LOCAL_EXPORT_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CHOLMOD/Include
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_STATIC_LIBRARIES += libsuitesparse_config  liblapack libblas libf2c  libamd libcamd libccolamd libcolamd libcxsparse libmetis
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libjpeg
LOCAL_SRC_FILES := 3rdparty/libs/armeabi-v7a/liblibjpeg.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := aer

LOCAL_SHARED_LIBRARIES += tango_client_api \
                          tango_support_api \
                          tango_3d_reconstruction \


LOCAL_STATIC_LIBRARIES += opencv_calib3d opencv_features2d opencv_highgui \
						    opencv_imgproc opencv_flann \
						    opencv_core tbb IlmImf  libjasper  libjpeg  libpng  libtiff \
						  png \
						  libsuitesparse_config  liblapack libblas libf2c libamd libcamd libccolamd libcolamd libcxsparse libmetis libcholmod \
						  boost_system \
						  boost_thread \
						  boost_filesystem \
						  boost_date_time \
						  boost_chrono  \
						  CudatestLib  \
							CudaLib  \
						  cudart_static


LOCAL_CFLAGS += -std=c++11 -O3 -g -D__ANDROID__ -DANDROID -pthread -DLOAD__EXT_geometry_shader

LOCAL_SRC_FILES += jni_interface.cpp \
                          tango_interface/logger.cpp \
                          tango_interface/mylogger.cpp \
													tango_interface/GroundTruthOdometry.cpp \
													tango_interface/runDataset.cpp \
													tango_interface/runTangoRGBDPoseData.cpp \
                          tango_interface/gui_interface.cpp \
                          tango_interface/myElasticFusion.cpp \
                          tango_interface/imu_interface.cpp \
                          tango_interface/camera_interface.cpp \
                          aer/aer.cpp \
                          tango_interface/gl_util.cpp  \
													elasticfusion/GPUTexture.cpp \
													elasticfusion/Utils/RGBDOdometry.cpp \
													elasticfusion/Utils/DeformationGraph.cpp \
													elasticfusion/Utils/CholeskyDecomp.cpp \
													elasticfusion/Ferns.cpp \
													elasticfusion/Deformation.cpp \
													elasticfusion/IndexMap.cpp \
													elasticfusion/GlobalModel.cpp \
													elasticfusion/ElasticFusion.cpp \
													elasticfusion/Shaders/GLExtensions.cpp \
													elasticfusion/Shaders/FeedbackBuffer.cpp \
													elasticfusion/Shaders/FillIn.cpp \
													elasticfusion/Shaders/ComputePack.cpp \
													elasticfusion/Shaders/Vertex.cpp \
													elasticfusion/Shaders/Resize.cpp

EIGEN_PATH := /usr/local/include/eigen3 /usr/local/include/eigen3/unsupported/

LOCAL_C_INCLUDES += $(PROJECT_ROOT)/third_party/glm/ \
		/home/sam/Downloads/OpenCV-android-sdk/sdk/native/jni/include   \
		thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config \
		thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config \
		thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config/ \
		thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config/ \
		$(PROJECT_ROOT)/boost_1_53_0/include \
		$(LOCAL_PATH)/elasticfusion \
		$(LOCAL_PATH)/elasticfusion/Utils \
		$(LOCAL_PATH)/elasticfusion/Shaders \
       $(LOCAL_PATH)/include \
       $(LOCAL_PATH)/include/cholmod \
		 $(LOCAL_PATH)

LOCAL_C_INCLUDES += $(EIGEN_PATH)
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/clapack/INCLUDE
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/SuiteSparse_config
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/AMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CAMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CCOLAMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/COLAMD/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CXSparse/Include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/Lib
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/GKlib
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/metis-5.1.0/include
LOCAL_C_INCLUDES += thirdparty/SuiteSparse_v4.5.5/CHOLMOD/Include


LOCAL_LDLIBS += -lstdc++ -L$(SYSROOT)/usr/lib -llog -lm -lc -ldl -landroid -lGLESv3 -lz -lEGL  \
-L$(MYDROID)/out/target/product/generic/system/lib/ -ljpeg
-Lthirdparty/SuiteSparse_v4.5.5/SuiteSparse_config/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/lapack/BLAS/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/lapack/F2CLIBS/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/lapack/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/AMD/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/CAMD/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/CCOLAMD/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/COLAMD/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/CXSparse/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/metis-5.1.0/obj/local/armeabi-v7a/ \
-Lthirdparty/SuiteSparse_v4.5.5/CHOLMOD/obj/local/armeabi-v7a/ \
               -L$(PROJECT_ROOT)/boost_1_53_0/armeabi-v7a/lib

LOCAL_CC := /usr/bin/gcc-5

LOCAL_ARM_NEON := true

LOCAL_NEON_CFLAGS := -mfloat-abi=softfp -mfpu=neon -march=armv7

include $(BUILD_SHARED_LIBRARY)

MAINDIR:= $(LOCAL_PATH)
include $(LOCAL_PATH)/cudatest/Android.mk
LOCAL_PATH := $(MAINDIR)
include $(LOCAL_PATH)/elasticfusion/Cuda/Android.mk

include $(CLEAR_VARS)
LOCAL_MODULE := IlmImf
LOCAL_SRC_FILES := 3rdparty/libs/armeabi-v7a/libIlmImf.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libjasper
LOCAL_SRC_FILES := 3rdparty/libs/armeabi-v7a/liblibjasper.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libjpeg
LOCAL_SRC_FILES := 3rdparty/libs/armeabi-v7a/liblibjpeg.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libpng
LOCAL_SRC_FILES := 3rdparty/libs/armeabi-v7a/liblibpng.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libtiff
LOCAL_SRC_FILES  := 3rdparty/libs/armeabi-v7a/liblibtiff.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libbb
LOCAL_SRC_FILES := 3rdparty/libs/armeabi-v7a/liblibtbb.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opencv_calib3d
LOCAL_SRC_FILES := libs/armeabi-v7a/libopencv_calib3d.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opencv_features2d
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_features2d.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opencv_highgui
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_highgui.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opencv_imgproc
LOCAL_SRC_FILES := libs/armeabi-v7a/libopencv_imgproc.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opencv_flann
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_flann.a
include $(PREBUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opencv_core
LOCAL_SRC_FILES  := libs/armeabi-v7a/libopencv_core.a
include $(PREBUILD_STATIC_LIBRARY)

$(call import-add-path, $(PROJECT_ROOT))
$(call import-add-path, $(PROJECT_ROOT)/third_party)
$(call import-add-path, $(LOCAL_PATH)/../..)
$(call import-module, boost_1_53_0)
$(call import-module, tango_client_api)
$(call import-module,tango_3d_reconstruction)
$(call import-module, tango_support_api)
