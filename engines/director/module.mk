MODULE := engines/director

MODULE_OBJS = \
	archive.o \
	cast.o \
	castmember.o \
	channel.o \
	cursor.o \
	detection.o \
	director.o \
	events.o \
	frame.o \
	graphics.o \
	images.o \
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
	lingo/xlibs/fileio.o \
	lingo/xlibs/flushxobj.o \
	lingo/xlibs/palxobj.o \
	lingo/xlibs/winxobj.o

director-grammar:
	`brew --prefix flex`/bin/flex engines/director/lingo/lingo-lex.l
	`brew --prefix bison`/bin/bison -dv engines/director/lingo/lingo-gr.y

# This module can be built as a plugin
ifeq ($(ENABLE_DIRECTOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
