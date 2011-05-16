My development environment:

Ubuntu 11.4 using windows XP inside Oracle Virtual Box.

The development code and BADA installation are located on ~/.wine/drive_c
and are accessed via a shared VB folder (mapped to drive f:);

Unfortunately using BADA-Ide (eclipse) under wine is somewhat flakey but 
having the code on the linux drives safe guards against any XP/VB corruption
and also allows native access to GIT. 

Build instructions:

1. Install BADA SDK (requires free registration):
http://developer.bada.com/apis/index.do

2. Install Cygwin:
http://www.cygwin.com/

Add the following to your cygwin .bash_profile:

export BADA_SDK=/cygdrive/f/bada/1.2.1
export PATH=$PATH:${BADA_SDK}/Tools/Toolchains/Win32/bin/
export PATH=$PATH:${BADA_SDK}/Tools/Toolchains/ARM/bin/

3. Build the ScummVM base library:

./configure --host=bada --enable-debug

Note the debug build will target the Win32 simulator 

4. Build the front end application using BADA-Ide:

TODO


backends/fs/posix/posix-fs.cpp:36: fatal error: dirent.h:
defined 