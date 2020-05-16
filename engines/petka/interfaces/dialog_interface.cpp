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

#include "common/system.h"

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
	_hasSound = 0;
	_field10 = 1;
}

void DialogInterface::start(uint a, QMessageObject *sender) {
	_field14 = a;
	_hasSound = 0;
	_field24 = 0;
	_field4 = 0;
	_field8 = 0;
	_talker = nullptr;
	_field10 = 1;
	_field18 = 3;
	_sender = sender;
	_soundName.clear();
	saveCursorState();
	next(-2);
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

void DialogInterface::next(int choice) {
	const char *soundName = nullptr;
	if (_field14 == -1 || (choice == -1 && _field18 == 2))
		return;
	if (_field18 == -1)
		return;
	int talkerId = -1;
	if (choice == -1 && !_field8) {
		g_vm->getBigDialogue()->getSpeechInfo(&talkerId, &soundName, -1);
	}
	_field8 = _field4;
	g_vm->getQSystem()->_cursor.get()->_isShown = 0;
	if (_field4)
		return;
	if (_field10)
		_field10 = 0;
	else
		g_vm->getBigDialogue()->next(choice);
	switch (g_vm->getBigDialogue()->opcode()) {
	case kOpcodePlay: {
		int talkerId2;
		const Common::U32String *text = g_vm->getBigDialogue()->getSpeechInfo(&talkerId2, &soundName, -1);
		g_vm->soundMgr()->removeSound(_soundName);
		if (talkerId != talkerId2) {
			sendMsg(kSaid);
		}
		_soundName = g_vm->getSpeechPath() + soundName;
		Sound *s = g_vm->soundMgr()->addSound(_soundName, Audio::Mixer::kSpeechSoundType);
		if (s) {
			s->play(0);
		}
		_hasSound = s != nullptr;
		_talker = g_vm->getQSystem()->findObject(talkerId2);
		if (talkerId != talkerId2) {
			sendMsg(kSay);
		}
		g_vm->getQSystem()->_mainInterface->setTextPhrase(*text, _talker->_dialogColor, g_vm->_system->getScreenFormat().RGBToColor(0x7F, 0, 0));
		_field18 = 1;
		break;
	}
	case kOpcodeMenu:
		break;
	case kOpcodeEnd:
		end();
		break;
	case kOpcode4:
		g_vm->soundMgr()->removeSound(_soundName);
		_talker = nullptr;
		_field18 = 1;
		break;
	default:
		break;
	}
}

void DialogInterface::sendMsg(uint16 opcode) {
	if (_talker) {
		_talker->processMessage(QMessage(_talker->_id, opcode, 0, 0, 0, nullptr, 0));
	}
}

void DialogInterface::end() {
	g_vm->soundMgr()->removeSound(_soundName);
	sendMsg(kSaid);
	_talker = nullptr;
	_field18 = 3;
	_field14 = -1;
	restoreCursorState();
	if (g_dialogReaction)
		processSavedReaction(&g_dialogReaction, _sender);
	_sender = nullptr;
}

} // End of namespace Petka
