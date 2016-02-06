MODULE := engines/titanic

MODULE_OBJS := \
	detection.o \
	font.o \
	image.o \
	main_game_window.o \
	screen_manager.o \
	titanic.o

# This module can be built as a plugin
ifeq ($(ENABLE_TITANIC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
