/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
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
		Warning = Info * 2,
		Error = Warning * 2,
		Engine = Error * 2,
		Lua = Engine * 2,
		Bitmaps = Lua * 2,
		Models = Bitmaps * 2,
		Actors = Models * 2,
		Costumes = Actors * 2,
		Chores = Costumes * 2,
		Fonts = Chores * 2,
		Keyframes = Fonts * 2,
		Materials = Keyframes * 2,
		Movie = Materials * 2,
		Imuse = Movie * 2,
		Scripts = Imuse * 2,
		Sets = Scripts * 2,
		TextObjects = Sets * 2,
		All = Info | Warning | Error | Engine | Lua | Bitmaps | Models |
		      Actors | Costumes | Chores | Fonts | Keyframes | Materials | Movie | Imuse |
		      Scripts | Sets | TextObjects
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
