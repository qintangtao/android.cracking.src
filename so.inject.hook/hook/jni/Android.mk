LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := hello
LOCAL_SRC_FILES :=  hello.c
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog $(LOCAL_PATH)/libTKHooklib.so
#LOCAL_SHARED_LIBRARIES := TKHooklib
include $(BUILD_SHARED_LIBRARY)

#include $(CLEAR_VARS)
#LOCAL_MODULE    := TKHooklib
#LOCAL_SRC_FILES :=  libTKHooklib.so
#include $(BUILD_SHARED_LIBRARY)