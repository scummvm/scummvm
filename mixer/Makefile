CXXFLAGS = -g -W -Wall `sdl-config --cflags`
RANLIB=ranlib
AR= ar rcu

OBJS=	\
	mixer.o \
	rate.o \
	audiostream.o

all: $(OBJS) libmixer.a

clean:
	rm -rf $(OBJS) libmixer.a

libmixer.a: $(OBJS)
	$(AR) $@ $(OBJS)
	$(RANLIB) $@
