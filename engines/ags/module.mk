MODULE := engines/ags

MODULE_OBJS = \
	ags.o \
	metaengine.o \
	lib/aastr-0.1.1/aarot.o \
	lib/aastr-0.1.1/aastr.o \
	lib/aastr-0.1.1/aautil.o \
	lib/allegro.o \
	lib/allegro/color.o \
	lib/allegro/config.o \
	lib/allegro/digi.o \
	lib/allegro/error.o \
	lib/allegro/file.o \
	lib/allegro/fixed.o \
	lib/allegro/gfx.o \
	lib/allegro/keyboard.o \
	lib/allegro/midi.o \
	lib/allegro/mouse.o \
	lib/allegro/sound.o \
	lib/allegro/system.o \
	lib/allegro/unicode.o \
	shared/ac/dynobj/scriptaudioclip.o \
	shared/ac/audiocliptype.o \
	shared/ac/characterinfo.o \
	shared/ac/common.o \
	shared/ac/dialogtopic.o \
	shared/ac/gamesetupstruct.o \
	shared/ac/gamesetupstructbase.o \
	shared/ac/inventoryiteminfo.o \
	shared/ac/mousecursor.o \
	shared/ac/spritecache.o \
	shared/ac/view.o \
	shared/ac/wordsdictionary.o \
	shared/core/asset.o \
	shared/core/assetmanager.o \
	shared/debugging/debugmanager.o \
	shared/game/customproperties.o \
	shared/game/interactions.o \
	shared/game/room_file.o \
	shared/game/room_file_deprecated.o \
	shared/game/roomstruct.o \
	shared/gfx/allegrobitmap.o \
	shared/gfx/bitmap.o \
	shared/gfx/image.o \
	shared/gui/guibutton.o \
	shared/gui/guiinv.o \
	shared/gui/guilabel.o \
	shared/gui/guilistbox.o \
	shared/gui/guimain.o \
	shared/gui/guiobject.o \
	shared/gui/guislider.o \
	shared/gui/guitextbox.o \
	shared/script/cc_error.o \
	shared/script/cc_options.o \
	shared/script/cc_script.o \
	shared/util/alignedstream.o \
	shared/util/bufferedstream.o \
	shared/util/compress.o \
	shared/util/datastream.o \
	shared/util/directory.o \
	shared/util/file.o \
	shared/util/filestream.o \
	shared/util/geometry.o \
	shared/util/inifile.o \
	shared/util/ini_util.o \
	shared/util/lzw.o \
	shared/util/misc.o \
	shared/util/mutifilelib.o \
	shared/util/path.o \
	shared/util/proxystream.o \
	shared/util/stream.o \
	shared/util/string.o \
	shared/util/string_compat.o \
	shared/util/string_utils.o \
	shared/util/textstreamreader.o \
	shared/util/textstreamwriter.o \
	shared/util/version.o \
	shared/util/wgt2allg.o \
	engine/game/game_init.o \
	engine/game/savegame.o \
	engine/game/savegame_components.o \
	engine/game/viewport.o \
	engine/gui/animatingguibutton.o \
	engine/gui/cscidialog.o \
	engine/gui/guidialog.o \
	engine/gui/gui_engine.o \
	engine/gui/mylabel.o \
	engine/gui/mylistbox.o \
	engine/gui/mypushbutton.o \
	engine/gui/mytextbox.o \
	engine/gui/newcontrol.o \
	engine/main/config.o \
	engine/main/engine.o \
	engine/main/engine_setup.o \
	engine/main/game_file.o \
	engine/main/game_run.o \
	engine/main/game_start.o \
	engine/main/graphics_mode.o \
	engine/main/quit.o \
	engine/main/update.o

# This module can be built as a plugin
ifeq ($(ENABLE_AGS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
