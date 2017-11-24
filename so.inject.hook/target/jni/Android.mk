LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := target
LOCAL_SRC_FILES :=  target.c
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
include $(BUILD_EXECUTABLE)