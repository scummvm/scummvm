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
	Common::HashMap<uint32, uint> _globalState;
	/** Holds the currently active ActionNodes */
	Common::List<Common::SharedPtr<ActionNode> > _activeNodes;
	/** References _globalState keys to Puzzles */
	Common::HashMap<uint32, Common::Array<Puzzle *> > _referenceTable;
	/** Holds the Puzzles that should be checked this frame */
	Common::Stack<Puzzle *> _puzzlesToCheck;
	/** Holds the currently active puzzles */
	Common::List<Puzzle> _activePuzzles;
	/** Holds the currently active controls */
	Common::List<Common::SharedPtr<Control> > _activeControls;

public:
	
	void initialize();
	void updateNodes(uint deltaTimeMillis);
	void checkPuzzleCriteria();

	uint getStateValue(uint32 key);
	void setStateValue(uint32 key, uint value);
	void addToStateValue(uint32 key, uint valueToAdd);

	void addActionNode(const Common::SharedPtr<ActionNode> &node);

	void changeLocation(char world, char room, char node, char view, uint32 x);

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
	 * @param criteria    Pointer to the Criteria object to fill
	 * @param stream      Scr file stream
	 * @return            Whether any criteria were read
	 */
	bool parseCriteria(Puzzle::Criteria *criteria, Common::SeekableReadStream &stream) const;

	/**
	 * Parses the stream into a ResultAction objects
	 * Helper method for parsePuzzle. 
	 *
	 * @param stream        Scr file stream
	 * @param actionList    The list where the results will be added
	 * @return              Created Results object
	 */
	void parseResults(Common::SeekableReadStream &stream, Common::List<Common::SharedPtr<ResultAction> > &actionList) const;

	/**
	 * Helper method for parsePuzzle. Parses the stream into a bitwise or of the StateFlags enum
	 *
	 * @param stream    Scr file stream
	 * @return          Bitwise OR of all the flags set within the puzzle
	 */
	uint parseFlags(Common::SeekableReadStream &stream) const;

	/**
	 * Helper method for parseScrFile. Parses the stream into a Control object
	 *
	 * @param line      The line initially read
	 * @param stream    Scr file stream
	 */
	bool parseControl(Common::String &line, Common::SeekableReadStream &stream, Common::SharedPtr<Control> &control);
};


} // End of namespace ZVision

#endif
