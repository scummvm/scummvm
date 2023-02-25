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

#include "backends/platform/sdl/macosx/macosx.h"

#include <Foundation/NSBundle.h>
#include <Foundation/NSFileManager.h>

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

	// If the bundle contains a game directory, auto-detect it
	NSString *gamePath = [[bundle resourcePath] stringByAppendingPathComponent:@"game"];
	BOOL isDir = false;
	BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:gamePath isDirectory:&isDir];
	if (exists && isDir) {
		// Ause auto-detection
		command = "auto-detect";
		settings["path"] = [gamePath fileSystemRepresentation];
	}
}
