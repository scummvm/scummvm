MODULE := engines/stark

MODULE_OBJS := \
	actor.o \
	adpcm.o \
	archive.o \
	archiveloader.o \
	console.o \
	detection.o \
	gfx/coordinate.o \
	gfx/driver.o \
	gfx/opengl.o \
	gfx/renderentry.o \
	gfx/tinygl.o \
	resources/anim.o \
	resources/animhierarchy.o \
	resources/animscript.o \
	resources/bonesmesh.o \
	resources/bookmark.o \
	resources/camera.o \
	resources/command.o \
	resources/direction.o \
	resources/floor.o \
	resources/floorface.o \
	resources/image.o \
	resources/item.o \
	resources/layer.o \
	resources/level.o \
	resources/location.o \
	resources/resource.o \
	resources/root.o \
	resources/textureset.o \
	resourceprovider.o \
	resourcereference.o \
	scene.o \
	skeleton.o \
	skeleton_anim.o \
	sound.o \
	stark.o \
	stateprovider.o \
	texture.o \
	visual/actor.o \
	visual/image.o \
	visual/smacker.o \
	xmg.o \
	xrcreader.o

# Include common rules
include $(srcdir)/rules.mk
