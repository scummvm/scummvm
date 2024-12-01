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

#include "backends/platform/sdl/macosx/macosx_wrapper.h"
#include "backends/platform/sdl/macosx/macosx-compat.h"

#include <Foundation/NSArray.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSPathUtilities.h>
#include <AvailabilityMacros.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFBundle.h>

Common::String getDesktopPathMacOSX() {
	// The recommended method is to use NSFileManager.
	// NSUrl *url = [[[NSFileManager defaultManager] URLsForDirectory:NSDesktopDirectory inDomains:NSUserDomainMask] firstObject];
	// However it is only available in OS X 10.6+. So use NSSearchPathForDirectoriesInDomains instead (available since OS X 10.0)
	// [NSArray firstObject] is also only available in OS X 10.6+. So we need to use [NSArray count] and [NSArray objectAtIndex:]
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDesktopDirectory, NSUserDomainMask, YES);
	if ([paths count] == 0)
		return Common::String();
	NSString *path = [paths objectAtIndex:0];
	if (path == nil)
		return Common::String();
	return Common::String([path fileSystemRepresentation]);
}

Common::String getDocumentsPathMacOSX() {
	// See comment in getDesktopPathMacOSX()
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	if ([paths count] == 0)
		return Common::String();
	NSString *path = [paths objectAtIndex:0];
	if (path == nil)
		return Common::String();
	return Common::String([path fileSystemRepresentation]);
}

Common::String getResourceAppBundlePathMacOSX() {
	NSString *bundlePath = [[NSBundle mainBundle] resourcePath];
	if (bundlePath == nil)
		return Common::String();
	return Common::String([bundlePath fileSystemRepresentation]);
}

Common::String getAppSupportPathMacOSX() {
	// See comments in getDesktopPathMacOSX() as we use the same methods
	NSArray *paths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
	if ([paths count] == 0)
		return Common::String();
	NSString *path = [paths objectAtIndex:0];
	if (path == nil)
		return Common::String();
	return Common::String([path fileSystemRepresentation]) + "/ScummVM";
}

Common::String getMacBundleName() {
	NSString *appName = [[[NSBundle mainBundle] infoDictionary] objectForKey:(id)kCFBundleNameKey];
	if (!appName)
		return Common::String("ScummVM");
	return Common::String([appName UTF8String]);
}
