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

#ifndef SHERLOCK_TALK_H
#define SHERLOCK_TALK_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/stack.h"
#include "sherlock/objects.h"
#include "sherlock/saveload.h"

namespace Sherlock {

#define SPEAKER_REMOVE 0x80
#define MAX_TALK_SEQUENCES 11

enum {
	OP_SWITCH_SPEAKER			= 0,
	OP_RUN_CANIMATION			= 1,
	OP_ASSIGN_PORTRAIT_LOCATION = 2,
	OP_PAUSE					= 3,
	OP_REMOVE_PORTRAIT			= 4,
	OP_CLEAR_WINDOW				= 5,
	OP_ADJUST_OBJ_SEQUENCE		= 6,
	OP_WALK_TO_COORDS			= 7,
	OP_PAUSE_WITHOUT_CONTROL	= 8,
	OP_BANISH_WINDOW			= 9,
	OP_SUMMON_WINDOW			= 10,
	OP_SET_FLAG					= 11,
	OP_SFX_COMMAND				= 12,
	OP_TOGGLE_OBJECT			= 13,
	OP_STEALTH_MODE_ACTIVE		= 14,
	OP_IF_STATEMENT				= 15,
	OP_ELSE_STATEMENT			= 16,
	OP_END_IF_STATEMENT			= 17,
	OP_STEALTH_MODE_DEACTIVATE	= 18,
	OP_TURN_HOLMES_OFF			= 19,
	OP_TURN_HOLMES_ON			= 20,
	OP_GOTO_SCENE				= 21,
	OP_PLAY_PROLOGUE			= 22,
	OP_ADD_ITEM_TO_INVENTORY	= 23,
	OP_SET_OBJECT				= 24,
	OP_CALL_TALK_FILE			= 25,
	OP_MOVE_MOUSE				= 26,
	OP_DISPLAY_INFO_LINE		= 27,
	OP_CLEAR_INFO_LINE			= 28,
	OP_WALK_TO_CANIMATION		= 29,
	OP_REMOVE_ITEM_FROM_INVENTORY = 30,
	OP_ENABLE_END_KEY			= 31,
	OP_DISABLE_END_KEY			= 32,
	OP_END_TEXT_WINDOW			= 33,

	OP_MOUSE_OFF_ON				= 34,
	OP_SET_WALK_CONTROL			= 35,
	OP_SET_TALK_SEQUENCE		= 36,
	OP_PLAY_SONG				= 37,
	OP_WALK_HOLMES_AND_NPC_TO_CANIM = 38,
	OP_SET_NPC_PATH_DEST		= 39,
	OP_NEXT_SONG				= 40,
	OP_SET_NPC_PATH_PAUSE		= 41,
	OP_NEED_PASSWORD			= 42,
	OP_SET_SCENE_ENTRY_FLAG		= 43,
	OP_WALK_NPC_TO_CANIM		= 44,
	OP_WALK_NPC_TO_COORDS		= 45,
	OP_WALK_HOLMES_AND_NPC_TO_COORDS = 46,
	OP_SET_NPC_TALK_FILE		= 47,
	OP_TURN_NPC_OFF				= 48,
	OP_TURN_NPC_ON				= 49,
	OP_NPC_DESC_ON_OFF			= 50,
	OP_NPC_PATH_PAUSE_TAKING_NOTES	= 51,
	OP_NPC_PATH_PAUSE_LOOKING_HOLMES = 52,
	OP_ENABLE_TALK_INTERRUPTS	= 53,
	OP_DISABLE_TALK_INTERRUPTS	= 54,
	OP_SET_NPC_INFO_LINE		= 55,
	OP_SET_NPC_POSITION			= 56,
	OP_NPC_PATH_LABEL			= 57,
	OP_PATH_GOTO_LABEL			= 58,
	OP_PATH_IF_FLAG_GOTO_LABEL	= 59,
	OP_NPC_WALK_GRAPHICS		= 60,
	OP_NPC_VERB					= 61,
	OP_NPC_VERB_CANIM			= 62,
	OP_NPC_VERB_SCRIPT			= 63,
	OP_RESTORE_PEOPLE_SEQUENCE	= 64,
	OP_NPC_VERB_TARGET			= 65,
	OP_TURN_SOUNDS_OFF			= 66,
	OP_NULL						= 67
};

enum OpcodeReturn { RET_EXIT = -1, RET_SUCCESS = 0, RET_CONTINUE = 1 };

class SherlockEngine;
class Talk;
namespace Scalpel { class ScalpelUserInterface; }

typedef OpcodeReturn(Talk::*OpcodeMethod)(const byte *&str);

struct SequenceEntry {
	int _objNum;
	Common::Array<byte> _sequences;
	Object *_obj;			// Pointer to the bgshape that these values go to
	short _frameNumber;		// Frame number in frame sequence to draw
	short _sequenceNumber;	// Start frame of sequences that are repeated
	int _seqStack;			// Allows gosubs to return to calling frame
	int _seqTo;				// Allows 1-5, 8-3 type sequences encoded
	int _seqCounter;		// How many times this sequence has been executed
	int _seqCounter2;

