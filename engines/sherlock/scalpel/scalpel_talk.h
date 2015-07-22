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
	OpcodeReturn cmdSwitchSpeaker(const byte *&str);
	OpcodeReturn cmdAssignPortraitLocation(const byte *&str);
	OpcodeReturn cmdGotoScene(const byte *&str);
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
	 * Trigger to play a 3DO talk dialog movie
	 */
	virtual void talk3DOMovieTrigger(int subIndex);

	/**
	 * Show the talk display
	 */
	virtual void showTalk();
public:
	ScalpelTalk(SherlockEngine *vm);
	virtual ~ScalpelTalk() {}

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
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
