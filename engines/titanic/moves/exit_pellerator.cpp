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

#include "titanic/moves/exit_pellerator.h"
#include "titanic/game/transport/pellerator.h"
#include "titanic/translation.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CExitPellerator, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(StatusChangeMsg)
	ON_MESSAGE(ChangeSeasonMsg)
END_MESSAGE_MAP()

CExitPelleratorStatics *CExitPellerator::_statics;

void CExitPellerator::init() {
	_statics = new CExitPelleratorStatics();
}

void CExitPellerator::deinit() {
	delete _statics;
}

void CExitPellerator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_statics->_v1, indent);
	file->writeNumberLine(_statics->_v2, indent);
	file->writeNumberLine(_statics->_isWinter, indent);

	CGameObject::save(file, indent);
}

void CExitPellerator::load(SimpleFile *file) {
	file->readNumber();
	_statics->_v1 = file->readString();
	_statics->_v2 = file->readNumber();
	_statics->_isWinter = file->readNumber();

	CGameObject::load(file);
}

bool CExitPellerator::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CString name = getName();

	if (name == "ExitPellerator") {
		if (_statics->_v2 != 2) {
			switch (getRandomNumber(2)) {
			case 0:
				CPellerator::_soundHandle = queueSound(TRANSLATE("z#457.wav", "z#194.wav"), CPellerator::_soundHandle);
				break;
			case 1:
				CPellerator::_soundHandle = queueSound(TRANSLATE("z#458.wav", "z#195.wav"), CPellerator::_soundHandle);
				break;
			default:
				CPellerator::_soundHandle = queueSound(TRANSLATE("z#464.wav", "z#201.wav"), CPellerator::_soundHandle);
				break;
			}
		}

		switch (_statics->_v2) {
		case 0:
			changeView("PromenadeDeck.Node 1.W");
			break;
		case 1:
			changeView("MusicRoomLobby.Node 1.S");
			break;
		case 4:
			changeView("TopOfWell.Node 6.N");
			break;
		case 5:
			changeView("1stClassRestaurant.Lobby Node.E");
			break;
		case 6:
			changeView(_statics->_isWinter ? "FrozenArboretum.Node 4.S" : "Arboretum.Node 4.W");
			break;
		default:
			petDisplayMessage(2, EXIT_FROM_OTHER_SIDE);
			CPellerator::_soundHandle = queueSound(TRANSLATE("z#438.wav", "z#183.wav"), CPellerator::_soundHandle);

		}
	} else if (name == "ExitPellerator2") {
		if (_statics->_v2 == 2) {
			switch (getRandomNumber(2)) {
			case 0:
				CPellerator::_soundHandle = queueSound(TRANSLATE("z#457.wav", "z#194.wav"), CPellerator::_soundHandle);
				break;
			case 1:
				CPellerator::_soundHandle = queueSound(TRANSLATE("z#458.wav", "z#195.wav"), CPellerator::_soundHandle);
				break;
			default:
				CPellerator::_soundHandle = queueSound(TRANSLATE("z#464.wav", "z#201.wav"), CPellerator::_soundHandle);
				break;
			}
		}

		if (_statics->_v2 == 2) {
			changeView("Bar.Node 1.N");
		} else {
			petDisplayMessage(2, EXIT_FROM_OTHER_SIDE);
			CPellerator::_soundHandle = queueSound(TRANSLATE("z#438.wav", "z#183.wav"), CPellerator::_soundHandle);
		}
	}

	return true;
}

bool CExitPellerator::StatusChangeMsg(CStatusChangeMsg *msg) {
	_statics->_v2 = msg->_newStatus;
	return true;
}

bool CExitPellerator::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	_statics->_isWinter = msg->_season == "Winter";
	return true;
}

} // End of namespace Titanic