	SequenceEntry();
};

struct ScriptStackEntry {
	Common::String _name;
	int _currentIndex;
	int _select;
};

struct Statement {
	Common::String _statement;
	Common::String _reply;
	Common::String _linkFile;
	Common::String _voiceFile;
	Common::Array<int> _required;
	Common::Array<int> _modified;
	int _portraitSide;
	int _quotient;
	int _talkMap;
	Common::Rect _talkPos;
	int _journal;

	/**
	 * Load the data for a single statement within a talk file
	 */
	void load(Common::SeekableReadStream &s, bool isRoseTattoo);
};

struct TalkHistoryEntry {
	bool _data[16];

	TalkHistoryEntry();
	bool &operator[](int index) { return _data[index]; }
};

class Talk {
	friend class Scalpel::ScalpelUserInterface;
private:
	/**
	 * Remove any voice commands from a loaded statement list
	 */
	void stripVoiceCommands();
protected:
	SherlockEngine *_vm;
	OpcodeMethod *_opcodeTable;
	Common::Stack<SequenceEntry> _savedSequences;
	Common::Stack<ScriptStackEntry> _scriptStack;
	Common::Array<TalkHistoryEntry> _talkHistory;
	int _talkIndex;
	int _scriptSelect;
	int _talkStealth;
	int _talkToFlag;
	int _scriptSaveIndex;
	int _3doSpeechIndex;

	// These fields are used solely by doScript, but are fields because all the script opcodes are
	// separate methods now, and need access to these fields
	int _yp;
	int _charCount;
	int _line;
	int _wait;
	bool _pauseFlag;
	bool _endStr, _noTextYet;
	int _seqCount;
	const byte *_scriptStart, *_scriptEnd;
protected:
	Talk(SherlockEngine *vm);

	OpcodeReturn cmdAddItemToInventory(const byte *&str);
	OpcodeReturn cmdAdjustObjectSequence(const byte *&str);
	OpcodeReturn cmdBanishWindow(const byte *&str);
	OpcodeReturn cmdDisableEndKey(const byte *&str);
	OpcodeReturn cmdEnableEndKey(const byte *&str);
	OpcodeReturn cmdEndTextWindow(const byte *&str);
	OpcodeReturn cmdHolmesOff(const byte *&str);
	OpcodeReturn cmdHolmesOn(const byte *&str);
	OpcodeReturn cmdPause(const byte *&str);
	OpcodeReturn cmdPauseWithoutControl(const byte *&str);
	OpcodeReturn cmdRemoveItemFromInventory(const byte *&str);
	OpcodeReturn cmdRunCAnimation(const byte *&str);
	OpcodeReturn cmdSetFlag(const byte *&str);
	OpcodeReturn cmdSetObject(const byte *&str);
	OpcodeReturn cmdStealthModeActivate(const byte *&str);
	OpcodeReturn cmdStealthModeDeactivate(const byte *&str);
	OpcodeReturn cmdToggleObject(const byte *&str);
	OpcodeReturn cmdWalkToCAnimation(const byte *&str);
protected:
	/**
	 * Checks if a character is an opcode
	 */
	bool isOpcode(byte checkCharacter);

