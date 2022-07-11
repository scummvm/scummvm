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
	echo This script is macOS-only
	exit
fi

echo_n "Downloading System 7.0.1 image..."
if test ! -f System_7.0.1.smi.bin; then
	curl -s https://download.info.apple.com/Apple_Support_Area/Apple_Software_Updates/English-North_American/Macintosh/System/Older_System/System_7.0.x/System_7.0.1.smi.bin -o System_7.0.1.smi.bin
fi

if test ! -f System_7.0.1.smi.bin; then
	echo "Cannot download System_7.0.1.smi.bin"
	exit
fi

echo done

echo_n "Decompressing System 7.0.1 image..."

macbinary decode System_7.0.1.smi.bin
rm System_7.0.1.smi.bin
./decompress-diskcopy-image.py System\ 7.0.1.smi sys.img

echo done

echo "Dumping floppy images..."

./dumper-companion.py iso sys.img .

echo Done

echo_n "Cutting Fonts.image..."
mv Fonts.image Fonts.image.bin
macbinary decode Fonts.image.bin

tail -c +85 Fonts.image | head -c 1474048 >Fonts1.image

echo done

echo "Extracting fonts..."

./dumper-companion.py iso Fonts1.image .

rm Fonts.image.bin

echo done

echo_n "Copying fonts..."

for i in Athens Cairo Chicago Courier Geneva Helvetica London "Los Angeles" Monaco "New York" Palatino "San Francisco" Symbol Times Venice
do
	mv "$i" "$i.bin"
done

echo ...done

zip -9 classicmacfonts *.bin
mv classicmacfonts.zip classicmacfonts.dat

echo_n "Cleaning up..."
rm *.bin
rm *.smi
rm *.image
echo done

ls -l classicmacfonts.dat
