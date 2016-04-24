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
#include "titanic/pet_control/pet_section.h"
#include "titanic/support/screen_manager.h"

namespace Titanic {

CPetRoomsGlyph::CPetRoomsGlyph() : CPetGlyph(),
	_field34(0), _field38(0), _field3C(0),
	_field40(nullptr), _field44(nullptr), _field48(nullptr), _field4C(nullptr),
	_field50(nullptr), _field54(nullptr), _field58(nullptr), _field5C(nullptr) {
}

void CPetRoomsGlyph::set34(int val) {
	_field34 = val;
}

void CPetRoomsGlyph::set38(int val) {
	_field38 = val;
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

void CPetRoomsGlyph::MouseButtonDownMsg(const Point &pt) {

}

int CPetRoomsGlyph::proc29(const Point &pt) {
	return 0; 
}

void CPetRoomsGlyph::proc32() {

}

int CPetRoomsGlyph::proc33() {
	return 1;
}

void CPetRoomsGlyph::proc39() {

}

} // End of namespace Titanic
