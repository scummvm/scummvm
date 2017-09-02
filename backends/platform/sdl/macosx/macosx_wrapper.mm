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

#include <AppKit/NSPasteboard.h>
#include <Foundation/NSArray.h>
#include <Foundation/NSPathUtilities.h>
#include <AvailabilityMacros.h>

bool hasTextInClipboardMacOSX() {
	return [[NSPasteboard generalPasteboard] availableTypeFromArray:[NSArray arrayWithObject:NSStringPboardType]] != nil;
}

Common::String getTextFromClipboardMacOSX() {
	if (!hasTextInClipboardMacOSX())
		return Common::String();
	// Note: on OS X 10.6 and above it is recommanded to use NSPasteboardTypeString rather than NSStringPboardType.
	// But since we still target older version use NSStringPboardType.
	NSPasteboard *pb = [NSPasteboard generalPasteboard];
	NSString* str = [pb  stringForType:NSStringPboardType];
	if (str == nil)
		return Common::String();
	// If the string cannot be represented using the requested encoding we get a null pointer below.
	// This is fine as ScummVM would not know what to do with non-ASCII characters (although maybe
	// we should use NSISOLatin1StringEncoding?).
	return Common::String([str cStringUsingEncoding:NSASCIIStringEncoding]);
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
