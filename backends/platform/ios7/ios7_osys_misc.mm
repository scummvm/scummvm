/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// Needs to be included first as system headers redefine NO and YES, which clashes
// with the DisposeAfterUse::Flag enum used in Common stream classes.
#include "common/file.h"
#include "common/translation.h"

#include "backends/platform/ios7/ios7_osys_main.h"
#include "base/version.h"

#include <Foundation/NSBundle.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSUserDefaults.h>
#include <UIKit/UIKit.h>
#include <SystemConfiguration/SCNetworkReachability.h>
#include "backends/platform/ios7/ios7_app_delegate.h"
#include "backends/platform/ios7/ios7_video.h"

static inline void execute_on_main_thread_async(void (^block)(void)) {
	if ([NSThread currentThread] == [NSThread mainThread]) {
		block();
	} else {
		dispatch_async(dispatch_get_main_queue(), block);
	}
}

void OSystem_iOS7::updateStartSettings(const Common::String &executable, Common::String &command, Common::StringMap &settings, Common::StringArray& additionalArgs) {
		NSBundle* bundle = [NSBundle mainBundle];
		// Check if scummvm is running from an app bundle
		if (!bundle || ![bundle bundleIdentifier]) {
			// Use default autostart implementation
			EventsBaseBackend::updateStartSettings(executable, command, settings, additionalArgs);
			return;
		}

		// If the bundle contains a scummvm.ini, use it as initial config
		NSString *iniPath = [bundle pathForResource:@"scummvm" ofType:@"ini"];
		if (iniPath && !settings.contains("initial-cfg"))
			settings["initial-cfg"] = "appbundle:/scummvm.ini";

		// If a command was specified on the command line, do not override it
		if (!command.empty())
			return;

		// Check if we have an autorun file with additional arguments
		NSString *autorunPath = [bundle pathForResource:@"scummvm-autorun" ofType:nil];
		if (autorunPath) {
			Common::File autorun;
			Common::String line;
			if (autorun.open(Common::FSNode("appbundle:/scummvm-autorun"))) {
				while (!autorun.eos()) {
					line = autorun.readLine();
					if (!line.empty() && line[0] != '#')
						additionalArgs.push_back(line);
				}
			}
			autorun.close();
		}

		// If the bundle contains a game directory, auto-detect it
		NSString *gamePath = [[bundle resourcePath] stringByAppendingPathComponent:@"game"];
		BOOL isDir = false;
		BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:gamePath isDirectory:&isDir];
		if (exists && isDir) {
			// Use auto-detection
			command = "auto-detect";
			settings["path"] = "appbundle:/game";
			return;
		}

		// The rest of the function has some commands executed only the first time after each version change
		// Check the last version stored in the user settings.
		NSString *versionString = [NSString stringWithUTF8String:gScummVMFullVersion];
		NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
		NSString *lastVersion = [defaults stringForKey:@"lastVersion"];
		if (lastVersion && [lastVersion isEqualToString:versionString])
			return;
		[defaults setObject:versionString forKey:@"lastVersion"];

		// If the bundle contains a games directory, add them to the launcher
		NSString *gamesPath = [[bundle resourcePath] stringByAppendingPathComponent:@"games"];
		isDir = false;
		exists = [[NSFileManager defaultManager] fileExistsAtPath:gamesPath isDirectory:&isDir];
		if (exists && isDir) {
			// Detect and add games
			command = "add";
			settings["path"] = "appbundle:/games";
			settings["recursive"] = "true";
			settings["exit"] = "false";
			return;
		}

}

Common::String OSystem_iOS7::getSystemLanguage() const {
	NSString *language = [[NSLocale preferredLanguages] firstObject];
	if (language == nil)
		return Common::String();
	Common::String lang([language cStringUsingEncoding:NSISOLatin1StringEncoding]);
	// Depending on the iOS version this may use an underscore (e.g. en_US) or a
	// dash (en-US). Make sure we always return one with an underscore.
	Common::replace(lang, "-", "_");
	return lang;
}

