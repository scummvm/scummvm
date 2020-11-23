MODULE := engines/ags

MODULE_OBJS = \
	ags.o \
	metaengine.o \
	lib/aastr-0.1.1/aarot.o \
	lib/aastr-0.1.1/aastr.o \
	lib/aastr-0.1.1/aautil.o \
	stubs/allegro.o \
	stubs/allegro/color.o \
	stubs/allegro/config.o \
	stubs/allegro/digi.o \
	stubs/allegro/error.o \
	stubs/allegro/file.o \
	stubs/allegro/fixed.o \
	stubs/allegro/gfx.o \
	stubs/allegro/keyboard.o \
	stubs/allegro/midi.o \
	stubs/allegro/mouse.o \
	stubs/allegro/sound.o \
	stubs/allegro/system.o \
	stubs/allegro/unicode.o \
	shared/gfx/allegrobitmap.o \
	shared/gfx/bitmap.o \
	shared/gfx/image.o \
	shared/gui/guibutton.o \
	shared/gui/guiinv.o \
	shared/gui/guilabel.o \
	shared/gui/guilistbox.o \
	shared/gui/guimain.o \
	shared/gui/guiobject.o \
	shared/gui/guislider.o \
	shared/gui/guitextbox.o \
	shared/script/cc_error.o \
	shared/script/cc_options.o \
	shared/script/cc_script.o \
	shared/util/alignedstream.o \
	shared/util/bufferedstream.o \
	shared/util/compress.o \
	shared/util/datastream.o \
	shared/util/directory.o \
	shared/util/file.o \
	shared/util/filestream.o \
	shared/util/geometry.o \
	shared/util/inifile.o \
	shared/util/ini_util.o \
	shared/util/lzw.o \
	shared/util/misc.o \
	shared/util/mutifilelib.o \
	shared/util/path.o \
	shared/util/proxystream.o \
	shared/util/stream.o \
	shared/util/string.o \
	shared/util/string_compat.o \
	shared/util/string_utils.o \
	shared/util/textstreamreader.o \
	shared/util/textstreamwriter.o \
	shared/util/version.o \
	shared/util/wgt2allg.o


# This module can be built as a plugin
ifeq ($(ENABLE_AGS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