	/**
	 * Form a table of the display indexes for statements
	 */
	void setTalkMap();

	/**
	 * When the talk window has been displayed, waits a period of time proportional to
	 * the amount of text that's been displayed
	 */
	virtual int waitForMore(int delay);

	/**
	 * Display the talk interface window
	 */
	virtual void talkInterface(const byte *&str) = 0;

	/**
	 * Pause when displaying a talk dialog on-screen
	 */
	virtual void talkWait(const byte *&str);

	/**
	 * Show the talk display
	 */
	virtual void showTalk() = 0;

	/**
	 * Called when a character being spoken to has no talk options to display
	 */
	virtual void nothingToSay() = 0;

	/**
	 * Called when the active speaker is switched
	 */
	virtual void switchSpeaker() {}
public:
	Common::Array<Statement> _statements;
	bool _talkToAbort;
	int _talkCounter;
	int _talkTo;
	int _scriptMoreFlag;
	bool _openTalkWindow;
	Common::String _scriptName;
	bool _moreTalkUp, _moreTalkDown;
	int _converseNum;
	const byte *_opcodes;
	int _speaker;
public:
	static Talk *init(SherlockEngine *vm);
	virtual ~Talk() {}

	/**
	 * Return a given talk statement
	 */
	Statement &operator[](int idx) { return _statements[idx]; }

	/**
	 * Called whenever a conversation or item script needs to be run. For standard conversations,
	 * it opens up a description window similar to how 'talk' does, but shows a 'reply' directly
	 * instead of waiting for a statement option.
	 * @remarks		It seems that at some point, all item scripts were set up to use this as well.
	 *	In their case, the conversation display is simply suppressed, and control is passed on to
	 *	doScript to implement whatever action is required.
	 */
	virtual void talkTo(const Common::String filename);

	/**
	 * Parses a reply for control codes and display text. The found text is printed within
	 * the text window, handles delays, animations, and animating portraits.
	 */
	void doScript(const Common::String &script);

	/**
	 * Main method for handling conversations when a character to talk to has been
	 * selected. It will make Holmes walk to the person to talk to, draws the
	 * interface window for the conversation and passes on control to give the
	 * player a list of options to make a selection from
	 */
	void initTalk(int objNum);

	/**
	 * Clear loaded talk data
	 */
	void freeTalkVars();

	/**
	 * Opens the talk file 'talk.tlk' and searches the index for the specified
	 * conversation. If found, the data for that conversation is loaded
	 */
	virtual void loadTalkFile(const Common::String &filename);

	/**
	 * Push the sequence of a background object that's an NPC that needs to be
	 * saved onto the sequence stack.
	 */
	void pushSequence(int speaker);

	/**
	 * Push the details of a passed object onto the saved sequences stack
	 */
	virtual void pushSequenceEntry(Object *obj) = 0;

	/**
	 * Clears the stack of pending object sequences associated with speakers in the scene
	 */
	virtual void clearSequences() = 0;

	/**
	 * Pops an entry off of the script stack
	 */
	void popStack();

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);

	/**
	 * Draws the interface for conversation display
	 */
	virtual void drawInterface() {}

	/**
	 * Display a list of statements in a window at the bottom of the screen that the
	 * player can select from.
	 */
	virtual bool displayTalk(bool slamIt) { return false; }

	/**
	 * Prints a single conversation option in the interface window
	 */
	virtual int talkLine(int lineNum, int stateNum, byte color, int lineY, bool slamIt) { return 0; }

	/**
	 * Pulls a background object sequence from the sequence stack and restore's the
	 * object's sequence
	 */
	virtual void pullSequence(int slot = -1) = 0;

	/**
	 * Returns true if the script stack is empty
	 */
	virtual bool isSequencesEmpty() const = 0;
};

} // End of namespace Sherlock

#endif
