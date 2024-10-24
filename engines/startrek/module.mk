MODULE := engines/startrek

MODULE_OBJS = \
	actors.o \
	awaymission.o \
	bitmap.o \
	bridge.o \
	common.o \
	console.o \
	events.o \
	font.o \
	graphics.o \
	intro.o \
	iwfile.o \
	lzss.o \
	menu.o \
	resource.o \
	metaengine.o \
	room.o \
	saveload.o \
	sound.o \
	space.o \
	sprite.o \
	startrek.o \
	text.o \
	textbox.o \
	rooms/demon0.o \
	rooms/demon1.o \
	rooms/demon2.o \
	rooms/demon3.o \
	rooms/demon4.o \
	rooms/demon5.o \
	rooms/demon6.o \
	rooms/feather0.o \
	rooms/feather1.o \
	rooms/feather2.o \
	rooms/feather3.o \
	rooms/feather4.o \
	rooms/feather5.o \
	rooms/feather6.o \
	rooms/feather7.o \
	rooms/love0.o \
	rooms/love1.o \
	rooms/love2.o \
	rooms/love3.o \
	rooms/love4.o \
	rooms/love5.o \
	rooms/lovea.o \
	rooms/mudd0.o \
	rooms/mudd1.o \
	rooms/mudd2.o \
	rooms/mudd3.o \
	rooms/mudd4.o \
	rooms/mudd5.o \
	rooms/mudda.o \
	rooms/trial0.o \
	rooms/trial1.o \
	rooms/trial2.o \
	rooms/trial3.o \
	rooms/trial4.o \
	rooms/trial5.o \
	rooms/tug0.o \
	rooms/tug1.o \
	rooms/tug2.o \
	rooms/tug3.o \
	rooms/sins0.o \
	rooms/sins1.o \
	rooms/sins2.o \
	rooms/sins3.o \
	rooms/sins4.o \
	rooms/sins5.o \
	rooms/veng0.o \
	rooms/veng1.o \
	rooms/veng2.o \
	rooms/veng3.o \
	rooms/veng4.o \
	rooms/veng5.o \
	rooms/veng6.o \
	rooms/veng7.o \
	rooms/veng8.o \
	rooms/venga.o



# This module can be built as a plugin
ifeq ($(ENABLE_STARTREK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
