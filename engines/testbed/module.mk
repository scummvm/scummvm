MODULE := engines/testbed

MODULE_OBJS := \
	config.o \
	config-params.o \
	events.o \
	fs.o \
	graphics.o \
	metaengine.o \
	midi.o \
	misc.o \
	networking.o \
	savegame.o \
	sound.o \
	testbed.o \
	testsuite.o \
	video.o

ifdef USE_CLOUD
ifdef USE_LIBCURL
MODULE_OBJS += \
	cloud.o
endif
endif

ifdef USE_SDL_NET
MODULE_OBJS += \
	webserver.o
endif

ifdef USE_TTS
MODULE_OBJS += \
	speech.o
endif

ifdef USE_IMGUI
MODULE_OBJS += \
	imgui.o
endif

MODULE_DIRS += \
	engines/testbed

# This module can be built as a plugin
ifeq ($(ENABLE_TESTBED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
