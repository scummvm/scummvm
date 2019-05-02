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

#include "titanic/moves/call_pellerator.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CCallPellerator, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(PETActivateMsg)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(TimerMsg)
END_MESSAGE_MAP()

void CCallPellerator::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CCallPellerator::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CCallPellerator::EnterViewMsg(CEnterViewMsg *msg) {
	CString viewName = msg->_newView->getFullViewName();
	if (viewName == "MusicRoomLobby.Node 1.N") {
		// WORKAROUND: In original, the Remote tab icons don't get loaded
		// until after the EnterViewMsg call is made when entering a new
		// room. So in the special case of the Music Room Lobby, since
		// you're immediately at the Pellerator when you enter, set up a
		// quick timer to not select the Call glyph until the remote is loaded
		addTimer(10);
	} else {
		showCallPellerator();
	}

	return true;
}

bool CCallPellerator::LeaveViewMsg(CLeaveViewMsg *msg) {
	petClear();
	return true;
}

bool CCallPellerator::PETActivateMsg(CPETActivateMsg *msg) {
	CString name = getFullViewName();

	if (msg->_name == "Pellerator") {
		if (petDoorOrBellbotPresent()) {
			petDisplayMessage(BOT_BLOCKING_PELLERATOR);
		} else if (name == "FrozenArboretum.Node 4.E") {
			petDisplayMessage(FROZEN_PELLERATOR);
		} else if (name == "Bar.Node 1.S") {
			changeView("Pellerator.Node 1.S");
		} else {
			changeView("Pellerator.Node 1.N");
		}
	}

	return true;
}

bool CCallPellerator::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	return true;
}

bool CCallPellerator::TimerMsg(CTimerMsg *msg) {
	showCallPellerator();
	return true;
}

void CCallPellerator::showCallPellerator() {
	petSetArea(PET_REMOTE);
	petHighlightGlyph(1);
	CString name = getFullViewName();

	if (name == "TopOfWell.Node 6.S") {
		petDisplayMessage(2, STANDING_OUTSIDE_PELLERATOR);
	}

	petSetRemoteTarget();
}

} // End of namespace Titanic
