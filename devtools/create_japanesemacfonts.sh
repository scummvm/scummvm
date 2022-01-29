#!/bin/bash
#
# This script downloads Mac OS X Lion installer from Apple and extracts fonts
# from it. Mac only, unfortunately.

echo_n() {
	printf "$@"
}

if test `uname` != "Darwin"; then
	echo This script is macOS-only
	exit
fi

echo "Downloading InstallMacOSX.dmg..."
if test ! -f InstallMacOSX.dmg; then
	curl https://updates.cdn-apple.com/2021/macos/041-7683-20210614-E610947E-C7CE-46EB-8860-D26D71F0D3EA/InstallMacOSX.dmg -o InstallMacOSX.dmg
fi

if test ! -f InstallMacOSX.dmg; then
	echo "Cannot download InstallMacOSX.dmg"
	exit
fi

echo "...Done"

echo_n "Mounting InstallMacOSX.dmg..."

hdiutil attach -quiet InstallMacOSX.dmg

if test ! -f "/Volumes/Install Mac OS X/InstallMacOSX.pkg"; then
	echo "Failed to attach InstallMacOSX.dmg"
	exit
fi

echo done

echo_n "Extracting InstallMacOSX.pkg..."

pkgutil --expand "/Volumes/Install Mac OS X/InstallMacOSX.pkg" InstallMacOSX
hdiutil detach -quiet `hdiutil info|grep "/Volumes/Install Mac OS X"|cut -f 1`

if test ! -f InstallMacOSX/InstallMacOSX.pkg/InstallESD.dmg; then
	echo "Failed to extract InstallMacOSX.pkg;"
	exit
fi

echo done

echo_n "Mounting InstallESD.dmg..."

hdiutil attach -quiet -nobrowse InstallMacOSX/InstallMacOSX.pkg/InstallESD.dmg

if test ! -f "/Volumes/Mac OS X Install ESD/Packages/Essentials.pkg"; then
	echo "Failed to attach InstallESD.dmg"
	exit
fi

echo done

echo_n "Extracting Essentials.pkg..."

pkgutil --expand-full "/Volumes/Mac OS X Install ESD/Packages/Essentials.pkg" Essentials
hdiutil detach -quiet `hdiutil info|grep "/Volumes/Mac OS X Install ESD"|cut -f 1`

if test ! -f Essentials/Payload/Library/Fonts/Osaka.ttf; then
	echo "Failed to extract Essentials.pkg;"
	exit
fi

echo done

echo_n "Copying fonts..."

for i in Osaka OsakaMono
do
	echo $i
	cp "Essentials/Payload/Library/Fonts/$i.ttf" .
done

echo ...Done

hdiutil detach -quiet `hdiutil info|grep "/Volumes/Fonts"|cut -f 1`

zip -9 japanesemacfonts *.ttf
mv japanesemacfonts.zip japanesemacfonts.dat

echo_n "Cleaning up..."
rm *.ttf
rm -rf InstallMacOSX
rm -rf Essentials
echo done

ls -l japanesemacfonts.dat
