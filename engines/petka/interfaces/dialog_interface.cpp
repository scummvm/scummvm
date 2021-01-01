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
#include "petka/objects/object_star.h"
#include "petka/objects/heroes.h"
#include "petka/q_system.h"
#include "petka/big_dialogue.h"
#include "petka/sound.h"
#include "petka/flc.h"
#include "petka/q_manager.h"

namespace Petka {

DialogInterface::DialogInterface() {
	_dialog = g_vm->getBigDialogue();
	_qsys = g_vm->getQSystem();
	_state = kIdle;
	_id = -1;
	_isUserMsg = false;
	_afterUserMsg = false;
	_talker = nullptr;
	_sender = nullptr;
	_reaction = nullptr;
	_firstTime = true;
}

DialogInterface::~DialogInterface() {
	delete _reaction;
}

void DialogInterface::start(uint id, QMessageObject *sender) {
	_id = id;
	_isUserMsg = false;
	_afterUserMsg = false;
	_talker = nullptr;
	_firstTime = true;
	_state = kIdle;
	_sender = sender;
	_soundName.clear();
	initCursor();
	next(-2);
}

void DialogInterface::initCursor() {
	QObjectCursor *cursor = _qsys->getCursor();

	_savedCursorId = cursor->_resourceId;
	_savedCursorActType = cursor->_actionType;
	_wasCursorAnim = cursor->_animate;
	_wasCursorShown = cursor->_isShown;

	cursor->show(false);
	cursor->_animate = true;
	cursor->_resourceId = 5006;
	cursor->_actionType = kActionTalk;
}

void DialogInterface::restoreCursor() {
	QObjectCursor *cursor = _qsys->getCursor();
	cursor->_isShown = _wasCursorShown;
	cursor->_animate = _wasCursorAnim;
	cursor->_resourceId = _savedCursorId;
	cursor->_actionType = _savedCursorActType;

	// original bug fix
	Common::Event ev;
	ev.type = Common::EVENT_MOUSEMOVE;
	ev.mouse = g_system->getEventManager()->getMousePos();
	_qsys->onEvent(ev);
}

void DialogInterface::next(int choice) {
	if (_id == -1)
		return;

	if ((choice == -1 && _state == kMenu) || (choice != -1 && _state == kPlaying))
		return;

	int prevTalkerId = -1;
	if (choice == -1 && !_afterUserMsg) {
		_dialog->getSpeechInfo(&prevTalkerId, nullptr, -1);
	}
	_afterUserMsg = _isUserMsg;

	_qsys->getCursor()->_isShown = false;
	if (_isUserMsg)
		return;
	if (_firstTime)
		_firstTime = false;
	else
		_dialog->next(choice);

	switch (_dialog->opcode()) {
	case kOpcodePlay:
		onPlayOpcode(prevTalkerId);
		break;
	case kOpcodeMenu:
		onMenuOpcode();
		break;
	case kOpcodeEnd:
		onEndOpcode();
		break;
	case kOpcodeUserMessage:
		onUserMsgOpcode();
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

void DialogInterface::onEndOpcode() {
	g_vm->soundMgr()->removeSound(_soundName);
	sendMsg(kSaid);
	_talker = nullptr;
	_state = kIdle;
	_id = -1;
	_qsys->_currInterface->removeTexts();
	restoreCursor();
	if (_reaction) {
		QReaction *reaction = _reaction;
		_reaction = nullptr;
		_sender->processReaction(reaction);
	}
	_sender = nullptr;
}

void DialogInterface::endUserMsg() {
	_isUserMsg = false;
	initCursor();
	next(-1);
}

void DialogInterface::startUserMsg(uint16 arg) {
	sendMsg(kSaid);
	_isUserMsg = true;
	restoreCursor();
	_qsys->addMessage(_qsys->getChapay()->_id, kUserMsg, arg);
}

bool DialogInterface::isActive() {
	return _state != kIdle;
}

void DialogInterface::setSender(QMessageObject *sender) {
	_sender = sender;
}

Sound *DialogInterface::findSound() {
	return g_vm->soundMgr()->findSound(_soundName);
}

void DialogInterface::removeSound() {
	g_vm->soundMgr()->removeSound(_soundName);
	_soundName.clear();
}

void DialogInterface::setReaction(QReaction *reaction) {
	delete _reaction;
	_reaction = reaction;
}

void DialogInterface::playSound(const Common::String &name) {
	removeSound();
	_soundName = name;
	Sound *s = g_vm->soundMgr()->addSound(name, Audio::Mixer::kSpeechSoundType);
	if (s) {
		FlicDecoder *flc = g_vm->resMgr()->getFlic(_talker->_resourceId);
		if (flc) {
			Common::Rect bounds = flc->getBounds();
			s->setBalance(bounds.left + _talker->_x + bounds.width(), _qsys->_sceneWidth);
		}
		s->play(false);
	}
}

void DialogInterface::setPhrase(const Common::U32String *text) {
	uint16 textColor;
	uint16 outlineColor;
	if (_talker->_dialogColor == -1) {
		textColor = g_system->getScreenFormat().RGBToColor(0xA, 0xA, 0xA);
		outlineColor = 0xFFFF;
	} else {
		textColor = _talker->_dialogColor;
		outlineColor = g_system->getScreenFormat().RGBToColor(0x7F, 0, 0);
	}
	_qsys->_currInterface->setTextPhrase(*text, textColor, outlineColor);
}

void DialogInterface::onPlayOpcode(int prevTalkerId) {
	int currTalkerId;
	const char *soundName = nullptr;
	const Common::U32String *text = _dialog->getSpeechInfo(&currTalkerId, &soundName, -1);

	if (prevTalkerId != currTalkerId) {
		sendMsg(kSaid);
	}

	_talker = _qsys->findObject(currTalkerId);
	playSound(g_vm->getSpeechPath() + soundName);
	setPhrase(text);

	if (prevTalkerId != currTalkerId) {
		sendMsg(kSay);
	}
	_state = kPlaying;
}

void DialogInterface::onMenuOpcode() {
	removeSound();

	sendMsg(kSaid);
	_talker = nullptr;

	Common::Array<Common::U32String> choices;
	_dialog->getMenuChoices(choices);
	_qsys->_mainInterface->setTextChoice(choices, 0xFFFF, g_system->getScreenFormat().RGBToColor(0xFF, 0, 0));

	_qsys->getCursor()->_isShown = true;
	_state = kMenu;
}

void DialogInterface::onUserMsgOpcode() {
	_qsys->_currInterface->setTextPhrase(Common::U32String(), 0, 0);
	removeSound();
	_talker = nullptr;
	_state = kPlaying;
}

void DialogInterface::fixCursor() {
	_isUserMsg = false;
	_qsys->getCursor()->show(true);
	_qsys->getStar()->_isActive = true;
}

} // End of namespace Petka
