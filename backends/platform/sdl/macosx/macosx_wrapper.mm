/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/platform/sdl/macosx/macosx_wrapper.h"
#include "backends/platform/sdl/macosx/macosx-compat.h"

#include <AppKit/NSPasteboard.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSPathUtilities.h>
#include <AvailabilityMacros.h>
#include <CoreFoundation/CFString.h>

#if MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_5
typedef unsigned long NSUInteger;

// Those are not defined in the 10.4 SDK, but they are defined when targetting
// Mac OS X 10.4 or above in the 10.5 SDK. So hopfully that means it works with 10.4 as well.
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4
enum {
	NSUTF32StringEncoding = 0x8c000100,
	NSUTF32BigEndianStringEncoding = 0x98000100,
	NSUTF32LittleEndianStringEncoding = 0x9c000100
};
#endif
#endif

bool hasTextInClipboardMacOSX() {
	return [[NSPasteboard generalPasteboard] availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]] != nil;
}

Common::U32String getTextFromClipboardMacOSX() {
	if (!hasTextInClipboardMacOSX())
		return Common::U32String();
	// Note: on OS X 10.6 and above it is recommanded to use NSPasteboardTypeString rather than NSStringPboardType.
	// But since we still target older version use NSStringPboardType.
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	NSString *str = [pb  stringForType:NSStringPboardType];
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
}

bool setTextInClipboardMacOSX(const Common::U32String &text) {
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	[pb declareTypes:[NSArray arrayWithObject:NSStringPboardType] owner:nil];

#ifdef SCUMM_LITTLE_ENDIAN
	NSStringEncoding stringEncoding = NSUTF32LittleEndianStringEncoding;
#else
	NSStringEncoding stringEncoding = NSUTF32BigEndianStringEncoding;
#endif
	NSString *nsstring = [[NSString alloc] initWithBytes:text.c_str() length:4*text.size() encoding: stringEncoding];
	bool status =  [pb setString:nsstring forType:NSStringPboardType];
	[nsstring release];
	return status;
}

Common::String getDesktopPathMacOSX() {
	// The recommanded method is to use NSFileManager.
	// NSUrl *url = [[[NSFileManager defaultManager] URLsForDirectory:NSDesktopDirectory inDomains:NSUserDomainMask] firstObject];
	// However it is only available in OS X 10.6+. So use NSSearchPathForDirectoriesInDomains instead (available since OS X 10.0)
	// [NSArray firstObject] is also only available in OS X 10.6+. So we need to use [NSArray count] and [NSArray objectAtIndex:]
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES);
	if ([paths count] == 0)
		return Common::String();
	NSString *path = [paths objectAtIndex:0];
	if (path == nil)
		return Common::String();
	return Common::String([path cStringUsingEncoding:NSASCIIStringEncoding]);
}
