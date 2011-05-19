Files:

missing.cpp - implements missing standard library type functions
bada.cpp - runs in the main thread, hold the BADA entry point
system.cpp - runs in a worker thread controlled in bada.cpp

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



//////////

TODO

http://developer.bada.com/article/Tips-for-porting-existing-OpenGL-applications-to-bada
http://developer.bada.com/help/index.jsp?topic=/com.osp.devguide.help/html/basic_bada_features/base/threads.htm
http://www.badadev.com/threads-part-1-simple-example/
// TODO convert to ospmain
//int main(int argc, char *argv[]) {
//Acquire and release a Mutex
//Mutex* pMutex = new Mutex;
//pMutex->Create(); 
//pMutex->Acquire();
// Use shared resource
//pMutex->Release();
