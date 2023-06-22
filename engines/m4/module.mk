MODULE := engines/m4

MODULE_OBJS = \
	m4.o \
	console.o \
	game.o \
	globals.o \
	kernel.o \
	metaengine.o \
	param.o \
	res.o \
	term.o \
	adv_r/adv_been.o \
	adv_r/adv_rails.o \
	adv_r/db_env.o \
	adv_r/db_rmlst.o \
	core/errors.o \
	core/imath.o \
	core/mouse.o \
	fileio/extensions.o \
	fileio/fileio.o \
	fileio/info.o \
	fileio/sys_file.o \
	graphics/gr_buff.o \
	graphics/gr_font.o \
	graphics/gr_pal.o \
	graphics/graphics.o \
	gui/gui_buffer.o \
	gui/gui_dialog.o \
	gui/gui_mouse.o \
	gui/gui_sys.o \
	gui/gui_vmng.o \
	mem/mem.o \
	mem/memman.o \
	mem/reloc.o \
	platform/timer.o \
	wscript/ws_cruncher.o \
	wscript/ws_hal.o \
	wscript/ws_load.o \
	wscript/ws_machine.o \
	wscript/ws_timer.o \
	wscript/wscript.o \
	burger/burger.o

# This module can be built as a plugin
ifeq ($(ENABLE_M4), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
