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

#include "titanic/pet_control/pet_remote_glyphs.h"
#include "titanic/pet_control/pet_remote.h"

namespace Titanic {

CPetRemote *CPetRemoteGlyphs::getOwner() const {
	return static_cast<CPetRemote *>(_owner);
}

void CPetRemoteGlyphs::generateMessage(RemoteMessage msgNum, const CString &name, int num) {
	getOwner()->generateMessage(msgNum, name, num);
}

/*------------------------------------------------------------------------*/

void CPetRemoteGlyph::setDefaults(const CString &name, CPetControl *petControl) {
	_gfxElement->setBounds(Rect(0, 0, 52, 52));
	_gfxElement->setup(MODE_UNSELECTED, name, petControl);
}

CPetRemoteGlyphs *CPetRemoteGlyph::getOwner() const {
	return static_cast<CPetRemoteGlyphs *>(_owner);
}

CPetGfxElement *CPetRemoteGlyph::getElement(uint id) const {
	CPetRemote *remote = static_cast<CPetRemote *>(_owner->getOwner());
	return remote->getElement(id);
}

/*------------------------------------------------------------------------*/

bool CSummonElevatorGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	setDefaults("3PetLift", petControl);
	if (owner)
		_gfxElement = getElement(18);
	return true;
}

void CSummonElevatorGlyph::draw2(CScreenManager *screenManager) {
	if (_gfxElement)
		_gfxElement->draw(screenManager);
}

bool CSummonElevatorGlyph::MouseButtonDownMsg(const Point &pt) {
	return _gfxElement && _gfxElement->MouseButtonDownMsg(pt); 
}

bool CSummonElevatorGlyph::MouseButtonUpMsg(const Point &pt) {
	if (_gfxElement && _gfxElement->MouseButtonUpMsg(pt)) {
		getOwner()->generateMessage(RMSG_ACTIVATE, "Lift");
		return true;
	}

	return false;
}

void CSummonElevatorGlyph::getTooltip(CPetText *text) {
	text->setText("Summon Elevator");
}

} // End of namespace Titanic
