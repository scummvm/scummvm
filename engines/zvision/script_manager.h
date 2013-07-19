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

#include "common/hashmap.h"
#include "common/stack.h"

#include "zvision/puzzle.h"
#include "zvision/control.h"

namespace Common {
class String;
class SeekableReadStream;
}

namespace ZVision {

class ZVision;
class ActionNode;

class ScriptManager {
public:
	ScriptManager(ZVision *engine);

private:
	ZVision *_engine;
	/** 
	 * Holds the global state variable. Do NOT directly modify this. Use the accessors and 
	 * mutators getStateValue() and setStateValue(). This ensures that Puzzles that reference a 
	 * particular state key are checked after the key is modified.
	 */
	Common::HashMap<uint32, byte> _globalState;
	/** Holds the currently active ActionNodes */
	Common::List<ActionNode *> _activeNodes;
	/** References _globalState keys to Puzzles */
	Common::HashMap<uint32, Common::Array<Puzzle *> > _referenceTable;
	/** Holds the Puzzles that should be checked this frame */
	Common::Stack<Puzzle *> _puzzlesToCheck;
	/** Holds the currently active puzzles */
	Common::List<Puzzle> _activePuzzles;
	/** Holds the currently active controls */
	Common::List<Control> _activeControls;

public:
	
	void initialize();
	void updateNodes(uint32 deltaTimeMillis);
	void checkPuzzleCriteria();

	byte getStateValue(uint32 key);
	void setStateValue(uint32 key, byte value);
	void addToStateValue(uint32 key, byte valueToAdd);

	void addActionNode(ActionNode *node);

private:
	void createReferenceTable();

	/**
	 * Parses a script file into triggers and events
	 *
	 * @param fileName		Name of the .scr file
	 */
	void parseScrFile(Common::String fileName);

	/**
	 * Parses the stream into a Puzzle object
	 * Helper method for parseScrFile. 
	 *
	 * @param puzzle    The object to store what is parsed
	 * @param stream    Scr file stream
	 */
	void parsePuzzle(Puzzle &puzzle, Common::SeekableReadStream &stream);

	/**
	 * Parses the stream into a Criteria object
	 * Helper method for parsePuzzle. 
	 *
	 * @param stream    Scr file stream
	 * @return          Created Criteria object
	 */
	Criteria parseCriteria(Common::SeekableReadStream &stream) const;

	/**
	 * Parses the stream into a Results object
	 * Helper method for parsePuzzle. 
	 *
	 * @param stream    Scr file stream
	 * @return          Created Results object
	 */
	void parseResult(Common::SeekableReadStream &stream, Common::List<ResultAction *> &actionList) const;

	/**
	 * Helper method for parsePuzzle. Parses the stream into a bitwise or of the StateFlags enum
	 *
	 * @param stream    Scr file stream
	 * @return          Bitwise or of all the flags set within the puzzle
	 */
	byte parseFlags(Common::SeekableReadStream &stream) const;

	/**
	 * Helper method for parseScrFile. Parses the stream into a Control object
	 *
	 * @param control    The object to store what is parsed
	 * @param stream     Scr file stream
	 */
	void parseControl(Control &control, Common::SeekableReadStream &stream);
};


} // End of namespace ZVision

#endif
