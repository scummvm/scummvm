INCLUDES = -I$(top_srcdir)/src/include @EXTRA_INCLUDES@
EXTRA_DIST = sci_dos.c treedef.1 treedef.2 treedef.3 hufftree.1 \
			 hufftree.2 hufftree.3 huffmake.pl      \
			 hashmap.c exe.h exe_dec.h games.h
noinst_LIBRARIES = libscicore.a
libscicore_a_SOURCES = aatree.c tools.c resource.c decompress0.c        \
		 versions.c decompress01.c decompress1.c decompress11.c \
		 script.c vocab.c vocab_debug.c old_objects.c modules.c	\
		 sci_memory.c resource_map.c resource_patch.c		\
		 fnmatch.c int_hashmap.c console.c exe.c exe_lzexe.c	\
		 exe_raw.c reg_t_hashmap.c
