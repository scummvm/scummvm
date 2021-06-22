export ANDROID_SDK="/Users/tpfaff/code/sdk/sdk" 
export ANDROID_NDK="/Users/tpfaff/code/ndk" 
export PATH=$PATH:/Users/tpfaff/code/ndk/toolchains/arm-linux-androideabi-4.8/prebuilt/darwin-x86_64/bin 
../configure --disable-engine=myst3 --disable-engine=monkey4 --disable-vorbis --disable-mpeg2 --disable-jpeg --disable-png --disable-freetype2 --host=android-v7a --backend=android --enable-opengl-shaders
