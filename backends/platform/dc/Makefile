# $URL$
# $Id$

ronindir = /usr/local/ronin

DYNAMIC_MODULES = 1

srcdir = ../../..
VPATH = $(srcdir)

CC      = sh-elf-gcc -ml -m4-single-only
CXX     = sh-elf-g++ -ml -m4-single-only
CXXFLAGS= -O3 -Wno-multichar -funroll-loops -fschedule-insns2 -fomit-frame-pointer -fdelete-null-pointer-checks -fno-exceptions
DEFINES = -D__DC__ -DNONSTANDARD_PORT -DUSE_MAD -DUSE_ZLIB -DDISABLE_DEFAULT_SAVEFILEMANAGER
LDFLAGS = -Wl,-Ttext,0x8c010000 -nostartfiles $(ronindir)/lib/crt0.o
INCLUDES= -I./ -I$(srcdir) -I$(ronindir)/include/ -I$(srcdir)/engines
LIBS	= -L$(ronindir)/lib -lmad -lronin -lz -lm
EXECUTABLE = scummvm.elf
DEPDIR = .deps
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
DISABLE_SCALERS = true

ifdef DYNAMIC_MODULES
DEFINES += -DDYNAMIC_MODULES
PRE_OBJS_FLAGS = -Wl,--whole-archive
POST_OBJS_FLAGS = -Wl,--no-whole-archive
endif

OBJS :=	dcmain.o time.o display.o audio.o input.o selector.o icon.o \
	label.o vmsave.o softkbd.o dcloader.o cache.o dc-fs.o

MODULE_DIRS += ./

include $(srcdir)/Makefile.common

scummvm.bin : scummvm.elf
	sh-elf-objcopy -S -R .stack -O binary $< $@

SCUMMVM.BIN : scummvm.bin
	scramble $< $@

plugin_dist :
	for p in plugins/*.plg; do \
	  sh-elf-strip -g -o "`basename \"$$p\" | tr '[:lower:]' '[:upper:]'`" "$$p"; \
	done

dist : SCUMMVM.BIN plugins plugin_dist

spotless : distclean
	$(RM) SCUMMVM.BIN scummvm.bin *.PLG
