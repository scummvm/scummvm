# $Header$

ronindir = /usr/local/ronin

VPATH = ../..

CXX     = sh-elf-g++ -ml -m4-single-only
CFLAGS  = -O1 -Wno-multichar
DEFINES = -D__DC__ -DNONSTANDARD_PORT -DNONSTANDARD_SAVE
LDFLAGS := -Wl,-Ttext,0x8c010000 -nostartfiles ronin/crt0.o
INCLUDES:= -I./ -I../.. -I../../common
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= ronin/libronin.a ronin/libz.a -lm
EXEEXT =
MKDIR = mkdir -p
ECHO = /usr/ucb/echo -n
CAT = cat
RM = rm -f
AR = sh-elf-ar cru
RANLIB = sh-elf-ranlib

OBJS =	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o

include ../../Makefile.common

INCS	+= portdefs.h dc.h

$(OBJS): Makefile ronin

ronin:
	ln -s $(ronindir) $@


