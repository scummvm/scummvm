INCLUDES = -I$(top_srcdir)/src/include @ac_png_includes@ @EXTRA_INCLUDES@
LDADD = ../scicore/libscicore.a
EXTRA_DIST = sciunpack.h bdf.h bdfP.h
bin_PROGRAMS = sciunpack scidisasm scipack bdftofont fonttoc scimusicplayer
sciunpack_SOURCES = sciunpack.c listwords.c scriptdump.c vocabdump.c
sciunpack_LDADD = ../scicore/libscicore.a @ac_glx_libraries@ @ac_glx_libraries@ @ac_png_libraries@
scidisasm_SOURCES = scidisasm.c
scipack_LDADD = ../scicore/libscicore.a
scipack_SOURCES = scipack.c
scimusicplayer_LDADD = ../sfx/libscisound.a ../scicore/libscicore.a \
	../sfx/player/libsciplayer.a \
	../sfx/seq/libsciseq.a ../sfx/timer/libscitimer.a \
	../sfx/pcm_device/libscipcm.a ../sfx/mixer/libscimixer.a \
	../sfx/softseq/libscisoftseq.a \
	../sfx/libscisoundlib.a \
	../sfx/device/libscisounddevice.a @LIB_M@
scimusicplayer_SOURCES = musicplayer.c
bdftofont_SOURCES = bdf.c bdfgrid.c bdfgname.c bdftofont.c
fonttoc_SOURCES = fonttoc.c
