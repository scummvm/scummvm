# ScummVM Maemo MakeFile
# $URL$
# $Id$

#control build
DISABLE_HQ_SCALERS = true

#DISABLE_SCUMM = 1
#DISABLE_HE = 1
DISABLE_SIMON = 1
DISABLE_SKY = 1
DISABLE_SWORD1 = 1
DISABLE_SWORD2 = 1
DISABLE_QUEEN = 1
DISABLE_KYRA = 1
DISABLE_SAGA = 1
DISABLE_GOB = 1
DISABLE_LURE = 1
DISABLE_CINE = 1
DISABLE_AGI = 1

srcdir = ../..
VPATH = $(srcdir)

CXX := g++
EXECUTABLE  := scummvm

INCDIR = ../../ . $(srcdir)/engines/

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

OBJS := main.o maemo-sdl.o

MODULE_DIRS += .


BACKEND := sdl
MODULES +=  backends/sdl base
MODULE_DIRS += .

HAVE_GCC3 = 1

include $(srcdir)/Makefile.common

