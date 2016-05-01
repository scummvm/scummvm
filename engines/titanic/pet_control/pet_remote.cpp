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

#include "titanic/pet_control/pet_remote.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

CPetRemote::CPetRemote() : CPetSection() {
}

bool CPetRemote::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetRemote::reset() {
	if (_petControl) {
		_onOff.reset("PetSwitchOn", _petControl, MODE_SELECTED);
		_onOff.reset("PetSwitchOff", _petControl, MODE_UNSELECTED);

		_up.reset("PetUp", _petControl, MODE_UNSELECTED);
		_down.reset("PetDown", _petControl, MODE_UNSELECTED);

		_left.reset("PetLeftUp", _petControl, MODE_SELECTED);
		_left.reset("PetLeft", _petControl, MODE_UNSELECTED);
		_right.reset("PetRightUp", _petControl, MODE_SELECTED);
		_right.reset("PetRight", _petControl, MODE_UNSELECTED);
		_top.reset("PetTopUp", _petControl, MODE_SELECTED);
		_top.reset("PetTop", _petControl, MODE_UNSELECTED);
		_bottom.reset("PetBottomUp", _petControl, MODE_SELECTED);
		_bottom.reset("PetBottom", _petControl, MODE_UNSELECTED);
		_action.reset("PetActionUp", _petControl, MODE_SELECTED);
		_action.reset("PetAction", _petControl, MODE_UNSELECTED);

		_send.reset("PetActSend0", _petControl, MODE_UNSELECTED);
		_send.reset("PetActSend1", _petControl, MODE_SELECTED);
		_receive.reset("PetActReceive0", _petControl, MODE_UNSELECTED);
		_receive.reset("PetActReceive1", _petControl, MODE_SELECTED);
		_call.reset("PetActCall0", _petControl, MODE_UNSELECTED);
		_call.reset("PetActCall1", _petControl, MODE_SELECTED);

		_items.reset();
		uint col = getColor(0);
		_text.setColor(col);
		_text.setLineColor(0, col);
	}

	return true;
}

void CPetRemote::draw(CScreenManager *screenManager) {
	_petControl->drawSquares(screenManager, 6);
	_items.draw(screenManager);
	_text.draw(screenManager);
}

bool CPetRemote::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return _items.MouseButtonDownMsg(msg->_mousePos);
}

bool CPetRemote::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return _items.MouseButtonUpMsg(msg->_mousePos);
}

bool CPetRemote::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	return _items.MouseButtonDownMsg(msg->_mousePos);
}

bool CPetRemote::VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) {
	return _items.VirtualKeyCharMsg(msg->_keyState.keycode);
}

bool CPetRemote::isValid(CPetControl *petControl) {
	return setupControl(petControl);
}

void CPetRemote::postLoad() {
	reset();
	CRoomItem *room = getRoom();
	if (room)
		enterRoom(room);
}

void CPetRemote::enter(PetArea oldArea) {
	if (_items.highlighted14())
		_text.setText(CString());
}

void CPetRemote::enterRoom(CRoomItem *room) {
	// TODO
}

CPetText *CPetRemote::getText() {
	return &_text;
}

CPetElement *CPetRemote::getElement(uint id) {
	switch (id) {
	case 0:
		return &_onOff;
	case 1:
		return &_up;
	case 2:
		return &_down;
	case 3:
		return &_left;
	case 4:
		return &_right;
	case 5:
		return &_top;
	case 6:
		return &_bottom;
	case 7:
		return &_action;
	case 16:
		return &_send;
	case 17:
		return &_receive;
	case 18:
		return &_call;
	default:
		return nullptr;
	}
}

void CPetRemote::proc38(int val) {
	int highlightIndex = getHighlightIndex(val);
	if (highlightIndex != -1)
		_items.highlight(highlightIndex);
}

bool CPetRemote::setupControl(CPetControl *petControl) {
	_petControl = petControl;
	if (!petControl)
		return false;

	_onOff.setBounds(Rect(0, 0, 15, 43));
	_onOff.translate(519, 381);
	_up.setBounds(Rect(0, 0, 21, 24));
	_up.translate(551, 381);
	_down.setBounds(Rect(0, 0, 21, 24));
	_down.translate(551, 402);
	_left.setBounds(Rect(0, 0, 22, 21));
	_left.translate(518, 393);
	_right.setBounds(Rect(0, 0, 21, 21));
	_right.translate(560, 393);
	_top.setBounds(Rect(0, 0, 21, 22));
	_top.translate(539, 371);
	_bottom.setBounds(Rect(0, 0, 21, 22));
	_bottom.translate(539, 414);
	_action.setBounds(Rect(0, 0, 21, 21));
	_action.translate(539, 393);
	_send.setBounds(Rect(0, 0, 62, 38));
	_send.translate(503, 373);
	_receive.setBounds(Rect(0, 0, 62, 38));
	_receive.translate(503, 420);
	_call.setBounds(Rect(0, 0, 62, 38));
	_call.translate(503, 383);

	Rect rect(0, 0, 580, 15);
	rect.moveTo(32, 445);
	_text.setBounds(rect);
	_text.setHasBorder(false);

	_items.setup(6, this);
	_items.setFlags(19);
	return true;
}

CRoomItem *CPetRemote::getRoom() const {
	if (_petControl) {
		CGameManager *gameManager = _petControl->getGameManager();
		if (gameManager)
			return gameManager->getRoom();
	}

	return nullptr;
}

int CPetRemote::getHighlightIndex(int val) {
	CRoomItem *room = getRoom();
	if (!room)
		return -1;

	int roomIndex = roomIndexOf(room->getName());
	if (roomIndex == -1)
		return -1;

	// TODO: more

	return -1;
}

int CPetRemote::roomIndexOf(const CString &name) {
	for (int idx = 0; idx < TOTAL_ROOMS; ++idx) {
		if (g_vm->_roomNames[idx] == name)
			return idx;
	}

	return -1;
}

} // End of namespace Titanic
