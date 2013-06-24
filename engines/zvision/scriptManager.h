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

#ifndef ZVISION_SCRIPT_MANAGER_H
#define ZVISION_SCRIPT_MANAGER_H

#include "common/str.h"
#include "common/stream.h"
#include "common/hashmap.h"

#include "zvision/puzzle.h"
#include "zvision/control.h"

namespace ZVision {

class ScriptManager {
public:
	ScriptManager();
	~ScriptManager();

private:
	Common::HashMap<uint32, byte> _globalState;

	/**
	 * Parses a script file into triggers and events
	 *
	 * @param fileName		Name of the .scr file
	 */
	void parseScrFile(Common::String fileName);

	/**
	 * Helper method for parseScrFile. Parses the stream into a Puzzle object
	 *
	 * @param puzzle	The object to store what is parsed
	 * @param stream	Scr file stream
	 */
	void parsePuzzle(Puzzle *puzzle, Common::SeekableReadStream &stream);

	/**
	 * Helper method for parsePuzzle. Parses the stream into a Criteria object
	 *
	 * @param stream	Scr file stream
	 * @return			Created Criteria object
	 */
	Criteria parseCriteria(Common::SeekableReadStream &stream) const;

	/**
	 * Helper method for parsePuzzle. Parses the stream into a Results object
	 *
	 * @param stream	Scr file stream
	 * @return			Created Results object
	 */
	Result parseResult(Common::SeekableReadStream &stream) const;

	/**
	 * Helper method for parseResults. Parses a number of strings into Object types.
	 *
	 * @param result			Results object to store the arguments in
	 * @param types				The type of the each of the arguments. IE. BOOL, UINT32, etc.
	 * @param numberOfArgs		The number of arguments. This has to equal the length of 'types' AND the number of optional arguments passed.
	 * @param 					String arguments wanting to be parsed
	 */
	void parseResultArguments(Result &result, const ObjectType *types, int numberOfArgs, ...) const;

	/**
	 * Helper method for parsePuzzle. Parses the stream into a bitwise or of the StateFlags enum
	 *
	 * @param stream	Scr file stream
	 * @return			Bitwise or of all the flags set within the puzzle
	 */
	byte parseFlags(Common::SeekableReadStream &stream) const;

	/**
	 * Helper method for parseScrFile. Parses the stream into a Control object
	 *
	 * @param control	The object to store what is parsed
	 * @param stream	Scr file stream
	 */
	void parseControl(Control *control, Common::SeekableReadStream &stream);
};


} // End of namespace ZVision

#endif
