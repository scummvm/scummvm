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

#ifndef SHERLOCK_SCALPEL_TALK_H
#define SHERLOCK_SCALPEL_TALK_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/stack.h"
#include "sherlock/talk.h"

namespace Sherlock {

namespace Scalpel {

class ScalpelTalk : public Talk {
private:
	Common::Stack<SequenceEntry> _sequenceStack;

	/**
	 * Get the center position for the current speaker, if any
	 */
	Common::Point get3doPortraitPosition() const;

	OpcodeReturn cmdSwitchSpeaker(const byte *&str);
	OpcodeReturn cmdAssignPortraitLocation(const byte *&str);
	OpcodeReturn cmdGotoScene(const byte *&str);
	OpcodeReturn cmdCallTalkFile(const byte *&str);
	OpcodeReturn cmdClearInfoLine(const byte *&str);
	OpcodeReturn cmdClearWindow(const byte *&str);
	OpcodeReturn cmdDisplayInfoLine(const byte *&str);
	OpcodeReturn cmdElse(const byte *&str);
	OpcodeReturn cmdIf(const byte *&str);
	OpcodeReturn cmdMoveMouse(const byte *&str);
	OpcodeReturn cmdPlayPrologue(const byte *&str);
	OpcodeReturn cmdRemovePortrait(const byte *&str);
	OpcodeReturn cmdSfxCommand(const byte *&str);
	OpcodeReturn cmdSummonWindow(const byte *&str);
	OpcodeReturn cmdWalkToCoords(const byte *&str);
protected:
	/**
	 * Display the talk interface window
	 */
	virtual void talkInterface(const byte *&str);

	/**
	 * Pause when displaying a talk dialog on-screen
	 */
	virtual void talkWait(const byte *&str);

	/**
	 * Called when the active speaker is switched
	 */
	virtual void switchSpeaker();

	/**
	 * Called when a character being spoken to has no talk options to display
	 */
	virtual void nothingToSay();

	/**
	 * Show the talk display
	 */
	virtual void showTalk();
public:
	ScalpelTalk(SherlockEngine *vm);
	virtual ~ScalpelTalk() {}

	Common::String _fixedTextWindowExit;
	Common::String _fixedTextWindowUp;
	Common::String _fixedTextWindowDown;

	byte _hotkeyWindowExit;
	byte _hotkeyWindowUp;
	byte _hotkeyWindowDown;

	/**
	 * Opens the talk file 'talk.tlk' and searches the index for the specified
	 * conversation. If found, the data for that conversation is loaded
	 */
	virtual void loadTalkFile(const Common::String &filename);

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
	 * When the talk window has been displayed, waits a period of time proportional to
	 * the amount of text that's been displayed
	 */
	virtual int waitForMore(int delay);

	/**
	 * Draws the interface for conversation display
	 */
	void drawInterface();

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
	 * Trigger to play a 3DO talk dialog movie
	 */
	bool talk3DOMovieTrigger(int subIndex);

	/**
	 * Handles skipping over bad text in conversations
	 */
	static void skipBadText(const byte *&msgP);

	/**
	 * Push the details of a passed object onto the saved sequences stack
	 */
	virtual void pushSequenceEntry(Object *obj);

	/**
	 * Pulls a background object sequence from the sequence stack and restore's the
	 * object's sequence
	 */
	virtual void pullSequence(int slot = -1);

	/**
	 * Returns true if the script stack is empty
	 */
	virtual bool isSequencesEmpty() const { return _sequenceStack.empty(); }

	/**
	 * Clears the stack of pending object sequences associated with speakers in the scene
	 */
	virtual void clearSequences();
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
