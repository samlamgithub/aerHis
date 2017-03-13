APP_ABI := armeabi-v7a
NDK_TOOLCHAIN_VERSION := 4.9
APP_STL := gnustl_static
APP_PLATFORM := android-19
APP_CPPFLAGS += -std=c++11 -frtti -fexceptions #-g
#APP_LDFLAGS += -g
APP_OPTIM := release