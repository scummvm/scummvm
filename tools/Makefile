CXX = g++
CXXFLAGS = $(CPPFLAGS) -g -O2 -Wall -I../lua -I..
CFLAGS = $(CPPFLAGS) -g -O2 -Wall
#LDFLAGS =
LIBS =

all: bmtoppm delua imc2wav int2flt mat2ppm set2fig unlab vima

bmtoppm: bmtoppm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ bmtoppm.c -lppm -lpbm

mat2ppm: mat2ppm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ mat2ppm.c -lppm

delua: delua.o ../localize.o ../registry.o ../debug.o
	$(CXX) $(LDFLAGS) -o $@ delua.o ../localize.o ../registry.o \
	../lua/lapi.o ../lua/lauxlib.o ../lua/lbuffer.o ../lua/lbuiltin.o \
	../lua/ldo.o ../lua/lfunc.o ../lua/lgc.o ../lua/liolib.o \
	../lua/llex.o ../lua/lmathlib.o ../lua/lmem.o ../lua/lobject.o \
	../lua/lparser.o ../lua/lrestore.o ../lua/lsave.o ../lua/lstate.o \
	../lua/lstring.o ../lua/lstrlib.o ../lua/ltable.o ../lua/ltask.o \
	../lua/ltm.o ../lua/lundump.o ../lua/lvm.o ../lua/lzio.o $(LIBS)

unlab: unlab.o util.o
	$(CC) $(LDFLAGS) -o $@ $+

.c.o:
	$(CC) $(CXXFLAGS) -c $<

clean:
	-rm -f *.o bmtoppm delua imc2wav int2flt mat2ppm set2fig unlab vima
