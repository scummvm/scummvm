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
#include "titanic/translation.h"

namespace Titanic {

static const char *const WAVE_NAMES_EN[10] = {
	"z#465.wav", "z#456.wav", "z#455.wav", "z#453.wav",
	"z#452.wav", "NoStandingInFunnyWays", "z#450.wav",
	"z#449.wav", "z#435.wav", "z#434.wav"
};

static const char *const WAVE_NAMES_DE[10] = {
	"z#202.wav", "z#193.wav", "z#192.wav", "z#190.wav",
	"z#189.wav", "NoStandingInFunnyWays", "z#187.wav",
	"z#186.wav", "z#180.wav", "z#179.wav"
};

BEGIN_MESSAGE_MAP(CPellerator, CTransport)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(EnterRoomMsg)
	ON_MESSAGE(MovieEndMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

int CPellerator::_soundHandle;
int CPellerator::_destination;

CPellerator::CPellerator() : CTransport() {
}

void CPellerator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeNumberLine(_soundHandle, indent);
	file->writeNumberLine(_destination, indent);

	CTransport::save(file, indent);
}

void CPellerator::load(SimpleFile *file) {
	file->readNumber();
	_soundHandle = file->readNumber();
	_destination = file->readNumber();

	CTransport::load(file);
}

bool CPellerator::StatusChangeMsg(CStatusChangeMsg *msg) {
	setVisible(true);
	int classNum = getPassengerClass();
	int newDest = msg->_newStatus;

	if (msg->_newStatus == _destination) {
		petDisplayMessage(1, ALREADY_AT_DESTINATION);
	} else if (classNum == 3 || (msg->_newStatus > 4 && classNum != 1)) {
		petDisplayMessage(1, CLASS_NOT_ALLOWED_AT_DEST);
	} else if (newDest > _destination) {
		playAmbientSound(TRANSLATE("z#74.wav", "z#605.wav"), VOL_QUIET, true, true, 0);

		CString name = getName();
		changeView(name == "PelleratorObject2" ?
			"Pellerator.Node 1.N" : "Pellerator.Node 1.S");

		if (name == "PelleratorObject") {
			for (; _destination < newDest; ++_destination) {
				switch (_destination) {
				case 0:
				case 1:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(299, 304, 0);
					playMovie(305, 313, MOVIE_WAIT_FOR_FINISH);
					break;

				case 2:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(299, 304, 0);
					for (int idx = 0; idx < 5; ++idx)
						playMovie(253, 263, 0);
					playMovie(153, 197, 0);
					for (int idx = 0; idx < 5; ++idx)
						playMovie(253, 263, 0);
					playMovie(290, 293, MOVIE_WAIT_FOR_FINISH);
					break;

				case 4:
					playMovie(267, 270, 0);
					for (int idx = 0; idx < 5; ++idx)
						playMovie(253, 263, 0);
					playMovie(3, 71, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(253, 263, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					break;

				case 5:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(299, 304, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(253, 263, 0);
					playMovie(3, 71, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(299, 304, 0);
					break;

				default:
					break;
				}
			}
		} else {
			for (; _destination < newDest; ++_destination) {
				switch (_destination) {
				case 0:
				case 1:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(299, 304, 0);
					playMovie(305, 313, MOVIE_WAIT_FOR_FINISH);
					break;

				case 2:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 4; ++idx)
						playMovie(299, 304, 0);
					for (int idx = 0; idx < 15; ++idx)
						playMovie(245, 255, 0);
					playMovie(264, 267, MOVIE_WAIT_FOR_FINISH);
					++_destination;
					break;

				case 4:
					playMovie(241, 244, 0);
					for (int idx = 0; idx < 15; ++idx)
						playMovie(245, 255, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					break;

				case 5:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(299, 304, 0);
					for (int idx = 0; idx < 12; ++idx)
						playMovie(245, 255, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(299, 304, 0);
					playMovie(305, 313, MOVIE_WAIT_FOR_FINISH);
					break;

				default:
					break;
				}
			}
		}

		playMovie(264, 264, MOVIE_NOTIFY_OBJECT);
		_destination = newDest;
	} else if (newDest < _destination) {
		playAmbientSound(TRANSLATE("z#74.wav", "z#605.wav"), VOL_QUIET, true, true, 0);

		CString name = getName();
		changeView(name == "PelleratorObject2" ?
			"Pellerator.Node 1.N" : "Pellerator.Node 1.S");

		if (name == "PelleratorObject") {
			for (--_destination; _destination > newDest; --_destination) {
				switch (_destination) {
				case 0:
				case 1:
					playMovie(351, 359, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					break;

				case 3:
					playMovie(241, 244, 0);
					for (int idx = 0; idx < 5; ++idx)
						playMovie(245, 255, 0);
					playMovie(197, 239, 0);
					for (int idx = 0; idx < 5; ++idx)
						playMovie(245, 255, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					--_destination;
					break;

				case 4:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(299, 304, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(245, 255, 0);
					playMovie(78, 149, 0);
					for (int idx = 0; idx < 5; ++idx)
						playMovie(245, 255, 0);
					playMovie(264, 267, MOVIE_WAIT_FOR_FINISH);
					break;

				case 5:
					playMovie(351, 359, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(336, 341, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(245, 255, 0);
					playMovie(78, 149, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					break;

				default:
					break;
				}
			}
		} else {
			for (--_destination; _destination >= newDest; --_destination) {
				switch (_destination) {
				case 0:
				case 1:
					playMovie(351, 359, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					break;

				case 3:
					playMovie(267, 270, 0);
					for (int idx = 0; idx < 15; ++idx)
						playMovie(253, 263, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					--_destination;
					break;

				case 4:
					playMovie(315, 323, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(299, 304, 0);
					for (int idx = 0; idx < 15; ++idx)
						playMovie(253, 263, 0);
					playMovie(290, 293, MOVIE_WAIT_FOR_FINISH);
					break;

				case 5:
					playMovie(351, 359, 0);
					for (int idx = 0; idx < 7; ++idx)
						playMovie(336, 341, 0);
					for (int idx = 0; idx < 13; ++idx)
						playMovie(253, 263, 0);
					for (int idx = 0; idx < 3; ++idx)
						playMovie(336, 341, 0);
					playMovie(342, 348, MOVIE_WAIT_FOR_FINISH);
					break;

				default:
					break;
				}
			}
		}

		playMovie(264, 264, MOVIE_NOTIFY_OBJECT);
		_destination = newDest;
	}

	CStatusChangeMsg statusMsg;
	statusMsg._newStatus = _destination;
	statusMsg.execute("ExitPellerator");

	return true;
}

bool CPellerator::EnterRoomMsg(CEnterRoomMsg *msg) {
	if (isEquals("PelleratorObject")) {
		if (g_language == Common::DE_DEU)
			_soundHandle = queueSound("z#200.wav", _soundHandle);

		for (int idx = 0; idx < 10; ++idx)
			loadSound(TRANSLATE(WAVE_NAMES_EN[idx], WAVE_NAMES_DE[idx]));
		addTimer(10000);
	}

	CString name = msg->_oldRoom ? msg->_oldRoom->getName() : "";
	int oldVal = _destination;

	if (name.empty()) {
		// WORKAROUND: Called when loading a savegame, the original reset the
		// destination to '4' resulting in potentially longer travel times.
		// Since the destination is saved as part of savegames anyway, I'm
		// changing this to leave it unchanged
		oldVal = _destination;
	} else if (name == "PromenadeDeck") {
		_destination = 0;
	} else if (name == "MusicRoomLobby") {
		_destination = 1;
	} else if (name == "Bar") {
		_destination = 2;
	} else if (name == "TopOfWell") {
		_destination = 4;
	} else if (name == "1stClassRestaurant") {
		_destination = 5;
	} else if (name == "Arboretum" || name == "FrozenArboretum") {
		_destination = 6;
	}

	if (_destination != oldVal) {
		CStatusChangeMsg statusMsg;
		statusMsg._newStatus = _destination;
		statusMsg.execute("ExitPellerator");
	}

	loadFrame(264);
	return true;
}

bool CPellerator::MovieEndMsg(CMovieEndMsg *msg) {
	setVisible(false);
	stopAmbientSound(true, -1);

	switch (_destination) {
	case 0:
		_soundHandle = queueSound(TRANSLATE("z#429.wav", "z#174.wav"), _soundHandle);
		break;
	case 1:
		_soundHandle = queueSound(TRANSLATE("z#430.wav", "z#175.wav"), _soundHandle);
		break;
	case 2:
		_soundHandle = queueSound(TRANSLATE("z#431.wav", "z#176.wav"), _soundHandle);
		break;
	case 4:
		_soundHandle = queueSound(TRANSLATE("z#428.wav", "z#173.wav"), _soundHandle);
		break;
	case 5:
		_soundHandle = queueSound(TRANSLATE("z#433.wav", "z#178.wav"), _soundHandle);
		break;
	case 6:
		_soundHandle = queueSound(TRANSLATE("z#432.wav", "z#177.wav"), _soundHandle);
		break;
	default:
		break;
	}

	return true;
}

bool CPellerator::TimerMsg(CTimerMsg *msg) {
	if (compareRoomNameTo("Pellerator")) {
		_soundHandle = queueSound(TRANSLATE(WAVE_NAMES_EN[getRandomNumber(9)],
				WAVE_NAMES_DE[getRandomNumber(9)]), _soundHandle);
		addTimer(20000 + getRandomNumber(10000));
	}

	return true;
}

} // End of namespace Titanic
