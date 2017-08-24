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
#include "titanic/game_manager.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/pet_control/pet_remote.h"
#include "titanic/star_control/star_control.h"
#include "titanic/support/strings.h"
#include "titanic/titanic.h"

namespace Titanic {

CPetRemote *CPetRemoteGlyphs::getOwner() const {
	return dynamic_cast<CPetRemote *>(_owner);
}

void CPetRemoteGlyphs::generateMessage(RemoteMessage msgNum, const CString &name, int num) {
	getOwner()->generateMessage(msgNum, name, num);
}

/*------------------------------------------------------------------------*/

void CPetRemoteGlyph::setDefaults(const CString &name, CPetControl *petControl) {
	_element.setBounds(Rect(0, 0, 52, 52));
	_element.setup(MODE_UNSELECTED, name, petControl);
}

CPetRemoteGlyphs *CPetRemoteGlyph::getOwner() const {
	return dynamic_cast<CPetRemoteGlyphs *>(_owner);
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
		_callButton = getElement(18);
	return true;
}

void CBasicRemoteGlyph::draw2(CScreenManager *screenManager) {
	if (_callButton)
		_callButton->draw(screenManager);
}

bool CBasicRemoteGlyph::MouseButtonDownMsg(const Point &pt) {
	return _callButton && _callButton->MouseButtonDownMsg(pt);
}

bool CBasicRemoteGlyph::MouseButtonUpMsg(const Point &pt) {
	if (_callButton && _callButton->MouseButtonUpMsg(pt)) {
		getOwner()->generateMessage(RMSG_ACTIVATE, _msgString);
		return true;
	}

	return false;
}

void CBasicRemoteGlyph::getTooltip(CTextControl *text) {
	text->setText(_tooltip);
}

/*------------------------------------------------------------------------*/

bool CToggleRemoteGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetGlyph::setup(petControl, owner);
	if (owner)
		_toggle = getElement(0);
	return true;
}

void CToggleRemoteGlyph::draw2(CScreenManager *screenManager) {
	_toggle->setMode(_toggleFlag ? MODE_SELECTED : MODE_UNSELECTED);
	_toggle->draw(screenManager);
}

bool CToggleRemoteGlyph::elementMouseButtonDownMsg(const Point &pt, int petNum) {
	return _toggle->MouseButtonDownMsg(pt);
}

bool CToggleRemoteGlyph::elementMouseButtonUpMsg(const Point &pt, int petNum) {
	if (!_toggle->MouseButtonUpMsg(pt))
		return false;

	CTreeItem *target = getPetControl()->_remoteTarget;
	if (target) {
		CPETActivateMsg msg("SGTSelector", petNum);
		msg.execute(target);
		_toggleFlag = !_toggleFlag;
		_toggle->setMode(_toggleFlag ? MODE_SELECTED : MODE_UNSELECTED);
	}

	return true;
}

/*------------------------------------------------------------------------*/

bool CRemoteGotoGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults(_gfxName, petControl);

	if (owner)
		_goButton = getElement(7);

	return true;
}

void CRemoteGotoGlyph::draw2(CScreenManager *screenManager) {
	if (_goButton)
		_goButton->draw(screenManager);
}

bool CRemoteGotoGlyph::MouseButtonDownMsg(const Point &pt) {
	return _goButton && _goButton->MouseButtonDownMsg(pt);
}

bool CRemoteGotoGlyph::MouseButtonUpMsg(const Point &pt) {
	if (!_goButton || !_goButton->MouseButtonUpMsg(pt))
		return false;

	CPetControl *petControl = getPetControl();
	if (petControl) {
		CGameManager *gameManager = petControl->getGameManager();

		if (gameManager) {
			CRoomItem *room = gameManager->getRoom();

			if (room) {
				CTransportMsg msg(g_vm->_roomNames[_roomIndex], 1, 0);
				msg.execute(room);
			}
		}
	}

	return true;
}

