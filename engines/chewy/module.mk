MODULE := engines/chewy

MODULE_OBJS = \
	ailclass.o \
	ani_dat.o \
	atds.o \
	ch_icm.o \
	chewy.o \
	cursor.o \
	datei.o \
	debug.o \
	debugger.o \
	detail.o \
	effect.o \
	episode1.o \
	episode2.o \
	episode3.o \
	episode4.o \
	episode5.o \
	events.o \
	events_base.o \
	fehler.o \
	file.o \
	flic.o \
	gedclass.o \
	global.o \
	inits.o \
	io_game.o \
	m_event.o \
	main.o \
	main_menu.o \
	maus.o \
	mcga.o \
	mcga_grafik.o \
	memory.o \
	menus.o \
	metaengine.o \
	movclass.o \
	ngstypes.o \
	objekt.o \
	objekte.o \
	r_event.o \
	room.o \
	resource.o \
	sound.o \
	sprite.o \
	t_event.o \
	temp_file.o \
	text.o \
	timer.o \
	types.o \
	video/cfo_decoder.o \
	rooms/room0.o \
	rooms/room2.o \
	rooms/room3.o \
	rooms/room4.o \
	rooms/room5.o \
	rooms/room6.o \
	rooms/room7.o \
	rooms/room8.o \
	rooms/room9.o \
	rooms/room10.o \
	rooms/room11.o \
	rooms/room12.o \
	rooms/room13.o \
	rooms/room14.o \
	rooms/room16.o \
	rooms/room17.o \
	rooms/room18.o \
	rooms/room19.o \
	rooms/room21.o \
	rooms/room22.o \
	rooms/room23.o \
	rooms/room24.o

# This module can be built as a plugin
ifeq ($(ENABLE_CHEWY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
