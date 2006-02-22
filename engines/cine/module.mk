MODULE := engines/cine

MODULE_OBJS = \
	anim.o \
	auto00.o \
	bg.o \
	bg_list.o \
	cine.o \
	flip_support.o \
	font.o \
	gfx.o \
	main_loop.o \
	msg.o \
	object.o \
	pal.o \
	part.o \
	prc.o \
	rel.o \
	resource.o \
	script.o \
	sfx_player.o \
	sound_driver.o \
	texte.o \
	unpack.o \
	various.o

MODULE_DIRS += \
	cine

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