void CRemoteGotoGlyph::getTooltip(CTextControl *text) {
	text->setText(_tooltip);
}

/*------------------------------------------------------------------------*/

bool CTelevisionControlGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetTV", petControl);
	if (owner) {
		_up = getElement(1);
		_down = getElement(2);
		_onOff = getElement(0);
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

void CTelevisionControlGlyph::getTooltip(CTextControl *text) {
	text->setText(TELEVISION_CONTROL);
}

/*------------------------------------------------------------------------*/

bool CEntertainmentDeviceGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetSGTtv", petControl);
	if (owner) {
		_up = getElement(1);
		_down = getElement(2);
	}

	return true;
}

void CEntertainmentDeviceGlyph::draw2(CScreenManager *screenManager) {
	CString viewName = getPetControl()->getFullViewName();
	if (viewName == "SGTState.Node 1.S") {
		_toggle->setSelected(_toggleFlag);
		_toggle->draw(screenManager);
	} else if (viewName == "SGTState.Node 4.E") {
		_toggle->setSelected(_flag2);
		_toggle->draw(screenManager);
		_up->draw(screenManager);
		_down->draw(screenManager);
	}
}

bool CEntertainmentDeviceGlyph::MouseButtonDownMsg(const Point &pt) {
	CString viewName = getPetControl()->getFullViewName();
	if (viewName == "SGTState.Node 1.S") {
		return elementMouseButtonDownMsg(pt, 4);
	} else if (viewName == "SGTState.Node 4.E") {
		return _toggle->MouseButtonDownMsg(pt)
			|| _up->MouseButtonDownMsg(pt)
			|| _down->MouseButtonDownMsg(pt);
	}

	return false;
}

bool CEntertainmentDeviceGlyph::MouseButtonUpMsg(const Point &pt) {
	CString viewName = getPetControl()->getFullViewName();
	if (viewName == "SGTState.Node 1.S") {
		return elementMouseButtonUpMsg(pt, 4);
	} else if (viewName == "SGTState.Node 4.E") {
		if (_toggle->MouseButtonUpMsg(pt)) {
			_flag2 = !_flag2;
			getOwner()->generateMessage(RMSG_ACTIVATE, "Television");
			return true;
		} else if (_up->MouseButtonUpMsg(pt)) {
			getOwner()->generateMessage(RMSG_UP, "Television");
			return true;
		} else if (_down->MouseButtonUpMsg(pt)) {
			getOwner()->generateMessage(RMSG_DOWN, "Television");
			return true;
		}
	}

	return false;
}

void CEntertainmentDeviceGlyph::getTooltip(CTextControl *text) {
	text->setText(OPERATE_ENTERTAINMENT);
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
	else if (_activate && _activate->MouseButtonUpMsg(pt))
		getOwner()->generateMessage(RMSG_ACTIVATE, "Light");

	return true;
}

void COperateLightsGlyph::getTooltip(CTextControl *text) {
	text->setText(OPERATE_LIGHTS);
}

/*------------------------------------------------------------------------*/

bool CDeployFloralGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetVase", petControl);
	return true;
}

void CDeployFloralGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_FLORAL_ENHANCEMENT);
}


/*------------------------------------------------------------------------*/

bool CDeployFullyRelaxationGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetBedfoot", petControl);
	return true;
}

void CDeployFullyRelaxationGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_FULLY_RELAXATION);
}

/*------------------------------------------------------------------------*/

bool CDeployComfortGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetToilet", petControl);
	return true;
}

void CDeployComfortGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_COMFORT_WORKSTATION);
}

/*------------------------------------------------------------------------*/

bool CDeployMinorStorageGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetDraw", petControl);
	return true;
}

void CDeployMinorStorageGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_MINOR_STORAGE);
}

/*------------------------------------------------------------------------*/

bool CDeployMajorRelaxationGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetArmChair", petControl);
	return true;
}

void CDeployMajorRelaxationGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_MAJOR_RELAXATION);
}

/*------------------------------------------------------------------------*/

bool CInflateRelaxationGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetBedhead", petControl);
	return true;
}

void CInflateRelaxationGlyph::getTooltip(CTextControl *text) {
	text->setText(INFLATE_RELAXATION_DEVICE);
}

/*------------------------------------------------------------------------*/

bool CDeployMaintenanceGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetWashstand", petControl);
	return true;
}

void CDeployMaintenanceGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_MAINTENANCE_HUB);
}

/*------------------------------------------------------------------------*/

bool CDeployWorkSurfaceGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetTable", petControl);
	return true;
}

void CDeployWorkSurfaceGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_EXECUTIVE_SURFACE);
}

/*------------------------------------------------------------------------*/

bool CDeployMinorRelaxationGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetDeskchair", petControl);
	return true;
}

void CDeployMinorRelaxationGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_MINOR_RELAXATION);
}

/*------------------------------------------------------------------------*/

bool CDeploySinkGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetBasin", petControl);
	return true;
}

void CDeploySinkGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_SINK);
}

/*------------------------------------------------------------------------*/

bool CDeployMajorStorageGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CToggleRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetChest", petControl);
	return true;
}

void CDeployMajorStorageGlyph::getTooltip(CTextControl *text) {
	text->setText(DEPLOY_MAJOR_STORAGE);
}

/*------------------------------------------------------------------------*/

bool CSuccubusDeliveryGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetSuccubus", petControl);

	if (owner) {
		_send = getElement(16);
		_receive = getElement(17);
	}

	return true;
}

void CSuccubusDeliveryGlyph::draw2(CScreenManager *screenManager) {
	_send->draw(screenManager);
	_receive->draw(screenManager);
}

bool CSuccubusDeliveryGlyph::MouseButtonDownMsg(const Point &pt) {
	return _send->MouseButtonDownMsg(pt)
		|| _receive->MouseButtonDownMsg(pt);
}

bool CSuccubusDeliveryGlyph::MouseButtonUpMsg(const Point &pt) {
	CTreeItem *target = getPetControl()->_remoteTarget;

	if (_send && _send->MouseButtonUpMsg(pt)) {
		if (target) {
			CPETDeliverMsg msg;
			msg.execute(target);
		}
	} else if (_receive && _receive->MouseButtonUpMsg(pt)) {
		if (target) {
			CPETReceiveMsg msg;
			msg.execute(target);
		}
	} else {
		return false;
	}

	return true;
}

void CSuccubusDeliveryGlyph::getTooltip(CTextControl *text) {
	text->setText(SUCCUBUS_DELIVERY_SYSTEM);
}

/*------------------------------------------------------------------------*/

bool CNavigationControllerGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetStarField", petControl);

	if (owner)
		_gfxElement = getElement(0);

	return true;
}

void CNavigationControllerGlyph::draw2(CScreenManager *screenManager) {
	_gfxElement->setMode(_flag ? MODE_SELECTED : MODE_UNSELECTED);
	_gfxElement->draw(screenManager);
}

bool CNavigationControllerGlyph::MouseButtonDownMsg(const Point &pt) {
	return _gfxElement->MouseButtonDownMsg(pt);
}

bool CNavigationControllerGlyph::MouseButtonUpMsg(const Point &pt) {
	if (!_gfxElement->MouseButtonUpMsg(pt))
		return false;

	CPetControl *pet = getPetControl();
	CStarControl *starControl = pet->getStarControl();
	_flag = !_flag;

	if (!starControl->isSkipped()) {
		CTreeItem *target = pet->_remoteTarget;
		if (target) {
			CPETHelmetOnOffMsg msg;
			msg.execute(target);
		}
	}

	return true;
}

void CNavigationControllerGlyph::getTooltip(CTextControl *text) {
	text->setText(NAVIGATION_CONTROLLER);
}

