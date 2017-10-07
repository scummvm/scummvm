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

#include "titanic/pet_control/pet_rooms_glyphs.h"
#include "titanic/events.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/room_flags.h"
#include "titanic/support/screen_manager.h"
#include "titanic/support/simple_file.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

CPetRoomsGlyph::CPetRoomsGlyph() : CPetGlyph(),
		_roomFlags(0), _mailFlag(0), _mode(RGM_UNASSIGNED),
		_chevLeftOnDim(nullptr), _chevLeftOffDim(nullptr), _chevLeftOnLit(nullptr), _chevLeftOffLit(nullptr),
		_chevRightOnDim(nullptr), _chevRightOffDim(nullptr), _chevRightOnLit(nullptr), _chevRightOffLit(nullptr) {
}

CPetRoomsGlyph::CPetRoomsGlyph(uint flags) : CPetGlyph(),
		_roomFlags(flags), _mailFlag(0), _mode(RGM_UNASSIGNED),
		_chevLeftOnDim(nullptr), _chevLeftOffDim(nullptr), _chevLeftOnLit(nullptr), _chevLeftOffLit(nullptr),
		_chevRightOnDim(nullptr), _chevRightOffDim(nullptr), _chevRightOnLit(nullptr), _chevRightOffLit(nullptr) {
}

bool CPetRoomsGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	if (!CPetGlyph::setup(petControl, owner))
		return false;

	CPetSection *section = owner->getOwner();
	_chevLeftOnDim = section->getBackground(8);
	_chevLeftOffDim = section->getBackground(9);
	_chevRightOnDim = section->getBackground(12);
	_chevRightOffDim = section->getBackground(13);
	_chevLeftOnLit = section->getBackground(10);
	_chevLeftOffLit = section->getBackground(11);
	_chevRightOnLit = section->getBackground(14);
	_chevRightOffLit = section->getBackground(15);
	return true;
}

void CPetRoomsGlyph::drawAt(CScreenManager *screenManager, const Point &pt, bool isHighlighted_) {
	// Clear background
	Rect rect(pt.x, pt.y, pt.x + 52, pt.y + 52);
	screenManager->fillRect(SURFACE_BACKBUFFER, &rect, 0, 0, 0);

	CRoomFlags roomFlags(_roomFlags);
	uint elevBits = roomFlags.getElevatorBits();
	uint classBits = roomFlags.getPassengerClassBits();
	uint floorBits = roomFlags.getFloorBits();
	uint roomBits = roomFlags.getRoomBits();

	// Save a copy of object pointers that may be modified
	CGameObject *leftOnDim = _chevLeftOnDim;
	CGameObject *leftOffDim = _chevLeftOffDim;
	CGameObject *rightOnDim = _chevRightOnDim;
	CGameObject *rightOffDim = _chevRightOffDim;

	if (_mailFlag || isHighlighted_) {
		_chevLeftOnDim = _chevLeftOnLit;
		_chevLeftOffDim = _chevLeftOffLit;
		_chevRightOnDim = _chevRightOnLit;
		_chevRightOffDim = _chevRightOffLit;
	}

	// Draw the chevron fragments for each line
	Point destPt = pt;
	drawObjects(classBits + elevBits * 4, destPt, screenManager);
	destPt.y += 10;
	drawObjects((floorBits >> 4) & 15, destPt, screenManager);
	destPt.y += 10;
	drawObjects(floorBits & 15, destPt, screenManager);
	destPt.y += 10;
	drawObjects(roomBits >> 3, destPt, screenManager);
	destPt.y += 10;
	drawObjects(((roomBits & 7) << 1) + (roomFlags.getBit0() ? 1 : 0),
		destPt, screenManager);

	// Restore original image pointers
	_chevLeftOnDim = leftOnDim;
	_chevLeftOffDim = leftOffDim;
	_chevRightOnDim = rightOnDim;
	_chevRightOffDim = rightOffDim;
}

void CPetRoomsGlyph::selectGlyph(const Point &topLeft, const Point &pt) {
	if (!isAssigned()) {
		bool isShiftPressed = g_vm->_events->getSpecialButtons() & MK_SHIFT;

		if (isShiftPressed) {
			int selection = getSelection(topLeft, pt);
			if (selection >= 0)
				_roomFlags ^= 1 << selection;
		}

		updateTooltip();
	}
}

bool CPetRoomsGlyph::dragGlyph(const Point &topLeft, CMouseDragStartMsg *msg) {
	bool isShiftPressed = g_vm->_events->getSpecialButtons() & MK_SHIFT;
	CPetControl *petControl = getPetControl();

	if (!isShiftPressed && petControl) {
		CGameObject *chevron = petControl->getHiddenObject("3PetChevron");

		if (chevron) {
			chevron->_destRoomFlags = _roomFlags;
			chevron->_isPendingMail = _mailFlag != 0;
			petControl->removeFromInventory(chevron, false, false);
			chevron->loadSurface();

			chevron->dragMove(msg->_mousePos);
			msg->_handled = true;

			if (msg->execute(chevron))
				return true;

			petControl->moveToHiddenRoom(chevron);
		}
	}

	return false;
}

