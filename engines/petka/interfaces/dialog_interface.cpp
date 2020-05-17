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
	_state = kIdle;
	_id = -1;
	_field24 = 0;
	_isUserMsg = 0;
	_afterUserMsg = 0;
	_talker = nullptr;
	_sender = nullptr;
	_hasSound = 0;
	_firstTime = 1;
}

void DialogInterface::start(uint id, QMessageObject *sender) {
	_id = id;
	_hasSound = 0;
	_field24 = 0;
	_isUserMsg = 0;
	_afterUserMsg = 0;
	_talker = nullptr;
	_firstTime = 1;
	_state = kIdle;
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
	if (_id == -1)
		return;

	if ((choice == -1 && _state == kMenu) || (choice != -1 && _state == kPlaying))
		return;

	QSystem *qsys = g_vm->getQSystem();
	BigDialogue *bigDialog = g_vm->getBigDialogue();

	const char *soundName = nullptr;
	int prevTalkerId = -1;

	if (choice == -1 && !_afterUserMsg) {
		bigDialog->getSpeechInfo(&prevTalkerId, &soundName, -1);
	}
	_afterUserMsg = _isUserMsg;

	qsys->_cursor->_isShown = 0;
	if (_isUserMsg)
		return;
	if (_firstTime)
		_firstTime = 0;
	else
		g_vm->getBigDialogue()->next(choice);

	switch (g_vm->getBigDialogue()->opcode()) {
	case kOpcodePlay: {
		int currTalkerId;
		const Common::U32String *text = bigDialog->getSpeechInfo(&currTalkerId, &soundName, -1);
		g_vm->soundMgr()->removeSound(_soundName);
		if (prevTalkerId != currTalkerId) {
			sendMsg(kSaid);
		}
		_soundName = g_vm->getSpeechPath() + soundName;
		Sound *s = g_vm->soundMgr()->addSound(_soundName, Audio::Mixer::kSpeechSoundType);
		if (s) {
			// todo pan
			s->play(0);
		}
		_hasSound = s != nullptr;
		_talker = qsys->findObject(currTalkerId);
		if (prevTalkerId != currTalkerId) {
			sendMsg(kSay);
		}
		qsys->_mainInterface->setTextPhrase(*text, _talker->_dialogColor, g_system->getScreenFormat().RGBToColor(0x7F, 0, 0));
		_state = kPlaying;
		break;
	}
	case kOpcodeMenu: {
		g_vm->soundMgr()->removeSound(_soundName);
		_soundName.clear();
		if (_talker) {
			sendMsg(kSaid);
			_talker = nullptr;
		}
		uint count = bigDialog->choicesCount();
		if (count == 0)
			break;

		Common::Array<Common::U32String> choices;
		for (uint i = 0; i < count; ++i) {
			int id;
			choices.push_back(*bigDialog->getSpeechInfo(&id, &soundName, -1));
		}
		qsys->_mainInterface->setTextChoice(choices, 0xFFFF, g_system->getScreenFormat().RGBToColor(0xFF, 0, 0));

		qsys->_cursor->_isShown = 1;
		_state = kMenu;
		break;
	}
	case kOpcodeEnd:
		end();
		break;
	case kOpcodeUserMessage:
		qsys->_mainInterface->setTextPhrase(Common::U32String(""), 0, 0);
		g_vm->soundMgr()->removeSound(_soundName);
		_soundName.clear();
		_talker = nullptr;
		_state = kPlaying;
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
	_state = kIdle;
	_id = -1;
	restoreCursorState();
	if (g_dialogReaction)
		processSavedReaction(&g_dialogReaction, _sender);
	_sender = nullptr;
}

} // End of namespace Petka
