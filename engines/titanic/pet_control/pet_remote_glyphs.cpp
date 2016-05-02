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

bool CBasicRemoteGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults(_gfxName, petControl);
	if (owner)
		_gfxElement = getElement(18);
	return true;
}

void CBasicRemoteGlyph::draw2(CScreenManager *screenManager) {
	if (_gfxElement)
		_gfxElement->draw(screenManager);
}

bool CBasicRemoteGlyph::MouseButtonDownMsg(const Point &pt) {
	return _gfxElement && _gfxElement->MouseButtonDownMsg(pt); 
}

bool CBasicRemoteGlyph::MouseButtonUpMsg(const Point &pt) {
	if (_gfxElement && _gfxElement->MouseButtonUpMsg(pt)) {
		getOwner()->generateMessage(RMSG_ACTIVATE, "Lift");
		return true;
	}

	return false;
}

void CBasicRemoteGlyph::getTooltip(CPetText *text) {
	text->setText(_tooltip);
}

/*------------------------------------------------------------------------*/

bool CToggleRemoteGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetGlyph::setup(petControl, owner);
	if (owner)
		_gfxElement = getElement(0);
	return true;
}

void CToggleRemoteGlyph::draw2(CScreenManager *screenManager) {
	_gfxElement->setMode(_flag ? MODE_SELECTED : MODE_UNSELECTED);
	_gfxElement->draw(screenManager);
}

/*------------------------------------------------------------------------*/

bool CTelevisionControlGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetTV", petControl);
	if (owner) {
		_up = getElement(1);
		_down = getElement(2);
		_onOff = getElement(4);
	}
	
	return true;
}

void CTelevisionControlGlyph::draw2(CScreenManager *screenManager) {
	_onOff->setSelected(_flag);
	_onOff->draw(screenManager);
	_up->draw(screenManager);
	_down->draw(screenManager);
}

bool CTelevisionControlGlyph::MouseButtonDownMsg(const Point &pt) {
	if (_onOff && _onOff->MouseButtonDownMsg(pt))
		return true;
	if (_up && _up->MouseButtonDownMsg(pt))
		return true;
	if (_down && _down->MouseButtonDownMsg(pt))
		return true;

	return false;
}

bool CTelevisionControlGlyph::MouseButtonUpMsg(const Point &pt) {
	if (_onOff && _onOff->MouseButtonUpMsg(pt)) {
		_flag = !_flag;
		getOwner()->generateMessage(RMSG_ACTIVATE, "Television");
		return true;
	}

	if (_up && _up->MouseButtonUpMsg(pt)) {
		getOwner()->generateMessage(RMSG_UP, "Television");
		return true;
	}

	if (_down && _down->MouseButtonUpMsg(pt)) {
		getOwner()->generateMessage(RMSG_DOWN, "Television");
		return true;
	}

	return false;
}

void CTelevisionControlGlyph::getTooltip(CPetText *text) {
	text->setText("Television control");
}

/*------------------------------------------------------------------------*/

bool CEntertainmentDeviceGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	if (owner) {
		_gfxElement2 = getElement(1);
		_gfxElement3 = getElement(2);
	}

	return true;
}

} // End of namespace Titanic
