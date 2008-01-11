.PHONY: all

all:


SRCDIR = ../../..

config:
	cd $(SRCDIR) ; CXXFLAGS="-Os -mcpu=arm926ej-s -fomit-frame-pointer -DMAEMO_SDL" ./configure  --host=arm-linux --prefix=/usr --disable-debug --disable-mt32emu  --disable-hq-scalers --with-tremor-prefix=/usr --enable-tremor --with-zlib-prefix=/usr --enable-zlib --with-mad-prefix=/usr --enable-mad --enable-flac --disable-fluidsynth

scummvm:
	cd $(SRCDIR) ; make
	echo "If there is compiler crash (in Kyra engine) then cd ../../.. and copy/run last line with -O3 instead of -Os and re-run make"
#	cp $(SRCDIR)/scummvm .


deb: $(SRCDIR)/scummvm
	cd $(SRCDIR) ; dpkg-buildpackage -rfakeroot -b -nc

