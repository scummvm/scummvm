CXX = g++
CXXFLAGS = -g -O2 -Wall -I../lua/include -I../lua/src -DLUA_NUM_TYPE=float
LDFLAGS = -g
LIBS = -L../lua/lib -llua

delua: delua.o
	$(CXX) $(LDFLAGS) -o $@ delua.o $(LIBS)

.c.o:
	$(CXX) $(CXXFLAGS) -c $<
