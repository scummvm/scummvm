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
#include "titanic/pet_control/pet_remote_glyphs.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/messages/pet_messages.h"
#include "titanic/game_manager.h"
#include "titanic/titanic.h"

namespace Titanic {

static const byte REMOTE_DATA[] = {
	0x00, 0x02,
		GLYPH_SUMMON_ELEVATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x01, 0x02,
		GLYPH_SUMMON_PELLERATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x02, 0x03,
		GLYPH_TELEVISION_CONTROL, GLYPH_OPERATE_LIGHTS, GLYPH_SUCCUBUS_DELIVERY,
	0x03, 0x02,
		GLYPH_SUMMON_ELEVATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x04, 0x02,
		GLYPH_TELEVISION_CONTROL, GLYPH_SUCCUBUS_DELIVERY,
	0x05, 0x02,
		GLYPH_SUMMON_PELLERATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x06, 0x02,
		GLYPH_SUMMON_PELLERATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x07, 0x03,
		GLYPH_TELEVISION_CONTROL, GLYPH_SUMMON_PELLERATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x08, 0x01,
		GLYPH_SUCCUBUS_DELIVERY,
	0x09, 0x01,
		GLYPH_SUCCUBUS_DELIVERY,
	0x0A, 0x02,
		GLYPH_SUMMON_ELEVATOR, GLYPH_SUCCUBUS_DELIVERY,
#if defined(DISABLE_SKIP)
	0x0B, 0x01,
		GLYPH_NAVIGATION_CONTROLLER,
#else
	0x0B, 0x02,
		GLYPH_NAVIGATION_CONTROLLER, GLYPH_SKIP_NAVIGATION,
#endif
	0x0C, 0x01,
		GLYPH_SUCCUBUS_DELIVERY,
	0x0D, 0x01,
		GLYPH_SUCCUBUS_DELIVERY,
	0x0E, 0x00,
	0x0F, 0x01,
		GLYPH_TELEVISION_CONTROL,
	0x10, 0x03,
		GLYPH_GOTO_BOTTOM_OF_WELL, GLYPH_GOTO_STATEROOM, GLYPH_GOTO_TOP_OF_WELL,
	0x11, 0x01,
		GLYPH_SUCCUBUS_DELIVERY,
	0x12, 0x00,
	0x13, 0x02,
		GLYPH_SUMMON_PELLERATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x14, 0x00,
	0x15, 0x02,
		GLYPH_SUCCUBUS_DELIVERY, GLYPH_TELEVISION_CONTROL,
	0x16, 0x00,
	0x17, 0x02,
		GLYPH_SUMMON_PELLERATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x18, 0x01,
		GLYPH_SUCCUBUS_DELIVERY,
	0x19, 0x00,
	0x1A, 0x00,
	0x1B, 0x00,
	0x1C, 0x00,
	0x1D, 0x02,
		GLYPH_SUMMON_ELEVATOR, GLYPH_SUCCUBUS_DELIVERY,
	0x1E, 0x0C,
		GLYPH_DEPLOY_FLORAL, GLYPH_DEPLOY_FULLY_RELAXATION, GLYPH_DEPLOY_COMFORT,
		GLYPH_DEPLOY_MINOR_STORAGE, GLYPH_ENTERTAINMENT_DEVICE,
		GLYPH_DEPLOY_MAJOR_RELAXATION, GLYPH_INFLATE_RELAXATION,
		GLYPH_DEPLOY_MAINTENANCE, GLYPH_DEPLOY_WORK_SURFACE,
		GLYPH_DEPLOY_MINOR_RELAXATION, GLYPH_DEPLOY_SINK,
		GLYPH_DEPLOY_MAJOR_STORAGE,
	0x1F, 0x01,
		GLYPH_SUCCUBUS_DELIVERY,
	0x20, 0x02,
		GLYPH_SUMMON_ELEVATOR, GLYPH_SUMMON_PELLERATOR,
	0x21, 0x00
};

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

