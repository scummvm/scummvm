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
#include "zvision/scripting/scripting_effect.h"

#include "common/hashmap.h"
#include "common/queue.h"
#include "common/events.h"

namespace Common {
class String;
class SeekableReadStream;
}

namespace ZVision {

class ZVision;

enum StateKey {
	StateKey_World = 3,
	StateKey_Room  = 4,
	StateKey_Node  = 5,
	StateKey_View  = 6,
	StateKey_ViewPos = 7,
	StateKey_KeyPress = 8,
	StateKey_InventoryItem = 9,
	StateKey_LMouse = 10,
	StateKey_NotSet = 11, // This key doesn't set
	StateKey_Rounds = 12,
	StateKey_Venus = 13,
	StateKey_RMouse = 18,
	StateKey_MenuState = 19,
	StateKey_RestoreFlag = 20,
	StateKey_Quitting = 39,
	StateKey_LastWorld = 40,
	StateKey_LastRoom = 41,
	StateKey_LastNode = 42,
	StateKey_LastView = 43,
	StateKey_LastViewPos = 44,
	StateKey_Menu_LastWorld = 45,
	StateKey_Menu_LastRoom = 46,
	StateKey_Menu_LastNode = 47,
	StateKey_Menu_LastView = 48,
	StateKey_Menu_LastViewPos = 49,
	StateKey_KbdRotateSpeed = 50,
	StateKey_Subtitles = 51,
	StateKey_StreamSkipKey = 52,
	StateKey_RotateSpeed = 53,
	StateKey_Volume = 56,
	StateKey_Qsound = 57,
	StateKey_VenusEnable = 58,
	StateKey_HighQuality = 59,
	StateKey_VideoLineSkip = 65,
	StateKey_Platform = 66,
	StateKey_InstallLevel = 67,
	StateKey_CountryCode = 68,
	StateKey_CPU = 69,
	StateKey_MovieCursor = 70,
	StateKey_NoTurnAnim = 71,
	StateKey_WIN958 = 72,
	StateKey_ShowErrorDlg = 73,
	StateKey_DebugCheats = 74,
	StateKey_JapanFonts = 75,
	StateKey_ExecScopeStyle = 76,
	StateKey_Brightness = 77,
	StateKey_MPEGMovies = 78,
	StateKey_EF9_R = 91,
	StateKey_EF9_G = 92,
	StateKey_EF9_B = 93,
	StateKey_EF9_Speed = 94,
	StateKey_Inv_Cnt_Slot = 100,
	StateKey_Inv_1_Slot = 101,
	StateKey_Inv_49_Slot = 149,
	// ZGI only
	StateKey_Inv_TotalSlots = 150,
	StateKey_Inv_StartSlot = 151,
	StateKey_Spell_1 = 191,
	StateKey_Active_Spell = 205,
	StateKey_Reversed_Spellbooc = 206
};

struct Location {
	Location() : world('g'), room('a'), node('r'), view('y'), offset(0) {}

	char world;
	char room;
	char node;
	char view;
	uint32 offset;
};

inline bool operator==(const Location& lhs, const Location& rhs) {
	return (
		lhs.world == rhs.world &&
		lhs.room == rhs.room &&
		lhs.node == rhs.node &&
		lhs.view == rhs.view
	);
}

inline bool operator==(const Location& lhs, const char* rhs) {
	Common::String lhsStr = Common::String::format("%c%c%c%c", lhs.world, lhs.room, lhs.node, lhs.view);
	return lhsStr == rhs;
}

inline bool operator!=(const Location& lhs, const Location& rhs) {
	return !(lhs == rhs);
}

inline bool operator!=(const Location& lhs, const char* rhs) {
	return !(lhs == rhs);
}

typedef Common::List<Puzzle *> PuzzleList;
typedef Common::Queue<Puzzle *> PuzzleQueue;
typedef Common::List<Control *> ControlList;
typedef Common::HashMap<uint32, int32> StateMap;
typedef Common::List<ScriptingEffect *> SideFXList;
typedef Common::List<Common::Event> EventList;

class ScriptManager {
public:
	ScriptManager(ZVision *engine);
	~ScriptManager();

private:
	ZVision *_engine;

	struct ScriptScope {
		uint32 procCount;

		PuzzleList *scopeQueue; // For adding puzzles to queue
		PuzzleList *execQueue;  // Switch to it when execute
		PuzzleList privQueueOne;
		PuzzleList privQueueTwo;

		PuzzleList  puzzles;
		ControlList controls;
	};

	struct PuzzleRef {
		Puzzle *puz;
		ScriptScope *scope;
	};

