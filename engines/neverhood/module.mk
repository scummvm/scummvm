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
	modules/module1000.o \
	modules/module1100.o \
	modules/module1200.o \
	modules/module1300.o \
	modules/module1400.o \
	modules/module1500.o \
	modules/module1600.o \
	modules/module1700.o \
	modules/module1800.o \
	modules/module1900.o \
	modules/module2000.o \
	modules/module2100.o \
	modules/module2200.o \
	modules/module2300.o \
	modules/module2400.o \
	modules/module2500.o \
	modules/module2600.o \
	modules/module2700.o \
	modules/module2800.o \
	modules/module2900.o \
	modules/module3000.o \
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
ifeq ($(ENABLE_NEVERHOOD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
