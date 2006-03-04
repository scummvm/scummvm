# ScummVM Maemo MakeFile
# $Header: /cvsroot/scummvm/scummvm/backends/psp/Makefile,v 1.6 2005/11/20 17:36:02 joostp Exp $

#control build
DISABLE_HQ_SCALERS = true

srcdir = ../..
VPATH = $(srcdir)

CXX := g++
EXECUTABLE  := scummvm

INCDIR = ../../

CXXFLAGS := -g -ansi -W -Wno-unused-parameter
CXXFLAGS += `pkg-config --cflags gconf-2.0 hildon-libs gtk+-2.0 libosso gdk-2.0`
CXXFLAGS += $(addprefix -I,$(INCDIR)) -I. -Wall $(CXXFLAGS)
CXXFLAGS += -O -Wuninitialized
CXXFLAGS += -Wno-long-long -Wno-multichar -Wno-unknown-pragmas
# Even more warnings...
CXXFLAGS += -pedantic -Wpointer-arith -Wcast-qual -Wconversion
CXXFLAGS += -Wshadow -Wimplicit -Wundef -Wnon-virtual-dtor
CXXFLAGS += -Wno-reorder -Wwrite-strings -fcheck-new

LIBS     += -lz -L/usr/lib -lSDL -lpthread -lXsp -losso
LIBS     += `pkg-config --libs gconf-2.0 hildon-libs gtk+-2.0 libosso gdk-2.0`
INCLUDES += -I/usr/include/SDL -D_REENTRANT -I/usr/X11R6/include
OBJS     += 
DEFINES  += -DUNIX -DNONSTANDARD_PORT -D__MAEMO__
LDFLAGS  += 

RANLIB  := ranlib
INSTALL := install
AR      := ar cru
MKDIR   := mkdir -p
ECHO    := printf
CAT     := cat
RM      := rm -f
RM_REC  := rm -f -r
CP      := cp

OBJS := main.o

MODULE_DIRS += .


BACKEND := sdl
MODULES +=  backends/sdl base
MODULE_DIRS += .

HAVE_GCC3 = 1

include $(srcdir)/Makefile.common

