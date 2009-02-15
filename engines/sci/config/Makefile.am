INCLUDES = -I$(top_srcdir)/src/include

noinst_LIBRARIES = libsciconfig.a
libsciconfig_a_SOURCES = config.l extension.c
EXTRA_DIST = lsl2.scifx sq3.scifx config.test

check_PROGRAMS = test-parse
test_parse_LDADD = libsciconfig.a ../scicore/libscicore.a
test_parse_SOURCES = test-parse.c

check-parse.sh : test-parse

TESTS = check-parse.sh