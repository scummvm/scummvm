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
#include "titanic/debugger.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

CPetConversations::CPetConversations() : CPetSection(),
		_logChanged(false), _field418(0), _npcNum(-1),
		_rect1(22, 352, 598, 478) {
	Rect logRect(85, 18, 513, 87);
	logRect.translate(20, 350);
	_log.setBounds(logRect);
	_log.resize(50);
	_log.setHasBorder(false);
	_log.setColor(getColor(2));
	_log.setup();
	_log.addLine("Welcome to your PET v1.0a");

	Rect inputRect(85, 95, 513, 135);
	inputRect.translate(20, 350);
	_textInput.setBounds(inputRect);
	_textInput.setHasBorder(false);
	_textInput.resize(2);
	_textInput.setMaxCharsPerLine(74);
	_textInput.setColor(getColor(0));
	_textInput.setup();

	_npcLevels[0] = _npcLevels[1] = _npcLevels[2] = 0;
}

bool CPetConversations::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetConversations::reset() {
	_dials[0].setup(MODE_UNSELECTED, "3PetDial1", _petControl);
	_dials[1].setup(MODE_UNSELECTED, "3PetDial2", _petControl);
	_dials[2].setup(MODE_UNSELECTED, "3PetDial3", _petControl);

	_dialBackground.reset("PetDialBack", _petControl);
	_scrollUp.reset("PetScrollUp", _petControl);
	_scrollDown.reset("PetScrollDown", _petControl);

	_doorBot.reset("PetCallDoorOut", _petControl, MODE_UNSELECTED);
	_doorBot.reset("PetCallDoorIn", _petControl, MODE_SELECTED);
	_bellBot.reset("PetCallBellOut", _petControl, MODE_UNSELECTED);
	_bellBot.reset("PetCallBellIn", _petControl, MODE_SELECTED);

	_indent.reset("PetSmallCharacterIndent", _petControl);
	_splitter.reset("PetSplitter", _petControl);

	_npcIcons[0].setup(MODE_UNSELECTED, "3PetSmlDoorbot", _petControl);
	_npcIcons[1].setup(MODE_UNSELECTED, "3PetSmlDeskbot", _petControl);
	_npcIcons[2].setup(MODE_UNSELECTED, "3PetSmlLiftbot", _petControl);
	_npcIcons[3].setup(MODE_UNSELECTED, "3PetSmlParrot", _petControl);
	_npcIcons[4].setup(MODE_UNSELECTED, "3PetSmlBarbot", _petControl);
	_npcIcons[5].setup(MODE_UNSELECTED, "3PetSmlChatterbot", _petControl);
	_npcIcons[6].setup(MODE_UNSELECTED, "3PetSmlBellbot", _petControl);
	_npcIcons[7].setup(MODE_UNSELECTED, "3PetSmlMaitreD", _petControl);
	_npcIcons[8].setup(MODE_UNSELECTED, "3PetSmlSuccubus", _petControl);

	if (_petControl->getPassengerClass() == 1) {
		uint col = getColor(0);
		_textInput.setColor(col);
		_textInput.setLineColor(0, col);

		// Replace the log colors with new 1st class ones
		uint colors1[5], colors2[5];
		copyColors(2, colors1);
		copyColors(1, colors2);
		_log.remapColors(5, colors1, colors2);

		_log.setColor(getColor(2));
	}

	// WORKAROUND: After loading, mark log as changed so the
	// current NPC portrait to display gets recalculated
	_logChanged = true;

	return true;
}

