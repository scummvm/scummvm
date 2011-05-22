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

 Copy the scummvm/dists/bada folder into a clean directory 
 outside of the scummvm package. Start the BADA IDE then 
 choose this folder as the eclipse workspace. Click
 Project / Build.




