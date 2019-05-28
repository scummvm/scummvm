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
#include "titanic/translation.h"

namespace Titanic {

CPetRooms::CPetRooms() :
		_chevLeftOnDim(nullptr), _chevLeftOffDim(nullptr),
		_chevRightOnDim(nullptr), _chevRightOffDim(nullptr),
		_chevLeftOnLit(nullptr), _chevLeftOffLit(nullptr),
		_chevRightOnLit(nullptr), _chevRightOffLit(nullptr),
		_floorNum(1), _elevatorNum(0), _roomNum(0), _sublevel(1),
		_wellEntry(0), _elevatorBroken(true) {
}

bool CPetRooms::setup(CPetControl *petControl) {
	if (petControl && setupControl(petControl))
		return reset();
	return false;
}

bool CPetRooms::reset() {
	if (_petControl) {
		_plinth.reset("PetChevPlinth", _petControl, MODE_UNSELECTED);
		_glyphs.reset();

		uint col = getColor(0);
		_text.setColor(col);
		_text.setLineColor(0, col);
	}

	return true;
}

void CPetRooms::draw(CScreenManager *screenManager) {
	_petControl->drawSquares(screenManager, 6);
	_plinth.draw(screenManager);
	_glyphs.draw(screenManager);
	_glyphItem.drawAt(screenManager, getGlyphPos(), false);
	_text.draw(screenManager);
}

bool CPetRooms::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_glyphs.MouseButtonDownMsg(msg->_mousePos))
		return true;

	if (!_glyphItem.contains(getGlyphPos(), msg->_mousePos))
		return false;

	_glyphItem.selectGlyph(getGlyphPos(), msg->_mousePos);
	return true;
}

bool CPetRooms::MouseDragStartMsg(CMouseDragStartMsg *msg) {
	if (_glyphs.MouseDragStartMsg(msg))
		return true;

	Point topLeft = getGlyphPos();
	if (!_glyphItem.contains(topLeft, msg->_mousePos))
		return false;

	_glyphItem.dragGlyph(topLeft, msg);
	return true;
}

bool CPetRooms::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	return false;
}

bool CPetRooms::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	return !_glyphs.MouseButtonDownMsg(msg->_mousePos);
}

bool CPetRooms::VirtualKeyCharMsg(CVirtualKeyCharMsg *msg) {
	return _glyphs.VirtualKeyCharMsg(msg);
}

bool CPetRooms::checkDragEnd(CGameObject *item) {
	// Ignore any item drops except onto mail items
	if (!item->_isPendingMail)
		return false;

	uint roomFlags = item->_destRoomFlags;
	CPetRoomsGlyph *glyph = _glyphs.findGlyphByFlags(roomFlags);
	if (glyph) {
		if (_glyphs.findGlyphByFlags(0)) {
			_glyphs.highlight(glyph);
			return false;
		}

		roomFlags = 0;
	}

	addRoom(roomFlags, true);
	return false;
}

void CPetRooms::displayMessage(const CString &msg) {
	_glyphs.resetHighlight();
	CPetSection::displayMessage(msg);
}

bool CPetRooms::isValid(CPetControl *petControl) {
	return setupControl(petControl);
}

void CPetRooms::load(SimpleFile *file, int param) {
	if (!param) {
		int count = file->readNumber();

		for (int idx = 0; idx < count; ++idx) {
			CPetRoomsGlyph *glyph = addGlyph(file->readNumber(), false);
			glyph->setMode((RoomGlyphMode)file->readNumber());
		}

		_glyphItem.setRoomFlags(file->readNumber());
		file->readNumber();
		_floorNum = file->readNumber();
		_elevatorNum = file->readNumber();
		_roomNum = file->readNumber();
		_sublevel = file->readNumber();
		_wellEntry = file->readNumber();
		_elevatorBroken = file->readNumber();
	}
}

void CPetRooms::postLoad() {
	reset();
}

void CPetRooms::save(SimpleFile *file, int indent) {
	_glyphs.saveGlyphs(file, indent);
	_glyphItem.saveGlyph(file, indent);
	file->writeNumberLine(_floorNum, indent);
	file->writeNumberLine(_elevatorNum, indent);
	file->writeNumberLine(_roomNum, indent);
	file->writeNumberLine(_sublevel, indent);
	file->writeNumberLine(_wellEntry, indent);
	file->writeNumberLine(_elevatorBroken, indent);
}

void CPetRooms::enter(PetArea oldArea) {
	if (!_glyphs.highlighted14())
		_text.setText("");
}

void CPetRooms::enterRoom(CRoomItem *room) {
	if (room)
		resetHighlight();
}

CTextControl *CPetRooms::getText() {
	return &_text;
}

CGameObject *CPetRooms::getBackground(int index) const {
	switch (index) {
	case 8:
		return _chevLeftOnDim;
	case 9:
		return _chevLeftOffDim;
	case 10:
		return _chevLeftOnLit;
	case 11:
		return _chevLeftOffLit;
	case 12:
		return _chevRightOnDim;
	case 13:
		return _chevRightOffDim;
	case 14:
		return _chevRightOnLit;
	case 15:
		return _chevRightOffLit;
	default:
		return nullptr;
	}
}

