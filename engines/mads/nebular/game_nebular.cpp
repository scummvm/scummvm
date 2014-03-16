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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/dialogs_nebular.h"
#include "mads/nebular/globals_nebular.h"

namespace MADS {

namespace Nebular {

GameNebular::GameNebular(MADSEngine *vm): Game(vm) {
	_surface = new MSurface(MADS_SCREEN_WIDTH, MADS_SCENE_HEIGHT);
	_storyMode = STORYMODE_NAUGHTY;
}

int GameNebular::checkCopyProtection() {
	/*
	// DEBUG: Flag copy protection failure
	_globals[5] = -1;
	
	if (!ConfMan.getBool("copy_protection"))
		return true;

	* DEBUG: Disabled for now
	CopyProtectionDialog *dlg = new CopyProtectionDialog(_vm, false);
	dlg->show();
	delete dlg;
	*/

	// DEBUG: Return that copy protection failed
	return -2;
}

void GameNebular::initialiseGlobals() {
	// Set specific values needed by the game
	_globals[4] = 8;
	_globals[33] = 1;
	_globals[10] = -1;
	_globals[13] = -1;
	_globals[15] = -1;
	_globals[19] = -1;
	_globals[20] = -1;
	_globals[21] = -1;
	_globals[95] = -1;

	_objects.setData(3, 17, nullptr);

	// Put the values 0 through 3 in a random order in global slots 83 to 86
	for (int i = 0; i < 4;) {
		int randomVal = _vm->getRandomNumber(3);
		_globals[83 + i] = randomVal;

		bool flag = false;
		for (int idx2 = 0; idx2 < i; ++idx2) {
			if (_globals[83 + idx2] == randomVal)
				flag = true;
		}

		if (!flag)
			++i;
	}

	// Put the values 0 through 3 in a random order in global slots 87 to 90
	for (int i = 0; i < 4;) {
		int randomVal = _vm->getRandomNumber(3);
		_globals[87 + i] = randomVal;

		bool flag = false;
		for (int idx2 = 0; idx2 < i; ++idx2) {
			if (_globals[87 + idx2] == randomVal)
				flag = true;
		}

		if (!flag)
			++i;
	}

	_globals[120] = 501;
	_globals[121] = -1;
	_globals[55] = -1;
	_globals[119] = 1;
	_globals[134] = 4;

	// Fill out the globals 200 to 209 with unique random values less than 10000
	for (int i = 0; i < 10; ++i) {
		int randomVal = _vm->getRandomNumber(9999);
		_globals[200 + i] = randomVal;

		bool flag = false;
		for (int idx2 = 0; idx2 < i; ++idx2) {
			if (_globals[200 + idx2] == randomVal)
				flag = true;
		}

		if (!flag)
			++i;
	}

	// Difficulty level control
	switch (_difficultyLevel) {
	case DIFFICULTY_HARD:
		_globals[35] = 0;
		_objects.setRoom(9, 1);
		_objects.setRoom(50, 1);
		_globals[137] = 5;
		_globals[136] = 0;
		break;
	case DIFFICULTY_MEDIUM:
		_globals[35] = 0;
		_objects.setRoom(8, 1);
		_globals[137] = -1;
		_globals[136] = 6;
		break;
	case DIFFICULTY_EASY:
		_globals[35] = 2;
		_objects.setRoom(8, 1);
		_objects.setRoom(27, 1);
		break;
	default:
		break;
	}

	_player._direction = 8;
	_player._newDirection = 8;

	loadResourceSequence("RXM", 1);
	loadResourceSequence("ROX", 1);
}

void GameNebular::setSectionHandler() {
	delete _sectionHandler;

	switch (_sectionNumber) {
	case 1:
		_sectionHandler = new Section1Handler(_vm);
		break;
	case 2:
		_sectionHandler = new Section2Handler(_vm);
		break;
	case 3:
		_sectionHandler = new Section3Handler(_vm);
		break;
	case 4:
		_sectionHandler = new Section4Handler(_vm);
		break;
	case 5:
		_sectionHandler = new Section5Handler(_vm);
		break;
	case 6:
		_sectionHandler = new Section6Handler(_vm);
		break;
	case 7:
		_sectionHandler = new Section7Handler(_vm);
		break;
	case 8:
		_sectionHandler = new Section8Handler(_vm);
		break;
	default:
		break;
	}
}

void GameNebular::checkShowDialog() {
	if (_vm->_dialogs->_pendingDialog && _player._stepEnabled && !_globals[5]) {
		_scene._spriteSlots.releasePlayerSprites();
		_vm->_dialogs->showDialog();
		_vm->_dialogs->_pendingDialog = DIALOG_NONE;
	}
}

} // End of namespace Nebular

} // End of namespace MADS
