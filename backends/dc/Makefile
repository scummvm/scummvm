# $Header$

ronindir = /usr/local/ronin

VPATH = ..

CC      = sh-elf-g++ -ml -m4-single-only
CFLAGS  = -O1 -Wno-multichar
DEFINES = -D__DC__ -DNONSTANDARD_PORT -DNONSTANDARD_SAVE
LDFLAGS := -Wl,-Ttext,0x8c010000 -nostartfiles ronin/crt0.o
INCLUDES:= -I./ -I../ -I../sound
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= ronin/libronin.a ronin/libz.a -lm
EXEEXT =

OBJS =	dcmain.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o

include ../Makefile.common

INCS	+= portdefs.h dc.h

$(OBJS): Makefile sound/.create simon/.create v3/.create v4/.create ronin

sound/.create:
	mkdir sound && touch $@

simon/.create:
	mkdir simon && touch $@

v3/.create:
	mkdir v3 && touch $@

v4/.create:
	mkdir v4 && touch $@

ronin:
	ln -s $(ronindir) $@

