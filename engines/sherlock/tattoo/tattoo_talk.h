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

#ifndef SHERLOCK_TATTOO_TALK_H
#define SHERLOCK_TATTOO_TALK_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/stack.h"
#include "sherlock/talk.h"
#include "sherlock/tattoo/widget_password.h"
#include "sherlock/tattoo/widget_talk.h"

namespace Sherlock {

namespace Tattoo {

#define TALK_SEQUENCE_STACK_SIZE 20

class WidgetTalk;

class TattooTalk : public Talk {
	friend class WidgetTalk;
private:
	WidgetTalk _talkWidget;
	WidgetPassword _passwordWidget;
	SequenceEntry _sequenceStack[TALK_SEQUENCE_STACK_SIZE];

	OpcodeReturn cmdCallTalkFile(const byte *&str);
	OpcodeReturn cmdSwitchSpeaker(const byte *&str);
	OpcodeReturn cmdMouseOnOff(const byte *&str);
	OpcodeReturn cmdGotoScene(const byte *&str);
	OpcodeReturn cmdWalkHolmesToCoords(const byte *&str);
	OpcodeReturn cmdNextSong(const byte *&str);
	OpcodeReturn cmdPassword(const byte *&str);
	OpcodeReturn cmdPlaySong(const byte *&str);
	OpcodeReturn cmdRestorePeopleSequence(const byte *&str);
	OpcodeReturn cmdSetNPCDescOnOff(const byte *&str);
	OpcodeReturn cmdSetNPCInfoLine(const byte *&str);
	OpcodeReturn cmdNPCLabelGoto(const byte *&str);
	OpcodeReturn cmdNPCLabelIfFlagGoto(const byte *&str);
	OpcodeReturn cmdNPCLabelSet(const byte *&str);
	OpcodeReturn cmdSetNPCOff(const byte *&str);
	OpcodeReturn cmdSetNPCOn(const byte *&str);
	OpcodeReturn cmdSetNPCPathDest(const byte *&str);
	OpcodeReturn cmdSetNPCPathPause(const byte *&str);
	OpcodeReturn cmdSetNPCPathPauseTakingNotes(const byte *&str);
	OpcodeReturn cmdSetNPCPathPauseLookingHolmes(const byte *&str);
	OpcodeReturn cmdSetNPCPosition(const byte *&str);
	OpcodeReturn cmdSetNPCTalkFile(const byte *&str);
	OpcodeReturn cmdSetNPCVerb(const byte *&str);
	OpcodeReturn cmdSetNPCVerbCAnimation(const byte *&str);
	OpcodeReturn cmdSetNPCVerbScript(const byte *&str);
	OpcodeReturn cmdSetNPCVerbTarget(const byte *&str);
	OpcodeReturn cmdSetNPCWalkGraphics(const byte *&str);
	OpcodeReturn cmdSetSceneEntryFlag(const byte *&str);
	OpcodeReturn cmdSetTalkSequence(const byte *&str);
	OpcodeReturn cmdSetWalkControl(const byte *&str);
	OpcodeReturn cmdTalkInterruptsDisable(const byte *&str);
	OpcodeReturn cmdTalkInterruptsEnable(const byte *&str);
	OpcodeReturn cmdTurnSoundsOff(const byte *&str);
	OpcodeReturn cmdWalkHolmesAndNPCToCAnimation(const byte *&str);
	OpcodeReturn cmdWalkNPCToCAnimation(const byte *&str);
	OpcodeReturn cmdWalkNPCToCoords(const byte *&str);
	OpcodeReturn cmdWalkHomesAndNPCToCoords(const byte *&str);
protected:
	/**
	 * Display the talk interface window
	 */
	void talkInterface(const byte *&str) override;

	/**
	 * Called when a character being spoken to has no talk options to display
	 */
	void nothingToSay() override;

	/**
	 * Show the talk display
	 */
	void showTalk() override;
public:
	TattooTalk(SherlockEngine *vm);
	~TattooTalk() override {}

	/**
	 * Called whenever a conversation or item script needs to be run. For standard conversations,
	 * it opens up a description window similar to how 'talk' does, but shows a 'reply' directly
	 * instead of waiting for a statement option.
	 * @remarks		It seems that at some point, all item scripts were set up to use this as well.
	 *	In their case, the conversation display is simply suppressed, and control is passed on to
	 *	doScript to implement whatever action is required.
	 */
	void talkTo(const Common::String filename) override;

	/**
	 * Push the details of a passed object onto the saved sequences stack
	 */
	void pushSequenceEntry(Object *obj) override;

	/**
	 * Pulls a background object sequence from the sequence stack and restore's the
	 * object's sequence
	 */
	void pullSequence(int slot = -1) override;

	/**
	 * Returns true if the script stack is empty
	 */
	bool isSequencesEmpty() const override;

	/**
	 * Clears the stack of pending object sequences associated with speakers in the scene
	 */
	void clearSequences() override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
