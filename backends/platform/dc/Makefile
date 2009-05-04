# $URL$
# $Id$

ronindir = /usr/local/ronin

srcdir = ..

VPATH = $(srcdir)

CXX     = sh-elf-g++ -ml -m4-single-only
CXXFLAGS= -O3 -Wno-multichar -funroll-loops -fschedule-insns2 -fomit-frame-pointer -fdelete-null-pointer-checks -fno-exceptions -I$(srcdir)/lua -I$(srcdir) -I$(ronindir)/include -D__DC__
LDFLAGS = -Wl,-Ttext,0x8c010000,-Map,$@.map -nostartfiles $(ronindir)/lib/crt0.o
LIBS	= -L$(ronindir)/lib -lronin -lz -lm


DRIVER_OBJS = \
	driver_ronin.o \
	driver_ronin_gfx.o \
	driver_ronin_sound.o \
	driver_ronin_event.o \
	matrix_ops.o \
	texture_manager.o


include $(srcdir)/Makefile.common


