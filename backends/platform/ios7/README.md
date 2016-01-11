# ScummVM for iOS 7.1+ #

This is a quick fix of the latest ScummVM (1.8.0) for iOS 7.1. It has been tested on real iPhone 6S+, and iPad Pro, and also on all the available Xcode simulators.

I tried to use all the latest iOS features to replace the old code. For instance, it uses gesture recognizers most of the time, it supports the new iPhones 6 / 6+ / 6s / 6s+ resolution, and you can copy your game files using iTunes.

## Compilation ##

First, clone the repository:
```
$ git clone https://github.com/scummvm/scummvm.git
```

### Compilation from Xcode ###

This is the recommended way to compile ScummVM, and the only one which makes it possible to run ScummVM on a non-jailbroken device!

The next step is to compile the **create_project** tool. Open the Xcode project you'll found in the **devtools/create\_project/xcode/** directory. Once compiled, copy the binary somewhere in your *PATH*, and create a **build** directory somewhere on your harddisk. It is recommended to create this directory next to the cloned repository (they share the same parent).

Execute the following commands in a terminal:
```
$ cd path_to_the_build_directory
$ create_project path_to_scummvm_repository --xcode --enable-fluidsynth --disable-jpeg --disable-bink --disable-16bit --disable-mt32emu --disable-nasm --disable-opengl --disable-theora --disable-taskbar
```

This will create an Xcode project for ScummVM, for both the OS X, and the iOS target.

Now, download the external libraries from http://bsr43.free.fr/scummvm/ScummVM-iOS-libraries.zip. Unzip the archive in your **build** directory. Please make sure that the **lib**, and **include** directories are at the root of the **build** directory, not in a subdirectory.

Now, your **build** directory should contain:
* a generated **engines** directory,
* a generated **scummvm.xcodeproj** project,
* an **include** directory,
* a **lib** directory.

You are ready to compile ScummVM: open the **scummvm.xcodeproj** project, and build it.

### Compilation from command line ###

For jailbroken devices, it is also possible to compile the project from command line. You'll need a working toolchain, and some tools, like **ldid**, to fake the code signature.

Here is a script to download, and compile all the required tools. This script has been wrote for Debian 8.2, and should be run as root.

```
#!/bin/bash

if [ $UID -ne 0 ]; then
	echo "This script should be run by the root user"
	exit 1
fi

# Install the Clang compiler
apt-get install -y clang-3.4 libclang-3.4-dev llvm-3.4 libtool bison flex automake subversion git pkg-config wget libssl-dev uuid-dev libxml2-dev || exit 1

# Add LLVM to the linker library path
echo /usr/lib/llvm-3.4/lib > /etc/ld.so.conf.d/libllvm-3.4.conf
ldconfig

# Add symlinks for the LLVM headers
ln -s /usr/lib/llvm-3.4/bin/llvm-config /usr/bin/llvm-config || exit 1
ln -s /usr/include/llvm-3.4/llvm /usr/include/llvm || exit 1
ln -s /usr/include/llvm-c-3.4/llvm-c /usr/include/llvm-c || exit 1
ln -s /usr/bin/clang-3.4 /usr/bin/clang || exit 1
ln -s /usr/bin/clang++-3.4 /usr/bin/clang++ || exit 1

# Build the linker
svn checkout http://ios-toolchain-based-on-clang-for-linux.googlecode.com/svn/trunk/cctools-porting || exit 1
cd cctools-porting
sed -i'' 's/proz -k=20  --no-curses/wget/g' cctools-ld64.sh
./cctools-ld64.sh || exit 1

cd cctools-855-ld64-236.3
./autogen.sh || exit 1
./configure --prefix=/usr/local --target=arm-apple-darwin11 || exit 1
make || exit 1
make install || exit 1
cd ../..

# Install ios-tools
wget https://ios-toolchain-based-on-clang-for-linux.googlecode.com/files/iphonesdk-utils-2.0.tar.gz || exit 1
tar xzf iphonesdk-utils-2.0.tar.gz
cd iphonesdk-utils-2.0
patch -p0 <<_EOF
*** genLocalization2/getLocalizedStringFromFile.cpp    2015-04-02 04:45:39.309837816 +0530
--- genLocalization2/getLocalizedStringFromFile.cpp    2015-04-02 04:45:11.525700021 +0530
***************
*** 113,115 ****
      clang::HeaderSearch headerSearch(headerSearchOptions,
-                                      fileManager,
                                       *pDiagnosticsEngine,
--- 113,115 ----
      clang::HeaderSearch headerSearch(headerSearchOptions,
+                                      sourceManager,
                                       *pDiagnosticsEngine,
***************
*** 129,134 ****
                  false);
-     clang::HeaderSearch headerSearch(fileManager,
                                       *pDiagnosticsEngine,
                                       languageOptions,
-                                      pTargetInfo);
      ApplyHeaderSearchOptions(headerSearch, headerSearchOptions, languageOptions, pTargetInfo->getTriple());
--- 129,134 ----
                  false);
+     clang::HeaderSearch headerSearch(fileManager);/*,
                                       *pDiagnosticsEngine,
                                       languageOptions,
+                                      pTargetInfo);*/
      ApplyHeaderSearchOptions(headerSearch, headerSearchOptions, languageOptio
_EOF

./autogen.sh || exit 1
CC=clang CXX=clang++ ./configure --prefix=/usr/local || exit 1
make || exit 1
make install || exit 1

# Install the iOS SDK 8.1
mkdir -p /usr/share/ios-sdk
cd /usr/share/ios-sdk
wget http://iphone.howett.net/sdks/dl/iPhoneOS8.1.sdk.tbz2 || exit 1
tar xjf iPhoneOS8.1.sdk.tbz2
rm iPhoneOS8.1.sdk.tbz2
```

Now, in order to compile ScummVM, execute the following commands:
```
$ export SDKROOT=/usr/share/ios-sdk/iPhoneOS8.1.sdk
$ export CC=ios-clang
$ export CXX=ios-clang++
$ ./configure --host=ios7 --disable-mt32emu --enable-release
$ make ios7bundle
```

At the end of the compilation, you'll find a **ScummVM.app** application: copy it over SSH, and reboot your device.

## Usage ##

The game data files can be copied on the iOS device using iTunes. Once done, add your games in ScummVM as usual.

Here is a list of the in-game gestures:

|Gesture|Description|
|-------|-----------|
|Two fingers swipe down|Display the ScummVM menu for loading, saving, etc.|
|Two fingers swipe right|Enable / disable the touchpad mode|
|Two fingers swipe up|Enable / disable the mouse-click-and-drag mode|
|Two fingers tap|Simulate a right click. You should tap with one finger, and then tap with another while keeping your first finger on the screen.|
|Two fingers double-tap|Skip the cinematic / video|

The iOS keyboard is visible when the device is in portrait mode, and hidden in landscape mode.
