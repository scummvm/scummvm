CXX = g++
CXXFLAGS = -g -O2 -Wall -I../lua/include -I../lua/src -I.. -DLUA_NUM_TYPE=float
LDFLAGS = -g
LIBS = -L../lua/lib -llua

delua: delua.o ../lua/lib/liblua.a ../localize.o ../registry.o ../debug.o
	$(CXX) $(LDFLAGS) -o $@ delua.o ../localize.o ../registry.o $(LIBS)

.c.o:
	$(CXX) $(CXXFLAGS) -c $<
