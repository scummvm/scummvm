MODULE := engines/sword25

MODULE_OBJS := \
	detection.o \
	sword25.o \
	fmv/movieplayer.o \
	fmv/movieplayer_script.o \
	fmv/theora_decoder.o \
	fmv/yuvtorgba.o \
	gfx/animation.o \
	gfx/animationdescription.o \
	gfx/animationresource.o \
	gfx/animationtemplate.o \
	gfx/animationtemplateregistry.o \
	gfx/bitmap.o \
	gfx/bitmapresource.o \
	gfx/dynamicbitmap.o \
	gfx/fontresource.o \
	gfx/framecounter.o \
	gfx/graphicengine.o \
	gfx/graphicengine_script.o \
	gfx/panel.o \
	gfx/renderobject.o \
	gfx/renderobjectmanager.o \
	gfx/renderobjectregistry.o \
	gfx/screenshot.o \
	gfx/staticbitmap.o \
	gfx/text.o \
	gfx/timedrenderobject.o \
	gfx/image/b25sloader.o \
	gfx/image/imageloader.o \
	gfx/image/pngloader.o \
	gfx/image/vectorimage.o \
	gfx/image/vectorimagerenderer.o \
	gfx/opengl/glimage.o \
	gfx/opengl/openglgfx.o \
	gfx/opengl/swimage.o \
	input/inputengine.o \
	input/inputengine_script.o \
	input/scummvminput.o \
	kernel/callbackregistry.o \
	kernel/filesystemutil.o \
	kernel/inputpersistenceblock.o \
	kernel/kernel.o \
	kernel/kernel_script.o \
	kernel/log.o \
	kernel/memleaks.o \
	kernel/outputpersistenceblock.o \
	kernel/persistenceservice.o \
	kernel/resmanager.o \
	kernel/resource.o \
	kernel/scummvmwindow.o \
	kernel/window.o \
	math/geometry.o \
	math/geometry_script.o \
	math/polygon.o \
	math/region.o \
	math/regionregistry.o \
	math/vertex.o \
	math/walkregion.o \
	package/packagemanager.o \
	package/packagemanager_script.o \
	package/scummvmpackagemanager.o \
	script/luabindhelper.o \
	script/luacallback.o \
	script/luascript.o \
	script/lua_extensions.o \
	sfx/fmodexsound.o \
	sfx/soundengine.o \
	sfx/soundengine_script.o \
	util/lua/lapi.o \
	util/lua/lauxlib.o \
	util/lua/lbaselib.o \
	util/lua/lcode.o \
	util/lua/ldblib.o \
	util/lua/ldebug.o \
	util/lua/ldo.o \
	util/lua/ldump.o \
	util/lua/lfunc.o \
	util/lua/lgc.o \
	util/lua/linit.o \
	util/lua/liolib.o \
	util/lua/llex.o \
	util/lua/lmathlib.o \
	util/lua/lmem.o \
	util/lua/loadlib.o \
	util/lua/lobject.o \
	util/lua/lopcodes.o \
	util/lua/loslib.o \
	util/lua/lparser.o \
	util/lua/lstate.o \
	util/lua/lstring.o \
	util/lua/lstrlib.o \
	util/lua/ltable.o \
	util/lua/ltablib.o \
	util/lua/ltm.o \
	util/lua/lua.o \
	util/lua/luac.o \
	util/lua/lundump.o \
	util/lua/lvm.o \
	util/lua/lzio.o \
	util/lua/print.o \
	util/pluto/pdep.o \
	util/pluto/pluto.o \
	util/pluto/plzio.o

# HACK. Use proper CC compiler here
%.o: %.c
	$(QUIET)$(MKDIR) $(*D)/$(DEPDIR)
	$(QUIET_CXX)gcc  $(CXX_UPDATE_DEP_FLAG) $(CXXFLAGS) $(CPPFLAGS) -c $(<) -o $*.o

LIBS += -lpng -ltheoradec -lart_lgpl_2
CXXFLAGS += -I/usr/include/libart-2.0

# This module can be built as a plugin
ifeq ($(ENABLE_SWORD25), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
