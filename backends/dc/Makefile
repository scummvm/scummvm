# $Header$

ronindir = /usr/local/ronin

# BUILD_PLUGINS = 1

srcdir = ../..
VPATH = $(srcdir)

CC      = sh-elf-gcc -ml -m4-single-only
CXX     = sh-elf-g++ -ml -m4-single-only
CXXFLAGS= -O3 -Wno-multichar -funroll-loops -fschedule-insns2 -fomit-frame-pointer -fdelete-null-pointer-checks -fno-exceptions
DEFINES = -D__DC__ -DNONSTANDARD_PORT
LDFLAGS = -Wl,-Ttext,0x8c010000 -nostartfiles $(ronindir)/lib/crt0.o
INCLUDES= -I./ -I$(srcdir) -I$(srcdir)/common -I$(ronindir)/include/
LIBS	= -L$(ronindir)/lib -lronin-netcd -lz -lm
EXECUTABLE = scummvm.elf
PLUGIN_PREFIX =
PLUGIN_SUFFIX = .plg
PLUGIN_EXTRA_DEPS = plugin.x plugin.syms scummvm.elf
PLUGIN_LDFLAGS = -nostartfiles -Wl,-q,-Tplugin.x,--just-symbols,scummvm.elf,--retain-symbols-file,plugin.syms -L$(ronindir)/lib
MKDIR = mkdir -p
RM = rm -f
RM_REC = rm -rf
AR = sh-elf-ar cru
RANLIB = sh-elf-ranlib
HAVE_GCC3 = true

ifdef BUILD_PLUGINS
DEFINES += -DDYNAMIC_MODULES
endif

OBJS :=	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o softkbd.o dcloader.o cache.o

MODULE_DIRS += .

include $(srcdir)/Makefile.common

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@

