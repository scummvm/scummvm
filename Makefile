CXX = g++
CXXFLAGS = -g -W -Wall -Ilua/include `sdl-config --cflags` # -O2
LDFLAGS = -g -W -Wall # -O2
LIBS = -lSDL -lGL -lGLU -Llua/lib -llua -llualib `sdl-config --libs`

OBJS = main.o lab.o bitmap.o model.o resource.o material.o debug.o \
	textsplit.o lua.o registry.o localize.o scene.o engine.o actor.o \
	sound.o mixer.o keyframe.o costume.o walkplane.o textobject.o \
	matrix3.o matrix4.o screen.o

DEPS = $(OBJS:.o=.d)

residual: $(OBJS) lua/lib/liblua.a lua/lib/liblualib.a
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -MMD -c $<

lua/lib/liblua.a lua/lib/liblualib.a: lua-build

lua-build:
	$(MAKE) -C lua

clean: lua-clean
	-rm -f residual *.o *.d *~

lua-clean:
	$(MAKE) -C lua clean

-include $(DEPS)

.PHONY: lua-build lua-clean
