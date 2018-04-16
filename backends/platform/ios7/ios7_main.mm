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

#include <UIKit/UIKit.h>
#include <Foundation/NSThread.h>

#include "backends/platform/ios7/ios7_video.h"


int iOS7_argc;
char **iOS7_argv;

int main(int argc, char **argv) {
	int returnCode;

	@autoreleasepool {
		iOS7_argc = argc;
		iOS7_argv = argv;

		returnCode = UIApplicationMain(argc, argv, @"UIApplication", @"iOS7AppDelegate");
	}

	return returnCode;
}

