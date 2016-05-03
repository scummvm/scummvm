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
#include "titanic/pet_control/pet_control.h"
#include "titanic/messages/pet_messages.h"

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

bool CToggleRemoteGlyph::elementMouseButtonDownMsg(const Point &pt) {
	return _gfxElement->MouseButtonDownMsg(pt);
}

bool CToggleRemoteGlyph::elementMouseButtonUpMsg(const Point &pt) {
	if (!_gfxElement->MouseButtonUpMsg(pt))
		return false;

	CTreeItem *target = getPetControl()->_remoteTarget;
	if (target) {
		CPETActivateMsg msg("SGTSelector", -1);
		msg.execute(target);
		_flag = !_flag;
		_gfxElement->setMode(_flag ? MODE_SELECTED : MODE_UNSELECTED);
	}

	return true;
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

void CEntertainmentDeviceGlyph::draw2(CScreenManager *screenManager) {
	CString viewName = getPetControl()->getFullViewName();
	if (viewName == "SGTState.Node 1.S") {
		_gfxElement->setSelected(_flag);
		_gfxElement->draw(screenManager);
	} else if (viewName == "SGTState.Node 4.E") {
		_gfxElement->setSelected(_flag2);
		_gfxElement->draw(screenManager);
		_gfxElement2->draw(screenManager);
		_gfxElement3->draw(screenManager);
	}
}

bool CEntertainmentDeviceGlyph::MouseButtonDownMsg(const Point &pt) {
	CString viewName = getPetControl()->getFullViewName();
	if (viewName == "SGTState.Node 1.S") {
		return elementMouseButtonDownMsg(pt);
	} else if (viewName == "SGTState.Node 4.E") {
		return _gfxElement->MouseButtonDownMsg(pt)
			|| _gfxElement2->MouseButtonDownMsg(pt)
			|| _gfxElement3->MouseButtonDownMsg(pt);
	}

	return false;
}

bool CEntertainmentDeviceGlyph::MouseButtonUpMsg(const Point &pt) {
	CString viewName = getPetControl()->getFullViewName();
	if (viewName == "SGTState.Node 1.S") {
		return elementMouseButtonUpMsg(pt);
	} else if (viewName == "SGTState.Node 4.E") {
		if (_gfxElement->MouseButtonUpMsg(pt)) {
			_flag2 = !_flag2;
			getOwner()->generateMessage(RMSG_ACTIVATE, "Television");
			return true;
		} else if (_gfxElement2->MouseButtonUpMsg(pt)) {
			getOwner()->generateMessage(RMSG_UP, "Television");
			return true;
		}
		else if (_gfxElement3->MouseButtonUpMsg(pt)) {
			getOwner()->generateMessage(RMSG_DOWN, "Television");
			return true;
		}
	}

	return false;
}

void CEntertainmentDeviceGlyph::getTooltip(CPetText *text) {
	text->setText("Operate visual entertainment device");
}

/*------------------------------------------------------------------------*/


bool COperateLightsGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetLights", petControl);

	if (owner) {
		_left = getElement(3);
		_right = getElement(4);
		_up = getElement(5);
		_down = getElement(6);
		_activate = getElement(7);
	}

	return true;
}

void COperateLightsGlyph::draw2(CScreenManager *screenManager) {
	_left->draw(screenManager);
	_right->draw(screenManager);
	_up->draw(screenManager);
	_down->draw(screenManager);
	_activate->draw(screenManager);
}

bool COperateLightsGlyph::MouseButtonDownMsg(const Point &pt) {
	if (_left->MouseButtonDownMsg(pt)
			|| _right->MouseButtonDownMsg(pt)
			|| _up->MouseButtonDownMsg(pt)
			|| _down->MouseButtonDownMsg(pt)
			|| _activate->MouseButtonDownMsg(pt))
		return true;
	return true;
}

bool COperateLightsGlyph::MouseButtonUpMsg(const Point &pt) {
	if (_left && _left->MouseButtonUpMsg(pt))
		getOwner()->generateMessage(RMSG_LEFT, "Light");
	else if (_right && _right->MouseButtonUpMsg(pt))
		getOwner()->generateMessage(RMSG_RIGHT, "Light");
	else if (_up && _up->MouseButtonUpMsg(pt))
		getOwner()->generateMessage(RMSG_UP, "Light");
	else if (_down && _down->MouseButtonUpMsg(pt))
		getOwner()->generateMessage(RMSG_DOWN, "Light");
	else
		getOwner()->generateMessage(RMSG_ACTIVATE, "Light");

	return true;
}

void COperateLightsGlyph::getTooltip(CPetText *text) {
	text->setText("Operate the lights");
}

} // End of namespace Titanic
