Build instructions:

1. Install BADA SDK (requires free registration):

 http://developer.bada.com/apis/index.do

2. Install Cygwin:

 http://www.cygwin.com/

 Add the following to your cygwin .bash_profile:

 export BADA_SDK=/cygdrive/f/bada/1.2.1

 #Build target: simulator
 export PATH=${BADA_SDK}/Tools/Toolchains/Win32/bin:${PATH}
 #Build target: device
 #export PATH=${BADA_SDK}/Tools/Toolchains/ARM/bin:${PATH}

3. Build dependencies

 zlib, libogg, libvorbis, libmad, FLAC

 see: "Building the libraries" under:
 http://wiki.scummvm.org/index.php/Compiling_ScummVM/MinGW#Building_the_libraries
 Ignore NASM and SDL; install the libraries to /usr/local in lieu of /mingw as 
 per the document.

 3.1 use this configure line when building libvorbis:
     ./configure --disable-shared --with-ogg=c:/cygwin/usr/local

 3.2 for each module, after a successful configure, add the following
     to the generated config.h (gzguts.h for zlib)

     #undef __MINGW32__
     #undef _WIN32
     #include "c:/src/scummvm/backends/platform/bada/portdefs.h"

 3.3 removed -fforce-mem from the libMAD Makefile

4. Build the ScummVM base library:

 ./configure --host=bada --enable-debug

Note the debug build will target the Win32 simulator 

5. Build the front end application using BADA-Ide:

 Copy the scummvm/dists/bada folder into a clean directory 
 outside of the scummvm package. Start the BADA IDE then 
 choose this folder as the eclipse workspace. Click
 Project / Build.




