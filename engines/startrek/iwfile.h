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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_IWFILE_H
#define STARTREK_IWFILE_H

#include "startrek/startrek.h"

/**
 * Files with the ".iw" extension define a set of "key points" through a room used for
 * pathing. This is a basic interface for that.
 */
namespace StarTrek {

const int MAX_KEY_POSITIONS = 32;

class IWFile {
public:
	IWFile(StarTrekEngine *vm, const Common::String &filename);

	int getNumEntries() {
		return _numEntries;
	}
	/**
	 * Returns the index of the nearest "key position" in the room that an object can walk to
	 * (in a straight line) from a given position.
	 */
	int getClosestKeyPosition(int16 x, int16 y);

	///< List of "key positions" used for pathing.
	Common::Point _keyPositions[MAX_KEY_POSITIONS];

	///< _iwEntries[i][j] is the index of the next key position to move to, when one is
	///   already at key position "i" and is trying to get to key position "j".
	byte _iwEntries[MAX_KEY_POSITIONS][MAX_KEY_POSITIONS];

private:
	StarTrekEngine *_vm;

	uint16 _numEntries;
};

} // End of namespace StarTrek

#endif
