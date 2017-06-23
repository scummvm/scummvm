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

#include "titanic/moves/exit_lift.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CExitLift, CGameObject)
	ON_MESSAGE(MouseButtonDownMsg)
END_MESSAGE_MAP()

CExitLift::CExitLift() : CGameObject(), _viewName("NULL") {
}

void CExitLift::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	file->writeQuotedLine(_viewName, indent);
	CGameObject::save(file, indent);
}

void CExitLift::load(SimpleFile *file) {
	file->readNumber();
	_viewName = file->readString();
	CGameObject::load(file);
}

bool CExitLift::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	CPetControl *pet = getPetControl();
	int floorNum = pet->getRoomsFloorNum();
	int elevNum = pet->getRoomsElevatorNum();

	if (floorNum == 39) {
		switch (elevNum) {
		case 1:
			_viewName = "BottomOfWell.Node 5.SE";
			break;
		case 3:
			_viewName = "BottomOfWell.Node 1.NW";
			break;
		default:
			break;
		}
	} else if (floorNum > 27) {
		switch (elevNum) {
		case 1:
		case 3:
			_viewName = "SgtLobby.Node 1.N";
			break;
		default:
			break;
		}
	} else if (floorNum > 19) {
		switch (elevNum) {
		case 1:
		case 3:
			_viewName = "2ndClassLobby.Node 8.N";
			break;
		case 2:
		case 4:
			_viewName = "2ndClassLobby.Node 1.N";
			break;
		default:
			break;
		}
	} else if (floorNum > 1) {
		switch (elevNum) {
		case 1:
		case 3:
			_viewName = "1stClassLobby.Node 1.W";
			break;
		case 2:
		case 4:
			_viewName = "1stClassLobby.Node 1.E";
			break;
		default:
			break;
		}
	} else {
		switch (elevNum) {
		case 1:
			_viewName = "TopOfWell.Node 6.E";
			break;
		case 2:
			_viewName = "TopOfWell.Node 6.W";
			break;
		case 3:
			_viewName = "TopOfWell.Node 10.W";
			break;
		case 4:
			_viewName = "TopOfWell.Node 10.E";
			break;
		default:
			break;
		}
	}

	changeView(_viewName);
	return true;
}

} // End of namespace Titanic
