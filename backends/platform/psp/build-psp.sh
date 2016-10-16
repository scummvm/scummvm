#!/bin/bash

export PATH=/opt/toolchains/psp/bin:$PATH
export PSPDEV=/opt/toolchains/psp
CXXFLAGS="-isystem /opt/toolchains/psp/include"
export CXXFLAGS
export LDFLAGS=-L/opt/toolchains/psp/lib
export LD_LIBRARY_PATH=/opt/toolchains/legacy-shared-objects
make distclean
./configure --host=psp --disable-debug --enable-plugins --default-dynamic --enable-release
make -j4
mkdir scummvm-1.9.0
cp -r AUTHORS COPYING COPYING.LGPL COPYING.BSD COPYRIGHT NEWS README gui/themes/translations.dat gui/themes/scummclassic.zip gui/themes/scummmodern.zip dists/engine-data/access.dat dists/engine-data/drascula.dat dists/engine-data/hugo.dat dists/engine-data/kyra.dat dists/engine-data/lure.dat dists/engine-data/mort.dat dists/engine-data/neverhood.dat dists/engine-data/queen.tbl dists/engine-data/sky.cpt dists/engine-data/teenagent.dat dists/engine-data/tony.dat dists/engine-data/toon.dat dists/engine-data/wintermute.zip dists/pred.dic EBOOT.PBP plugins backends/platform/psp/kbd.zip scummvm-1.9.0
zip -r9 scummvm-1.9.0-psp.zip scummvm-1.9.0

