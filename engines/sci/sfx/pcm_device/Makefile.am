noinst_LIBRARIES = libscipcm.a
INCLUDES = -I$(top_srcdir)/src/include @EXTRA_INCLUDES@
AM_CFLAGS = $(SDL_CFLAGS)
EXTRA_DIST = audiobuf.h
libscipcm_a_SOURCES = audiobuf.c pcm_devices.c sdl.c alsa.c
audbuf_test_LDADD = libscipcm.a ../../scicore/libscicore.a
check_PROGRAMS = audbuf_test
