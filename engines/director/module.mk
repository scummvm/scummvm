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
	lingo/xlibs/barakeobj.o \
	lingo/xlibs/cdromxobj.o \
	lingo/xlibs/fileio.o \
	lingo/xlibs/flushxobj.o \
	lingo/xlibs/fplayxobj.o \
	lingo/xlibs/gpid.o \
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
	lingo/xlibs/registercomponent.o \
	lingo/xlibs/serialportxobj.o \
	lingo/xlibs/soundjam.o \
	lingo/xlibs/videodiscxobj.o \
	lingo/xlibs/winxobj.o \
	lingo/xlibs/xplayanim.o

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
