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

#include "titanic/pet_control/pet_conversations.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CPetConversations::CPetConversations() : CPetSection(),
		_logScrolled(false), _field418(0) {
}

bool CPetConversations::isValid(CPetControl *petControl) {
	return setupControl(petControl);
}

bool CPetConversations::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return
		_scrollUp.MouseButtonDownMsg(msg->_mousePos) ||
		_scrollDown.MouseButtonDownMsg(msg->_mousePos) ||
		_doorBot.MouseButtonDownMsg(msg->_mousePos) ||
		_bellBot.MouseButtonDownMsg(msg->_mousePos);
}

bool CPetConversations::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (_scrollUp.MouseButtonUpMsg(msg->_mousePos) ||
			_scrollDown.MouseButtonUpMsg(msg->_mousePos))
		return true;

	if (_doorBot.MouseButtonUpMsg(msg->_mousePos)) {
		switch (canSummonNPC("DoorBot")) {
		case SUMMON_CANT:
			_log.addLine("Sadly, it is not possible to summon the DoorBot from this location.", getColor(1));
			break;
		case SUMMON_CAN:
			summonNPC("DoorBot");
			return true;
		default:
			break;
		}

		// Scroll to the bottom of the log
		scrollToBottom();
		return true;
	}

	if (_bellBot.MouseButtonUpMsg(msg->_mousePos)) {
		switch (canSummonNPC("BellBot")) {
		case SUMMON_CANT:
			_log.addLine("Sadly, it is not possible to summon the BellBot from this location.", getColor(1));
			break;
		case SUMMON_CAN:
			summonNPC("BellBot");
			return true;
		default:
			break;
		}

		// Scroll to the bottom of the log
		scrollToBottom();
		return true;
	}

	return false;
}

bool CPetConversations::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	return _scrollDown.MouseDoubleClickMsg(msg->_mousePos)
		|| _scrollUp.MouseDoubleClickMsg(msg->_mousePos);
}

bool CPetConversations::KeyCharMsg(CKeyCharMsg *msg) {
	Common::KeyState keyState;
	keyState.ascii = msg->_key;
	return handleKey(keyState);
}

bool CPetConversations::VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) {
	return handleKey(msg->_keyState);
}

void CPetConversations::displayMessage(const CString &msg) {
	_log.addLine(msg, getColor(1));
	scrollToBottom();
}

void CPetConversations::load(SimpleFile *file, int param) {
	_textInput.load(file, param);
	_log.load(file, param);

	for (int idx = 0; idx < 3; ++idx)
		_valArray3[idx] = file->readNumber();
}

void CPetConversations::postLoad() {
	reset();
}

void CPetConversations::save(SimpleFile *file, int indent) const {
	_textInput.save(file, indent);
	_log.save(file, indent);

	for (int idx = 0; idx < 3; ++idx)
		file->writeNumberLine(_valArray3[idx], indent);
}

void CPetConversations::enter(PetArea oldArea) {
	if (_petControl && _petControl->_activeNPC)
		// Start a timer for the NPC
		startNPCTimer();

	// Show the text cursor
	_textInput.showCursor(-2);
}

void CPetConversations::leave() {
	_textInput.hideCursor();
	stopNPCTimer();
}

bool CPetConversations::setupControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		_val3.setBounds(Rect(0, 0, 21, 130));
		_val3.translate(20, 350);
		
		const Rect rect1(0, 0, 22, 36);
		_gfxList[0].setBounds(rect1);
		_gfxList[0].translate(20, 359);
		_gfxList[1].setBounds(rect1);
		_gfxList[1].translate(20, 397);
		_gfxList[2].setBounds(rect1);
		_gfxList[2].translate(20, 434);

		const Rect rect2(0, 0, 11, 24);
		_scrollUp.setBounds(rect2);
		_scrollUp.translate(87, 374);
		_scrollDown.setBounds(rect2);
		_scrollDown.translate(87, 421);

		const Rect rect3(0, 0, 39, 39);
		_doorBot.setBounds(rect3);
		_doorBot.translate(546, 372);
		_bellBot.setBounds(rect3);
		_bellBot.translate(546, 418);

		_val6.setBounds(Rect(0, 0, 37, 70));
		_val6.translate(46, 374);
		_val9.setBounds(Rect(0, 0, 435, 3));
		_val9.translate(102, 441);

		const Rect rect4(0, 0, 33, 66);
		for (int idx = 0; idx < 9; ++idx) {
			_valArray2[idx].setBounds(rect4);
			_valArray2[idx].translate(48, 376);
		}
	}

	return true;
}

void CPetConversations::scrollUp() {
	_log.scrollUp(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logScrolled = true;
}

void CPetConversations::scrollDown() {
	_log.scrollDown(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logScrolled = true;
}

void CPetConversations::scrollUpPage() {
	_log.scrollUpPage(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logScrolled = true;
}

void CPetConversations::scrollDownPage() {
	_log.scrollDownPage(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logScrolled = true;
}

void CPetConversations::scrollToTop() {
	_log.scrollToTop(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logScrolled = true;
}

void CPetConversations::scrollToBottom() {
	_log.scrollToBottom(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logScrolled = true;
}

int CPetConversations::canSummonNPC(const CString &name) {
	return _petControl ? _petControl->canSummonNPC(name) : SUMMON_CANT;
}

void CPetConversations::summonNPC(const CString &name) {
	if (_petControl) {
		if (_petControl->getPassengerClass() >= 4) {
			_petControl->displayMessage("Sorry, you must be at least 3rd class before you can summon for help.");
		} else {
			_petControl->summonNPC(name, 0);
		}
	}
}

void CPetConversations::startNPCTimer() {
	_petControl->startPetTimer(1, 1000, 1000, this);
}

void CPetConversations::stopNPCTimer() {
	_petControl->stopPetTimer(1);
}

bool CPetConversations::handleKey(const Common::KeyState &keyState) {
	switch (keyState.keycode) {
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP8:
		scrollUp();
		break;
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP2:
		scrollDown();
		break;
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_KP9:
		scrollUpPage();
		break;
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_KP3:
		scrollDownPage();
		break;
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_KP7:
		scrollToTop();
		break;
	case Common::KEYCODE_END:
	case Common::KEYCODE_KP1:
		scrollToBottom();
		break;
	case Common::KEYCODE_BACKSPACE:
		// Erase key in text input
		_textInput.handleKey((char)Common::KEYCODE_BACKSPACE);
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		// Text line finished
		textLineEntered(_textInput.getText());
		return true;
	default:
		if (keyState.ascii >= 32 && keyState.ascii)
			_textInput.handleKey(keyState.ascii);
		return true;
	}

	return false;
}

void CPetConversations::textLineEntered(const CString &textLine) {
	if (textLine.empty() || !_petControl)
		return;

	if (_petControl->_activeNPC) {
		_log.addLine("- " + textLine, getColor(0));

		CTextInputMsg inputMsg(textLine, "");
		inputMsg.execute(_petControl->_activeNPC);

		if (!inputMsg._response.empty())
			_log.addLine(inputMsg._response);
	} else {
		_log.addLine("There is no one here to talk to", getColor(1));
	}

	// Clear input line and scroll log down to end to show response
	_textInput.setup();
	scrollToBottom();
}

} // End of namespace Titanic