void CPetConversations::draw(CScreenManager *screenManager) {
	_dialBackground.draw(screenManager);
	_splitter.draw(screenManager);
	_dials[0].draw(screenManager);
	_dials[1].draw(screenManager);
	_dials[2].draw(screenManager);

	_indent.draw(screenManager);
	_doorBot.draw(screenManager);
	_bellBot.draw(screenManager);
	_scrollUp.draw(screenManager);
	_scrollDown.draw(screenManager);
	_log.draw(screenManager);
	_textInput.draw(screenManager);

	if (_logChanged) {
		int endIndex = _log.displayEndIndex();
		if (endIndex >= 0) {
			int npcNum = _log.getNPCNum(1, endIndex);
			if (npcNum > 0 && npcNum < 10)
				_npcNum = npcNum - 1;
		}

		_logChanged = false;
	}

	if (_npcNum >= 0)
		_npcIcons[_npcNum].draw(screenManager);
}

Rect CPetConversations::getBounds() const {
	Rect rect = _dials[0].getBounds();
	rect.combine(_dials[1].getBounds());
	rect.combine(_dials[2].getBounds());

	return rect;
}

bool CPetConversations::isValid(CPetControl *petControl) {
	return setupControl(petControl);
}

bool CPetConversations::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_scrollDown.MouseButtonDownMsg(msg->_mousePos)) {
		scrollDown();
		return true;
	} else if (_scrollUp.MouseButtonDownMsg(msg->_mousePos)) {
		scrollUp();
		return true;
	}

	return
		_doorBot.MouseButtonDownMsg(msg->_mousePos) ||
		_bellBot.MouseButtonDownMsg(msg->_mousePos);
}

