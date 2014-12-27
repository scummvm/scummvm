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
	gfx/tinygl.o \
	resources/camera.o \
	resources/command.o \
	resources/floor.o \
	resources/floorface.o \
	resources/level.o \
	resources/location.o \
	resources/resource.o \
	resources/root.o \
	resourceprovider.o \
	resourcereference.o \
	scene.o \
	skeleton.o \
	skeleton_anim.o \
	sound.o \
	stark.o \
	texture.o \
	xmg.o \
	xrcreader.o

# Include common rules
include $(srcdir)/rules.mk
