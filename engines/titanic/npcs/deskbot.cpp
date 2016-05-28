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

#include "titanic/npcs/deskbot.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CDeskbot, CTrueTalkNPC)
	ON_MESSAGE(TurnOn)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(TrueTalkTriggerActionMsg)
	ON_MESSAGE(NPCPlayTalkingAnimationMsg)
	ON_MESSAGE(NPCPlayIdleAnimationMsg)
	ON_MESSAGE(TrueTalkNotifySpeechStartedMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
	ON_MESSAGE(TurnOff)
END_MESSAGE_MAP()

int CDeskbot::_v1;
int CDeskbot::_v2;

CDeskbot::CDeskbot() : CTrueTalkNPC(), _field108(0), _field10C(0) {
}

void CDeskbot::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_v1, indent);
	file->writeNumberLine(_v2, indent);
	file->writeNumberLine(_field108, indent);
	file->writeNumberLine(_field10C, indent);

	CTrueTalkNPC::save(file, indent);
}

void CDeskbot::load(SimpleFile *file) {
	file->readNumber();
	_v1 = file->readNumber();
	_v2 = file->readNumber();
	_field108 = file->readNumber();
	_field10C = file->readNumber();

	CTrueTalkNPC::load(file);
}

bool CDeskbot::TurnOn(CTurnOn *msg) {
	// TODO
	return true;
}

bool CDeskbot::EnterViewMsg(CEnterViewMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::ActMsg(CActMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::MovieEndMsg(CMovieEndMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::LeaveViewMsg(CLeaveViewMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::TrueTalkTriggerActionMsg(CTrueTalkTriggerActionMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::NPCPlayTalkingAnimationMsg(CNPCPlayTalkingAnimationMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::NPCPlayIdleAnimationMsg(CNPCPlayIdleAnimationMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	// TODO
	return true;
}

bool CDeskbot::TurnOff(CTurnOff *msg) {
	// TODO
	return true;
}

} // End of namespace Titanic
