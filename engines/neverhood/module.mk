MODULE := engines/neverhood

MODULE_OBJS = \
	background.o \
	blbarchive.o \
	collisionman.o \
	detection.o \
	gamemodule.o \
	graphics.o \
	klayman.o \
	module.o \
	module1000.o \
	module1500.o \
	neverhood.o \
	palette.o \
	resource.o \
	resourceman.o \
	scene.o \
	screen.o \
	smackerscene.o \
	smackerplayer.o \
	sprite.o \
	staticdata.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
