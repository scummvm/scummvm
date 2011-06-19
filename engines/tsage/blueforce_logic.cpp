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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/blueforce_logic.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

void BlueForceGame::start() {
	// Start the game
	_globals->_sceneManager.changeScene(20);

	_globals->_events.setCursor(CURSOR_WALK);
}

Scene *BlueForceGame::createScene(int sceneNumber) {
	switch (sceneNumber) {
	case 20:
	case 50:
	case 60:
		error("Scene group 0 not implemented");
	case 100:
	case 109:
	case 110:
	case 114:
	case 115:
	case 125:
	case 140:
	case 150:
	case 160:
	case 180:
	case 190:
		error("Scene group 1 not implemented");
	case 200:
	case 210:
	case 220:
	case 225:
	case 265:
	case 270:
	case 271:
	case 280:
		error("Scene group 2 not implemented");
	case 300:
	case 315:
	case 325:
	case 330:
	case 340:
	case 342:
	case 350:
	case 355:
	case 360:
	case 370:
	case 380:
	case 385:
	case 390:
		error("Scene group 3 not implemented");
	case 410:
	case 415:
	case 440:
	case 450:
		error("Scene group 4 not implemented");
	case 550:
	case 551:
	case 560:
	case 570:
	case 580:
	case 590:
		error("Scene group 5 not implemented");
	case 600:
	case 620:
	case 666:
	case 690:
		error("Scene group 6 not implemented");
	case 710:
		error("Scene group 7 not implemented");
	case 800:
	case 810:
	case 820:
	case 830:
	case 840:
	case 850:
	case 860:
	case 870:
	case 880:
		error("Scene group 8 not implemented");
	case 900:
	case 910:
	case 920:
	case 930:
	case 935:
	case 940:
		error("Scene group 9 not implemented");
	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

} // End of namespace tSage
