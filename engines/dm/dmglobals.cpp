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

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "common/system.h"
#include "dm/dm.h"
#include "gfx.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "champion.h"
#include "loadsave.h"
#include "objectman.h"
#include "inventory.h"
#include "text.h"
#include "movesens.h"
#include "string.h"


namespace DM {

void DMEngine::initArrays() {
	// G0233_ai_Graphic559_DirectionToStepEastCount
	_dirIntoStepCountEast[0] = 0;  // North
	_dirIntoStepCountEast[1] = 1;  // East
	_dirIntoStepCountEast[2] = 0;  // West
	_dirIntoStepCountEast[3] = -1; // South

	// G0234_ai_Graphic559_DirectionToStepNorthCount
	_dirIntoStepCountNorth[0] = -1; // North
	_dirIntoStepCountNorth[1] = 0;  // East
	_dirIntoStepCountNorth[2] = 1;  // West
	_dirIntoStepCountNorth[3] = 0;  // South
}

void DMEngine::f19_displayErrorAndStop(int16 errorIndex) {
	debug("Stuff hit the fun: ");
	debug(Common::String::format("%d", errorIndex).c_str());
	Common::Event event;
	while (_system->getEventManager()->pollEvent(event) || true)
		;
}
} // End of namespace DM
