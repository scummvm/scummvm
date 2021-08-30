MODULE := engines/hypno

MODULE_OBJS := \
	cursors.o \
	metaengine.o \
	actions.o \
	scene.o \
	arcade.o \
	lexer_mis.o \
	grammar_mis.o \
	lexer_arc.o \
	grammar_arc.o \
	spider/spider.o \
	spider/talk.o \
	spider/arcade.o \
	wet/wet.o \
	wet/arcade.o \
	boyz/boyz.o \
	hypno.o

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