		_left.reset("PetLeftUp", _petControl, MODE_UNSELECTED);
		_left.reset("PetLeft", _petControl, MODE_SELECTED);
		_right.reset("PetRightUp", _petControl, MODE_UNSELECTED);
		_right.reset("PetRight", _petControl, MODE_SELECTED);
		_top.reset("PetTopUp", _petControl, MODE_UNSELECTED);
		_top.reset("PetTop", _petControl, MODE_SELECTED);
		_bottom.reset("PetBottomUp", _petControl, MODE_UNSELECTED);
		_bottom.reset("PetBottom", _petControl, MODE_SELECTED);
		_action.reset("PetActionUp", _petControl, MODE_UNSELECTED);
		_action.reset("PetAction", _petControl, MODE_SELECTED);

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
	return _items.VirtualKeyCharMsg(msg);
}

bool CPetRemote::MouseWheelMsg(CMouseWheelMsg *msg) {
	if (msg->_wheelUp)
		_items.scrollLeft();
	else
		_items.scrollRight();

	return true;
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
	clearGlyphs();

	if (room) {
		CString roomName = room->getName();
		int roomIndex = roomIndexOf(roomName);
		if (roomIndex != -1) {
			Common::Array<uint> indexes;
			if (getRemoteData(roomIndex, indexes)) {
				loadGlyphs(indexes);
				_items.scrollToStart();
			}
		}
	}
}

CTextControl *CPetRemote::getText() {
	return &_text;
}

