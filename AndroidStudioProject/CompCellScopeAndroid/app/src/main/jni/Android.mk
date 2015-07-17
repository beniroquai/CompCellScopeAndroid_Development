# See file:///home/zkphil/develop/NVPACK/android-ndk-r9d/docs/ANDROID-MK.html
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#include $(OPENCV_ROOT)/sdk/native/jni/OpenCV.mk
OPENCV_INSTALL_MODULES:=on
include /Users/joelwhang/AndroidStudioProjects/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk
LOCAL_MODULE    := nativeProcessing
LOCAL_SRC_FILES := native.cpp
LOCAL_LDLIBS +=  -llog -ldl
LOCAL_C_INCLUDE := opencv2/core/core.hpp
include $(BUILD_SHARED_LIBRARY)