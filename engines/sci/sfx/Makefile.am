SUBDIRS = seq timer player device mixer pcm_device softseq
INCLUDES = -I$(top_srcdir)/src/include @EXTRA_INCLUDES@
EXTRA_DIST = timetest.c adlib.h device.h mixer.h sequencer.h softseq.h
noinst_LIBRARIES = libscisound.a libscisoundlib.a
libscisound_a_SOURCES = iterator.c songlib.c core.c pcm-iterator.c
libscisoundlib_a_SOURCES = time.c adlib.c

bin_PROGRAMS = test-iterator
test_iterator_SOURCE = test-iterator.c
test_iterator_LDFLAGS = @SCIV_LDFLAGS@
LDADD = \
	libscisound.a \
	player/libsciplayer.a \
	seq/libsciseq.a \
	timer/libscitimer.a \
	pcm_device/libscipcm.a \
	mixer/libscimixer.a \
	softseq/libscisoftseq.a \
	libscisoundlib.a \
	device/libscisounddevice.a \
        ../scicore/libscicore.a \
	@LIB_M@


TESTS = test-iterator


