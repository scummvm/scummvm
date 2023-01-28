MODULE := engines/director

MODULE_OBJS = \
	archive.o \
	cast.o \
	castmember.o \
	channel.o \
	cursor.o \
	director.o \
	debugger.o \
	events.o \
	fonts.o \
	frame.o \
	game-quirks.o \
	graphics.o \
	images.o \
	metaengine.o \
	movie.o \
	resource.o \
	score.o \
	sound.o \
	sprite.o \
	stxt.o \
	tests.o \
	transitions.o \
	types.o \
	util.o \
	window.o \
	lingo/lingo.o \
	lingo/lingo-builtins.o \
	lingo/lingo-bytecode.o \
	lingo/lingo-code.o \
	lingo/lingo-codegen.o \
	lingo/lingo-events.o \
	lingo/lingo-funcs.o \
	lingo/lingo-gr.o \
	lingo/lingo-lex.o \
	lingo/lingo-object.o \
	lingo/lingo-patcher.o \
	lingo/lingo-preprocessor.o \
	lingo/lingo-the.o \
	lingo/lingo-utils.o \
	lingo/xlibs/aiff.o \
	lingo/xlibs/applecdxobj.o \
	lingo/xlibs/askuser.o \
	lingo/xlibs/barakeobj.o \
	lingo/xlibs/batqt.o \
	lingo/xlibs/blitpict.o \
	lingo/xlibs/cdromxobj.o \
	lingo/xlibs/darkenscreen.o \
	lingo/xlibs/developerStack.o \
	lingo/xlibs/draw.o \
	lingo/xlibs/ednox.o \
	lingo/xlibs/fileexists.o \
	lingo/xlibs/fileio.o \
	lingo/xlibs/findfolder.o \
	lingo/xlibs/findsys.o \
	lingo/xlibs/flushxobj.o \
	lingo/xlibs/fplayxobj.o \
	lingo/xlibs/gpid.o \
	lingo/xlibs/iscd.o \
	lingo/xlibs/ispippin.o \
	lingo/xlibs/jitdraw3.o \
	lingo/xlibs/jwxini.o \
	lingo/xlibs/labeldrvxobj.o \
	lingo/xlibs/memoryxobj.o \
	lingo/xlibs/miscx.o \
	lingo/xlibs/moovxobj.o \
	lingo/xlibs/movemousexobj.o \
	lingo/xlibs/movutils.o \
	lingo/xlibs/orthoplayxobj.o \
	lingo/xlibs/palxobj.o \
	lingo/xlibs/popupmenuxobj.o \
	lingo/xlibs/porta.o \
	lingo/xlibs/qtmovie.o \
	lingo/xlibs/registercomponent.o \
	lingo/xlibs/serialportxobj.o \
	lingo/xlibs/soundjam.o \
	lingo/xlibs/spacemgr.o \
	lingo/xlibs/unittest.o \
	lingo/xlibs/videodiscxobj.o \
	lingo/xlibs/volumelist.o \
	lingo/xlibs/widgetxobj.o \
	lingo/xlibs/winxobj.o \
	lingo/xlibs/xio.o \
	lingo/xlibs/xplayanim.o \
	lingo/xlibs/yasix.o

# HACK: Skip this when including the file for detection objects.
ifeq "$(USE_RULES)" "1"
director-grammar:
	`brew --prefix flex`/bin/flex engines/director/lingo/lingo-lex.l
	`brew --prefix bison`/bin/bison -dv engines/director/lingo/lingo-gr.y
endif

# This module can be built as a plugin
ifeq ($(ENABLE_DIRECTOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
