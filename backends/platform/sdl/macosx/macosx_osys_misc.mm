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

#include "backends/platform/sdl/macosx/macosx.h"
#include "backends/platform/sdl/macosx/macosx-compat.h"
#include "base/version.h"

#include <Foundation/NSBundle.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSPathUtilities.h>
#include <Foundation/NSUserDefaults.h>
#include <AppKit/NSPasteboard.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_6
#define NSPasteboardTypeString NSStringPboardType
#endif

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
// https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/Cocoa64BitGuide/64BitChangesCocoa/64BitChangesCocoa.html
#if __LP64__ || NS_BUILD_32_LIKE_64
typedef unsigned long NSUInteger;
#else
typedef unsigned int NSUInteger;
#endif

// Those are not defined in the 10.4 SDK, but they are defined when targeting
// Mac OS X 10.4 or above in the 10.5 SDK, and they do work with 10.4.
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
enum {
	NSUTF32StringEncoding = 0x8c000100,
	NSUTF32BigEndianStringEncoding = 0x98000100,
	NSUTF32LittleEndianStringEncoding = 0x9c000100
};
#endif
#endif

void OSystem_MacOSX::updateStartSettings(const Common::String & executable, Common::String &command, Common::StringMap &settings, Common::StringArray& additionalArgs) {
	NSBundle* bundle = [NSBundle mainBundle];
	// Check if scummvm is running from an app bundle
	if (!bundle || ![bundle bundleIdentifier]) {
		// Use default autostart implementation
		OSystem_POSIX::updateStartSettings(executable, command, settings, additionalArgs);
		return;
	}

	// If the bundle contains a scummvm.ini, use it as initial config
	NSString *iniPath = [bundle pathForResource:@"scummvm" ofType:@"ini"];
	if (iniPath && !settings.contains("initial-cfg"))
		settings["initial-cfg"] = Common::String([iniPath fileSystemRepresentation]);

	// If a command was specified on the command line, do not override it
	if (!command.empty())
		return;

	// Check if we have an autorun file with additional arguments
	NSString *autorunPath = [bundle pathForResource:@"scummvm-autorun" ofType:nil];
	if (autorunPath) {
		Common::File autorun;
		Common::String line;
		if (autorun.open(Common::FSNode([autorunPath fileSystemRepresentation]))) {
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
		settings["path"] = [gamePath fileSystemRepresentation];
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
		settings["path"] = [gamesPath fileSystemRepresentation];
		settings["recursive"] = "true";
		settings["exit"] = "false";
		return;
	}
}

bool OSystem_MacOSX::hasTextInClipboard() {
	return [[NSPasteboard generalPasteboard] availableTypeFromArray:[NSArray arrayWithObject:NSPasteboardTypeString]] != nil;
}

Common::U32String OSystem_MacOSX::getTextFromClipboard() {
	if (!hasTextInClipboard())
		return Common::U32String();

	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	NSString *str = [pb  stringForType:NSPasteboardTypeString];
	if (![str respondsToSelector:@selector(getBytes:maxLength:usedLength:encoding:options:range:remainingRange:)])
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
	Common::u32char_type_t *text = new Common::u32char_type_t[textLength];
	if (![str getBytes:text maxLength:4*textLength usedLength:NULL encoding: stringEncoding options:0 range:NSMakeRange(0, textLength) remainingRange:NULL]) {
		delete[] text;
		return Common::U32String();
	}
	Common::U32String u32String(text, textLength);
	delete[] text;

	return u32String;
}

bool OSystem_MacOSX::setTextInClipboard(const Common::U32String &text) {
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	[pb declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];

#ifdef SCUMM_LITTLE_ENDIAN
	NSStringEncoding stringEncoding = NSUTF32LittleEndianStringEncoding;
#else
	NSStringEncoding stringEncoding = NSUTF32BigEndianStringEncoding;
#endif
	NSString *nsstring = [[NSString alloc] initWithBytes:text.c_str() length:4*text.size() encoding: stringEncoding];
	bool status =  [pb setString:nsstring forType:NSPasteboardTypeString];
	[nsstring release];
	return status;
}
