LOCAL_PATH  := $(call my-dir)
ROOT_PATH   := $(LOCAL_PATH)/..
TARGET_NAME := scummvm
HAVE_OPENGLES2 := 1

# Reset flags not reset to Makefile.common
DEFINES   :=

# All current 64-bit archs have 64 in the abi name
ifneq (,$(findstring 64,$(TARGET_ARCH_ABI)))
  TARGET_64BIT := 1
endif

ifneq (,$(findstring arm64,$(TARGET_ARCH_ABI)))
  HAVE_NEON := 1
endif

include $(ROOT_PATH)/Makefile.common

include $(addprefix $(SCUMMVM_PATH)/, $(addsuffix /module.mk,$(MODULES)))

OBJS_MODULES := $(addprefix $(SCUMMVM_PATH)/, $(foreach MODULE,$(MODULES),$(MODULE_OBJS-$(MODULE))))

#TODO:
# -O2 or higher causes segmentation fault with some engines (e.g. hopkins)
# -Fortify triggers abort with some engines (e.g. sword25)
COREFLAGS := $(DEFINES) -DUSE_CXX11 -O1 -U_FORTIFY_SOURCE

ifeq ($(TARGET_ARCH),arm)
  COREFLAGS += -D_ARM_ASSEM_
endif

include $(CLEAR_VARS)
LOCAL_MODULE          := retro
LOCAL_MODULE_FILENAME := libretro
LOCAL_SRC_FILES       := $(DETECT_OBJS:%.o=$(SCUMMVM_PATH)/%.cpp)  $(OBJS_DEPS:%.o=%.c) $(OBJS_MODULES:%.o=%.cpp) $(OBJS:%.o=%.cpp)
LOCAL_C_INCLUDES      := $(subst -I,,$(INCLUDES))
LOCAL_CPPFLAGS        := $(COREFLAGS) -std=c++11
LOCAL_CFLAGS          := $(COREFLAGS)
LOCAL_LDFLAGS         := -Wl,-version-script=$(ROOT_PATH)/link.T
LOCAL_LDLIBS          := -lz -llog
LOCAL_CPP_FEATURES    := rtti
LOCAL_ARM_MODE        := arm
include $(BUILD_SHARED_LIBRARY)
