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

#include <Foundation/NSArray.h>
#include <Foundation/NSURL.h>
#include <Foundation/NSFileManager.h>
#include <Foundation/NSBundle.h>
#include <Foundation/NSPathUtilities.h>
#include "backends/platform/ios7/ios7_common.h"
#include <common/str.h>

Common::String iOS7_getDocumentsDir() {
#if TARGET_OS_IOS
	NSURL *url = [[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] firstObject];
#else
	NSURL *url = [[[NSFileManager defaultManager] URLsForDirectory:NSCachesDirectory inDomains:NSUserDomainMask] firstObject];
#endif
	return Common::String([url fileSystemRepresentation]);
}

Common::String iOS7_getAppBundleDir() {
	NSString *bundlePath = [[NSBundle mainBundle] resourcePath];
	if (bundlePath == nil)
		return Common::String();
	return Common::String([bundlePath fileSystemRepresentation]);
}
