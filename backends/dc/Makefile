# $Header$

ronindir = /usr/local/ronin

srcdir = ../..
VPATH = $(srcdir)

CXX     = sh-elf-g++ -ml -m4-single-only
CXXFLAGS= -O1 -Wno-multichar -funroll-loops -fschedule-insns2 -fomit-frame-pointer -fdelete-null-pointer-checks -fno-exceptions
DEFINES = -D__DC__ -DNONSTANDARD_PORT
LDFLAGS := -Wl,-Ttext,0x8c010000 -nostartfiles $(ronindir)/lib/crt0.o
INCLUDES:= -I./ -I$(srcdir) -I$(srcdir)/common -I$(ronindir)/include/
CPPFLAGS= $(DEFINES) $(INCLUDES)
LIBS	= -L$(ronindir)/lib -lronin -lz -lm
EXEEXT = .elf
EXECUTABLE:= scummvm$(EXEEXT)
MKDIR = mkdir -p
ECHO = /usr/ucb/echo -n
CAT = cat
RM = rm -f
AR = sh-elf-ar cru
RANLIB = sh-elf-ranlib

OBJS :=	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o backends/fs/posix/posix-fs.o

include $(srcdir)/Makefile.common

INCS	+= portdefs.h dc.h

$(OBJS): Makefile