bool OSystem_iOS7::hasTextInClipboard() {
#if TARGET_OS_IOS
	return [[UIPasteboard generalPasteboard] containsPasteboardTypes:UIPasteboardTypeListString];
#else
	return false;
#endif
}

Common::U32String OSystem_iOS7::getTextFromClipboard() {
	if (!hasTextInClipboard())
		return Common::U32String();

#if TARGET_OS_IOS
	UIPasteboard *pb = [UIPasteboard generalPasteboard];
	NSString *str = pb.string;
	if (str == nil)
		return Common::U32String();

	// If translations are supported, use the current TranslationManager charset and otherwise
	// use ASCII. If the string cannot be represented using the requested encoding we get a null
	// pointer below, which is fine as ScummVM would not know what to do with the string anyway.
#ifdef SCUMM_LITTLE_ENDIAN
	NSStringEncoding stringEncoding = NSUTF32LittleEndianStringEncoding;
#else
	NSStringEncoding stringEncoding = NSUTF32BigEndianStringEncoding;
#endif
	NSUInteger textLength = [str length];
	uint32 *text = new uint32[textLength];

	if (![str getBytes:text maxLength:4*textLength usedLength:NULL encoding: stringEncoding options:0 range:NSMakeRange(0, textLength) remainingRange:NULL]) {
		delete[] text;
		return Common::U32String();
	}

	Common::U32String u32String(text, textLength);
	delete[] text;

	return u32String;
#else
	return Common::U32String();
#endif
}

bool OSystem_iOS7::setTextInClipboard(const Common::U32String &text) {
#if TARGET_OS_IOS
#ifdef SCUMM_LITTLE_ENDIAN
	NSStringEncoding stringEncoding = NSUTF32LittleEndianStringEncoding;
#else
	NSStringEncoding stringEncoding = NSUTF32BigEndianStringEncoding;
#endif
	UIPasteboard *pb = [UIPasteboard generalPasteboard];
	NSString *nsstring = [[NSString alloc] initWithBytes:text.c_str() length:4*text.size() encoding: stringEncoding];
	[pb setString:nsstring];
	[nsstring release];
	return true;
#else
	return false;
#endif
}

bool OSystem_iOS7::openUrl(const Common::String &url) {
	UIApplication *application = [UIApplication sharedApplication];
	NSURL *nsurl = [NSURL URLWithString:[NSString stringWithCString:url.c_str() encoding:NSISOLatin1StringEncoding]];
	// The way to oipen a URL has changed in iOS 10. Check if the iOS 10 method is recognized
	// and otherwise use the old method.
	if ([application respondsToSelector:@selector(openURL:options:completionHandler:)]) {
		execute_on_main_thread_async(^ {
			[application openURL:nsurl options:@{} completionHandler:nil];
		});
	} else {
		execute_on_main_thread_async(^ {
			[application openURL:nsurl];
		});
	}
	return true;
}

bool OSystem_iOS7::isConnectionLimited() {
	// If we are connected to the internet through a cellular network, return true
	SCNetworkReachabilityRef ref = SCNetworkReachabilityCreateWithName(CFAllocatorGetDefault(), [@"www.google.com" UTF8String]);
	if (!ref)
		return false;
	SCNetworkReachabilityFlags flags = 0;
	SCNetworkReachabilityGetFlags(ref, &flags);
	CFRelease(ref);
	return (flags & kSCNetworkReachabilityFlagsIsWWAN);
}

Common::HardwareInputSet *OSystem_iOS7::getHardwareInputSet() {
	using namespace Common;

	CompositeHardwareInputSet *inputSet = new CompositeHardwareInputSet();
	// Mouse is always supported, either through touch or device
	inputSet->addHardwareInputSet(new MouseHardwareInputSet(defaultMouseButtons));

	if ([[iOS7AppDelegate iPhoneView] isGamepadControllerSupported]) {
		inputSet->addHardwareInputSet(new JoystickHardwareInputSet(defaultJoystickButtons, defaultJoystickAxes));
	}

	inputSet->addHardwareInputSet(new KeyboardHardwareInputSet(defaultKeys, defaultModifiers));

	return inputSet;
}

