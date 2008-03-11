CXX = g++
CC = gcc
AR = ar rcu
CXXFLAGS = -g -W -Wall `sdl-config --cflags` -I. -DUNIX -Wno-multichar -Wno-unknown-pragmas  # -O2
LDFLAGS = -g -W -Wall
LIBS = `sdl-config --libs` -lz 

# Comment this out for Mac OS X ...
LIBS += -lGL -lGLU

include Makefile.common
