LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := samp

LOCAL_LDLIBS += -llog

LOCAL_CFLAGS += -w
LOCAL_CFLAGS += -s
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -fvisibility-inlines-hidden
LOCAL_CFLAGS += -fpack-struct=1
LOCAL_CFLAGS += -fexceptions
LOCAL_CFLAGS += -pthread
LOCAL_CFLAGS += -Wall
LOCAL_CFLAGS += -O2
LOCAL_CFLAGS += -DNDEBUG
LOCAL_CFLAGS += -DHOST_ENDIAN_IS_LITTLE
LOCAL_CFLAGS += -DLITTLE_ENDIAN

LOCAL_CFLAGS += -std=c11
LOCAL_CPPFLAGS += -std=c++14

FILE_LIST := $(wildcard $(LOCAL_PATH)/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/game/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/net/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/util/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/game/RW/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/gui/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/vendor/hash/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/vendor/imgui/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/vendor/inih/cpp/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/vendor/inih/*.c)
FILE_LIST += $(wildcard $(LOCAL_PATH)/vendor/POSIXRegex/*.cpp)
FILE_LIST += $(wildcard $(LOCAL_PATH)/vendor/SimpleIni/*.c)
FILE_LIST += $(wildcard $(LOCAL_PATH)/vendor/RakNet/*.cpp)

LOCAL_SRC_FILES := $(FILE_LIST:$(LOCAL_PATH)/%=%)

include $(BUILD_SHARED_LIBRARY)