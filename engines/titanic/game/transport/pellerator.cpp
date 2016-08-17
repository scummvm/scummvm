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

#include "titanic/game/transport/pellerator.h"
#include "titanic/core/room_item.h"

namespace Titanic {

static const char *const WAVE_NAMES[10] = {
	"z#465.wav", "z#456.wav", "z#455.wav", "z#453.wav",
	"z#452.wav", "NoStandingInFunnyWays", "z#450.wav",
	"z#449.wav", "z#435.wav", "z#434.wav"
};

BEGIN_MESSAGE_MAP(CPellerator, CTransport)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

int CPellerator::_soundHandle;
int CPellerator::_v2;

CPellerator::CPellerator() : CTransport() {
}

void CPellerator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_v2, indent);

	CTransport::save(file, indent);
}

void CPellerator::load(SimpleFile *file) {
	file->readNumber();
	_soundHandle = file->readNumber();
	_v2 = file->readNumber();

	CTransport::load(file);
}

bool CPellerator::StatusChangeMsg(CStatusChangeMsg *msg) {
	warning("CPellerator::StatusChangeMsg");
	return true;
}

bool CPellerator::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (isEquals("PelleratorObject")) {
		for (int idx = 0; idx < 10; ++idx)
			loadSound(WAVE_NAMES[idx]);
		addTimer(10000);
	}

	CString name = msg->_oldRoom ? msg->_oldRoom->getName() : "";
	int oldVal = _v2;

	if (name.empty()) {
		_v2 = 4;
		oldVal = 4;
	} else if (name == "PromenadeDeck") {
		_v2 = 0;
	} else if (name == "MusicRoomLobby") {
		_v2 = 1;
	} else if (name == "Bar") {
		_v2 = 2;
	} else if (name == "TopOfWell") {
		_v2 = 4;
	} else if (name == "1stClassRestaurant") {
		_v2 = 5;
	} else if (name == "Arboretum" || name == "FrozenArboretum") {
		_v2 = 6;
	}

	if (_v2 != oldVal) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = _v2;
		statusMsg.execute("ExitPellerator");
	}

	loadFrame(264);
	return true;
}

bool CPellerator::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	stopGlobalSound(true, -1);

	switch (_v2) {
	case 0:
		_soundHandle = queueSound("z#429.wav", _soundHandle);
		break;
	case 1:
		_soundHandle = queueSound("z#430.wav", _soundHandle);
		break;
	case 2:
		_soundHandle = queueSound("z#431.wav", _soundHandle);
		break;
	case 4:
		_soundHandle = queueSound("z#428.wav", _soundHandle);
		break;
	case 5:
		_soundHandle = queueSound("z#433.wav", _soundHandle);
		break;
	case 6:
		_soundHandle = queueSound("z#432.wav", _soundHandle);
		break;
	default:
		break;
	}

	return true;
}

bool CPellerator::TimerMsg(CTimerMsg *msg) {
	if (compareRoomNameTo("Pellerator")) {
		_soundHandle = queueSound(WAVE_NAMES[getRandomNumber(9)], _soundHandle);
		addTimer(20000 + getRandomNumber(10000));
	}

	return true;
}

} // End of namespace Titanic
