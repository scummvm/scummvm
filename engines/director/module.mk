MODULE := engines/director

MODULE_OBJS = \
	archive.o \
	cast.o \
	cachedmactext.o \
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
	util.o \
	lingo/lingo-gr.o \
	lingo/lingo.o \
	lingo/lingo-builtins.o \
	lingo/lingo-bytecode.o \
	lingo/lingo-code.o \
	lingo/lingo-codegen.o \
	lingo/lingo-events.o \
	lingo/lingo-funcs.o \
	lingo/lingo-lex.o \
	lingo/lingo-preprocessor.o \
	lingo/lingo-the.o

director-grammar:
	flex engines/director/lingo/lingo-lex.l
	bison -dv engines/director/lingo/lingo-gr.y

# This module can be built as a plugin
ifeq ($(ENABLE_DIRECTOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
