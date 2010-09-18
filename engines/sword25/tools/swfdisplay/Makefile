CC = g++
DEBFLAGS = -g -Wall -O0
LDFLAGS = -lgdk_pixbuf-2.0 -lpng
CXXFLAGS = -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -I/usr/include/gdk-pixbuf-2.0/
BCFLAGS = -I/usr/include/libart-2.0 -I/usr/include/gtk-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include/ -I/usr/include/gdk-pixbuf-2.0/
BIN_TARGET = swfrender
OBJ_TARGETS = \
	swfrender.o \
	art.o \
	art_svp_intersect.o \
	art_svp_render_aa.o \
	art_svp_vpath.o \
	art_svp_vpath_stroke.o \
	art_vpath_bpath.o \
	vectorimage.o \
	vectorimagerenderer.o

all: ${BIN_TARGET}

${BIN_TARGET}: ${OBJ_TARGETS}
	${CC}  ${DEBFLAGS} $^ -o $@ ${LDFLAGS}

%.o: %.cpp Makefile
	${CC} ${CXXFLAGS} ${DEBFLAGS} -c $< -o $@

bezierrender: bezierrender.o
	${CC}  ${DEBFLAGS} $^ -o $@ ${LDFLAGS} -lart_lgpl_2

bezierrender.o: bezierrender.cpp
	${CC} ${BCFLAGS} ${DEBFLAGS} -c $< -o $@

clean:  ${BIN_TARGET} Makefile
	rm -f *~ *.o ${BIN_TARGET} 2>/dev/null
