CXX = g++
CXXFLAGS = -g -W -Wall -Ilua/include `sdl-config --cflags` -DUNIX # -O2
LDFLAGS = -g -W -Wall # -O2
LIBS = -lSDL -lGL -lGLU -Llua/lib -llua -llualib `sdl-config --libs` \
	-Lmixer -lmixer
OBJS = main.o lab.o bitmap.o model.o resource.o material.o debug.o \
	textsplit.o lua.o registry.o localize.o scene.o engine.o actor.o \
	sound.o timer.o keyframe.o costume.o walkplane.o textobject.o \
	matrix3.o matrix4.o screen.o blocky16.o smush.o vima.o

DEPS = $(OBJS:.o=.d)

residual: $(OBJS) lua/lib/liblua.a lua/lib/liblualib.a mixer/libmixer.a
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -MMD -c $<

lua/lib/liblua.a lua/lib/liblualib.a: lua-build

mixer/libmixer.a: mixer-build

lua-build:
	$(MAKE) -C lua

mixer-build:
	$(MAKE) -C mixer

clean: lua-clean mixer-clean
	-rm -f residual *.o *.d *~

lua-clean:
	$(MAKE) -C lua clean

mixer-clean:
	$(MAKE) -C mixer clean

-include $(DEPS)

.PHONY: lua-build lua-clean mixer-build mixer-clean
