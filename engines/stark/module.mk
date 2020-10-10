MODULE := engines/stark

MODULE_OBJS := \
	console.o \
	gfx/driver.o \
	gfx/framelimiter.o \
	gfx/opengls.o \
	gfx/openglsactor.o \
	gfx/openglsfade.o \
	gfx/openglsprop.o \
	gfx/openglssurface.o \
	gfx/opengltexture.o \
	gfx/renderentry.o \
	gfx/surfacerenderer.o \
	gfx/texture.o \
	formats/biff.o \
	formats/biffmesh.o \
	formats/dds.o \
	formats/iss.o \
	formats/tm.o \
	formats/xarc.o \
	formats/xmg.o \
	formats/xrc.o \
	metaengine.o \
	model/animhandler.o \
	model/model.o \
	model/skeleton_anim.o \
	movement/followpath.o \
	movement/followpathlight.o \
	movement/movement.o \
	movement/shortestpath.o \
	movement/stringpullingpath.o \
	movement/turn.o \
	movement/walk.o \
	resources/anim.o \
	resources/animhierarchy.o \
	resources/animscript.o \
	resources/animsoundtrigger.o \
	resources/bonesmesh.o \
	resources/bookmark.o \
	resources/camera.o \
	resources/container.o \
	resources/command.o \
	resources/dialog.o \
	resources/direction.o \
	resources/floor.o \
	resources/floorface.o \
	resources/floorfield.o \
	resources/fmv.o \
	resources/image.o \
	resources/item.o \
	resources/knowledge.o \
	resources/knowledgeset.o \
	resources/layer.o \
	resources/level.o \
	resources/light.o \
	resources/lipsync.o \
	resources/location.o \
	resources/object.o \
	resources/path.o \
	resources/pattable.o \
	resources/root.o \
	resources/script.o \
	resources/scroll.o \
	resources/sound.o \
	resources/speech.o \
	resources/string.o \
	resources/textureset.o \
	resourcereference.o \
	savemetadata.o \
	scene.o \
	services/archiveloader.o \
	services/dialogplayer.o \
	services/diary.o \
	services/fontprovider.o \
	services/gameinterface.o \
	services/global.o \
	services/resourceprovider.o \
	services/services.o \
	services/stateprovider.o \
	services/staticprovider.o \
	services/userinterface.o \
	services/settings.o \
	services/gamechapter.o \
	services/gamemessage.o \
	stark.o \
	tools/abstractsyntaxtree.o \
	tools/block.o \
	tools/command.o \
	tools/decompiler.o \
	ui/cursor.o \
	ui/dialogbox.o \
	ui/menu/diaryindex.o \
	ui/menu/locationscreen.o \
	ui/menu/mainmenu.o \
	ui/menu/settingsmenu.o \
	ui/menu/saveloadmenu.o \
	ui/menu/fmvmenu.o \
	ui/menu/diarypages.o \
	ui/menu/dialogmenu.o \
	ui/window.o \
	ui/world/actionmenu.o \
	ui/world/button.o \
	ui/world/clicktext.o \
	ui/world/topmenu.o \
	ui/world/dialogpanel.o \
	ui/world/fmvscreen.o \
	ui/world/gamescreen.o \
	ui/world/gamewindow.o \
	ui/world/inventorywindow.o \
	visual/actor.o \
	visual/effects/bubbles.o \
	visual/effects/effect.o \
	visual/effects/fireflies.o \
	visual/effects/fish.o \
	visual/explodingimage.o \
	visual/flashingimage.o \
	visual/image.o \
	visual/prop.o \
	visual/smacker.o \
	visual/text.o

# This module can be built as a plugin
ifeq ($(ENABLE_STARK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
