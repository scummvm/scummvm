#!/bin/bash
#
# This script downloads Mac OS X Lion installer from Apple and extracts fonts

echo_n() {
	printf "$@"
}

echo "Downloading InstallMacOSX.dmg..."
if test ! -f InstallMacOSX.dmg; then
	curl https://updates.cdn-apple.com/2021/macos/041-7683-20210614-E610947E-C7CE-46EB-8860-D26D71F0D3EA/InstallMacOSX.dmg -o InstallMacOSX.dmg
fi

if test ! -f InstallMacOSX.dmg; then
	echo "Cannot download InstallMacOSX.dmg"
	exit
fi

echo "...Done"

echo_n "Extracting InstallMacOSX.pkg..."

7z e InstallMacOSX.dmg "Install Mac OS X/InstallMacOSX.pkg"
rm InstallMacOSX.dmg

if test ! -f "InstallMacOSX.pkg"; then
	echo "Failed to extract InstallMacOSX.pkg"
	exit
fi

echo done

echo_n "Extracting InstallESD.dmg..."

python xar-unpacker.py unpack InstallMacOSX.pkg InstallMacOSX.pkg/InstallESD.dmg InstallESD.dmg
rm InstallMacOSX.pkg

if test ! -f "InstallESD.dmg"; then
	echo "Failed to unpack InstallESD.dmg"
	exit
fi

echo done

echo_n "Extracting Essentials.pkg..."

7z e InstallESD.dmg "Mac OS X Install ESD/Packages/Essentials.pkg"
rm InstallESD.dmg

if test ! -f "Essentials.pkg"; then
	echo "Failed to extract Essentials.pkg;"
	exit
fi

echo done

echo_n "Extracting Payload..."

python xar-unpacker.py unpack Essentials.pkg Payload Payload.cpio.gz
rm Essentials.pkg

if test ! -f "Payload.cpio.gz"; then
	echo "Failed to extract Payload.cpio.gz;"
	exit
fi

echo done

echo_n "Decompressing Payload..."

7z e Payload.cpio.gz
rm Payload.cpio.gz

if test ! -f "Payload.cpio"; then
	echo "Failed to extract Payload.cpio;"
	exit
fi

echo done

echo_n "Extracting fonts..."

7z e Payload.cpio "./Library/Fonts/Osaka*.ttf"
rm Payload.cpio

if test ! -f Osaka.ttf; then
	echo "Failed to extract Osaka.ttf;"
	exit
fi

if test ! -f OsakaMono.ttf; then
	echo "Failed to extract OsakaMono.ttf;"
	exit
fi

echo done

zip -9 japanesemacfonts *.ttf
mv japanesemacfonts.zip japanesemacfonts.dat

echo_n "Cleaning up..."
rm Osaka.ttf
rm OsakaMono.ttf
echo done

ls -l japanesemacfonts.dat
