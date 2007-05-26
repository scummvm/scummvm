MODULE := engines/cine

MODULE_OBJS = \
	anim.o \
	bg.o \
	bg_list.o \
	cine.o \
	detection.o \
	gfx.o \
	main_loop.o \
	msg.o \
	object.o \
	pal.o \
	part.o \
	prc.o \
	rel.o \
	script.o \
	sound.o \
	texte.o \
	unpack.o \
	various.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk
