LDLIBS= -lm -ldl -llua
LDFLAGS = -rdynamic # -L../lua-5.1.3/src
# CFLAGS= -g3 -Wall -fprofile-arcs -ftest-coverage
CFLAGS= -g3 -Wall -ansi -pedantic

LIBTOOL=libtool --tag=CC

default: pluto.so pptest puptest

%.lo: %.c
	$(LIBTOOL) --mode=compile cc $(CFLAGS) -c $<

pluto.so: pluto.lo pdep.lo lzio.lo
	$(LIBTOOL) --mode=link cc -rpath /usr/local/lib/lua/5.1 -o libpluto.la $^
	mv .libs/libpluto.so.0.0.0 $@

test: pptest puptest pptest.lua puptest.lua pluto.so
	./pptest
	./puptest

pptest: pptest.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

puptest: puptest.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS) 

clean:
	-rm -r *.so *.la *.lo .libs *.a *.o *.bb *.bbg *.da *.gcov pptest puptest test.plh

