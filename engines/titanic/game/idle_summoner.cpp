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

#include "titanic/game/idle_summoner.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CIdleSummoner, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(TimerMsg)
	ON_MESSAGE(ActMsg)
	ON_MESSAGE(LoadSuccessMsg)
END_MESSAGE_MAP()

CIdleSummoner::CIdleSummoner() : CGameObject(), _fieldBC(360000),
		_fieldC0(60000), _fieldC4(360000), _fieldC8(60000),
		_fieldCC(0), _fieldD0(0), _fieldD4(0), _fieldD8(0), _ticks(0) {
}

void CIdleSummoner::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_fieldBC, indent);
	file->writeNumberLine(_fieldC0, indent);
	file->writeNumberLine(_fieldC4, indent);
	file->writeNumberLine(_fieldC8, indent);
	file->writeNumberLine(_fieldCC, indent);
	file->writeNumberLine(_fieldD0, indent);
	file->writeNumberLine(_fieldD4, indent);
	file->writeNumberLine(_fieldD8, indent);
	file->writeNumberLine(_ticks, indent);

	CGameObject::save(file, indent);
}

void CIdleSummoner::load(SimpleFile *file) {
	file->readNumber();
	_fieldBC = file->readNumber();
	_fieldC0 = file->readNumber();
	_fieldC4 = file->readNumber();
	_fieldC8 = file->readNumber();
	_fieldCC = file->readNumber();
	_fieldD0 = file->readNumber();
	_fieldD4 = file->readNumber();
	_fieldD8 = file->readNumber();
	_ticks = file->readNumber();

	CGameObject::load(file);
}

bool CIdleSummoner::EnterViewMsg(CEnterViewMsg *msg) {
	CActMsg actMsg("Enable");
	actMsg.execute(this);
	return true;
}

bool CIdleSummoner::TimerMsg(CTimerMsg *msg) {
	uint nodesCtr = getNodeChangedCtr();
	if (msg->_actionVal == 1 && !petDoorOrBellbotPresent()
			&& nodesCtr > 0 && _fieldD8) {
		if (!compareRoomNameTo("TopOfWell") && !compareRoomNameTo("EmbLobby"))
			return true;

		int region = talkGetDialRegion("BellBot", 1);
		uint delay = region == 1 ? 15000 : 12000;
		uint enterTicks = MIN(getNodeEnterTicks(), _ticks);

		CString name;
		uint ticks = getTicksCount() - enterTicks;
		if (ticks > delay) {
			if (region == 1 || getRandomNumber(1) == 1) {
				name = "BellBot";
			} else {
				name = "DoorBot";
			}
			_fieldD8 = nodesCtr;

			if (getRoom()) {
				CSummonBotQueryMsg queryMsg(name);
				if (queryMsg.execute(this)) {
					CSummonBotMsg summonMsg(name, 1);
					summonMsg.execute(this);
				}
			}
		}
	}

	return true;
}

bool CIdleSummoner::ActMsg(CActMsg *msg) {
	if (msg->_action == "Enable") {
		if (!_fieldD4)
			_fieldD4 = addTimer(15000, 15000);
	} else if (msg->_action == "Disable") {
		if (_fieldD4 > 0) {
			stopAnimTimer(_fieldD4);
			_fieldD4 = 0;
		}
	} else if (msg->_action == "DoorbotDismissed" || msg->_action == "BellbotDismissed") {
		_ticks = getTicksCount();
	}

	return true;
}

bool CIdleSummoner::LoadSuccessMsg(CLoadSuccessMsg *msg) {
	_ticks = getTicksCount();
	return true;
}

} // End of namespace Titanic