/*------------------------------------------------------------------------*/

CSummonElevatorGlyph::CSummonElevatorGlyph() : CBasicRemoteGlyph(
		"3PetLift", g_vm->_strings[SUMMON_ELEVATOR], "Lift") {
}

/*------------------------------------------------------------------------*/

CSummonPelleratorGlyph::CSummonPelleratorGlyph() : CBasicRemoteGlyph(
		"3PetPellerator", g_vm->_strings[SUMMON_PELLERATOR], "Pellerator") {
}

/*------------------------------------------------------------------------*/

CGotoBottomOfWellGlyph::CGotoBottomOfWellGlyph() : CRemoteGotoGlyph("3PetBotOfWell",
		g_vm->_strings[GO_TO_BOTTOM_OF_WELL], 10) {
}

/*------------------------------------------------------------------------*/

CGotoTopOfWellGlyph::CGotoTopOfWellGlyph() : CRemoteGotoGlyph("3PetTopOfWell",
		g_vm->_strings[GO_TO_TOP_OF_WELL], 32) {
}

/*------------------------------------------------------------------------*/

CGotoStateroomGlyph::CGotoStateroomGlyph() : CRemoteGotoGlyph("3PetRoom",
		g_vm->_strings[GO_TO_STATEROOM], 33) {
}

/*------------------------------------------------------------------------*/

CGotoBarGlyph::CGotoBarGlyph() : CRemoteGotoGlyph("3PetBar",
		g_vm->_strings[GO_TO_BAR], 7) {
}

/*------------------------------------------------------------------------*/

CGotoPromenadeDeckGlyph::CGotoPromenadeDeckGlyph() : CRemoteGotoGlyph("3PetPromDeck",
		g_vm->_strings[GO_TO_PROMENADE_DECK], 23) {
}

/*------------------------------------------------------------------------*/

CGotoArboretumGlyph::CGotoArboretumGlyph() : CRemoteGotoGlyph("3PetArboretum",
		g_vm->_strings[GO_TO_ARBORETUM], 5) {
}

/*------------------------------------------------------------------------*/

CGotoMusicRoomGlyph::CGotoMusicRoomGlyph() : CRemoteGotoGlyph("3PetMusicRoom",
		g_vm->_strings[GO_TO_MUSIC_ROOM], 20) {
}

/*------------------------------------------------------------------------*/

CGotoRestaurantGlyph::CGotoRestaurantGlyph() : CRemoteGotoGlyph("3Pet1stClassRest",
		g_vm->_strings[GO_TO_1ST_CLASS_RESTAURANT], 1) {
}

/*------------------------------------------------------------------------*/

bool CSkipNavigationGlyph::setup(CPetControl *petControl, CPetGlyphs *owner) {
	CPetRemoteGlyph::setup(petControl, owner);
	setDefaults("3PetTV", petControl);
	if (owner) {
		_button = getElement(7);
	}

	return true;
}

void CSkipNavigationGlyph::draw2(CScreenManager *screenManager) {
	_button->draw(screenManager);
}

bool CSkipNavigationGlyph::MouseButtonDownMsg(const Point &pt) {
	if (_button && _button->MouseButtonDownMsg(pt))
		return true;

	return false;
}

bool CSkipNavigationGlyph::MouseButtonUpMsg(const Point &pt) {
	if (_button && _button->MouseButtonUpMsg(pt)) {
		CPetRemote *remote = static_cast<CPetRemote *>(_owner->getOwner());
		CStarControl *starControl = remote->getPetControl()->getStarControl();
		starControl->forceSolved();

		CActMsg actMsg("SetDestin");
		actMsg.execute("CaptainsWheel");
		return true;
	}

	return false;
}

void CSkipNavigationGlyph::getTooltip(CTextControl *text) {
	text->setText(SKIP_NAVIGATION);
}

} // End of namespace Titanic
