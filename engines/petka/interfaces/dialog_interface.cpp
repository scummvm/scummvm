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

#include "petka/interfaces/dialog_interface.h"
#include "petka/interfaces/main.h"
#include "petka/petka.h"
#include "petka/objects/object_cursor.h"
#include "petka/q_system.h"
#include "petka/big_dialogue.h"
#include "petka/sound.h"

namespace Petka {

DialogInterface::DialogInterface() {
	_field18 = 3;
	_field14 = -1;
	_field24 = 0;
	_field4 = 0;
	_field8 = 0;
	_talker = nullptr;
	_sender = nullptr;
	_fieldC = 0;
	_field10 = 1;
}

void DialogInterface::start(uint a, QMessageObject *sender) {
	_field14 = a;
	_fieldC = 0;
	_field24 = 0;
	_field4 = 0;
	_field8 = 0;
	_talker = nullptr;
	_field10 = 1;
	_field18 = 3;
	_sender = sender;
	_soundName.clear();
	saveCursorState();
	sub_4155D0(-2);
}

void DialogInterface::saveCursorState() {
	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	_savedCursorId = cursor->_resourceId;
	_wasCursorAnim = cursor->_animate;
	_wasCursorShown = cursor->_isShown;
	cursor->_isShown = false;
	cursor->_animate = true;
	cursor->_resourceId = 5006;
}

void DialogInterface::restoreCursorState() {
	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	cursor->_isShown = _wasCursorShown;
	cursor->_animate = _wasCursorAnim;
	cursor->_resourceId = _savedCursorId;
}

void DialogInterface::sub_4155D0(int a) {
	if (_field14 == -1 || (a == -1 && _field18 == 2))
		return;
	if (_field18 == -1)
		return;
	int talkerId = -1;
	if (a == -1 && !_field8) {
		talkerId = g_vm->getBigDialogue()->getSpeechInfo()->speakerId;
	}
	_field8 = _field4;
	g_vm->getQSystem()->_cursor.get()->_isShown = 0;
	if (_field4)
		return;
	if (_field10)
		_field10 = 0;
	else
		g_vm->getBigDialogue()->sub40B670(a);
	switch (g_vm->getBigDialogue()->opcode()) {

	case 1: {
		const SpeechInfo *info = g_vm->getBigDialogue()->getSpeechInfo();
		Common::String soundName = g_vm->getSpeechPath() + info->soundName;
		Sound *s = g_vm->soundMgr()->addSound(soundName, Audio::Mixer::kSpeechSoundType);
		g_vm->soundMgr()->removeSound(_soundName);
		_soundName = soundName;
		if (s) {
			s->play(0);
		}
		g_trackedSound = s;
		if (talkerId != info->speakerId && _talker) {
			QMessage msg;
			msg.objId = _talker->_id;
			msg.opcode = kSaid;
			msg.arg1 = 0;
			msg.arg2 = 0;
			msg.arg3 = 0;
			msg.unk = 0;
			msg.sender = nullptr;
			_talker->processMessage(msg);
		}
		_talker = g_vm->getQSystem()->findObject(info->speakerId);
		QMessage msg;
		msg.objId = _talker->_id;
		msg.opcode = kSay;
		msg.arg1 = 0;
		msg.arg2 = 0;
		msg.arg3 = 0;
		msg.unk = 0;
		msg.sender = nullptr;
		_talker->processMessage(msg);
		_field18 = 1;
		break;
	}
	case 3:
		g_vm->soundMgr()->removeSound(_soundName);
		if (_talker) {
			QMessage msg;
			msg.objId = _talker->_id;
			msg.opcode = kSaid;
			msg.arg1 = 0;
			msg.arg2 = 0;
			msg.arg3 = 0;
			msg.unk = 0;
			msg.sender = nullptr;
		}
		_talker = nullptr;
		_field18 = 3;
		_field14 = -1;
		restoreCursorState();
		// g_qReaction
		_sender = nullptr;
		break;
	case 4:
		g_vm->soundMgr()->removeSound(_soundName);
		_talker = nullptr;
		_field18 = 1;
		break;
	default:
		break;
	}
}

} // End of namespace Petka
