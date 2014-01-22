LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

APP_ABI := armeabi
LOCAL_MODULE := scummvm
LOCAL_SRC_FILES := ../libscummvm.so

include $(PREBUILT_SHARED_LIBRARY)
