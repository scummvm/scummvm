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

#ifndef ZVISION_SCRIPT_MANAGER_H
#define ZVISION_SCRIPT_MANAGER_H

#include "zvision/scripting/puzzle.h"
#include "zvision/scripting/control.h"

#include "common/hashmap.h"
#include "common/queue.h"


namespace Common {
class String;
class SeekableReadStream;
}

namespace ZVision {

class ZVision;

struct Location {
	Location() : world('g'), room('a'), node('r'), view('y'), offset(0) {}

	char world;
	char room;
	char node;
	char view;
	uint32 offset;
};

typedef Common::HashMap<uint32, Common::Array<Puzzle *> > PuzzleMap;
typedef Common::List<Puzzle *> PuzzleList;
typedef Common::Queue<Puzzle *> PuzzleQueue;
typedef Common::List<Control *> ControlList;
typedef Common::HashMap<uint32, uint32> StateMap;
typedef Common::HashMap<uint32, uint> StateFlagMap;

class ScriptManager {
public:
	ScriptManager(ZVision *engine);
	~ScriptManager();

public:
	enum StateFlags {
		ONCE_PER_INST = 0x01,
		DO_ME_NOW = 0x02, // Somewhat useless flag since anything that needs to be done immediately has no criteria
		DISABLED = 0x04
	};

private:
	ZVision *_engine;
	/**
	 * Holds the global state variable. Do NOT directly modify this. Use the accessors and
	 * mutators getStateValue() and setStateValue(). This ensures that Puzzles that reference a
	 * particular state key are checked after the key is modified.
	 */
	StateMap _globalState;
	/**
	 * Holds the flags for the global states. This is used to enable/disable puzzles and/or
	 * controls as well as which puzzles should are allowed to be re-executed
	 */
	StateFlagMap _globalStateFlags;
	/** References _globalState keys to Puzzles */
	PuzzleMap _referenceTable;
	/** Holds the Puzzles that should be checked this frame */
	PuzzleQueue _puzzlesToCheck;
	/** Holds the currently active puzzles */
	PuzzleList _activePuzzles;
	/** Holds the global puzzles */
	PuzzleList _globalPuzzles;
	/** Holds the currently active controls */
	ControlList _activeControls;

	Location _currentLocation;

	uint32 _currentlyFocusedControl;

public:
	void initialize();
	void update(uint deltaTimeMillis);

	uint getStateValue(uint32 key);
	void setStateValue(uint32 key, uint value);
	void addToStateValue(uint32 key, uint valueToAdd);

	uint getStateFlags(uint32 key);
	void setStateFlags(uint32 key, uint flags);

	void addControl(Control *control);
	Control *getControl(uint32 key);

	void focusControl(uint32 key);

	/**
	 * Called when LeftMouse is pushed.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	void onMouseDown(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	/**
	 * Called when LeftMouse is lifted.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	void onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	/**
	 * Called on every MouseMove.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 * @return                           Was the cursor changed?
	 */
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	/**
	 * Called when a key is pressed.
	 *
	 * @param keycode    The key that was pressed
	 */
	void onKeyDown(Common::KeyState keyState);
	/**
	 * Called when a key is released.
	 *
	 * @param keycode    The key that was pressed
	 */
	void onKeyUp(Common::KeyState keyState);

	void changeLocation(char world, char room, char node, char view, uint32 offset);

	void serializeStateTable(Common::WriteStream *stream);
	void deserializeStateTable(Common::SeekableReadStream *stream);
	void serializeControls(Common::WriteStream *stream);
	void deserializeControls(Common::SeekableReadStream *stream);

	Location getCurrentLocation() const;

private:
	void createReferenceTable();
	void updateNodes(uint deltaTimeMillis);
	void checkPuzzleCriteria();
	void cleanStateTable();

// TODO: Make this private. It was only made public so Console::cmdParseAllScrFiles() could use it
public:
	/**
	 * Parses a script file into triggers and events
	 *
	 * @param fileName    Name of the .scr file
	 * @param isGlobal    Are the puzzles included in the file global (true). AKA, the won't be purged during location changes
	 */
	void parseScrFile(const Common::String &fileName, bool isGlobal = false);

private:
	/**
	 * Parses the stream into a Puzzle object
	 * Helper method for parseScrFile.
	 *
	 * @param puzzle    The object to store what is parsed
	 * @param stream    Scr file stream
	 */
	void parsePuzzle(Puzzle *puzzle, Common::SeekableReadStream &stream);

	/**
	 * Parses the stream into a Criteria object
	 * Helper method for parsePuzzle.
	 *
	 * @param criteria    Pointer to the Criteria object to fill
	 * @param stream      Scr file stream
	 * @return            Whether any criteria were read
	 */
	bool parseCriteria(Common::SeekableReadStream &stream, Common::List<Common::List<Puzzle::CriteriaEntry> > &criteriaList) const;

	/**
	 * Parses the stream into a ResultAction objects
	 * Helper method for parsePuzzle.
	 *
	 * @param stream        Scr file stream
	 * @param actionList    The list where the results will be added
	 * @return              Created Results object
	 */
	void parseResults(Common::SeekableReadStream &stream, Common::List<ResultAction *> &actionList) const;

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
	void parseControl(Common::String &line, Common::SeekableReadStream &stream);
};


} // End of namespace ZVision

#endif
