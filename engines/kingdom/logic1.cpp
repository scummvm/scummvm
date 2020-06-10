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

void Logic::GPL1_10() {
	_nodeNum = 1;
	_vm->initPlay();
	_replay = false;
	GPLogic1_SubSP10();
}

void Logic::GPL1_11() {
	switch(_vm->_userInput) {
	case 0x190:
		_statPlay = 20;
		_vm->_wizard = false;
		_vm->_aTimer = 0;
		break;
	case 0x191:
		_statPlay = 20;
		_vm->_wizard = true;
		_vm->_aTimer = 0;
		break;
	case 0x192:
		_vm->_aTimer = 0;
		_vm->fadeToBlack2();
		_vm->playMovie(1);
		GPLogic1_SubSP10();
		break;
	case 0x193:
		_vm->_aTimer = 0;
		_vm->_keyActive = false;
		_vm->_noIFScreen = true;
		_vm->playSound(0);
		while(!_vm->_keyActive) {
			_vm->fadeToBlack2();
			_vm->playMovie(54);
		}
		GPLogic1_SubSP10();
		break;
	case 0x194:
		// CHECKME	_QuitFlag = 2;
		_vm->_quit = true;
		break;
	case 0x24A:
		_vm->fadeToBlack2();
		_vm->setMouse();
		_vm->eraseCursor();
		_vm->restoreGame();
		break;
	case 0x2F1:
		_vm->eraseCursorAsm();
		_vm->_cursorDrawn = false;
		_vm->fadeToBlack2();
		_vm->playSound(0);
		_vm->playMovie(54);
		GPLogic1_SubSP10();
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_20() {
	_vm->initPlay();
	_vm->_noIFScreen = false;
	_vm->eraseCursorAsm();
	_vm->_cursorDrawn = false;
	_vm->fadeToBlack2();
	_vm->showPic(106);
	_statPlay = 30;
	_vm->_userInput = 0;
	_vm->_loopFlag = true;
}

void Logic::GPL1_30() {
	_nodeNum = 3;
	_vm->_userInput = 0;
	if (_nodes[3] == 0) {
		_vm->playMovie(177);
		_nodes[3] = 9;
		if (!_vm->_wizard) {
			_statPlay = 50;
			_vm->_loopFlag = true;
			return;
		}
	}
	_vm->drawLocation();
	if (_nodes[4] != 0 && _nodes[6] != 1)
		_vm->_sound = true;
	else
		_vm->_sound = false;

	_vm->playMovie(7);
	_statPlay = 31;
	_currMap = 10;
	_vm->playSound(24);
	_vm->_userInput = 0;
}

void Logic::GPL1_31() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 32;
		_vm->_zoom = 2;
		_vm->processMap(3, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_32() {
	_vm->processMapInput(3);
}

void Logic::GPL1_40() {
	_nodeNum = 4;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[4] == 0)
		_vm->playMovie(62);
	if (_nodes[4] == 9) {
		_vm->playMovie(8);
		_vm->_tsIconOnly = true;
	}
	_currMap = 10;
	_vm->playSound(14);
	_statPlay = 41;
}

void Logic::GPL1_41() {
	switch(_vm->_userInput) {
	case 0x429:
		_vm->playMovie(178);
		_vm->playMovie(63);
		inventoryDel(1);
		_currMap = 10;
		_nodes[4] = 9;
		_statPlay = 41;
		_vm->_tsIconOnly = true;
		break;
	case 0x43A:
		_statPlay = 42;
		_vm->_zoom = 2;
		_vm->processMap(4, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_42() {
	_vm->processMapInput(4);
}

void Logic::GPL1_50() {
	_nodeNum = 5;
	_vm->drawLocation();
	if (_nodes[5] != 0 && !_resurrect) {
		_vm->_frameStop = 38;
		_vm->playMovie(5);
	}

	if (_nodes[5] == 0)
		_vm->playMovie(5);

	_resurrect = false;
	_vm->playSound(1);

	if (_spell1 && _spell2 && _spell3) {
		_vm->_sound = true;
		_vm->playMovie(2);
		_currMap = 10;
		_statPlay = 51;
		_vm->_loopFlag = true;
	} else if (_vm->_daelonCntr) {
		_vm->playMovie(2);
		_currMap = 10;
		_statPlay = 51;
	} else {
		switch(_nodes[5]) {
			case 0:
				inventoryAdd(1);
				break;
			case 1:
				_vm->playMovie(60);
				inventoryAdd(0);
				_spell1 = true;
				break;
			case 2:
				_vm->playMovie(59);
				inventoryAdd(2);
				_spell2 = true;
				break;
			case 3:
				_vm->_sound = true;
				_vm->playMovie(59);
				inventoryAdd(3);
				_spell3 = true;
				break;
			default:
			    break;
		}
		if (_spell1 && _spell2 && _spell3) {
			_vm->playMovie(6);
			_currMap = 7;
			_statPlay = 51;
			_vm->_loopFlag = true;
		} else {
			_eye = false;
			_vm->playMovie(61);
			_vm->playSound(1);

			if (!_spell1)
				_vm->drawPic(116);

			if (!_spell2)
				_vm->drawPic(117);

			if (!_spell3)
				_vm->drawPic(118);

			_currMap = 6;
			_statPlay = 53;
			_vm->_loopFlag = true;
		}
	}
}

void Logic::GPL1_51() {
	switch(_vm->_userInput) {
	case 0x406:
		_statPlay = 70;
		_vm->_loopFlag = true;
		break;
	case 0x40A:
		_statPlay = 110;
		_vm->_loopFlag = true;
		break;
	case 0x417:
		_statPlay = 30;
		_vm->_loopFlag = true;
		break;
	case 0x43A:
		_statPlay = 52;
		_vm->_zoom = 2;
		_vm->processMap(5, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_52() {
	_vm->processMapInput(5);
}

void Logic::GPL1_53() {
	switch(_vm->_userInput) {
	case 0x428:
		_nodes[5] = 1;
		_vm->_daelonCntr = 5;
		_vm->playMovie(6);
		_currMap = 7;
		_statPlay = 51;
		_eye = true;
		break;
	case 0x42A:
		_nodes[5] = 2;
		_vm->_daelonCntr = 5;
		_vm->playMovie(6);
		_currMap = 7;
		_statPlay = 51;
		_eye = true;
		break;
	case 0x42B:
		_nodes[5] = 3;
		_vm->_daelonCntr = 5;
		_vm->playMovie(6);
		_currMap = 7;
		_statPlay = 51;
		_eye = true;
		break;
	case 0x43E:
		if (!_spell1 && !_spell2 && !_spell3 && !_vm->_wizard) {
			_vm->playMovie(177);
			_vm->drawLocation();
			_vm->playMovie(5);
		}

		_vm->playSound(1);
		_eye = false;
		_vm->playMovie(61);

		if (!_spell1)
			_vm->drawPic(116);

		if (!_spell2)
			_vm->drawPic(117);

		if (!_spell3)
			_vm->drawPic(118);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_60() {
	_nodeNum = 6;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[4] == 0) {
		disableUIButtons();
		_vm->playMovie(9);
		_statPlay = 991;
	} else {
		_vm->playMovie(64);
		_statPlay = 200;
	}
	_currMap = 10;
	_vm->_loopFlag = true;
}

void Logic::GPL1_70() {
	_nodeNum = 7;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[7] == 0) {
		_vm->playMovie(11);
		_vm->playSound(2);
		_currMap = 9;
	} else {
		_vm->_sound = true;
		_vm->playMovie(13);
		_vm->playSound(43);
		_currMap = 10;
		_vm->_tsIconOnly = true;
	}
	_statPlay = 71;
}

void Logic::GPL1_71() {
	switch(_vm->_userInput) {
	case 0x42F:
		if (_nodes[7] == 9) {
			_vm->displayIcon(137);
		} else {
			_vm->playMovie(12);
			_vm->playSound(30);
			inventoryAdd(7);
			_nodes[7] = 9;
			_currMap = 10;
		}
		_vm->_userInput = 0;
		break;
	case 0x43A:
		_statPlay = 72;
		_vm->_zoom = 2;
		_vm->processMap(7, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_72() {
	_vm->processMapInput(7);
}

void Logic::GPL1_80() {
	_nodeNum = 8;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[8] == 9) {
		_vm->playMovie(24);
		_vm->playSound(3);
		_currMap = 13;
		_vm->_tsIconOnly = true;
	} else {
		_nodes[8] = 0;
		disableUIButtons();
		_vm->playMovie(14);
		_currMap = 10;
		_vm->setATimer();
	}
	_statPlay = 81;
}

void Logic::GPL1_81() {
	switch(_vm->_userInput) {
	case 0x2F1:
		disableUIButtons();
		_vm->playMovie(15);
		_statPlay = 991;
		_vm->_loopFlag = true;
		break;
	case 0x429:
		if (!_nodes[8]) {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(18);
			_vm->playSound(3);
			inventoryDel(1);
			_vm->_userInput = 0;
			_currMap = 13;
			_nodes[8] = 1;
		}
		break;
	case 0x42A:
		if (!_nodes[8]) {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(18);
			_vm->playSound(3);
			inventoryDel(2);
			_vm->_userInput = 0;
			_currMap = 13;
			_nodes[8] = 1;
		}
		break;
	case 0x42C:
		if (!_nodes[8]) {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(16);
			_vm->playSound(3);
			_nodes[8] = 9;
			_vm->_tsIconOnly = true;
			_currMap = 13;
		}
		break;
	case 0x42D:
		if (!_nodes[8]) {
			enableUIButtons();
			_vm->playMovie(17);
			inventoryDel(5);
			_vm->playSound(3);
			_vm->_aTimer = 0;
			_vm->_userInput = 0;
			_currMap = 13;
			_nodes[8] = 1;
		}
		break;
	case 0x43A:
		_statPlay = 82;
		_vm->_zoom = 2;
		_vm->processMap(8, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		_vm->_aTimer = 0;
		break;
	case 0x453:
		if (_nodes[8] == 1) {
			_statPlay = 80;
			_vm->_loopFlag = true;
		} else {
			_statPlay = 160;
			_vm->_loopFlag = true;
			_vm->_userInput = 0;
		}
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_82() {
	_vm->processMapInput(8);
}

void Logic::GPL1_90() {
	if (_nodeNum == 18 && _nodes[17] == 0) {
		_statPlay = 170;
		_vm->_loopFlag = true;
		_nextNode = 90;
	} else {
		_nodeNum = 9;
		_vm->drawLocation();
		_vm->_userInput = 0;
		if (_nodes[9] == 0) {
			_vm->_sound = false;
			_nodes[9] = 9;
		} else
			_vm->_sound = true;

		if (!_vm->_wizard)
			_vm->_frameStop = 90;

		_vm->playMovie(19);
		_vm->playSound(15);
		if (!_vm->_wizard)
			_vm->showPic(109);
		else
			_vm->showPic(319);

		_currMap = 10;
		_statPlay = 91;
	}
}

void Logic::GPL1_91() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 92;
		_vm->_zoom = 2;
		_vm->processMap(9, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		if (!_vm->_wizard)
			_vm->_frameStop = 90;

		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		if (!_vm->_wizard)
			_vm->showPic(109);
		else
			_vm->showPic(319);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_92() {
	_vm->processMapInput(9);
}

void Logic::GPL1_100() {
	if (_nodeNum == 18 && _nodes[17] == 0) {
		_statPlay = 170;
		_vm->_loopFlag = true;
		_nextNode = 100;
	} else {
		_nodeNum = 10;
		_vm->drawLocation();
		_vm->_userInput = 0;
		if (_nodes[10] == 1)
			_nodes[10] = 9;

		if (_nodes[10] != 9) {
			_vm->_sound = false;
			_nodes[10] = 1;
		} else if (_vm->_wizard)
			_vm->_sound = true;

		_vm->playMovie(20);
		_vm->playSound(41);
		_currMap = 10;
		_statPlay = 101;
	}
}

void Logic::GPL1_101() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 102;
		_vm->_zoom = 2;
		_vm->processMap(10, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_102() {
	_vm->processMapInput(10);
}

void Logic::GPL1_110() {
	_nodeNum = 11;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[22] == 0)
		_vm->_sound = false;
	else
		_vm->_sound = true;
	_vm->playMovie(21);
	_vm->playSound(42);
	_currMap = 10;
	_statPlay = 111;
}

void Logic::GPL1_111() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 112;
		_vm->_zoom = 1;
		_vm->processMap(11, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->playMovie(21);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_112() {
	_vm->processMapInput(11);
}

void Logic::GPL1_120() {
	_nodeNum = 12;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[12] == 0) {
		_vm->playMovie(22);
		_currMap = 14;
	} else {
		_vm->playMovie(25);
		_currMap = 15;
	}
	_eye = false;
	_vm->playSound(3);
	_statPlay = 121;
}

void Logic::GPL1_121() {
	switch (_vm->_userInput) {
	case 0x437:
		if (_nodes[12] == 9) {
			_vm->displayIcon(145);
			_vm->_userInput = 0;
		} else
			GPLogic1_SubSP121();
		break;
	case 0x43E:
		if (_vm->_pMovie == 023)
			GPLogic1_SubSP121();
		else {
			_vm->_sound = _vm->_lastSound;
			_vm->playMovie(_vm->_pMovie);
			_vm->playSound(3);
		}
		break;
	case 0x445:
		_statPlay = 150;
		_vm->_loopFlag = true;
		_eye = true;
		_vm->_userInput = 0;
		break;
	case 0x446:
		_statPlay = 80;
		_vm->_loopFlag = true;
		_eye = true;
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_130() {
	_nodeNum = 13;
	_vm->drawLocation();
	_vm->_userInput = 0;
	disableUIButtons();
	_vm->playMovie(4);
	_statPlay = 991;
	_vm->_loopFlag = true;
}

void Logic::GPL1_140() {
	_nodeNum = 14;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (wound()) {
		_vm->playMovie(26);
		_vm->playSound(35);
		_currMap = 10;
		_statPlay = 141;
	} else {
		disableUIButtons();
		_vm->playMovie(27);
		_statPlay = 991;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
	}
}

void Logic::GPL1_141() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 142;
		_vm->_zoom = 2;
		_vm->processMap(14, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_142() {
	_vm->processMapInput(14);
}

void Logic::GPL1_150() {
	_nodeNum = 15;
	_vm->drawLocation();
	_vm->playMovie(28);
	_vm->playSound(3);
	_currMap = 16;
	_statPlay = 151;
	_eye = false;
}

void Logic::GPL1_151() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		_vm->_aTimer = 0;
		break;
	case 0x445:
		_statPlay = 80;
		enableUIButtons();
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_160() {
	_nodeNum = 16;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_vm->playMovie(29);
	_vm->playSound(3);
	_currMap = 85;
	_statPlay = 161;
}

void Logic::GPL1_161() {
	switch(_vm->_userInput) {
	case 0x198: {
		_vm->fadeToBlack1();
		_vm->drawRect(4, 17, 228, 161, 0);
		_replay = true;
		int oldPMovie = _vm->_pMovie;
		_vm->playMovie(208);
		_vm->_pMovie = oldPMovie;
		_vm->_asMode = false;
		_vm->restoreAS();
		_help = true;
		_eye = true;
		_pouch = true;
		_currMap = 85;
		}
		break;
	case 0x199: {
		_vm->fadeToBlack1();
		_vm->drawRect(4, 17, 228, 161, 0);
		_replay = true;
		// Not present in the demo, but it's a bug
		int oldPMovie = _vm->_pMovie;
		_vm->playMovie(207);
		// Ditto
		_vm->_pMovie = oldPMovie;
		_vm->_asMode = false;
		_vm->restoreAS();
		_help = true;
		_eye = true;
		_pouch = true;
		_currMap = 85;
		}
		break;
	case 0x19A:
		_vm->fShowPic(110);
		_vm->_bTimer = 190;
		_vm->readMouse();
		while(_vm->_bTimer != 0 && _vm->_mouseButton == 0) {
			_vm->checkTimers();
			_vm->refreshSound();
			_vm->readMouse();
		}
		_vm->fShowPic(105);
		break;
	case 0x42E:
		_nodes[15] = 1;
		_nodes[16] = 1;
		_statPlay = 120;
		_vm->_loopFlag = true;
		break;
	case 0x43A:
		_statPlay = 162;
		_vm->_zoom = 2;
		_vm->processMap(16, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		if (_vm->_pMovie > 206)
			_vm->saveAS();
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		if (_vm->_pMovie > 206)
			_vm->restoreAS();
		break;
	case 0x444:
		_vm->saveAS();
		_help = false;
		_eye = false;
		_pouch = false;
		_replay = false;
		_vm->fShowPic(105);
		_currMap = 86;
		break;
	case 0x445:
		if (_nodes[16] || !_vm->_wizard) {
			_statPlay = 120;
			_vm->_loopFlag = true;
		}
		break;
	case 0x446:
		_statPlay = 80;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	case 0x450:
		_vm->saveAS();
		_vm->fShowPic(123);
		_vm->_bTimer = 76;
		_vm->readMouse();

		while(_vm->_bTimer != 0 && _vm->_mouseButton == 0) {
			_vm->checkTimers();
			_vm->refreshSound();
			_vm->readMouse();
		}

		_vm->fadeToBlack1();
		_vm->drawRect(4, 17, 228, 161, 0);
		_vm->restoreAS();
		break;	
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_162() {
	_vm->processMapInput(16);
}

void Logic::GPL1_170() {
	if (_nodes[17] == 9) {
		_statPlay = _nextNode;
		_vm->_loopFlag = true;
	} else {
		_nodeNum = 17;
		_vm->drawLocation();
		disableUIButtons();
		_vm->playMovie(30);
		_currMap = 10;
		_statPlay = 171;
		_vm->setATimer();
		_vm->_userInput = 0;
	}
}

void Logic::GPL1_171() {
	switch(_vm->_userInput) {
	case 0x2F1:
		enableUIButtons();
		if (wound()) {
			_vm->_aTimer = 0;
			_replay = false;
			_vm->playMovie(31);
			_statPlay = _nextNode;
			_vm->_loopFlag = true;
		} else {
			disableUIButtons();
			_vm->playMovie(32);
			_statPlay = 991;
			_vm->_loopFlag = true;
		}
		break;
	case 0x42C:
		_vm->_aTimer = 0;
		_replay = false;
		enableUIButtons();
		_vm->playMovie(34);
		_vm->_userInput = 0;
		_nodes[17] = 9;
		_statPlay = _nextNode;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_172() {
	if (_vm->_userInput == 0x2F1) {
		_statPlay = 171;
		_vm->_loopFlag = true;
		switchAS();
	} else
		_vm->processMapInput(_nodeNum);
}

void Logic::GPL1_180() {
	if ((_nodeNum != 9 && _nodeNum != 10) || (_nodes[17] != 0)) {
		_nodeNum = 18;
		_vm->drawLocation();
		_vm->_userInput = 0;
		if (_nodes[18] == 0 || _nodes[18] == 1) {
			_nodes[18] = 0;
			_vm->playMovie(33);
			_currMap = 10;
		}
		if (_nodes[18] == 9) {
			_vm->playMovie(37);
			_currMap = 19;
			_vm->_tsIconOnly = true;
		}
		_vm->playSound(16);
		_statPlay = 181;
	} else {
		_statPlay = 170;
		_vm->_loopFlag = true;
		_nextNode = 180;
	}
}

void Logic::GPL1_181() {
	switch(_vm->_userInput) {
	case 0x407:
		_statPlay = 80;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	case 0x428:
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		inventoryDel(0);
		_vm->_userInput = 0;
		break;
	case 0x429:
		if (!_nodes[18]) {
			inventoryDel(1);
			_nodes[18] = 1;
		}
		_vm->playMovie(35);
		_vm->_userInput = 0;
		break;
	case 0x42A:
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		inventoryDel(2);
		_vm->_userInput = 0;
		break;
	case 0x42C:
		_vm->saveAS();
		_vm->playMovie(34);
		_vm->restoreAS();
		break;
	case 0x42F:
		if (_nodes[18] == 1) {
			_vm->playMovie(36);
			inventoryDel(7);
			_nodes[18] = 9;
			_currMap = 19;
			_vm->_tsIconOnly = true;
		} else {
			_vm->_frameStop = 27;
			_fstFwd = false;
			_vm->playMovie(36);
			_fstFwd = true;
		}
		_vm->_userInput = 0;
		break;
	case 0x43A:
		_statPlay = 182;
		_vm->_zoom = 2;
		if (_nodes[18] == 9)
			_vm->processMap(80, _vm->_zoom);
		else
			_vm->processMap(18, _vm->_zoom);

		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		if (_vm->_pMovie == 36 && _nodes[18] != 9) {
			_vm->_frameStop = 27;
			_fstFwd = false;
			_vm->playMovie(36);
			_fstFwd = true;
		} else
			_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_182() {
	_vm->processMapInput(18);
}

void Logic::GPL1_190() {
	_nodeNum = 19;
	_vm->drawLocation();
	_vm->_userInput = 0;
	disableUIButtons();
	_vm->playMovie(38);
	_statPlay = 991;
	_vm->_loopFlag = true;
}

void Logic::GPL1_200() {
	_nodeNum = 20;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (!_nodes[20]) {
		_vm->playMovie(39);
		increaseHealth();
	} else {
		_vm->playMovie(40);
		_tideCntl = true;
		_vm->drawPic(178);
		_nodes[20] = 0;
	}
	_vm->playSound(42);
	_currMap = 10;
	_statPlay = 201;
}

void Logic::GPL1_201() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 202;
		_vm->_zoom = 0;
		_vm->processMap(20, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_202() {
	_vm->processMapInput(20);
}

void Logic::GPL1_210() {
	_nodeNum = 21;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (!_vm->_wizard)
		_nodes[21] = 9;

	if (_nodes[21] == 9) {
		// Present in the original but useless
		// _nodes[21] = 9;
		_vm->playMovie(_vm->isDemo() ? 49 : 209);
		_vm->playSound(38);
		_currMap = 81;
	} else {
		disableUIButtons();
		_vm->playMovie(45);
		_nodes[21] = 0;
		_currMap = 10;
		_vm->setATimer();
	}
	_statPlay = 211;
}

void Logic::GPL1_211() {
	switch(_vm->_userInput) {
	case 0x2F1:
		disableUIButtons();
		_vm->playMovie(46);
		_statPlay = 991;
		_vm->_loopFlag = true;
		break;
	case 0x428:
		inventoryDel(0);
		if (_nodes[21] == 9) {
			_vm->saveAS();
			_vm->playMovie(65);
			_vm->restoreAS();
			_vm->_userInput = 0;
		} else {
			_vm->playMovie(178);
			disableUIButtons();
			_vm->playMovie(46);
			_statPlay = 991;
			_vm->_loopFlag = true;
		}
		break;
	case 0x42A:
		if (!_nodes[21]) {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(48);
			inventoryDel(2);
			_nodes[21] = 1;
			_vm->_tsIconOnly = true;
			_vm->playSound(38);
			_vm->_userInput = 0;
		}
		break;
	case 0x42D:
		if (!_nodes[21]) {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(47);
			_vm->playSound(38);
			inventoryDel(5);
			_nodes[21] = 9;
			_vm->_userInput = 0;
			_currMap = 81;
			_vm->_tsIconOnly = true;
		}
		break;
	case 0x43A:
		_statPlay = 212;
		_vm->_zoom = 2;
		_vm->processMap(21, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		if (_vm->_pMovie == 65) {
			_vm->saveAS();
			_vm->playMovie(65);
			_vm->restoreAS();
			_vm->_userInput = 0;
		} else {
			_vm->_sound = _vm->_lastSound;
			_vm->playMovie(_vm->_pMovie);
		}
		break;
	case 0x441:
		_statPlay = 220;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	case 0x442:
		if (_vm->_tsIconOnly) {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(49);
			_vm->_tsIconOnly = false;
			_vm->playSound(38);
			_vm->_userInput = 0;
		}
		break;
	case 0x448:
		_statPlay = 140;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_212() {
	if (_vm->_userInput == 0x2F1) {
		_vm->_userInput = 0;
		_vm->_aTimer = 0;
		_statPlay = 211;
		_vm->_loopFlag = true;
		switchAS();
	}
	_vm->processMapInput(21);
}

void Logic::GPL1_220() {
	_nodeNum = 22;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (!_nodes[22]) {
		_vm->playMovie(41);
		_currMap = 82;
	} else {
		_vm->_tsIconOnly = true;
		_vm->playMovie(44);
		_currMap = 10;
	}
	_vm->playSound(38);
	_statPlay = 221;
}

void Logic::GPL1_221() {
	switch(_vm->_userInput) {
	case 0x428:
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		inventoryDel(0);
		_vm->_userInput = 0;
		break;
	case 0x429:
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		inventoryDel(1);
		_vm->_userInput = 0;
		break;
	case 0x42A:
		_vm->playMovie(42);
		_vm->playSound(30);
		if (!_nodes[22]) {
			inventoryDel(2);
			inventoryAdd(4);
			_vm->_tsIconOnly = true;
			_nodes[22] = 9;
		}
		_vm->_userInput = 0;
		_currMap = 10;
		break;
	case 0x42C:
		if (!_nodes[22]) {
			disableUIButtons();
			_vm->playMovie(43);
			_vm->playSound(34);
			_statPlay = 991;
			_vm->_loopFlag = true;
		} else {
			_vm->playMovie(42);
			_vm->playSound(30);
		}
		break;
	case 0x43A:
		_statPlay = 222;
		_vm->_zoom = 2;
		_vm->processMap(22, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL1_222() {
	_vm->processMapInput(22);
}

void Logic::GPL1_230() {
	_nodeNum = 23;
	_vm->drawLocation();
	_vm->_userInput = 0;
	disableUIButtons();
	_vm->playMovie(50);
	_statPlay = 991;
	_vm->_loopFlag = true;
}

void Logic::GPL1_240() {
	_nodeNum = 24;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (!_nodes[24]) {
		_vm->playMovie(51);
		_currMap = 83;
	} else {
		_vm->playMovie(53);
		_vm->showPic(119);
		_currMap = 84;
		_vm->_tsIconOnly = true;
	}
	_vm->playSound(4);
	_statPlay = 241;
}

void Logic::GPL1_241() {
	switch(_vm->_userInput) {
	case 0x428:
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		inventoryDel(0);
		break;
	case 0x429:
		if (_nodes[24] != 9) {
			_vm->_tsIconOnly = true;
			_vm->playMovie(66);
			inventoryDel(1);
			_currMap = 84;
			_nodes[24] = 9;
		}
		break;
	case 0x42A:
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		inventoryDel(2);
		break;
	case 0x42D:
		if (_inventory[5] >= 1)
			_vm->displayIcon(135);
		else {
			_vm->saveAS();
			_vm->playMovie(52);
			_vm->playSound(30);
			_vm->restoreAS();
			inventoryAdd(5);
		}
		break;
	case 0x43A:
		_statPlay = 242;
		_vm->_zoom = 2;
		_vm->processMap(24, _vm->_zoom);
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		if (_vm->_pMovie == 53) {
			_vm->playMovie(53);
			_vm->showPic(119);
			_currMap = 84;
		} else
			_vm->playMovie(_vm->_pMovie);
		break;
	case 0x441:
		_currMap = 10;
		_statPlay = 300;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
	_vm->_userInput = 0;
}

void Logic::GPL1_242() {
	_vm->processMapInput(24);
}

void Logic::GPLogic1_SubSP10() {
	_vm->playSound(0);
	_vm->playSound(18);
	_vm->fadeToBlack2();
	_vm->showPic(_vm->isDemo() ? 198 : 115);
	_vm->showPic(114);

	_currMap = 4;
	_vm->_userInput = 0;
	_vm->_aTimer = 595;
	_statPlay = 11;
}

void Logic::GPLogic1_SubSP121() {
	_vm->playMovie(23);
	_vm->playSound(30);
	_vm->_bTimer = 190;
	_vm->readMouse();
	
	while (_vm->_bTimer != 0 && _vm->_mouseButton == 0) {
		_vm->checkTimers();
		_vm->refreshSound();
		_vm->checkMainScreen();
		_vm->readMouse();
	}

	if (!_vm->isDemo()) {
		_vm->playSound(23);
		inventoryAdd(15);
		_currMap = 10;
		_vm->fShowPic(150);
		_nodes[12] = 9;
		_vm->playSound(3);
		_currMap = 15;
	} else {
		inventoryAdd(15);
		_currMap = 10;
		_vm->fShowPic(150);
		_nodes[10] = 9;
		_vm->playSound(3);
		_vm->_bTimer = 95;
		while (_vm->_bTimer != 0 && _vm->_mouseButton == 0) {
			_vm->checkTimers();
			_vm->refreshSound();
			_vm->checkMainScreen();
		}
		_vm->readMouse();
		while (_vm->_mouseButton == 0) {
			_vm->refreshSound();
			_vm->checkMainScreen();
			_vm->readMouse();
		}

		_vm->fadeToBlack2();
		_vm->_quit = true;
		//_QuitFlag = 1;
	}
}

} // NameSpace
