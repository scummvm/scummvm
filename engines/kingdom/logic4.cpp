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

#include "kingdom/kingdom.h"
#include "kingdom/logic.h"

namespace Kingdom {

void Logic::GPL4_900() {
	if (_vm->_aTimer > 0) {
		_vm->playSound(0);
		_vm->_aTimer = 133;
	}
	if (_vm->_asMode) {
		_vm->switchMtoA();
		_mapStat = 0;
		_oldStatPlay--;
		_vm->_asMode = false;
	}
	_vm->saveAS();
	_vm->fadeToBlack2();

	if (_health == 10 || _health == 6 || _health == 2) {
		_vm->showPic(121);
		_vm->_bTimer = 38;
		_vm->readMouse();
		while (_vm->_bTimer && !_vm->_mouseButton) {
			_vm->checkTimers();
			_vm->refreshSound();
			_vm->readMouse();
		}

		if (_vm->_aTimer > 0)
			_vm->setATimer();

		_vm->_userInput = 0;
		GPL4_901();
	} else {
		if (_vm->_aTimer > 0) {
			_vm->setATimer();
			_vm->_aTimer = 133;
		}
		_vm->showPic(120);
		_statPlay = 901;
	}
}

void Logic::GPL4_901() {
	_vm->setMouse();
	_vm->eraseCursor();
	_vm->fadeToBlack2();
	_vm->showPic(106);
	_vm->drawIcon(4, 0, 12 - _healthOld);
	if (_tideCntl)
		_vm->drawPic(178);
	else
		_vm->drawPic(179);

	_vm->_iconRedraw = true;
	if (_vm->_userInput == 0x2F1) {
		_vm->_asMode = false;
		_vm->restoreAS();
		_statPlay = _oldStatPlay;
		_vm->_loopFlag = true;
	} else {
		if (_health == 10 || _health == 6 || _health == 2 || _vm->_userInput < 0x400 || _vm->_userInput > 0x427) {
			_vm->_asMode = false;
			_vm->restoreAS();
			_statPlay = _oldStatPlay;
		} else {
			_statPlay = _mapExit[_vm->_userInput - 0x400];
			_vm->_userInput = 0;
			wound();
			_vm->playMovie(10);
			inventoryDel(3);
			_vm->_tsIconOnly = false;
			enableUIButtons();
			switchAS();
			_mapStat = 0;
		}
		_vm->_userInput = 0;
		_vm->_loopFlag = true;
	}
}

void Logic::GPL4_991() {
	_currMap = 10;
	_vm->_userInput = 0;
	_vm->playSound(0);
	if (_health == 10 || _health == 6 || _health == 2)
		_health -= 2;
	else
		_health -= 4;

	if (_health == 0) {
		_vm->playMovie(194);
		_vm->_noIFScreen = true;
		_vm->fShowPic(107);

		characterDeath();
	} else {
		_vm->_bTimer = 36;
		while(_vm->_bTimer) {
			_vm->checkTimers();
			_vm->refreshSound();
		}

		_vm->_loopFlag = true;

		if (_nodes[28] == 1) {
			_nodes[28] = 0;
			_nodes[48] = 0;
		}

		_vm->_itemInhibit = false;
		_tideCntl = false;
		_vm->drawPic(179);
		_vm->_skylarTimer = 0;
		_vm->_skylarTimerFlag = false;
		enableUIButtons();
		_vm->_mapEx = true;
		_nodeNum = 5;
		_vm->drawLocation();
		_vm->playSound(0);
		if (_nodes[5] > 0) {
			_vm->playMovie(197);
			_resurrect = true;
		}
		inventoryDel(12);
		_statPlay = 50;
	}
}

void Logic::GPL4_992() {
	_currMap = 10;
	_vm->_userInput = 0;
	_vm->playSound(0);
	if (_health == 10 || _health == 6 || _health == 2)
		_health -= 2;
	else
		_health -= 4;

	if (_health == 0) {
		_vm->playMovie(194);
		_vm->_noIFScreen = true;
		_vm->fShowPic(107);

		characterDeath();
	} else {
		_vm->_bTimer = 36;
		while(_vm->_bTimer != 0) {
			_vm->checkTimers();
			_vm->refreshSound();
		}

		_vm->_loopFlag = true;
		if (_nodes[28] == 1) {
			_nodes[28] = 0;
			_nodes[48] = 0;
		}

		_vm->_itemInhibit = false;
		_tideCntl = false;
		_vm->drawPic(179);
		_vm->_skylarTimer = 0;
		_vm->_skylarTimerFlag = false;
		enableUIButtons();
		_vm->_mapEx = true;
		_nodeNum = 27;
		_vm->drawLocation();
		_resurrect = true;
		_vm->playSound(0);
		_vm->playMovie(195);
		inventoryDel(12);
		_statPlay = 270;
	}
}

void Logic::GPL4_992_demo() {
	_currMap = 10;
	_vm->_userInput = 0;
	_vm->playSound(0);
	if (_health == 10 || _health == 6 || _health == 2)
		_health -= 2;
	else
		_health -= 4;

	if (_health == 0) {
		_vm->playMovie(194);
		_vm->_noIFScreen = true;
		_vm->fShowPic(107);

		characterDeath();
	} else {
		_vm->_bTimer = 36;
		while(_vm->_bTimer != 0) {
			_vm->checkTimers();
			_vm->refreshSound();
		}

		_vm->_loopFlag = true;
		if (_nodes[28] == 1) {
			_nodes[28] = 0;
			_nodes[48] = 0;
		}

		_vm->_itemInhibit = false;
		_tideCntl = false;
		_vm->drawPic(179);
		_vm->_skylarTimer = 0;
		_vm->_skylarTimerFlag = false;
		enableUIButtons();
		_nodeNum = 27;
		_vm->drawLocation();
		_resurrect = true;
		_vm->playSound(0);
		_vm->playMovie(195);
		inventoryDel(12);
		_statPlay = 270;
	}
}

void Logic::GPL4_993() {
	_currMap = 10;
	_vm->_userInput = 0;
	_vm->playSound(0);
	if (_health == 10 || _health == 6 || _health == 2)
		_health -= 2;
	else
		_health -= 4;

	if (_health == 0) {
		_vm->playMovie(194);
		_vm->_noIFScreen = true;
		_vm->fShowPic(107);

		characterDeath();
	} else {
		_vm->_bTimer = 36;
		while (_vm->_bTimer != 0) {
			_vm->checkTimers();
			_vm->refreshSound();
		}

		_vm->_loopFlag = true;

		if (_nodes[28] == 1) {
			_nodes[28] = 0;
			_nodes[48] = 0;
		}

		if (_nodes[67] == 1)
			_nodes[67] = 0;

		_vm->_itemInhibit = false;
		_tideCntl = false;
		_vm->drawPic(179);
		_vm->_skylarTimer = 0;
		_vm->_skylarTimerFlag = false;
		enableUIButtons();
		_vm->_mapEx = true;
		_nodeNum = 52;
		_vm->drawLocation();
		_resurrect = true;
		_vm->playSound(0);
		_vm->playMovie(196);
		inventoryDel(12);
		_statPlay = 520;
	}
}

void Logic::GPL4_993_demo() {
	_currMap = 10;
	_vm->_userInput = 0;
	_vm->playSound(0);
	if (_health == 10 || _health == 6 || _health == 2)
		_health -= 2;
	else
		_health -= 4;

	if (_health == 0) {
		_vm->playMovie(194);
		_vm->_noIFScreen = true;
		_vm->fShowPic(107);

		characterDeath();
	} else {
		_vm->_bTimer = 36;
		while (_vm->_bTimer != 0) {
			_vm->checkTimers();
			_vm->refreshSound();
		}

		_vm->_loopFlag = true;
		if (_nodes[28] == 1) {
			_nodes[28] = 0;
			_nodes[48] = 0;
		}

		if (_nodes[67] == 1)
			_nodes[67] = 0;

		_vm->_itemInhibit = false;
		_tideCntl = false;
		_vm->drawPic(179);
		_vm->_skylarTimer = 0;
		_vm->_skylarTimerFlag = false;
		enableUIButtons();
		_nodeNum = 52;
		_vm->drawLocation();
		_resurrect = true;
		_vm->playSound(0);
		_vm->playMovie(196);
		inventoryDel(12);
		_statPlay = 520;
	}
}

void Logic::GPL4_994() {
	switch(_vm->_userInput) {
	case 0x190:
		_vm->initPlay();
		// TODO _QuitFlag = 1;
		_vm->_quit = true;
		break;
	case 0x191:
		// TODO _QuitFlag = 2;
		_vm->_quit = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

} // Namespace
