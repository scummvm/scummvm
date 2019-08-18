# ScummVM for iOS 7.1+ #

This is a quick fix of the latest ScummVM (1.8.0) for iOS 7.1. It has been tested on real iPhone 6S+, and iPad Pro, and also on all the available Xcode simulators.

I tried to use all the latest iOS features to replace the old code. For instance, it uses gesture recognizers most of the time, it supports the new iPhones 6 / 6+ / 6s / 6s+ resolution, and you can copy your game files using iTunes.

## Compilation ##

See https://wiki.scummvm.org/index.php/Compiling_ScummVM/iPhone

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
