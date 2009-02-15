EXTRA_DIST = dc.c
noinst_LIBRARIES = libscimixer.a
INCLUDES = -I$(top_srcdir)/src/include @EXTRA_INCLUDES@
libscimixer_a_SOURCES = mixers.c soft.c
test_LDADD = libscimixer.a ../../scicore/libscicore.a
check_PROGRAMS = test
