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
#include "common/serializer.h"
#include "common/stream.h"
#include "common/stack.h"

namespace Sherlock {

#define MAX_TALK_SEQUENCES 11
#define MAX_TALK_FILES 500

enum {
	SWITCH_SPEAKER				= 128,
	RUN_CANIMATION				= 129,
	ASSIGN_PORTRAIT_LOCATION	= 130,
	PAUSE						= 131,
	REMOVE_PORTRAIT				= 132,
	CLEAR_WINDOW				= 133,
	ADJUST_OBJ_SEQUENCE			= 134,
	WALK_TO_COORDS				= 135,
	PAUSE_WITHOUT_CONTROL		= 136,
	BANISH_WINDOW				= 137,
	SUMMON_WINDOW				= 138,
	SET_FLAG					= 139,
	SFX_COMMAND					= 140,
	TOGGLE_OBJECT				= 141,
	STEALTH_MODE_ACTIVE			= 142,
	IF_STATEMENT				= 143,
	ELSE_STATEMENT				= 144,
	END_IF_STATEMENT			= 145,
	STEALTH_MODE_DEACTIVATE		= 146,
	TURN_HOLMES_OFF				= 147,
	TURN_HOLMES_ON				= 148,
	GOTO_SCENE					= 149,
	PLAY_PROLOGUE				= 150,
	ADD_ITEM_TO_INVENTORY		= 151,
	SET_OBJECT					= 152,
	CALL_TALK_FILE				= 153,
	MOVE_MOUSE					= 154,
	DISPLAY_INFO_LINE			= 155,
	CLEAR_INFO_LINE				= 156,
	WALK_TO_CANIMATION			= 157,
	REMOVE_ITEM_FROM_INVENTORY	= 158,
	ENABLE_END_KEY				= 159,
	DISABLE_END_KEY				= 160,
	CARRIAGE_RETURN				= 161
};

struct SequenceEntry {
	int _objNum;
	Common::Array<byte> _sequences;
	int _frameNumber;
	int _seqTo;

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

	/**
	 * Load the data for a single statement within a talk file
	 */
	void synchronize(Common::SeekableReadStream &s);
};

struct TalkHistoryEntry {
	bool _data[16];

	TalkHistoryEntry();
	bool &operator[](int index) { return _data[index]; }
};

struct TalkSequences {
	byte _data[MAX_TALK_SEQUENCES];

	TalkSequences() { clear(); }
	TalkSequences(const byte *data);

	byte &operator[](int idx) { return _data[idx]; }
	void clear();
};

class SherlockEngine;
class UserInterface;

class Talk {
	friend class UserInterface;
private:
	SherlockEngine *_vm;
	Common::Stack<SequenceEntry> _savedSequences;
	Common::Stack<SequenceEntry> _sequenceStack;
	Common::Stack<ScriptStackEntry> _scriptStack;
	Common::Array<Statement> _statements;
	TalkHistoryEntry _talkHistory[MAX_TALK_FILES];
	int _speaker;
	int _talkIndex;
	int _scriptSelect;
	int _talkStealth;
	int _talkToFlag;
	int _scriptSaveIndex;
private:
	/**
	 * Remove any voice commands from a loaded statement list
	 */
	void stripVoiceCommands();
	
	/**
	 * Form a table of the display indexes for statements
	 */
	void setTalkMap();

	/**
	 * Display a list of statements in a window at the bottom of the screen that the
	 * player can select from.
	 */
	bool displayTalk(bool slamIt);

	/**
	 * Prints a single conversation option in the interface window
	 */
	int talkLine(int lineNum, int stateNum, byte color, int lineY, bool slamIt);

	/**
	 * Parses a reply for control codes and display text. The found text is printed within
	 * the text window, handles delays, animations, and animating portraits.
	 */
	void doScript(const Common::String &script);

	/**
	 * When the talk window has been displayed, waits a period of time proportional to
	 * the amount of text that's been displayed
	 */
	int waitForMore(int delay);
public:
	bool _talkToAbort;
	int _talkCounter;
	int _talkTo;
	int _scriptMoreFlag;
	Common::String _scriptName;
	bool _moreTalkUp, _moreTalkDown;
	int _converseNum;
public:
	Talk(SherlockEngine *vm);

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
	void talkTo(const Common::String &filename);

	/**
	 * Main method for handling conversations when a character to talk to has been
	 * selected. It will make Holmes walk to the person to talk to, draws the
	 * interface window for the conversation and passes on control to give the
	 * player a list of options to make a selection from
	 */
	void talk(int objNum);

	/**
	 * Clear loaded talk data
	 */
	void freeTalkVars();

	/**
	 * Draws the interface for conversation display
	 */
	void drawInterface();

	/**
	 * Opens the talk file 'talk.tlk' and searches the index for the specified
	 * conversation. If found, the data for that conversation is loaded
	 */
	void loadTalkFile(const Common::String &filename);

	/**
	 * Change the sequence of a background object corresponding to a given speaker.
	 * The new sequence will display the character as "listening"
	 */
	void setStillSeq(int speaker);

	/**
	 * Clears the stack of pending object sequences associated with speakers in the scene
	 */
	void clearSequences();
	
	/**
	 * Pulls a background object sequence from the sequence stack and restore's the
	 * object's sequence
	 */
	void pullSequence();

	/**
	 * Push the sequence of a background object that's an NPC that needs to be
	 * saved onto the sequence stack.
	 */
	void pushSequence(int speaker);
	
	/**
	 * Change the sequence of the scene background object associated with the current speaker.
	 */
	void setSequence(int speaker);

	/**
	 * Returns true if the script stack is empty
	 */
	bool isSequencesEmpty() const { return _scriptStack.empty(); }

	/**
	 * Pops an entry off of the script stack
	 */
	void popStack();

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Common::Serializer &s);
};

} // End of namespace Sherlock

#endif
