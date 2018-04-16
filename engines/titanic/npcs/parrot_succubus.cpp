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
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CParrotSuccUBus, CSuccUBus)
	ON_MESSAGE(HoseConnectedMsg)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(LeaveNodeMsg)
END_MESSAGE_MAP()

CParrotSuccUBus::CParrotSuccUBus() : CSuccUBus(), _hoseConnected(false),
	_pumpingSound(0), _hoseRemovalStartFrame(376), _hoseRemovalEndFrame(393) {
}

void CParrotSuccUBus::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_hoseConnected, indent);
	file->writeQuotedLine(_pumpingTarget, indent);
	file->writeNumberLine(_pumpingSound, indent);

	CSuccUBus::save(file, indent);
}

void CParrotSuccUBus::load(SimpleFile *file) {
	file->readNumber();
	_hoseConnected = file->readNumber();
	_pumpingTarget = file->readString();
	_pumpingSound = file->readNumber();

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
		_pumpingTarget = item->getName();
		CHoseConnectedMsg hoseMsg(1, this);
		hoseMsg.execute(msg->_object);
		item->petMoveToHiddenRoom();

		CPumpingMsg pumpingMsg(1, this);
		pumpingMsg.execute(_pumpingTarget);
		_hoseConnected = true;

		if (_isOn) {
			_isOn = false;
		} else {
			playMovie(_onStartFrame, _onEndFrame, 0);
			playSound(TRANSLATE("z#26.wav", "z#557.wav"));
		}

		playMovie(_hoseStartFrame, _hoseEndFrame, MOVIE_NOTIFY_OBJECT);
	} else {
		stopMovie();
		stopSound(_pumpingSound);
		playMovie(_hoseRemovalStartFrame, _hoseRemovalEndFrame, MOVIE_NOTIFY_OBJECT);

		CPumpingMsg pumpingMsg(0, this);
		pumpingMsg.execute(_pumpingTarget);

		CGameObject *obj = getHiddenObject(_pumpingTarget);
		if (obj) {
			obj->petAddToInventory();
			obj->setVisible(true);
		}

		_isOn = true;
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
		_pumpingSound = playSound(TRANSLATE("z#472.wav", "z#209.wav"));
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
		getHiddenObject(_pumpingTarget);
		if (CHose::_statics->_actionTarget.empty()) {
			playSound(TRANSLATE("z#51.wav", "z#582.wav"));
			CHoseConnectedMsg hoseMsg;
			hoseMsg._connected = false;
			hoseMsg.execute(this);
		}
	}

	return true;
}

} // End of namespace Titanic
