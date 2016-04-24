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

#include "titanic/pet_control/pet_rooms.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

CPetRooms::CPetRooms() :
		_chevLeftOnDim(nullptr), _chevLeftOffDim(nullptr),
		_chevRightOnDim(nullptr), _chevRightOffDim(nullptr),
		_chevLeftOnLit(nullptr), _chevLeftOffLit(nullptr),
		_chevRightOnLit(nullptr), _chevRightOffLit(nullptr),
		_field1C0(0), _field1C4(0), _field1C8(0), _field1CC(0),
		_field1D0(0), _field1D4(0) {
}

bool CPetRooms::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetRooms::reset() {
	return true;
}

void CPetRooms::draw(CScreenManager *screenManager) {

}

bool CPetRooms::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CPetRooms::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	return true;
}

bool CPetRooms::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return true;
}

bool CPetRooms::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	return true;
}

bool CPetRooms::VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) {
	return true;
}

int CPetRooms::proc14() {
	return 0;
}

void CPetRooms::displayMessage(const CString &msg) {

}

bool CPetRooms::isValid(CPetControl *petControl) {
	return true;
}

void CPetRooms::load(SimpleFile *file, int param) {
	if (!param) {
		int count = file->readNumber();

		for (int idx = 0; idx < count; ++idx) {
			int v1 = file->readNumber();
			int v2 = file->readNumber();
			warning("TODO: CPetRoomsSection::load - %d,%d", v1, v2);
		}

		_glyphItem.set34(file->readNumber());
		file->readNumber();
		_field1C0 = file->readNumber();
		_field1C4 = file->readNumber();
		_field1C8 = file->readNumber();
		_field1CC = file->readNumber();
		_field1D0 = file->readNumber();
		_field1D4 = file->readNumber();
	}
}

void CPetRooms::postLoad() {

}

void CPetRooms::save(SimpleFile *file, int indent) const {

}

void CPetRooms::enter(PetArea oldArea) {

}

void CPetRooms::enterRoom(CRoomItem *room) {

}

CPetText *CPetRooms::getText() {
	return &_text;
}

CGameObject *CPetRooms::getBackground(int index) {
	return nullptr;
}

bool CPetRooms::setupControl(CPetControl *petControl) {
	_petControl = petControl;
	if (!petControl)
		return false;

	Rect rect1(0, 0, 470, 15);
	rect1.moveTo(32, 445);
	_text.setBounds(rect1);
	_text.setHasBorder(false);

	Rect rect2(0, 0, 81, 81);
	rect2.moveTo(374, 494);
	_element.setBounds(rect2);

	_chevLeftOnDim = petControl->getHiddenObject("3PetChevLeftOnDim");
	_chevLeftOffDim = petControl->getHiddenObject("3PetChevLeftOffDim");
	_chevRightOnDim = petControl->getHiddenObject("3PetChevRightOnDim");
	_chevRightOffDim = petControl->getHiddenObject("3PetChevRightOffDim");
	_chevLeftOnLit = petControl->getHiddenObject("3PetChevLeftOnLit");
	_chevLeftOffLit = petControl->getHiddenObject("3PetChevLeftOffLit");
	_chevRightOnLit = petControl->getHiddenObject("3PetChevRightOnLit");
	_chevRightOffLit = petControl->getHiddenObject("3PetChevRightOffLit");

	_glyphs.setup(6, this);
	_glyphs.setFlags(GFLAG_16);
	_glyphItem.setup(petControl, &_glyphs);
	_glyphItem.set38(1);
}

} // End of namespace Titanic
