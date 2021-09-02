#!/bin/sh

srcdir=$(realpath $(dirname $0)/../../..)
mkdir -p all-builds

for build in scumm access agi agos bbvs cge cge2 cine cruise director draci dragons drascula dreamweb gob griffon hugo illusions kyra lab lure made mads parallaction queen saga sci sherlock sky supernova teenagent tinsel tsage tucker voyeur xeen; do
	echo "--- Setting up build $build ---"
	mkdir -p build-$build && cd build-$build
	$srcdir/configure --host=ds --disable-translation --disable-debug --disable-all-engines --enable-engine=$build --disable-ogg
	make -j$(nproc) DESCRIPTION="Enabled engines: $build"
	cp scummvm.nds ../all-builds/scummvm-$build.nds
	cd ..
	echo DONE
	echo
done
