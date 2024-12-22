MODULE := engines/sci

MODULE_OBJS := \
	console.o \
	detection_internal.o \
	dialogs.o \
	event.o \
	metaengine.o \
	sci.o \
	util.o \
	engine/features.o \
	engine/file.o \
	engine/gc.o \
	engine/guest_additions.o \
	engine/kernel.o \
	engine/kevent.o \
	engine/kfile.o \
	engine/kgraphics.o \
	engine/klists.o \
	engine/kmath.o \
	engine/kmenu.o \
	engine/kmisc.o \
	engine/kmovement.o \
	engine/kparse.o \
	engine/kpathing.o \
	engine/kscripts.o \
	engine/ksound.o \
	engine/kstring.o \
	engine/kvideo.o \
	engine/message.o \
	engine/object.o \
	engine/savegame.o \
	engine/script.o \
	engine/scriptdebug.o \
	engine/script_patches.o \
	engine/selector.o \
	engine/seg_manager.o \
	engine/segment.o \
	engine/state.o \
	engine/static_selectors.o \
	engine/tts.o \
	engine/vm.o \
	engine/vm_types.o \
	engine/workarounds.o \
	graphics/animate.o \
	graphics/cache.o \
	graphics/compare.o \
	graphics/controls16.o \
	graphics/coordadjuster.o \
	graphics/cursor.o \
	graphics/fontkorean.o \
	graphics/fontsjis.o \
	graphics/macfont.o \
	graphics/maciconbar.o \
	graphics/menu.o \
	graphics/paint16.o \
	graphics/palette.o \
	graphics/picture.o \
	graphics/portrait.o \
	graphics/ports.o \
	graphics/remap.o \
	graphics/scifont.o \
	graphics/scifx.o \
	graphics/screen.o \
	graphics/text16.o \
	graphics/transitions.o \
	graphics/view.o \
	graphics/drivers/cga.o \
	graphics/drivers/cgabw.o \
	graphics/drivers/common.o \
	graphics/drivers/default.o \
	graphics/drivers/ega.o \
	graphics/drivers/hercules.o \
	graphics/drivers/init.o \
	graphics/drivers/pc98_8col_sci0.o \
	graphics/drivers/pc98_8col_sci1.o \
	graphics/drivers/pc98_16col.o \
	graphics/drivers/upscaled.o \
	graphics/drivers/vgagrey.o \
	graphics/drivers/win16col.o \
	graphics/drivers/win256col.o \
	parser/grammar.o \
	parser/said.o \
	parser/vocabulary.o \
	resource/decompressor.o \
	resource/resource.o \
	resource/resource_audio.o \
	resource/resource_patcher.o \
	sound/audio.o \
	sound/midiparser_sci.o \
	sound/music.o \
	sound/soundcmd.o \
	sound/sync.o \
	sound/drivers/adlib.o \
	sound/drivers/amigamac0.o \
	sound/drivers/amigamac1.o \
	sound/drivers/casio.o \
	sound/drivers/cms.o \
	sound/drivers/fb01.o \
	sound/drivers/fmtowns.o \
	sound/drivers/midi.o \
	sound/drivers/pcjr.o \
	sound/drivers/pc9801.o \
	video/seq_decoder.o


ifdef ENABLE_SCI32
MODULE_OBJS += \
	engine/hoyle5poker.o \
	engine/kgraphics32.o \
	graphics/celobj32.o \
	graphics/controls32.o \
	graphics/frameout.o \
	graphics/paint32.o \
	graphics/plane32.o \
	graphics/palette32.o \
	graphics/remap32.o \
	graphics/screen_item32.o \
	graphics/text32.o \
	graphics/transitions32.o \
	graphics/video32.o \
	graphics/cursor32.o \
	graphics/maccursor32.o \
	sound/audio32.o \
	sound/decoders/sol.o \
	video/robot_decoder.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_SCI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_SCI), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/detection_internal.o
endif
