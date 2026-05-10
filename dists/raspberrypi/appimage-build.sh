#!/bin/bash

export VERSION="2.2.0"
export SCRIPT_DIRECTORY=$(pwd)

# This script needs to be run with superuser privileges, e.g. via sudo.
# It automatically prepares a build environment for a ScummVM AppImage bundle
# and builds the image afterwards.
#
# Intended for building a ScummVM AppImage for the
# Raspberry Pi 3+ and higher, up to the Raspberry Pi 400
# and running on top of the Raspberry Pi OS (32 bit version/armhf)
# 
# WARNING: This script will modify your host system!
#          I recommend to use a separate SD card for building those images,
#          unless you don't mind having some additional packages installed on your system. 

#
# Begin.
#

# Download required ScummVM build dependencies:
apt install g++ \
            build-essential \
            git \
            liba52-dev \
            libjpeg62-turbo-dev \
            libmpeg2-4-dev \
            libogg-dev \
            libvorbis-dev \
            libgtk-3-dev \
            libflac-dev \
            libmad0-dev \
            libpng-dev \
            libtheora-dev \
            libfaad-dev \
            libfluidsynth-dev \
            libfreetype6-dev \
            zlib1g-dev \
            libfribidi-dev \
            libglew-dev \
            libsdl2-dev \
            libsdl2-net-dev \
            libcurl4-openssl-dev \
            libspeechd-dev -y

# Download required AppImage and linuxdeploy build dependencies
apt install cmake \
            autopoint \
            automake \
            libtool \
            libfuse-dev \
            libglib2.0-dev \
            libcairo2-dev \
            libssl-dev \
            cimg-dev \
            patchelf \
            squashfs-tools -y

# Build AppImage toolchain if necessary
if ! which appimagetool; then
    git clone https://github.com/AppImage/AppImageKit /tmp/AppImageKit/
    cd /tmp/AppImageKit/
    git submodule update --init --recursive
    cmake . -DUSE_CCACHE=OFF
    make -j$(nproc)
    make install
    cp -a /tmp/AppImageKit/src/appimagetool /usr/local/bin/
    cp -a /tmp/AppImageKit/src/AppRun /usr/local/bin/
    cp -a /tmp/AppImageKit/src/digest /usr/local/bin/
    cp -a /tmp/AppImageKit/src/runtime /usr/local/bin/
    cp -a /tmp/AppImageKit/src/validate /usr/local/bin/
    cp -a /tmp/AppImageKit/src/embed-magic-bytes-in-file.sh /usr/local/bin/
    cd ..
    rm -rf /tmp/AppImageKit/
fi

