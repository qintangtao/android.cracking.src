LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := inject
LOCAL_SRC_FILES :=  inject.c ptrace_utils.c inject_utils.c
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog
include $(BUILD_EXECUTABLE)