void OSystem_iOS7::handleEvent_applicationSaveState() {
	[[iOS7AppDelegate iPhoneView] beginBackgroundSaveStateTask];
	saveState();
	[[iOS7AppDelegate iPhoneView] endBackgroundSaveStateTask];
}

void OSystem_iOS7::handleEvent_applicationRestoreState() {
	restoreState();
}

void OSystem_iOS7::handleEvent_applicationClearState() {
	clearState();
}

#if TARGET_OS_IOS
static const char * const helpTabs[] = {

_s("Getting help"),
"",
_s(
"## Help, I'm lost!\n"
"\n"
"First, make sure you have the games and necessary game files ready. Check the **Where to Get the Games** section under the **General** tab. Once obtained, follow the steps outlined in the **Adding Games** tab to finish adding them on this device.\n"
"\n"
"Need more help? Refer to our [online documentation for iOS](https://docs.scummvm.org/en/latest/other_platforms/ios.html). Got questions? Swing by our [support forums](https://forums.scummvm.org/viewforum.php?f=15) or hop on our [Discord server](https://discord.gg/4cDsMNtcpG), which includes an [iOS support channel](https://discord.com/channels/581224060529148060/1149456560922316911).\n"
"\n"
"Oh, and heads up, many of our supported games are intentionally tricky, sometimes mind-bogglingly so. If you're stuck in a game, think about checking out a game walkthrough. Good luck!\n"
),

_s("Touch Controls"),
"ios-help.zip",
_s(
"## Touch control modes\n"
"The touch control mode can be changed by tapping or clicking on the controller icon in the upper right corner, by swiping two fingers from left to right, or in the global settings from the Launcher go to **Global Options > Backend > Choose the preferred touch mode**. It's possible to configure the touch mode for three situations (ScummVM menus, 2D games and 3D games).\n"
"\n"
"### Direct mouse \n"
"\n"
"The touch controls are direct. The pointer jumps to where the finger touches the screen (default for menus).\n"
"\n"
"  ![Direct mouse mode](mouse.png \"Direct mouse mode\"){w=10em}\n"
"\n"
"### Touchpad emulation \n"
"\n"
"The touch controls are indirect, like on a laptop touchpad.\n"
"\n"
"  ![Touchpad mode](touchpad.png \"Touchpad mode\"){w=10em}\n"
"\n"
"To select the preferred touch mode for menus, 2D games, and 3D games, go to **Global Options > Backend > Choose the preferred touch mode**.\n"
"\n"
"## Touch actions \n"
"\n"
"| Gesture           | Action            \n"
"| ------------------|-------------------\n"
"| `One finger tap`  | Left mouse click  \n"
"| `Two fingers tap` | Right mouse click \n"
"| `Two fingers double tap` | ESC \n"
"| `One finger press & hold for >0.5s` | Left mouse button hold and drag, such as for selection from action wheel in Curse of Monkey Island \n"
"| `Two fingers press & hold for >0.5s` | Right mouse button hold and drag, such as for selection from action wheel in Tony Tough \n"
"| `Two fingers swipe (left to right)` | Toggles between the touch modes \n"
"| `Two fingers swipe (right to left)` | Toggles virtual controller (>iOS 15) \n"
"| `Two fingers swipe (top to bottom)` | Access Global Main Menu in games \n"
"| `Pinch (zoom in/out)` | Enables/disables keyboard \n"
"\n"
"### Virtual Gamepad \n"
"\n"
"Devices running iOS 15 or later can connect virtual gamepad controller by swiping two fingers from right to left or through **Global Options > Backend**. The directional button can be configured to either a thumbstick or a dpad.\n"
"**Note** While the virtual controller is connected it is not possible to perform mouse clicks using tap gestures since they are disabled as long as the virtual controller is visible. Left mouse clicks are performed by pressing the A button. Tap gestures are enabled again when virtual controller is disconnected.\n"
"\n"
"### Global Main Menu\n"
"\n"
"To open the Global Main Menu, tap on the menu icon at the top right of the screen or swipe two fingers downwards.\n"
"\n"
"  ![Menu icon](menu.png \"Menu icon\"){w=10em}\n"
"\n"
"## Virtual keyboard\n"
"\n"
"To open the virtual keyboard, long press on the controller icon at the top right of the screen, perform a pinch gesture (zoom out) or tap on any editable text field. To hide the virtual keyboard, tap the controller icon again, do an opposite pinch gesture (zoom in) or tap outside the text field.\n"
"\n"
"\n"
"  ![Keyboard icon](keyboard.png \"Keyboard icon\"){w=10em}\n"
"\n"
),
_s("External keyboard"),
"",
_s(
"## Use of keyboard\n"
"External keyboards are supported and from iOS 13.4 most of the special keys, e.g. function keys, Home and End, are mapped. \n"
"For external keyboards missing the special keys, e.g. the Apple Magic Keyboard for iPads, the special keys can be triggered using the following key combinations: \n"
"\n"
"\n"
"| Key combination   | Action            \n"
"| ------------------|-------------------\n"
"| `CMD + 1`         | F1                \n"
"| `CMD + 2`         | F2                \n"
"| `...`             | ...               \n"
"| `CMD + 0`         | F10               \n"
"| `CMD + SHIFT + 1` | F11               \n"
"| `CMD + SHIFT + 2` | F12               \n"
"| `CMD + UP`        | PAGE UP           \n"
"| `CMD + DOWN`      | PAGE DOWN         \n"
"| `CMD + LEFT`      | HOME              \n"
"| `CMD + RIGHT`     | END               \n"
"\n"
),
_s("Adding Games"),
"ios-help.zip",
_s(
"## Adding Games \n"
"\n"
"1. Copy the required game files to the ScummVM application. There are several ways to do that, see our [Transferring game files documentation](https://docs.scummvm.org/en/latest/other_platforms/ios.html#transferring-game-files) for more information.\n"
"\n"
"2. Select **Add Game...** from the launcher. \n"
"\n"
"3. In the ScummVM file browser, double-tap to browse to your added folder. Add a game by selecting the sub-folder containing the game files, then tap **Choose**. \n"
"\n"
"To add more games, repeat the steps above. \n"
"\n"
"See our [iOS documentation](https://docs.scummvm.org/en/latest/other_platforms/ios.html) for more information.\n"
),

0 // End of list
};
#else //TVOS
static const char * const helpTabs[] = {

_s("Touch Controls"),
"",
_s(
"## Touch actions \n"
"\n"
"### Press Touch area \n"
"\n"
"Press Touch area to perform a left mouse click"
"\n"
"### Play/Pause\n"
"\n"
"Press Play/Pause to perform a right mouse click\n"
"\n"
"### Global Main Menu\n"
"\n"
"To open the Global Main Menu, press Back/Menu button.\n"
"\n"
"## Virtual keyboard\n"
"\n"
"To open the virtual keyboard, press and hold the Play/Pause button. To hide the virtual keyboard, press the Back/Menu button.\n"
"\n"
),

_s("Adding Games"),
"",
_s(
"## Adding Games \n"
"\n"
"1. Copy the required game files to the ScummVM application. There are several ways to do that, see our [Transferring game files documentation](https://docs.scummvm.org/en/latest/other_platforms/tvos.html#transferring-game-files) for more information.\n"
"\n"
"2. Select **Add Game...** from the launcher. \n"
"\n"
"3. In the ScummVM file browser, double-tap to browse to your added folder. Add a game by selecting the sub-folder containing the game files, then tap **Choose**. \n"
"\n"
"To add more games, repeat the steps above. \n"
"\n"
"See our [tvOS documentation](https://docs.scummvm.org/en/latest/other_platforms/tvos.html) for more information.\n"
),

0 // End of list
};
#endif

const char * const *OSystem_iOS7::buildHelpDialogData() {
	return helpTabs;
}
