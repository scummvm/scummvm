CXX = g++
CXXFLAGS = -g -O2 -Wall -I../lua/include -I../lua/src -I.. -DLUA_NUM_TYPE=float
CFLAGS = -g -O2 -Wall
LDFLAGS = -g
LIBS = -L../lua/lib -llua

all: bmtoppm delua imc2wav int2flt mat2ppm unlab vima

bmtoppm: bmtoppm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ bmtoppm.c -lppm

mat2ppm: mat2ppm.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ mat2ppm.c -lppm

delua: delua.o ../lua/lib/liblua.a ../localize.o ../registry.o ../debug.o
	$(CXX) $(LDFLAGS) -o $@ delua.o ../localize.o ../registry.o $(LIBS)

.c.o:
	$(CXX) $(CXXFLAGS) -c $<
