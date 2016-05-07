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

#include "titanic/npcs/true_talk_npc.h"
#include "titanic/core/view_item.h"
#include "titanic/titanic.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CTrueTalkNPC, CCharacter)
	ON_MESSAGE(TextInputMsg)
	ON_MESSAGE(TrueTalkGetAssetDetailsMsg)
	ON_MESSAGE(DismissBotMsg)
	ON_MESSAGE(TrueTalkNotifySpeechStartedMsg)
	ON_MESSAGE(TrueTalkNotifySpeechEndedMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(NPCQueueIdleAnimMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(NPCPlayAnimationMsg)
END_MESSAGE_MAP()

CTrueTalkNPC::CTrueTalkNPC() : _assetName("z451.dlg"),
	_assetNumber(0x11170), _fieldE4(0), _npcFlags(0), _fieldEC(0), _fieldF0(0),
	_fieldF4(0), _fieldF8(0), _speechTimerId(0), _field100(0), _field104(0) {
}

void CTrueTalkNPC::save(SimpleFile *file, int indent) const {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_assetNumber, indent);
	file->writeQuotedLine(_assetName, indent);
	file->writeNumberLine(_fieldE4, indent);
	file->writeNumberLine(_npcFlags, indent);
	file->writeNumberLine(_fieldEC, indent);
	file->writeNumberLine(_fieldF0, indent);
	file->writeNumberLine(_fieldF4, indent);
	file->writeNumberLine(_fieldF8, indent);
	file->writeNumberLine(_speechTimerId, indent);
	file->writeNumberLine(_field100, indent);
	file->writeNumberLine(_field104, indent);

	CCharacter::save(file, indent);
}

void CTrueTalkNPC::load(SimpleFile *file) {
	file->readNumber();
	_assetNumber = file->readNumber();
	_assetName = file->readString();
	_fieldE4 = file->readNumber();
	_npcFlags = file->readNumber();
	_fieldEC = file->readNumber();
	_fieldF0 = file->readNumber();
	_fieldF4 = file->readNumber();
	_fieldF8 = file->readNumber();
	_speechTimerId = file->readNumber();
	_field100 = file->readNumber();
	_field104 = file->readNumber();

	CCharacter::load(file);
}

bool CTrueTalkNPC::TextInputMsg(CTextInputMsg *msg) {
	processInput(msg, _field104 ? findView() : nullptr);
	return true;
}

bool CTrueTalkNPC::TrueTalkGetAssetDetailsMsg(CTrueTalkGetAssetDetailsMsg *msg) {
	msg->_filename = _assetName;
	msg->_numValue = _assetNumber;
	return true;
}

bool CTrueTalkNPC::DismissBotMsg(CDismissBotMsg *msg) {
	performAction(1, 0);
	return true;
}

bool CTrueTalkNPC::TrueTalkNotifySpeechStartedMsg(CTrueTalkNotifySpeechStartedMsg *msg) {
	_npcFlags |= NPCFLAG_SPEAKING;
	++_field100;

	if (!(_npcFlags & NPCFLAG_8)) {
		if (_speechTimerId)
			stopTimer(_speechTimerId);

		_fieldEC = msg->_value1;
		_fieldF0 = g_vm->_events->getTicksCount();

		if (hasActiveMovie() || (_npcFlags & NPCFLAG_2)) {
			_npcFlags &= ~NPCFLAG_2;
			stopMovie();

			CNPCPlayTalkingAnimationMsg msg1(_fieldEC, 0, 0);
			msg1.execute(this);

			if (msg1._value3) {
				CNPCPlayAnimationMsg msg2(msg1._value3, msg1._value1);
				msg2.execute(this);
			}
		}
	}

	return true;
}

bool CTrueTalkNPC::TrueTalkNotifySpeechEndedMsg(CTrueTalkNotifySpeechEndedMsg *msg) {
	_npcFlags &= ~NPCFLAG_SPEAKING;
	--_field100;
	_fieldEC = 0;

	if (!(_npcFlags & NPCFLAG_8)) {
		CNPCPlayTalkingAnimationMsg msg1(0, 2, 0);
		msg1.execute(this);
		CNPCQueueIdleAnimMsg msg2;
		msg2.execute(this);
	}

	return true;
}

bool CTrueTalkNPC::MovieEndMsg(CMovieEndMsg *msg) {
	if (_npcFlags & NPCFLAG_2) {
		_npcFlags &= ~NPCFLAG_2;
		CNPCQueueIdleAnimMsg idleMsg;
		idleMsg.execute(this);
		return true;
	} else if (!(_npcFlags & NPCFLAG_SPEAKING)) {
		return false;
	}

	int diff = g_vm->_events->getTicksCount() - _fieldF0;
	int ticks = MAX((int)_fieldEC - diff, 0);
	CNPCPlayTalkingAnimationMsg msg1(ticks, ticks > 1000 ? 2 : 1, 0);
	msg1.execute(this);

	if (msg1._value3) {
		CNPCPlayAnimationMsg msg2(msg1._value3, msg1._value1);
		msg2.execute(this);
	}
	
	return true;
}

bool CTrueTalkNPC::NPCQueueIdleAnimMsg(CNPCQueueIdleAnimMsg *msg) {
	// TODO
	return false;
}

bool CTrueTalkNPC::TimerMsg(CTimerMsg *msg) {
	// TODO
	return false;
}

bool CTrueTalkNPC::NPCPlayAnimationMsg(CNPCPlayAnimationMsg *msg) {
	// TODO
	return false;
}

void CTrueTalkNPC::processInput(CTextInputMsg *msg, CViewItem *view) {
	// TODO
}

void CTrueTalkNPC::performAction(int val1, int val2) {
	// TODO
}

} // End of namespace Titanic
