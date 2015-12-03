# ScummVM for iOS 7.1+ #

This is a quick fix of the latest ScummVM (1.8.0) for iOS 7.1. It has been tested on real iPhone 6S+, and iPad Pro, and also on all the available Xcode simulators.

I tried to use all the latest iOS features to replace the old code. For instance, it uses gesture recognizers most of the time, it supports the new iPhones 6 / 6+ / 6s / 6s+ resolution, and you can copy your game files using iTunes.

## Compilation ##

First, clone the repository:
```
$ git clone -b ios-fix --recursive https://github.com/bSr43/scummvm.git
```

### Compilation from Xcode ###

This is the recommanded way to compile ScummVM, and the only one which makes it possible to run ScummVM on a non-jailbroken device!

The next step is to compile the **create_project** tool. Open the Xcode project you'll found in the **devtools/create\_project/xcode/** directory. Once compiled, copy the binary somewhere in your *PATH*, and create a **build** directory somewhere on your harddisk. It is recommended to create this directory next to the cloned repository (they share the same parent).

Execute the following commands in a terminal:
```
$ cd path_to_the_build_directory
$ create_project path_to_scummvm_repository --xcode --disable-mad --disable-jpeg --disable-bink --disable-scalers --disable-hqscalers --disable-16bit --disable-mt32emu --disable-nasm --disable-opengl --disable-theora --disable-taskbar
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

For jailbroken devices, it is also possible to compile the project from command line. You'll need some tools, like **ldid**, to fake the code signature.

Open a terminal, and execute the following commands:
```
$ cd path_to_the_scummvm_sources
$ SDKROOT=$(xcrun --sdk iphoneos --show-sdk-path) CC=clang CXX=clang++ ./configure --host=iphone --disable-scalers --disable-mt32emu --enable-release
$ make iphone
$ make iphonebundle
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
