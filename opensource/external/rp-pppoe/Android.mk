LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    src/pppoe.c \
    src/if.c \
    src/debug.c \
    src/common.c \
    src/ppp.c \
    src/discovery.c \

LOCAL_STATIC_LIBRARIES :=
LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)

LOCAL_MODULE_TAGS := eng
LOCAL_MODULE := pppoe
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src

LOCAL_CFLAGS += '-DVERSION="3.10"' '-DPPPD_PATH="/system/bin/pppd"'
include $(BUILD_EXECUTABLE)

