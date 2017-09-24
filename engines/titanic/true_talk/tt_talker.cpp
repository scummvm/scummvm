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

#include "titanic/true_talk/tt_talker.h"
#include "titanic/messages/messages.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

void TTtalker::speechStarted(const CString &dialogueStr, uint dialogueId, uint speechDuration) {
	_line = dialogueStr;
	_dialogueId = dialogueId;

	CTrueTalkNotifySpeechStartedMsg msg(speechDuration, dialogueId, 0);
	msg.execute(_npc, nullptr, MSGFLAG_BREAK_IF_HANDLED);
}

void TTtalker::endSpeech(int val) {
	_done = true;
	_talkEndState = val;
}

void TTtalker::speechEnded() {
	CPetControl *petControl = _npc->getPetControl();
	if (petControl)
		// Add in final line
		petControl->convAddLine(_line);

	// Notify the end of the speech
	CTrueTalkNotifySpeechEndedMsg endedMsg(_talkEndState, _dialogueId);
	endedMsg.execute(_npc, nullptr, MSGFLAG_BREAK_IF_HANDLED);
}

} // End of namespace Titanic
