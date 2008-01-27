CXX ?= g++
CC ?= gcc

LDFLAGS ?=
CXXFLAGS ?= -O2
CFLAGS ?= -O2
CPPFLAGS ?=

BIN=create_kyradat

OBJS=create_kyradat.o pak.o md5.o util.o

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(BIN)

clean:
	rm -f $(BIN)
	rm -f $(OBJS)

create_kyradat.o: amiga.h eng.h esp.h fre.h ger.h misc.h pak.h towns.h util.h md5.h
pak.o: pak.h util.h

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CXX) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
