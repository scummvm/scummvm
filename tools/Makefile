CXX = g++
CXXFLAGS = $(CPPFLAGS) -g -O2 -Wall -I../lua -I.. -DLUA_NUM_TYPE=float
CFLAGS = $(CPPFLAGS) -g -O2 -Wall
#LDFLAGS =
LIBS =

all: bmtoppm delua imc2wav int2flt mat2ppm set2fig unlab vima

bmtoppm: bmtoppm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ bmtoppm.c -lppm -lpbm

mat2ppm: mat2ppm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ mat2ppm.c -lppm

delua: delua.o ../localize.o ../registry.o ../debug.o
	$(CXX) $(LDFLAGS) -o $@ delua.o ../localize.o ../registry.o $(LIBS)

unlab: unlab.o util.o
	$(CC) $(LDFLAGS) -o $@ $+


.c.o:
	$(CC) $(CXXFLAGS) -c $<
