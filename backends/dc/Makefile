# $Header$

ronindir = /usr/local/ronin

VPATH = ../..

CXX     = sh-elf-g++ -ml -m4-single-only
CXXFLAGS= -O4 -Wno-multichar -funroll-loops -fschedule-insns2 -fomit-frame-pointer -fdelete-null-pointer-checks -fno-exceptions
DEFINES = -D__DC__ -DNONSTANDARD_PORT
LDFLAGS := -Wl,-Ttext,0x8c010000 -nostartfiles ronin/crt0.o
INCLUDES:= -I./ -I../.. -I../../common
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= ronin/libronin.a ronin/libz.a -lm
EXEEXT = .elf
MKDIR = mkdir -p
ECHO = /usr/ucb/echo -n
CAT = cat
RM = rm -f
AR = sh-elf-ar cru
RANLIB = sh-elf-ranlib

OBJS :=	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o dc-fs.o

include ../../Makefile.common

INCS	+= portdefs.h dc.h
LIBS    := scumm/libscumm.a common/libcommon.a $(LIBS)

$(OBJS): Makefile ronin

ronin:
	ln -s $(ronindir) $@

%/module.mk :
	@mkdir -p $*
	@test -h common.rules || ln -s ../../common.rules ./
	@test ! -f ../../$@ || \
	  ln -s `echo $* | sed -e 's/[^/][^/]*/../g'`/../../$@ $@

