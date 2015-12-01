# ScummVM for iOS 9 #

This is a quick fix of the latest ScummVM (1.8.0) for iOS 9. It has been tested on real iPhone 6S+, and iPad Pro, and also on all the available Xcode simulators.

I tried to use all the latest iOS features to replace the old code. For instance, it uses gesture recognizers most of the time, it supports the new iPhones 6 / 6+ / 6s / 6s+ resolution, and you can copy your game files using iTunes.

*That being said, this port has been done in only a few hours, and without any knowledge about the project architecture. Feel free to improve!*

For instance, I should have modified the **create_project** tool in order to generate an iOS Xcode project. But instead of struggling with this *meta-level* from the start, I generated an OS X Xcode project, and then I cleaned the project in order to build the iOS version.

*The support for the iOS project generation will come later (maybe...)*

## Compilation ##

All the needed libraries are embedded as Xcode sub-projects, and added to the repository as GIT submodules.

In order to download the project, run this command in the Terminal application:
```
git clone --recursive https://github.com/bSr43/scummvm.git
```

Once done, double-click on the Xcode project that you'll find in the path **dists/iphone/scummvm.xcodeproj**.

By default, Xcode should have selected the correct scheme (*ScummVM-iPhone*). Choose a target device, or a simulator, and click on the Xcode build button.

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