bool CPetRooms::setupControl(CPetControl *petControl) {
	_petControl = petControl;
	if (!petControl)
		return false;

	Rect rect1(0, 0, 470, TRANSLATE(15, 32));
	rect1.moveTo(32, TRANSLATE(445, 439));
	_text.setBounds(rect1);
	_text.setHasBorder(false);

	Rect rect2(0, 0, 81, 81);
	_plinth.setBounds(rect2);
	_plinth.translate(494, 374);

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
	_glyphItem.setFlag(1);
	return true;
}

void CPetRooms::resetHighlight() {
	_glyphItem.setRoomFlags(getRoomFlags());
	_glyphs.resetHighlight();
	_glyphItem.updateTooltip();
	areaChanged(PET_ROOMS);
}

uint CPetRooms::getRoomFlags() const {
	CRoomFlags roomFlags;
	CString roomName = _petControl->getRoomName();

	uint flags = roomFlags.getSpecialRoomFlags(roomName);
	if (flags)
		return flags;

	PassengerClass classNum = roomFlags.whatPassengerClass(_floorNum);
	roomFlags.setPassengerClassBits(classNum);
	roomFlags.setFloorNum(_floorNum);

	switch (classNum) {
	case FIRST_CLASS:
		roomFlags.setElevatorNum(_elevatorNum);
		roomFlags.setRoomBits(_roomNum);
		break;

	case SECOND_CLASS:
		if (_roomNum > 0) {
			if (_roomNum >= 3) {
				roomFlags.setElevatorNum(_elevatorNum == 1 || _elevatorNum == 2 ? 1 : 3);
			} else {
				roomFlags.setElevatorNum(_elevatorNum == 1 || _elevatorNum == 2 ? 2 : 4);
			}

			roomFlags.setRoomBits(((_roomNum - 1) & 1) + (_sublevel > 1 ? 3 : 1));
		} else {
			roomFlags.setRoomBits(0);
		}
		break;

	case THIRD_CLASS:
		roomFlags.setElevatorNum(_elevatorNum);
		roomFlags.setRoomBits(_roomNum + _sublevel * 6 - 6);
		break;

	default:
		break;
	}

	return roomFlags.get();
}

void CPetRooms::reassignRoom(PassengerClass passClassNum) {
	CPetRoomsGlyph *glyph = _glyphs.findAssignedRoom();
	if (glyph)
		// Flag the old assigned room as no longer assigned
		glyph->setMode(RGM_PREV_ASSIGNED_ROOM);

	CRoomFlags roomFlags;
	roomFlags.setRandomLocation(passClassNum, _elevatorBroken);
	glyph = addRoom(roomFlags, true);
	if (glyph) {
		// Flag the new room as assigned to the player, and highlight it
		glyph->setMode(RGM_ASSIGNED_ROOM);
		_glyphs.highlight(glyph);
	}
}

CPetRoomsGlyph *CPetRooms::addRoom(uint roomFlags, bool highlight_) {
	// Ensure that we don't add room if the room is already present
	if (_glyphs.hasFlags(roomFlags))
		return nullptr;

	if (_glyphs.size() >= 32) {
		// Too many room glyphs present. Scan for and remove the first
		// glyph that isn't for an assigned bedroom
		for (CPetRoomsGlyphs::iterator i = _glyphs.begin(); i != _glyphs.end(); ++i) {
			CPetRoomsGlyph *glyph = dynamic_cast<CPetRoomsGlyph *>(*i);
			if (!glyph->isAssigned()) {
				_glyphs.erase(i);
				break;
			}
		}
	}

	// Add the glyph
	return addGlyph(roomFlags, highlight_);
}

CPetRoomsGlyph *CPetRooms::addGlyph(uint roomFlags, bool highlight_) {
	CPetRoomsGlyph *glyph = new CPetRoomsGlyph(roomFlags);
	if (!glyph->setup(_petControl, &_glyphs)) {
		delete glyph;
		return nullptr;
	} else {
		_glyphs.push_back(glyph);
		if (highlight_)
			_glyphs.highlight(glyph);

		return glyph;
	}
}

bool CPetRooms::changeLocationClass(PassengerClass newClassNum) {
	CPetRoomsGlyph *glyph = _glyphs.findAssignedRoom();
	if (!glyph)
		return 0;

	glyph->changeClass(newClassNum);
	return true;
}

bool CPetRooms::isAssignedRoom(uint roomFlags) const {
	for (CPetRoomsGlyphs::const_iterator i = _glyphs.begin(); i != _glyphs.end(); ++i) {
		const CPetRoomsGlyph *glyph = static_cast<const CPetRoomsGlyph *>(*i);
		if (glyph->isAssigned() && glyph->getRoomFlags() == roomFlags)
			return true;
	}

	return false;
}

uint CPetRooms::getAssignedRoomFlags() const {
	CPetRoomsGlyph *glyph = _glyphs.findAssignedRoom();
	return glyph ? glyph->getRoomFlags() : 0;
}

int CPetRooms::getAssignedRoomNum() const {
	uint flags = getAssignedRoomFlags();
	if (!flags)
		return 0;

	return CRoomFlags(flags).getRoomNum();
}

int CPetRooms::getAssignedFloorNum() const {
	uint flags = getAssignedRoomFlags();
	if (!flags)
		return 0;

	return CRoomFlags(flags).getFloorNum();
}

int CPetRooms::getAssignedElevatorNum() const {
	uint flags = getAssignedRoomFlags();
	if (!flags)
		return 0;

	return CRoomFlags(flags).getElevatorNum();
}

} // End of namespace Titanic
