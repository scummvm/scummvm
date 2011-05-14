export BADA_SDK=/cygdrive/f/bada/1.2.1
export PATH=$PATH:${BADA_SDK}/Tools/Toolchains/Win32/bin/
export PATH=$PATH:${BADA_SDK}/Tools/Toolchains/ARM/bin/
./configure --host=bada --enable-debug
