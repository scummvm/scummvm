LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_ABI := armeabi
LOCAL_MODULE := residualvm
LOCAL_SRC_FILES := ../libresidualvm.so

include $(PREBUILT_SHARED_LIBRARY)
