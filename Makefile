CXX = g++
CC = gcc
AR = ar rcu
CXXFLAGS = -g -W -Wall -Ilua `sdl-config --cflags` -I. -DUNIX -Wno-multichar # -O2
LDFLAGS = -g -W -Wall # -O2
LIBS = `sdl-config --libs` -lz 

# Comment this out for Mac OS X ...
LIBS += -lGL -lGLU

include Makefile.common
