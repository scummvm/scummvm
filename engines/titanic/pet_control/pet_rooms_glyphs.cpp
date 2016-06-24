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
#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_section.h"
#include "titanic/support/screen_manager.h"
#include "titanic/room_flags.h"
#include "titanic/titanic.h"

namespace Titanic {

CPetRoomsGlyph::CPetRoomsGlyph() : CPetGlyph(),
	_roomFlags(0), _field38(0), _mode(RGM_UNASSIGNED),
	_field40(nullptr), _field44(nullptr), _field48(nullptr), _field4C(nullptr),
	_field50(nullptr), _field54(nullptr), _field58(nullptr), _field5C(nullptr) {
}

CPetRoomsGlyph::CPetRoomsGlyph(uint flags) : CPetGlyph(),
	_roomFlags(flags), _field38(0), _mode(RGM_UNASSIGNED),
	_field40(nullptr), _field44(nullptr), _field48(nullptr), _field4C(nullptr),
	_field50(nullptr), _field54(nullptr), _field58(nullptr), _field5C(nullptr) {
}

bool CPetRoomsGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	if (!CPetGlyph::setup(petControl, owner))
		return false;

	CPetSection *section = owner->getOwner();
	_field40 = section->getBackground(9);
	_field44 = section->getBackground(12);
	_field50 = section->getBackground(13);
	_field54 = section->getBackground(10);
	_field48 = section->getBackground(11);
	_field4C = section->getBackground(14);
	_field58 = section->getBackground(15);
	_field5C = _field58;
	return true;
}

void CPetRoomsGlyph::drawAt(CScreenManager *screenManager, const Point &pt) {
	// Clear background
	Rect rect(pt.x, pt.y, pt.x + 52, pt.y + 52);
	screenManager->fillRect(SURFACE_BACKBUFFER, &rect, 0, 0, 0);

	warning("TODO: CPetRoomsGlyph::drawAt");
}

void CPetRoomsGlyph::proc28(const Point &topLeft, const Point &pt) {
	if (isAssigned()) {
		bool isShiftPressed = g_vm->_events->getSpecialButtons() & MK_SHIFT;

		if (isShiftPressed) {
			int selection = getSelection(topLeft, pt);
			if (selection >= 0)
				_roomFlags |= 1 << selection;
		}

		updateTooltip();
	}
}

int CPetRoomsGlyph::proc29(const Point &pt) {
	bool isShiftPressed = g_vm->_events->getSpecialButtons() & MK_SHIFT;
	CPetControl *petControl = getPetControl();

	if (!isShiftPressed && petControl) {
		CGameObject *chevron = petControl->getHiddenObject("3PetChevron");

		if (chevron) {
			chevron->_id = _roomFlags;
			chevron->_isMail = _field38;
			petControl->removeFromInventory(chevron, false, false);
			chevron->loadSurface();

			warning("TODO: CPetRoomsGlyph::proc29");
			// TODO
		}
	}

	return 0; 
}

void CPetRoomsGlyph::getTooltip(CPetText *text) {
	CRoomFlags roomFlags(_roomFlags);
	CPetSection *owner = getPetSection();

	CString msg;
	if (isCurrentlyAssigned()) {
		msg = "Your assigned room: ";
	} else if (isPreviouslyAssigned()) {
		msg = "A previously assigned room: ";
	} else if (!_field38) {
		msg = "Saved Chevron: ";
	} else if (_field38 == 1 && getRoomFlags() == _roomFlags) {
		msg = "Current location: ";
	}

	// TODO: More stuff
}

void CPetRoomsGlyph::save2(SimpleFile *file, int indent) const {
	file->writeNumberLine(_roomFlags, indent);
	file->writeNumberLine(_mode, indent);
}

int CPetRoomsGlyph::proc33() {
	return 1;
}

void CPetRoomsGlyph::loadFlags(SimpleFile *file, int val) {
	if (!val) {
		_roomFlags = file->readNumber();
	}
}

void CPetRoomsGlyph::changeLocation(int newClassNum) {
	CRoomFlags roomFlags(_roomFlags);
	roomFlags.changeLocation(newClassNum);
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

/*------------------------------------------------------------------------*/

void CPetRoomsGlyphs::save2(SimpleFile *file, int indent) const {
	file->writeNumberLine(size(), indent);

	for (const_iterator i = begin(); i != end(); ++i)
		(*i)->save2(file, indent);
}

CPetRoomsGlyph *CPetRoomsGlyphs::findAssignedRoom() const {
	for (const_iterator i = begin(); i != end(); ++i) {
		CPetRoomsGlyph *glyph = static_cast<CPetRoomsGlyph *>(*i);
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
