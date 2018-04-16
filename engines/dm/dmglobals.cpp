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
#include "dm/gfx.h"
#include "dm/dungeonman.h"
#include "dm/eventman.h"
#include "dm/menus.h"
#include "dm/champion.h"
#include "dm/loadsave.h"
#include "dm/objectman.h"
#include "dm/inventory.h"
#include "dm/text.h"
#include "dm/movesens.h"

namespace DM {

void DMEngine::initConstants() {
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

} // End of namespace DM
