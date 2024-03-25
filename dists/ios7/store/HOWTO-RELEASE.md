## Releasing iOS port on AppStore

ScummVM is represented on Apple AppStore since version 2.8.0. This document describes how to release new versions to Apple AppStore. The porter doing the release should have some experience of Xcode and app management on https://appstoreconnect.com

### Preparations

Before the release has been tagged:
 - Update the text file `metadata-en-US.txt` with the new release notes.
After the release has been tagged:
 - Setup the iOS Xcode project according to the latest instructions on [our wiki](https://docs.scummvm.org/en/latest/other_platforms/ios_build.html). Make sure that the version of Xcode is supported by Apple to publish applications on AppStore.
 - Read the Apple submission guide [here](https://developer.apple.com/ios/submit/)

### Making changes for the AppStore version

The ScummVM application bundle name is normally `org.scummvm.scummvm`. Unfortunately the bundle name was already in use and could not be transferred to the ScummVM project. Therefore the bundle name has to be changed to `org.scummvm.app` before archiving the project.

### Archive and publish the application

Create an archive, which will build an application bundle with Release settings, by choosing `Product->Archive` in the Xcode menu list. When finished the archive will show in the application `Organizer`. `Organizer` can also be opened by choosing `Window->Organizer` in the Xcode menu. This is useful if one wants to inspect old archives. Select the archive that should be published and click the `Distribute App` button. Select `App Store Connect` as the method in the next window. The application will be uploaded to https://appstoreconnect.com and one has to login with an Apple Id linked to the ScummVM Apple Id to perform further actions. The uploaded version will now undergo a review session by the Apple Review Team. Once finished and approved the application is ready for distribution. Make sure to update the text fields with the metadata from `metadata-en-US.txt`.

### Bundling games

Games can be bundled into the application before publishing on AppStore. To do this first create an archive according to the instructions above. Open `Organizer` to show the archive and right click on the created archive. Choose `Show in Finder` which will open the folder containing the archive. Right-click on the .xcarchive file and choose `Show Package Contents`. The scummvm binary can be in the folder `Products/Applications`. Right-click on the `scummvm` binary and choose `Show Package Contents`. Create a new folder in the root directory called `games`. NOTE! It is important that the folder is called `games` and nothing else. Copy the folderd containing the game data files that should be bundled to the new `games` folder. Publish the app according to the instructions above.
Version 2.8.1 on AppStore was bundled with `Dráscula: The Vampire Strikes Back`.

### Useful information

The ScummVM project Apple account is registered on Eugene Sandulenko (sev). Access to the Apple account can only be granted by sev or rootfather.

Another application bundle name is used for TestFlight builds: org.scummvm.beta