	typedef Common::HashMap<uint32, Common::Array<PuzzleRef> > PuzzleMap;

	/**
	 * Holds the global state variable. Do NOT directly modify this. Use the accessors and
	 * mutators getStateValue() and setStateValue(). This ensures that Puzzles that reference a
	 * particular state key are checked after the key is modified.
	 */
	StateMap _globalState;
	/** Holds execute flags */
	StateMap _globalStateFlags;
	/** References _globalState keys to Puzzles */
	PuzzleMap _referenceTable;
	/** Holds the currently active controls */
	ControlList *_activeControls;

	EventList _controlEvents;

	ScriptScope universe;
	ScriptScope world;
	ScriptScope room;
	ScriptScope nodeview;

	/** Holds the currently active timers, musics, other */
	SideFXList _activeSideFx;

	Location _currentLocation;
	Location _nextLocation;
	int _changeLocationDelayCycles;

	uint32 _currentlyFocusedControl;

public:
	void initialize();
	void update(uint deltaTimeMillis);
	void queuePuzzles(uint32 key);

	int getStateValue(uint32 key);
	void setStateValue(uint32 key, int value);

	uint getStateFlag(uint32 key);
	void setStateFlag(uint32 key, uint value);
	void unsetStateFlag(uint32 key, uint value);

	void addControl(Control *control);
	Control *getControl(uint32 key);

	void enableControl(uint32 key);
	void disableControl(uint32 key);

	void focusControl(uint32 key);
	// Only change focus control without call focus/unfocus.
	void setFocusControlKey(uint32 key);

	void addSideFX(ScriptingEffect *fx);
	ScriptingEffect *getSideFX(uint32 key);
	void deleteSideFx(uint32 key);
	void stopSideFx(uint32 key);
	void killSideFx(uint32 key);
	void killSideFxType(ScriptingEffect::ScriptingEffectType type);

	void addEvent(Common::Event);
	void flushEvent(Common::EventType type);

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

	/** Mark next location */
	void changeLocation(char world, char room, char node, char view, uint32 offset);
	void changeLocation(const Location &_newLocation);

	void serialize(Common::WriteStream *stream);
	void deserialize(Common::SeekableReadStream *stream);

	Location getCurrentLocation() const;
	Location getLastLocation();
	Location getLastMenuLocation();

	/**
	 * Removes any line comments using '#' as a sequence start.
	 * Then removes any trailing and leading 'whitespace' using String::trim()
	 * Note: String::trim uses isspace() to determine what is whitespace and what is not.
	 *
	 * @param string    The string to modify. It is modified in place
	 */
	void trimCommentsAndWhiteSpace(Common::String *string) const;

private:
	void referenceTableAddPuzzle(uint32 key, PuzzleRef ref);
	void addPuzzlesToReferenceTable(ScriptScope &scope);
	void updateNodes(uint deltaTimeMillis);
	void updateControls(uint deltaTimeMillis);
	bool checkPuzzleCriteria(Puzzle *puzzle, uint counter);
	void cleanStateTable();
	void cleanScriptScope(ScriptScope &scope);
	bool execScope(ScriptScope &scope);

	/** Perform change location */
	void ChangeLocationReal(bool isLoading);

	int8 inventoryGetCount();
	void inventorySetCount(int8 cnt);
	int16 inventoryGetItem(int8 id);
	void inventorySetItem(int8 id, int16 item);

	void setStateFlagSilent(uint32 key, uint value);
	void setStateValueSilent(uint32 key, int value);

public:
	void inventoryAdd(int16 item);
	void inventoryDrop(int16 item);
	void inventoryCycle();

private:
	/**
	 * Parses a script file into triggers and events
	 *
	 * @param fileName    Name of the .scr file
	 * @param isGlobal    Are the puzzles included in the file global (true). AKA, the won't be purged during location changes
	 */
	void parseScrFile(const Common::String &fileName, ScriptScope &scope);

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
	 * @param key         Puzzle key (for workarounds)
	 * @return            Whether any criteria were read
	 */
	bool parseCriteria(Common::SeekableReadStream &stream, Common::List<Common::List<Puzzle::CriteriaEntry> > &criteriaList, uint32 key) const;

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
	Control *parseControl(Common::String &line, Common::SeekableReadStream &stream);
};

class ValueSlot {
public:
	ValueSlot(ScriptManager *scriptManager, const char *slotValue);
	int16 getValue();
private:
	int16 value;
	bool slot;
	ScriptManager *_scriptManager;
};

} // End of namespace ZVision

#endif
