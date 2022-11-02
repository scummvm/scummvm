MODULE := engines/hypno

MODULE_OBJS := \
	actions.o \
	arcade.o \
	boyz/arcade.o \
	boyz/boyz.o \
	boyz/hard.o \
	boyz/scene.o \
	cursors.o \
	grammar_mis.o \
	grammar_arc.o \
	hypno.o \
	lexer_mis.o \
	lexer_arc.o \
	libfile.o \
	metaengine.o \
	scene.o \
	spider/arcade.o \
	spider/hard.o \
	spider/spider.o \
	spider/talk.o \
	video.o \
	wet/arcade.o \
	wet/hard.o \
	wet/wet.o

MODULE_DIRS += \
	engines/hypno

# HACK: Skip this when including the file for detection objects.
ifeq "$(USE_RULES)" "1"
hypno-grammar:
	flex engines/hypno/lexer_arc.l
	bison engines/hypno/grammar_arc.y
	flex engines/hypno/lexer_mis.l
	bison engines/hypno/grammar_mis.y
endif

# This module can be built as a plugin
ifeq ($(ENABLE_HYPNO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
