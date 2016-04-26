MODULE := engines/gnap

MODULE_OBJS := \
	datarchive.o \
	debugger.o \
	detection.o \
	gamesys.o \
	gnap.o \
	grid.o \
	menu.o \
	resource.o \
	sound.o \
	scenes/groupcs.o \
	scenes/scene00.o \
	scenes/scene01.o \
	scenes/scene02.o \
	scenes/scene03.o \
	scenes/scene04.o \
	scenes/scene05.o \
	scenes/scene06.o \
	scenes/scene07.o \
	scenes/scene08.o \
	scenes/scene09.o \
	scenes/scene10.o \
	scenes/scene11.o \
	scenes/scene12.o \
	scenes/scene13.o \
	scenes/scene14.o \
	scenes/scene15.o \
	scenes/scene17.o \
	scenes/scene18.o \
	scenes/scene19.o \
	scenes/scene20.o \
	scenes/scene21.o \
	scenes/scene22.o \
	scenes/scene23.o \
	scenes/scene24.o \
	scenes/scene25.o \
	scenes/scene26.o \
	scenes/scene27.o \
	scenes/scene28.o \
	scenes/scene29.o \
	scenes/scene30.o \
	scenes/scene31.o \
	scenes/scene32.o \
	scenes/scene33.o \
	scenes/scene38.o \
	scenes/scene39.o \
	scenes/scene40.o \
	scenes/scene41.o \
	scenes/scene42.o \
	scenes/scene43.o \
	scenes/scene44.o \
	scenes/scene45.o \
	scenes/scene46.o \
	scenes/scene49.o \
	scenes/scene50.o \
	scenes/scene51.o \
	scenes/scene52.o \
	scenes/scene53.o \
	scenes/scenecore.o

# This module can be built as a plugin
ifeq ($(ENABLE_GNAP), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
