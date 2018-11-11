MODULE := engines/gargoyle

MODULE_OBJS := \
	conf.o \
	detection.o \
	events.o \
	fonts.o \
	gargoyle.o \
	glk.o \
	picture.o \
	screen.o \
	selection.o \
	streams.o \
	time.o \
	unicode.o \
	unicode_gen.o \
	utils.o \
	windows.o \
	window_graphics.o \
	window_pair.o \
	window_text_buffer.o \
	window_text_grid.o \
	frotz/detection.o \
	frotz/detection_tables.o \
	frotz/frotz.o \
	scott/detection.o \
	scott/scott.o

# This module can be built as a plugin
ifeq ($(ENABLE_GARGOYLE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
