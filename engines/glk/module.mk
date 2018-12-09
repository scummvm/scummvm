MODULE := engines/glk

MODULE_OBJS := \
	blorb.o \
	conf.o \
	detection.o \
	events.o \
	glk.o \
	glk_api.o \
	picture.o \
	raw_decoder.o \
	screen.o \
	selection.o \
	sound.o \
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
	frotz/config.o \
	frotz/detection.o \
	frotz/frotz.o \
	frotz/glk_interface.o \
	frotz/mem.o \
	frotz/pics.o \
	frotz/pics_decoder.o \
	frotz/processor.o \
	frotz/processor_buffer.o \
	frotz/processor_input.o \
	frotz/processor_maths.o \
	frotz/processor_mem.o \
	frotz/processor_objects.o \
	frotz/processor_screen.o \
	frotz/processor_streams.o \
	frotz/processor_table.o \
	frotz/processor_text.o \
	frotz/processor_variables.o \
	frotz/processor_windows.o \
	frotz/quetzal.o \
	frotz/screen.o \
	frotz/sound_folder.o \
	glulxe/detection.o \
	glulxe/glulxe.o \
	scott/detection.o \
	scott/scott.o \
	tads/detection.o \
	tads/tads.o \
	tads/tads2/data.o \
	tads/tads2/ler.o \
	tads/tads2/os.o \
	tads/tads2/regex.o \
	tads/tads2/tads2.o \
	tads/tads2/tads2_cmap.o \
	tads/tads2/vocabulary.o \
	tads/tads3/tads3.o

# This module can be built as a plugin
ifeq ($(ENABLE_GLK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
