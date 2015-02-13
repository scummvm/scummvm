MODULE := engines/stark

MODULE_OBJS := \
	actor.o \
	console.o \
	detection.o \
	gfx/coordinate.o \
	gfx/driver.o \
	gfx/opengl.o \
	gfx/opengltexture.o \
	gfx/renderentry.o \
	gfx/texture.o \
	formats/iss.o \
	formats/tm.o \
	formats/xarc.o \
	formats/xmg.o \
	formats/xrc.o \
	resources/anim.o \
	resources/animhierarchy.o \
	resources/animscript.o \
	resources/bonesmesh.o \
	resources/bookmark.o \
	resources/camera.o \
	resources/command.o \
	resources/dialog.o \
	resources/direction.o \
	resources/floor.o \
	resources/floorface.o \
	resources/image.o \
	resources/item.o \
	resources/knowledge.o \
	resources/knowledgeset.o \
	resources/layer.o \
	resources/level.o \
	resources/location.o \
	resources/object.o \
	resources/root.o \
	resources/script.o \
	resources/scroll.o \
	resources/sound.o \
	resources/speech.o \
	resources/textureset.o \
	resourcereference.o \
	scene.o \
	services/archiveloader.o \
	services/dialogplayer.o \
	services/global.o \
	services/resourceprovider.o \
	services/services.o \
	services/stateprovider.o \
	services/userinterface.o \
	skeleton.o \
	skeleton_anim.o \
	stark.o \
	visual/actor.o \
	visual/image.o \
	visual/smacker.o

# Include common rules
include $(srcdir)/rules.mk
