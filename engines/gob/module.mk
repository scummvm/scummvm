MODULE := engines/gob

MODULE_OBJS := \
	dataio.o \
	detection.o \
	draw.o \
	draw_v1.o \
	draw_v2.o \
	draw_bargon.o \
	draw_fascin.o \
	driver_vga.o \
	expression.o \
	game.o \
	global.o \
	gob.o \
	goblin.o \
	goblin_v1.o \
	goblin_v2.o \
	goblin_v3.o \
	goblin_v4.o \
	hotspots.o \
	init.o \
	init_v1.o \
	init_v2.o \
	init_v3.o \
	init_v4.o \
	init_v6.o \
	inter.o \
	inter_v1.o \
	inter_v2.o \
	inter_bargon.o \
	inter_fascin.o \
	inter_playtoons.o \
	inter_v3.o \
	inter_v4.o \
	inter_v5.o \
	inter_v6.o \
	map.o \
	map_v1.o \
	map_v2.o \
	mult.o \
	mult_v1.o \
	mult_v2.o \
	palanim.o \
	resources.o \
	scenery.o \
	scenery_v1.o \
	scenery_v2.o \
	script.o \
	totfile.o \
	util.o \
	variables.o \
	video.o \
	video_v1.o \
	video_v2.o \
	video_v6.o \
	videoplayer.o \
	demos/demoplayer.o \
	demos/scnplayer.o \
	demos/batplayer.o \
	save/savefile.o \
	save/savehandler.o \
	save/saveload.o \
	save/saveload_v2.o \
	save/saveload_v3.o \
	save/saveload_v4.o \
	save/saveload_v6.o \
	save/saveload_playtoons.o \
	save/saveconverter.o \
	save/saveconverter_v2.o \
	save/saveconverter_v3.o \
	save/saveconverter_v4.o \
	save/saveconverter_v6.o \
	sound/sound.o \
	sound/sounddesc.o \
	sound/pcspeaker.o \
	sound/adlib.o \
	sound/infogrames.o \
	sound/protracker.o \
	sound/soundmixer.o \
	sound/soundblaster.o \
	sound/cdrom.o \
	sound/bgatmosphere.o

# This module can be built as a plugin
ifeq ($(ENABLE_GOB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
