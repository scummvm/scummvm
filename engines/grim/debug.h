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

#ifndef GRIM_DEBUG_H
#define GRIM_DEBUG_H

namespace Grim {

class Debug {
public:
	enum DebugChannel {
		Actors = 1,
		Bitmaps,
		Chores,
		Costumes,
		Engine,
		Fonts,
		Keyframes,
		Lipsync,
		LuaTrace,
		Materials,
		Models,
		Movie,
		Patchr,
		Scripts,
		Sets,
		Sound,
		Sprites,
		TextObjects
	};

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
};

}

#endif
