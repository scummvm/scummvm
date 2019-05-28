Build instructions (using linux)

1. Install the Tizen SDK

http://www.tizen.org

To use an alternative Java SDK to run the Tizen IDE (eclipse), edit ~/.profile 

export JAVA_HOME=/opt/jdk1.6.0_45
export PATH=${PATH}:${JAVA_HOME}/bin

2. Add the following to your ~/.bashrc file

export TIZEN_SDK=${HOME}/tizen-sdk
export TIZEN_ROOTSTRAP=${TIZEN_SDK}/platforms/tizen2.1/rootstraps/tizen-device-2.1.native
export TIZEN_BIN=${TIZEN_SDK}/tools/arm-linux-gnueabi-gcc-4.5/bin
export TIZEN_LIBS=${HOME}/tizen-lib
export PATH=${PATH}:${TIZEN_BIN}:~/bin
export CHOST=arm-linux-gnueabi
export LDFLAGS="--sysroot=${TIZEN_ROOTSTRAP} -L${TIZEN_LIBS}/lib"
export CPPFLAGS="--sysroot=${TIZEN_ROOTSTRAP} -fmessage-length=0 -fPIC\
  -I${TIZEN_ROOTSTRAP}/usr/include -I${TIZEN_LIBS}/include"
export CFLAGS=${CPPFLAGS}

3. Build dependencies

 See: "Building the libraries" under:
 https://wiki.scummvm.org/index.php/Compiling_ScummVM/MinGW#Building_the_libraries
 for instructions on how to obtain these modules

 3.1 zlib

 $ ./configure --static --prefix=${TIZEN_LIBS}
 $ make && make install

 3.2 freetype, libtheora, libogg, libvorbis, libmad, FLAC

 $ ./configure --host=arm-linux-gnueabi --prefix=${TIZEN_LIBS} --disable-shared
 $ make && make install

 Note: you can ignore the ranlib errors when doing make install.

 Modify the resulting ~/tizen-lib/bin/freetype-config file to include -lz when printing libs

 3.3 Linker ordering: scummvm, freetype, theoradec, vorbis, vorbisfile, mad, FLAC, ogg, z

4. Build the ScummVM base library:

  ./configure --host=tizen --enable-release --with-freetype2-prefix=${TIZEN_LIBS}/bin

  For development:

  ./configure --host=tizen --enable-verbose-build --enable-debug

5. Build the front end application using Tizen IDE

 Copy the scummvm/dists/bada folder into a clean directory
 outside of the scummvm package. Start the BADA IDE then
 choose this folder as the eclipse workspace. Click
 Project / Build.

Links:

A short turorial on implementing OpenGL ES 1.1 in BADA:
  http://forums.badadev.com/viewtopic.php?f=7&t=208

HelvB14 font files:
 http://www.cl.cam.ac.uk/~mgk25/ucs-fonts.html
 http://www.cl.cam.ac.uk/~mgk25/download/ucs-fonts-75dpi100dpi.tar.gz

 Then run the following command:
 $ ./ucs2any.pl 100dpi/helvB14.bdf MAPPINGS/8859-1.TXT iso8859-1 \
   MAPPINGS/8859-2.TXT iso8859-2 MAPPINGS/8859-3.TXT iso8859-3

=====================================================================
Archived build instruction for BADA/cygwin

1. Install BADA SDK (requires free registration):

 http://developer.bada.com/apis/index.do

2. Install Cygwin:

 http://www.cygwin.com/

 Add the following to your cygwin .bash_profile:

 alias mmake=/cygdrive/c/MinGW/bin/mingw32-make.exe
 export BADA_SDK=/cygdrive/c/bada/1.2.1
 export ARM_BIN=c:/bada/1.2.1/Tools/Toolchains/ARM/bin
 export CPPFLAGS="-fpic -fshort-wchar -mcpu=cortex-a8 -mfpu=vfpv3 \
                  -mfloat-abi=hard -mlittle-endian -mthumb-interwork -Wno-psabi \
                  -fno-strict-aliasing -fno-short-enums"
 export LDFLAGS="-nostdlib -lc-newlib -lm-newlib -LC:/bada/1.2.1/Model/Wave_LP1/Target"
 #export PATH=${BADA_SDK}/Tools/Toolchains/Win32/bin:${PATH}
 export PATH=${BADA_SDK}/Tools/Toolchains/ARM/bin:~/utils:${PATH}
 alias gcc=${ARM_BIN}/arm-samsung-nucleuseabi-gcc.exe
 alias ar=${ARM_BIN}/arm-samsung-nucleuseabi-ar.exe

 The following were added to ~/utils for zlib:

 ar:
 #!/bin/sh
 ${ARM_BIN}/arm-samsung-nucleuseabi-ar.exe $*

 gcc:
 #!/bin/sh
 ${ARM_BIN}/arm-samsung-nucleuseabi-gcc.exe $*