# Build linuxdeploy if necessary
if ! which linuxdeploy; then
    git clone https://github.com/linuxdeploy/linuxdeploy /tmp/linuxdeploy/
    cd /tmp/linuxdeploy
    git submodule update --init --recursive
    cmake . -DUSE_CCACHE=OFF
    make -j$(nproc)
    make install
    cp -a /tmp/linuxdeploy/bin/* /usr/local/bin/
    cd ..
    rm -rf /tmp/linuxdeploy
fi

# Build linuxdeploy-plugin-appimage if necessary
if ! which linuxdeploy-plugin-appimage; then
    git clone https://github.com/linuxdeploy/linuxdeploy-plugin-appimage /tmp/linuxdeploy-plugin-appimage
    cd /tmp/linuxdeploy-plugin-appimage
    git submodule update --init --recursive
    cmake . -DUSE_CCACHE=OFF
    make -j$(nproc)
    make install
    cd ..
    rm -rf /tmp/linuxdeploy-plugin-appimage
fi

# Create new temporary directory, clean if necessary
mkdir -p    /tmp/scummvm-appimage-builds/
rm -rf      /tmp/scummvm-appimage-builds/*

# Build zenity, so we can bundle it with the AppImage
git clone https://gitlab.gnome.org/GNOME/zenity /tmp/zenity
cd /tmp/zenity
./autogen.sh
# Patch broken zenity configure script
sed -i s'/AX_CHECK_ENABLE_DEBUG(yes,GNOME_ENABLE_DEBUG)/#AX_CHECK_ENABLE_DEBUG(yes,GNOME_ENABLE_DEBUG)/g' configure
./configure
make -j$(nproc)
strip --strip-all /tmp/zenity/src/zenity

# Fetch ScummVM sources
git clone https://github.com/scummvm/scummvm /tmp/scummvm-appimage-builds/scummvm-src
git -C /tmp/scummvm-appimage-builds/scummvm-src checkout tags/v${VERSION}

# Build ScummVM
cd /tmp/scummvm-appimage-builds/scummvm-src
CXXFLAGS="$CXXFLAGS -fuse-ld=gold -flto=$(nproc) -ffunction-sections -fdata-sections" \
LDFLAGS="$LDFLAGS   -fuse-ld=gold -flto=$(nproc) -Wl,--gc-sections" \
./configure \
    --prefix=../scummvm-build \
    --datadir=../scummvm-build/share/scummvm \
    \
    --disable-debug \
    --enable-release

CXXFLAGS="$CXXFLAGS -fuse-ld=gold -flto=$(nproc) -ffunction-sections -fdata-sections" \
LDFLAGS="$LDFLAGS   -fuse-ld=gold -flto=$(nproc) -Wl,--gc-sections" \
make -j$(nproc)
strip --strip-all scummvm
make install
cd ../../

# Create ScummVM desktop file
cat << EOF >> /tmp/scummvm-appimage-builds/scummvm-build/scummvm.desktop
[Desktop Entry]
Name=ScummVM
Comment=Interpreter for numerous adventure games and RPGs
Comment[pl]=Interpreter graficznych gier przygodowych
Comment[sv]=Tolk för flera äventyrsspel
Comment[he]=פרשן למספר משחקי הרפתקאות
Comment[de]=Interpreter für zahlreiche Abenteuerspiele und RPGs
Comment[es]=Intérprete para varias aventuras gráficas
Comment[ca]=Intèrpret per diverses aventures gràfiques
Exec=scummvm.wrapper
Icon=scummvm
Terminal=false
Type=Application
Categories=Game;AdventureGame;
StartupNotify=false
EOF

# Remove duplicate binary
rm -rf /tmp/scummvm-appimage-builds/scummvm-build/usr/bin/scummvm

# Add zenity and scummmvm.wrapper script for AppImage desktop integration
cp /tmp/zenity/src/zenity /tmp/scummvm-appimage-builds/scummvm-build/bin/zenity
cp /tmp/zenity/src/zenity.ui /tmp/scummvm-appimage-builds/scummvm-build/bin/zenity.ui
cp $SCRIPT_DIRECTORY/scummvm.wrapper /tmp/scummvm-appimage-builds/scummvm-build/bin/scummvm.wrapper
chmod +x /tmp/scummvm-appimage-builds/scummvm-build/bin/scummvm.wrapper

# Copy ScummVM icon from source
mkdir -p /tmp/scummvm-appimage-builds/scummvm-build/usr/share/icons/hicolor/128x128/apps/
wget https://github.com/scummvm/scummvm-media/raw/master/scummvm_icon_128.png -O /tmp/scummvm-appimage-builds/scummvm-build/usr/share/icons/hicolor/128x128/apps/scummvm.png

# Build the AppImage
linuxdeploy   --appdir=/tmp/scummvm-appimage-builds/scummvm-build/ \
              --executable=/tmp/scummvm-appimage-builds/scummvm-build/bin/scummvm \
              --custom-apprun=/tmp/scummvm-appimage-builds/scummvm-build/bin/scummvm.wrapper \
              -d /tmp/scummvm-appimage-builds/scummvm-build/scummvm.desktop \
              -i /tmp/scummvm-appimage-builds/scummvm-build/usr/share/icons/hicolor/128x128/apps/scummvm.png \
              -o appimage

mv /tmp/ScummVM-$VERSION*.AppImage $SCRIPT_DIRECTORY/

# Cleanup
cd /tmp/scummvm-appimage-builds/scummvm-src/
make uninstall
cd ..

rm -rf /tmp/zenity
rm -rf /tmp/scummvm-appimage-builds

# Done
echo "Done!"