CPetGfxElement *CPetRemote::getElement(uint id) {
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

void CPetRemote::highlight(int id) {
	int highlightIndex = getHighlightIndex((RemoteGlyph)id);
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

int CPetRemote::getHighlightIndex(RemoteGlyph val) {
	CRoomItem *room = getRoom();
	if (!room)
		return -1;

	int roomIndex = roomIndexOf(room->getName());
	if (roomIndex == -1)
		return -1;

	Common::Array<uint> remoteData;
	getRemoteData(roomIndex, remoteData);

	// Loop through the data for the room
	for (uint idx = 0; idx < remoteData.size(); ++idx) {
		if ((RemoteGlyph)remoteData[idx] == val)
			return idx;
	}

	return -1;
}

int CPetRemote::roomIndexOf(const CString &name) {
	for (int idx = 0; idx < TOTAL_ROOMS; ++idx) {
		if (g_vm->_roomNames[idx] == name)
			return idx;
	}

	return -1;
}

bool CPetRemote::getRemoteData(int roomIndex, Common::Array<uint> &indexes) {
	const byte *p = &REMOTE_DATA[0];
	for (int idx = 0; idx < TOTAL_ROOMS; ++idx) {
		if (*p == roomIndex) {
			for (int ctr = 0; ctr < p[1]; ++ctr)
				indexes.push_back(p[ctr + 2]);
			return true;
		}

		p += *(p + 1) + 2;
	}

	return false;
}

bool CPetRemote::loadGlyphs(const Common::Array<uint> &indexes) {
	for (uint idx = 0; idx < indexes.size(); ++idx) {
		if (!loadGlyph(indexes[idx]))
			return false;
	}

	return true;
}

bool CPetRemote::loadGlyph(int glyphIndex) {
	CPetRemoteGlyph *glyph = nullptr;

	switch (glyphIndex) {
	case GLYPH_SUMMON_ELEVATOR:
		glyph = new CSummonElevatorGlyph();
		break;

	case GLYPH_SUMMON_PELLERATOR:
		glyph = new CSummonPelleratorGlyph();
		break;

	case GLYPH_TELEVISION_CONTROL:
		glyph = new CTelevisionControlGlyph();
		break;

	case GLYPH_ENTERTAINMENT_DEVICE:
		glyph = new CEntertainmentDeviceGlyph();
		break;

	case GLYPH_OPERATE_LIGHTS:
		glyph = new COperateLightsGlyph();
		break;

	case GLYPH_DEPLOY_FLORAL:
		glyph = new CDeployFloralGlyph();
		break;

	case GLYPH_DEPLOY_FULLY_RELAXATION:
		glyph = new CDeployFullyRelaxationGlyph();
		break;

	case GLYPH_DEPLOY_COMFORT:
		glyph = new CDeployComfortGlyph();
		break;

	case GLYPH_DEPLOY_MINOR_STORAGE:
		glyph = new CDeployMinorStorageGlyph();
		break;

	case GLYPH_DEPLOY_MAJOR_RELAXATION:
		glyph = new CDeployMajorRelaxationGlyph();
		break;

	case GLYPH_INFLATE_RELAXATION:
		glyph = new CInflateRelaxationGlyph();
		break;

	case GLYPH_DEPLOY_MAINTENANCE:
		glyph = new CDeployMaintenanceGlyph();
		break;

	case GLYPH_DEPLOY_WORK_SURFACE:
		glyph = new CDeployWorkSurfaceGlyph();
		break;

	case GLYPH_DEPLOY_MINOR_RELAXATION:
		glyph = new CDeployMinorRelaxationGlyph();
		break;

	case GLYPH_DEPLOY_SINK:
		glyph = new CDeploySinkGlyph();
		break;

	case GLYPH_DEPLOY_MAJOR_STORAGE:
		glyph = new CDeployMajorStorageGlyph();
		break;

	case GLYPH_SUCCUBUS_DELIVERY:
		glyph = new CSuccubusDeliveryGlyph();
		break;

	case GLYPH_NAVIGATION_CONTROLLER:
		glyph = new CNavigationControllerGlyph();
		break;

#if !defined(DISABLE_SKIP)
	case GLYPH_SKIP_NAVIGATION:
		glyph = new CSkipNavigationGlyph();
		break;
#endif

	case GLYPH_GOTO_BOTTOM_OF_WELL:
		glyph = new CGotoBottomOfWellGlyph();
		break;

	case GLYPH_GOTO_TOP_OF_WELL:
		glyph = new CGotoTopOfWellGlyph();
		break;

	case GLYPH_GOTO_STATEROOM:
		glyph = new CGotoStateroomGlyph();
		break;

	case GLYPH_GOTO_BAR:
		glyph = new CGotoBarGlyph();
		break;

	case GLYPH_GOTO_PROMENADE:
		glyph = new CGotoPromenadeDeckGlyph();
		break;

	case GLYPH_GOTO_ARBORETUM:
		glyph = new CGotoArboretumGlyph();
		break;

	case GLYPH_GOTO_MUSIC_ROOM:
		glyph = new CGotoMusicRoomGlyph();
		break;

	case GLYPH_GOTO_RESTAURANT:
		glyph = new CGotoRestaurantGlyph();
		break;

	default:
		break;
	}

	if (glyph) {
		if (glyph->setup(_petControl, &_items)) {
			_items.push_back(glyph);
			return true;
		}
	}

	return false;
}

void CPetRemote::generateMessage(RemoteMessage msgNum, const CString &name, int num) {
	switch (msgNum) {
	case RMSG_LEFT: {
		CPETLeftMsg msg(name, num);
		msg.execute(_petControl->_remoteTarget);
		break;
	}

	case RMSG_RIGHT: {
		CPETRightMsg msg(name, num);
		msg.execute(_petControl->_remoteTarget);
		break;
	}

	case RMSG_UP: {
		CPETUpMsg msg(name, num);
		msg.execute(_petControl->_remoteTarget);
		break;
	}

	case RMSG_DOWN: {
		CPETDownMsg msg(name, num);
		msg.execute(_petControl->_remoteTarget);
		break;
	}

	case RMSG_ACTIVATE: {
		CPETActivateMsg msg(name, num);
		msg.execute(_petControl->_remoteTarget);
		break;
	}

	default:
		break;
	}
}

} // End of namespace Titanic
