#!/bin/bash
#
# This script downloads System 7.0.1 image from Apple and extracts fonts
# from it. Mac only, unfortunately.
#
# On Windows you perhaps can perform the extraction manually with use of
# HFSxplorer: http://www.catacombae.org/hfsexplorer/
#
# More information could be found in the vMac documentation: http://www.gryphel.com/c/image/
#
# Alternatively you may use vMac instructions for extracting these disk images:
#   http://www.gryphel.com/c/minivmac/recipes/sys7inst/
#
# Based on instructions posted at
# http://apple.stackexchange.com/questions/58243/can-i-get-the-original-mac-font-chicago-on-a-mountain-lion-mac

echo_n() {
	printf "$@"
}

if test `uname` != "Darwin"; then
	echo This script is Mac OS X-only
	exit
fi

echo_n "Downloading System 7.0.1 image..."
if test ! -f System_7.0.1.smi.bin; then
	curl -s http://download.info.apple.com/Apple_Support_Area/Apple_Software_Updates/English-North_American/Macintosh/System/Older_System/System_7.0.x/System_7.0.1.smi.bin -o System_7.0.1.smi.bin
fi

if test ! -f System_7.0.1.smi.bin; then
	echo "Cannot download System_7.0.1.smi.bin"
	exit
fi

echo done

echo_n "Mounting System 7.0.1 image..."

macbinary decode System_7.0.1.smi.bin
hdiutil convert -quiet System\ 7.0.1.smi -format UDRO -o sys7.dmg
hdiutil attach -quiet sys7.dmg

if test ! -f /Volumes/7.0.1\ \(1440k.images\)/Fonts.image; then
	echo "Failed to attach sys7.dmg"
	exit
fi

echo done

echo_n "Mounting Fonts disk image..."

hdiutil convert -quiet /Volumes/7.0.1\ \(1440k.images\)/Fonts.image -format UDRO -o fonts.dmg
hdiutil detach -quiet `hdiutil info|grep "/Volumes/7.0.1 (1440k.images)"|cut -f 1`
hdiutil attach -quiet fonts.dmg

if test ! -f /Volumes/Fonts/Chicago; then
	echo "Failed to attach fonts.dmg"
	exit
fi

echo done

echo_n "Copying fonts..."

for i in Athens Cairo Chicago Courier Geneva Helvetica London "Los Angeles" Monaco "New York" Palatino "San Francisco" Symbol Times Venice
do
	echo $i
	macbinary encode "/Volumes/Fonts/$i" -o "$i.bin" -n
done

echo ...Done

hdiutil detach -quiet `hdiutil info|grep "/Volumes/Fonts"|cut -f 1`

if test ! -f fondu_src-060102.tgz; then
	echo_n "Getting fondu_src-060102.tgz..."
	curl -s http://fondu.sourceforge.net/fondu_src-060102.tgz -o fondu_src-060102.tgz
	tar xf fondu_src-060102.tgz
fi

if test ! -d fondu-060102; then
	echo "Failed to download fondu_src-060102.tgz"
	exit
fi

echo done

if test ! -x fondu-060102/fondu; then
	echo_n "Compiling fondu..."
	cd fondu-060102
	./configure >configure.log 2>&1 && make 2>&1 >make.log
	cd ..
fi

if test ! -x fondu-060102/fondu; then
	echo "Failed to build fondu. See configure.log and make.log"
	exit
else
	rm -f configure.log make.log
fi

echo done

echo_n "Converting fonts..."
fondu-060102/fondu -force *.bin
echo done

zip -9 classicmacfonts *.bdf
mv classicmacfonts.zip classicmacfonts.dat

echo_n "Cleaning up..."
rm *.bdf
rm *.ttf
rm *.bin
rm *.dmg
echo done

ls -l classicmacfonts.dat