bool CPetConversations::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (_scrollUp.MouseButtonUpMsg(msg->_mousePos) ||
			_scrollDown.MouseButtonUpMsg(msg->_mousePos))
		return true;

	if (_doorBot.MouseButtonUpMsg(msg->_mousePos)) {
		switch (canSummonBot("DoorBot")) {
		case SUMMON_CANT:
			_log.addLine(g_vm->_strings[CANT_SUMMON_DOORBOT], getColor(1));
			break;
		case SUMMON_CAN:
			summonBot("DoorBot");
			return true;
		default:
			break;
		}

		// Scroll to the bottom of the log
		scrollToBottom();
		return true;
	}

	if (_bellBot.MouseButtonUpMsg(msg->_mousePos)) {
		switch (canSummonBot("BellBot")) {
		case SUMMON_CANT:
			_log.addLine(g_vm->_strings[CANT_SUMMON_BELLBOT], getColor(1));
			break;
		case SUMMON_CAN:
			summonBot("BellBot");
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

bool CPetConversations::MouseWheelMsg(CMouseWheelMsg *msg) {
	if (msg->_wheelUp)
		scrollUp();
	else
		scrollDown();

	return true;
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

	for (int idx = 0; idx < TOTAL_DIALS; ++idx)
		_npcLevels[idx] = file->readNumber();
}

void CPetConversations::postLoad() {
	reset();
}

void CPetConversations::save(SimpleFile *file, int indent) {
	_textInput.save(file, indent);
	_log.save(file, indent);

	for (int idx = 0; idx < TOTAL_DIALS; ++idx)
		file->writeNumberLine(_npcLevels[idx], indent);
}

void CPetConversations::enter(PetArea oldArea) {
	resetDials();

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

void CPetConversations::timerExpired(int val) {
	if (val != 1) {
		CPetSection::timerExpired(val);
	} else {
		CString name = _field418 ? _npcName : getActiveNPCName();

		for (int idx = 0; idx < TOTAL_DIALS; ++idx) {
			if (!_dials[idx].hasActiveMovie())
				updateDial(idx, name);
		}
	}
}

void CPetConversations::displayNPCName(CGameObject *npc) {
	const Strings &strings = g_vm->_strings;

	if (npc) {
		displayMessage(CString());
		CString msg = strings[TALKING_TO];
		CString name = npc->getName();
		int id = 1;

		if (name.containsIgnoreCase("Doorbot")) {
			msg += strings[DOORBOT_NAME];
		} else if (name.containsIgnoreCase("Deskbot")) {
			id = 2;
			msg += strings[DESKBOT_NAME];
		} else if (name.containsIgnoreCase("LiftBot")) {
			id = 3;
			msg += strings[LIFTBOT_NAME];
		} else if (name.containsIgnoreCase("Parrot")) {
			id = 4;
			msg += strings[PARROT_NAME];
		} else if (name.containsIgnoreCase("BarBot")) {
			id = 5;
			msg += strings[BARBOT_NAME];
		} else if (name.containsIgnoreCase("ChatterBot")) {
			id = 6;
			msg += strings[CHATTERBOT_NAME];
		} else if (name.containsIgnoreCase("BellBot")) {
			id = 7;
			msg += strings[BELLBOT_NAME];
		} else if (name.containsIgnoreCase("Maitre")) {
			id = 8;
			msg += strings[MAITRED_NAME];
		} else if (name.containsIgnoreCase("Succubus") || name.containsIgnoreCase("Sub")) {
			id = 9;
			msg += strings[SUCCUBUS_NAME];
		} else {
			msg += strings[UNKNOWN_NAME];
		}

		_log.setNPC(1, id);
		displayMessage(msg);
	}
}

void CPetConversations::setNPC(const CString &name) {
	_field418 = 0;
	resetDials(name);
	startNPCTimer();
}

void CPetConversations::resetNPC() {
	stopNPCTimer();
	resetDials("0");
}

void CPetConversations::showCursor() {
	_textInput.showCursor(-2);
}

void CPetConversations::hideCursor() {
	_textInput.hideCursor();
}

bool CPetConversations::setupControl(CPetControl *petControl) {
	if (petControl) {
		_petControl = petControl;

		_dialBackground.setBounds(Rect(0, 0, 21, 130));
		_dialBackground.translate(20, 350);

		const Rect rect1(0, 0, 22, 36);
		_dials[0].setBounds(rect1);
		_dials[0].translate(20, 359);
		_dials[1].setBounds(rect1);
		_dials[1].translate(20, 397);
		_dials[2].setBounds(rect1);
		_dials[2].translate(20, 434);

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

		_indent.setBounds(Rect(0, 0, 37, 70));
		_indent.translate(46, 374);
		_splitter.setBounds(Rect(0, 0, 435, 3));
		_splitter.translate(102, 441);

		const Rect rect4(0, 0, 33, 66);
		for (int idx = 0; idx < 9; ++idx) {
			_npcIcons[idx].setBounds(rect4);
			_npcIcons[idx].translate(48, 376);
		}
	}

	return true;
}

void CPetConversations::scrollUp() {
	_log.scrollUp(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logChanged = true;
}

void CPetConversations::scrollDown() {
	_log.scrollDown(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logChanged = true;
}

void CPetConversations::scrollUpPage() {
	_log.scrollUpPage(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logChanged = true;
}

void CPetConversations::scrollDownPage() {
	_log.scrollDownPage(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logChanged = true;
}

void CPetConversations::scrollToTop() {
	_log.scrollToTop(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logChanged = true;
}

void CPetConversations::scrollToBottom() {
	_log.scrollToBottom(CScreenManager::_screenManagerPtr);
	if (_petControl)
		_petControl->makeDirty();
	_logChanged = true;
}

int CPetConversations::canSummonBot(const CString &name) {
	return _petControl ? _petControl->canSummonBot(name) : SUMMON_CANT;
}

void CPetConversations::summonBot(const CString &name) {
	if (_petControl) {
		if (_petControl->getPassengerClass() >= UNCHECKED) {
			_petControl->displayMessage(AT_LEAST_3RD_CLASS_FOR_HELP);
		} else {
			_petControl->summonBot(name, 0);
		}
	}
}

void CPetConversations::startNPCTimer() {
	_petControl->startPetTimer(1, 1000, 1000, this);
}

void CPetConversations::stopNPCTimer() {
	_petControl->stopPetTimer(1);
}

TTnpcScript *CPetConversations::getNPCScript(const CString &name) const {
	if (name.empty() || !_petControl)
		return nullptr;
	CGameManager *gameManager = _petControl->getGameManager();
	if (!gameManager)
		return nullptr;
	CTrueTalkManager *trueTalk = gameManager->getTalkManager();
	if (!trueTalk)
		return nullptr;

	return trueTalk->getTalker(name);
}

bool CPetConversations::handleKey(const Common::KeyState &keyState) {
	switch (keyState.keycode) {
	case Common::KEYCODE_PAGEUP:
	case Common::KEYCODE_KP9:
		scrollUpPage();
		return true;
	case Common::KEYCODE_PAGEDOWN:
	case Common::KEYCODE_KP3:
		scrollDownPage();
		return true;
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_KP7:
		scrollToTop();
		return true;
	case Common::KEYCODE_END:
	case Common::KEYCODE_KP1:
		scrollToBottom();
		return true;
	default:
		if (keyState.ascii > 0 && keyState.ascii <= 127
				&& keyState.ascii != Common::KEYCODE_TAB) {
			if (_textInput.handleKey(keyState.ascii))
				// Text line finished, so process line
				textLineEntered(_textInput.getText());
			return true;
		}
		break;
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
		_log.addLine(g_vm->_strings[NO_ONE_TO_TALK_TO], getColor(1));
	}

	// Clear input line and scroll log down to end to show response
	_textInput.setup();
	scrollToBottom();
}

void CPetConversations::setActiveNPC(const CString &name) {
	_npcName = name;
	_field418 = 1;
	resetDials();
	startNPCTimer();
}

void CPetConversations::updateDial(uint dialNum, const CString &npcName) {
	TTnpcScript *script = getNPCScript(npcName);
	uint newLevel = getDialLevel(dialNum, script);
	npcDialChange(dialNum, _npcLevels[dialNum], newLevel);
	_npcLevels[dialNum] = newLevel;
}

uint CPetConversations::getDialLevel(uint dialNum, TTnpcScript *script, bool flag) {
	if (!script)
		return 0;
	else
		return MAX(script->getDialLevel(dialNum, flag), 15);
}

void CPetConversations::npcDialChange(uint dialNum, uint oldLevel, uint newLevel) {
	const uint ascending[2] = { 0, 21 };
	const uint descending[2] = { 43, 22 };
	assert(oldLevel <= 100 && newLevel <= 100);

	if (newLevel != oldLevel) {
		debugC(DEBUG_DETAILED, kDebugScripts, "Dial %d change from %d to %d",
			dialNum, oldLevel, newLevel);
		uint src = ascending[0], dest = ascending[1];
		if (newLevel < oldLevel) {
			src = descending[0];
			dest = descending[1];
		}

		uint val1 = (oldLevel * dest) + (100 - oldLevel) * src;
		uint startFrame = val1 / 100;

		uint val2 = (newLevel * dest) + (100 - newLevel) * src;
		uint endFrame = val2 / 100;

		if (startFrame != endFrame)
			_dials[dialNum].playMovie(startFrame, endFrame);
	}
}

void CPetConversations::resetDials() {
	resetDials(getActiveNPCName());
}

void CPetConversations::resetDials(const CString &name) {
	TTnpcScript *script = getNPCScript(name);

	for (int idx = 0; idx < TOTAL_DIALS; ++idx) {
		uint oldLevel = _npcLevels[idx];
		uint newLevel = getDialLevel(idx, script);
		npcDialChange(idx, oldLevel, newLevel);
		_npcLevels[idx] = newLevel;
	}
}

void CPetConversations::resetDials0() {
	stopNPCTimer();
	resetDials("0");
}

void CPetConversations::addLine(const CString &line) {
	_log.addLine(line);
	scrollToBottom();
}

} // End of namespace Titanic
