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

#include "titanic/npcs/parrot_succubus.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/carry/hose.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrotSuccUBus, CSuccUBus)
	ON_MESSAGE(HoseConnectedMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveNodeMsg)
END_MESSAGE_MAP()

CParrotSuccUBus::CParrotSuccUBus() : CSuccUBus(), _hoseConnected(false),
	_field1EC(0), _field1F0(376), _field1F4(393) {
}

void CParrotSuccUBus::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_hoseConnected, indent);
	file->writeQuotedLine(_string3, indent);
	file->writeNumberLine(_field1EC, indent);

	CSuccUBus::save(file, indent);
}

void CParrotSuccUBus::load(SimpleFile *file) {
	file->readNumber();
	_hoseConnected = file->readNumber();
	_string3 = file->readString();
	_field1EC = file->readNumber();

	CSuccUBus::load(file);
}

bool CParrotSuccUBus::HoseConnectedMsg(CHoseConnectedMsg *msg) {
	CPetControl *pet = getPetControl();
	if (msg->_connected == _hoseConnected)
		return true;
	if (mailExists(pet->getRoomFlags()))
		return false;

	_hoseConnected = msg->_connected;
	if (_hoseConnected) {
		CGameObject *item = msg->_object;
		_string3 = item->getName();
		CHoseConnectedMsg hoseMsg(1, this);
		hoseMsg.execute(msg->_object);
		item->petMoveToHiddenRoom();

		CPumpingMsg pumpingMsg(1, this);
		pumpingMsg.execute(this);
		_hoseConnected = true;

		if (_enabled) {
			_enabled = false;
		} else {
			playMovie(_onStartFrame, _onEndFrame, 0);
			playSound("z#26.wav");
		}

		playMovie(_hoseStartFrame, _hoseEndFrame, MOVIE_NOTIFY_OBJECT);
	} else {
		stopMovie();
		stopSound(_field1EC);
		playMovie(_field1F0, _field1F4, MOVIE_NOTIFY_OBJECT);

		CPumpingMsg pumpingMsg(0, this);
		pumpingMsg.execute(_string3);

		CGameObject *obj = getHiddenObject(_string3);
		if (obj) {
			obj->petAddToInventory();
			obj->setVisible(true);
		}

		_enabled = true;
		CTurnOff offMsg;
		offMsg.execute(this);
	}

	return true;
}

bool CParrotSuccUBus::EnterViewMsg(CEnterViewMsg *msg) {
	if (_hoseConnected) {
		playMovie(_pumpingStartFrame, _pumpingEndFrame, MOVIE_REPEAT);
		return true;
	} else {
		return CSuccUBus::EnterViewMsg(msg);
	}
}

bool CParrotSuccUBus::MovieEndMsg(CMovieEndMsg *msg) {
	if (msg->_endFrame == _hoseEndFrame) {
		playMovie(_pumpingStartFrame, _pumpingEndFrame, MOVIE_REPEAT);
		_field1EC = playSound("z#472.wav");
		return true;
	} else {
		return CSuccUBus::MovieEndMsg(msg);
	}
}

bool CParrotSuccUBus::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (_hoseConnected) {
		CHoseConnectedMsg hoseMsg;
		hoseMsg._connected = false;
		hoseMsg.execute(this);
		return true;
	} else {
		return CSuccUBus::MouseButtonDownMsg(msg);
	}
}

bool CParrotSuccUBus::LeaveNodeMsg(CLeaveNodeMsg *msg) {
	if (_hoseConnected) {
		getHiddenObject(_string3);
		if (CHose::_statics->_actionTarget.empty()) {
			playSound("z#51.wav");
			CHoseConnectedMsg hoseMsg;
			hoseMsg._connected = false;
			hoseMsg.execute(this);
		}
	}

	return true;
}

} // End of namespace Titanic