void CPetRoomsGlyph::getTooltip(CTextControl *text) {
	CRoomFlags roomFlags(_roomFlags);
	CPetRooms *owner = static_cast<CPetRooms *>(getPetSection());

	CString prefix;
	if (isCurrentlyAssigned()) {
		prefix = g_vm->_strings[YOUR_ASSIGNED_ROOM];
	} else if (isPreviouslyAssigned()) {
		prefix = g_vm->_strings[PREVIOUSLY_ASSIGNED_ROOM];
	} else if (!_mailFlag) {
		prefix = g_vm->_strings[SAVED_CHEVRON];
	} else if (_mailFlag == 1 && owner->getRoomFlags() == _roomFlags) {
		prefix = g_vm->_strings[CURRENT_LOCATION];
	}

	// Get the room description
	CString roomStr = roomFlags.getRoomDesc();

	if (roomStr == TRANSLATE("The Elevator", "Der Aufzug")) {
		int elevNum = owner->getElevatorNum();
		roomStr = CString::format(g_vm->_strings[ELEVATOR_NUM].c_str(), elevNum);
	}

	roomStr += g_vm->_strings[SHIFT_CLICK_TO_EDIT];
	text->setText(prefix + roomStr);
}

void CPetRoomsGlyph::saveGlyph(SimpleFile *file, int indent) {
	file->writeNumberLine(_roomFlags, indent);
	file->writeNumberLine(_mode, indent);
}

bool CPetRoomsGlyph::proc33(CPetGlyph *glyph) {
	CPetRoomsGlyph *roomGlyph = static_cast<CPetRoomsGlyph *>(glyph);

	return CPetGlyph::proc33(glyph) && _roomFlags == roomGlyph->_roomFlags;
}

void CPetRoomsGlyph::loadFlags(SimpleFile *file, int val) {
	if (!val) {
		_roomFlags = file->readNumber();
	}
}

void CPetRoomsGlyph::changeClass(PassengerClass newClassNum) {
	CRoomFlags roomFlags(_roomFlags);
	roomFlags.changeClass(newClassNum);
	_roomFlags = roomFlags.get();
}

int CPetRoomsGlyph::getSelection(const Point &topLeft, const Point &pt) {
	Rect rects[4] = {
		Rect(topLeft.x, topLeft.y, topLeft.x + 13, topLeft.y + 10),
		Rect(topLeft.x + 13, topLeft.y, topLeft.x + 26, topLeft.y + 10),
		Rect(topLeft.x + 26, topLeft.y, topLeft.x + 39, topLeft.y + 10),
		Rect(topLeft.x + 39, topLeft.y, topLeft.x + 52, topLeft.y + 10)
	};

	for (int idx = 0, btnIndex = 19; idx < 5; ++idx, btnIndex -= 4) {
		// Iterate through each of the four rects, seeing if there's a match.
		// If not, move it down to the next row for the next loop iteration
		for (int i = 0; i < 4; ++i) {
			if (rects[i].contains(pt))
				return btnIndex - i;

			rects[i].translate(0, 10);
		}
	}

	return -1;
}

void CPetRoomsGlyph::drawObjects(uint flags, const Point &pt, CScreenManager *screenManager) {
	if (_chevLeftOnDim && _chevLeftOffDim && _chevRightOnDim && _chevRightOffDim) {
		Point destPos = pt;
		((flags & 8) ? _chevLeftOnDim : _chevLeftOffDim)->draw(screenManager, destPos);
		destPos.x += 13;
		((flags & 4) ? _chevRightOnDim : _chevRightOffDim)->draw(screenManager, destPos);
		destPos.x += 13;
		((flags & 2) ? _chevLeftOnDim : _chevLeftOffDim)->draw(screenManager, destPos);
		destPos.x += 13;
		((flags & 1) ? _chevRightOnDim : _chevRightOffDim)->draw(screenManager, destPos);
	}
}


/*------------------------------------------------------------------------*/

void CPetRoomsGlyphs::saveGlyphs(SimpleFile *file, int indent) {
	file->writeNumberLine(size(), indent);

	for (const_iterator i = begin(); i != end(); ++i)
		(*i)->saveGlyph(file, indent);
}

CPetRoomsGlyph *CPetRoomsGlyphs::findAssignedRoom() const {
	for (const_iterator i = begin(); i != end(); ++i) {
		CPetRoomsGlyph *glyph = dynamic_cast<CPetRoomsGlyph *>(*i);
		if (glyph->isCurrentlyAssigned())
			return glyph;
	}

	return nullptr;
}

CPetRoomsGlyph *CPetRoomsGlyphs::findGlyphByFlags(uint flags) const {
	for (const_iterator i = begin(); i != end(); ++i) {
		CPetRoomsGlyph *glyph = static_cast<CPetRoomsGlyph *>(*i);
		if (glyph->getRoomFlags() == flags)
			return glyph;
	}

	return nullptr;
}

} // End of namespace Titanic
