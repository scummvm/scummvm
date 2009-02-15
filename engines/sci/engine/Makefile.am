INCLUDES = -I$(top_srcdir)/src/include @EXTRA_INCLUDES@
EXTRA_DIST = cfsml.pl savegame.cfsml gc.h heap.h kernel_compat.h        \
	kernel_types.h sci_graphics.h
noinst_LIBRARIES = libsciengine.a
libsciengine_a_SOURCES = savegame.c kernel.c kscripts.c klists.c scriptconsole.c	\
	kfile.c kgraphics.c kmath.c kevent.c kstring.c kmenu.c		\
	kmovement.c kpathing.c ksound.c vm.c game.c scriptdebug.c	\
	said.y grammar.c seg_manager.c sys_strings.c gc.c message.c

savegame.c: savegame.cfsml
	cat savegame.cfsml | ./cfsml.pl -l @CFSML_FLAGS@ -f savegame.cfsml > savegame.c
