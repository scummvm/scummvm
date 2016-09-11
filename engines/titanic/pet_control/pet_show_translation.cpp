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

#include "titanic/pet_control/pet_show_translation.h"
#include "titanic/pet_control/pet_control.h"

namespace Titanic {

BEGIN_MESSAGE_MAP(CPetShowTranslation, CGameObject)
	ON_MESSAGE(EnterViewMsg)
	ON_MESSAGE(LeaveViewMsg)
	ON_MESSAGE(ChangeSeasonMsg)
	ON_MESSAGE(ArboretumGateMsg)
END_MESSAGE_MAP()


void CPetShowTranslation::save(SimpleFile *file, int indent) {
	file->writeNumberLine(1, indent);
	CGameObject::save(file, indent);
}

void CPetShowTranslation::load(SimpleFile *file) {
	file->readNumber();
	CGameObject::load(file);
}

bool CPetShowTranslation::EnterViewMsg(CEnterViewMsg *msg) {
	CPetControl *pet = getPetControl();
	if (!pet)
		return true;

	CString viewName = getFullViewName();
	CString nodeView = msg->_newView->getNodeViewName();

	if (viewName == "Arboretum.Node 2.N" || viewName == "FrozenArboretum.Node 2.N") {
		switch (stateGetSeason()) {
		case SEASON_SUMMER:
			pet->addTranslation(DE_SUMMER_ARBORETUM, DE_SUMMER);
			break;

		case SEASON_AUTUMN:
			pet->addTranslation(DE_AUTUMN_ARBORETUM, DE_AUTUMN);
			break;

		case SEASON_WINTER:
			pet->addTranslation(DE_WINTER_ARBORETUM, DE_WINTER);
			break;

		case SEASON_SPRING:
			pet->addTranslation(DE_SPRING_ARBORETUM, DE_SPRING);
			break;

		default:
			break;
		}

		pet->addTranslation(DE_ARBORETUM_MSG1, DE_ARBORETUM_MSG2);
	} else if (compareRoomNameTo("Bridge")) {
		if (nodeView == "Node 3.N") {
			pet->addTranslation(DE_BRIDGE_MSG1, DE_BRIDGE_MSG2);
			pet->addTranslation(DE_BRIDGE_MSG3, DE_BRIDGE_MSG4);
			pet->addTranslation(DE_BRIDGE_MSG5, DE_BRIDGE_MSG6);
			pet->addTranslation(DE_BRIDGE_MSG7, DE_BRIDGE_MSG8);
		} else if (nodeView == "Node 4.N") {
			pet->addTranslation(DE_BRIDGE_MSG9, DE_BRIDGE_MSG10);
		}
	} else if (compareRoomNameTo("PromenadeDeck")) {
		if (nodeView == "Node 2.S") {
			pet->addTranslation(DE_PROMENADE_DECK_MSG1, DE_PROMENADE_DECK_MSG2);
			pet->addTranslation(DE_PROMENADE_DECK_MSG3, DE_PROMENADE_DECK_MSG4);
			pet->addTranslation(DE_PROMENADE_DECK_MSG5, DE_PROMENADE_DECK_MSG6);
			pet->addTranslation(DE_PROMENADE_DECK_MSG7, DE_PROMENADE_DECK_MSG8);
			pet->addTranslation(DE_PROMENADE_DECK_MSG9, DE_PROMENADE_DECK_MSG10);
		}
	} else if (compareRoomNameTo("SgtLobby")) {
		if (nodeView == "Node 17.S") {
			pet->addTranslation(DE_SGTLOBBY_MSG1, DE_SGTLOBBY_MSG2);
			pet->addTranslation(DE_SGTLOBBY_MSG3, DE_SGTLOBBY_MSG4);
			pet->addTranslation(DE_SGTLOBBY_MSG5, DE_SGTLOBBY_MSG6);
			pet->addTranslation(DE_SGTLOBBY_MSG7, DE_SGTLOBBY_MSG8);
		}
	} else if (compareRoomNameTo("Titania")) {
		if (nodeView == "Node 9.N") {
			pet->addTranslation(DE_TITANIA_MSG1, DE_TITANIA_MSG2);
		} else if (nodeView == "Node 10.N") {
			pet->addTranslation(DE_TITANIA_MSG3, DE_TITANIA_MSG4);
		} else if (nodeView == "Node 11.N") {
			pet->addTranslation(DE_TITANIA_MSG5, DE_TITANIA_MSG6);
		} else if (nodeView == "Node 13.N") {
			pet->addTranslation(DE_TITANIA_MSG7, DE_TITANIA_MSG8);
		}
	}

	return true;
}

bool CPetShowTranslation::LeaveViewMsg(CLeaveViewMsg *msg) {
	return true;
}

bool CPetShowTranslation::ChangeSeasonMsg(CChangeSeasonMsg *msg) {
	return true;
}

bool CPetShowTranslation::ArboretumGateMsg(CArboretumGateMsg *msg) {
	return true;
}

} // End of namespace Titanic
