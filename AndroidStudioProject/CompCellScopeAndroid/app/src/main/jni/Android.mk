# See file:///home/zkphil/develop/NVPACK/android-ndk-r9d/docs/ANDROID-MK.html
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# CHANGE THIS TO THE DIRECTORY WHICH FPM IS CHECKED OUT TO
FPM_PATH := /Users/zfphil/develop/FPM/

# CHANGE THIS TO CVCOMPLEX CHECKOUT DIRECTORY
CVCOMPLEX_PATH := /Users/zfphil/develop/cvComplex

OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED

# CHANGE THIS TO POINT TO CORRECT LOCATION ON NEW MACHINE
include /Users/zfphil/develop/OpenCV-android-sdk/sdk/native/jni/OpenCV.mk

LOCAL_MODULE    := nativeProcessing
LOCAL_SRC_FILES := native.cpp fpmMain.cpp jsoncpp.cpp cvComplex.cpp
LOCAL_LDLIBS +=  -llog -ldl
LOCAL_C_INCLUDE := opencv2/core/core.hpp opencv2/contrib/contrib.hpp  jsoncpp.h cvComplex.h
include $(BUILD_SHARED_LIBRARY)

# For using FPM and CV_COMPLEX repositotories directly
LOCAL_SRC_FILES_2 := native.cpp $(FPM_PATH)fpmMain.cpp $(FPM_PATH)include/jsoncpp.cpp $(CVCOMPLEX_PATH)cvComplex.cpp
LOCAL_C_INCLUDE_2 := opencv2/core/core.hpp $(FPM_PATH)/include/jsoncpp.h $(CVCOMPLEX_PATH)cvCmomplex.h