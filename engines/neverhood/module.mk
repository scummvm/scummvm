MODULE := engines/neverhood

MODULE_OBJS = \
	background.o \
	blbarchive.o \
	detection.o \
	diskplayerscene.o \
	entity.o \
	gamemodule.o \
	gamevars.o \
	graphics.o \
	klaymen.o \
	menumodule.o \
	microtiles.o \
	module.o \
	module1000.o \
	module1100.o \
	module1200.o \
	module1300.o \
	module1400.o \
	module1500.o \
	module1600.o \
	module1700.o \
	module1800.o \
	module1900.o \
	module2000.o \
	module2100.o \
	module2200.o \
	module2300.o \
	module2400.o \
	module2500.o \
	module2600.o \
	module2700.o \
	module2800.o \
	module2900.o \
	module3000.o \
	mouse.o \
	navigationscene.o \
	neverhood.o \
	palette.o \
	resource.o \
	resourceman.o \
	saveload.o \
	scene.o \
	screen.o \
	smackerscene.o \
	smackerplayer.o \
	sound.o \
	sprite.o \
	staticdata.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
