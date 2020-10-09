/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#ifndef GRIM_DEBUG_H
#define GRIM_DEBUG_H

#include "common/debug.h"
#include "common/streamdebug.h"

namespace Grim {

class Debug {
public:
	enum DebugChannel {
		Info = 1,
		Warning = 2 << 0,
		Error = 2 << 1,
		Engine = 2 << 2,
		Lua = 2 << 3,
		Bitmaps = 2 << 4,
		Models = 2 << 5,
		Actors = 2 << 6,
		Costumes = 2 << 7,
		Chores = 2 << 8,
		Fonts = 2 << 9,
		Keyframes = 2 << 10,
		Materials = 2 << 11,
		Movie = 2 << 12,
		Sound = 2 << 13,
		Scripts = 2 << 14,
		Sets = 2 << 15,
		TextObjects = 2 << 16,
		Patchr = 2 << 17,
		Lipsync = 2 << 18,
		Sprites = 2 << 19
	};

	static void registerDebugChannels();
	static bool isChannelEnabled(DebugChannel chan);

	/**
	 * Prints a message to the console (stdout), only if the specified debug channel
	 * or the channel Info are active.
	 *
	 * @param channel The debug channel to use.
	 */
	static void debug(DebugChannel channel, const char *s, ...);
	/**
	 * Prints a message to the console (sterr), only if the specified debug channel
	 * or the channel Warning are active.
	 *
	 * @param channel The debug channel to use.
	 */
	static void warning(DebugChannel channel, const char *s, ...);
	/**
	 * Prints a message to the console (stderr) and exit the program immediately,
	 * only if the specified debug channel or the channel Error are active.
	 *
	 * @param channel The debug channel to use.
	 */
	static void error(DebugChannel channel, const char *s, ...);
	/**
	 * Prints a message to the console (stderr) and exit the program immediately,
	 * only if the debug channel Error is active.
	 *
	 * @param channel The debug channel to use.
	 */
	static void error(const char *s, ...);
};

inline Debug::DebugChannel operator|(Debug::DebugChannel a, Debug::DebugChannel b) {
	return (Debug::DebugChannel)((int)a | (int) b);
}

}

#endif
