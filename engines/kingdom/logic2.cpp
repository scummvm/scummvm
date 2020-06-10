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

void Logic::GPL2_250() {
	bool highNode = (_nodeNum > 25);
	_nodeNum = 25;

	_vm->drawLocation();
	disableUIButtons();
	if (_nodes[24] != 0 || highNode) {
		if (!_vm->isDemo())
			_eye = true;
		_vm->playMovie(72);
		_vm->_aTimerFlag = false;
		_vm->_aTimer = 114;
		_vm->playSound(34);
		_vm->_userInput = 0;
		_currMap = 10;
		_statPlay = 251;
	} else {
		_vm->_frameStop = 67;
		_vm->playMovie(72);
		if (!_vm->isDemo())
			disableUIButtons();
		_vm->playMovie(73);
		disableUIButtons();
		_statPlay = 991;
		_vm->_loopFlag = true;
	}
}

void Logic::GPL2_251() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_vm->_aTimer = 0;
		_vm->playMovie(73);
		disableUIButtons();
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 0x43A:
		_statPlay = 252;
		_vm->_zoom = 2;
		_vm->processMap(30, _vm->_zoom);
		_vm->setATimer();
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_252() {
	_vm->processMapInput(30);
}

void Logic::GPL2_260() {
	_nodeNum = 26;
	_vm->drawLocation();
	disableUIButtons();
	_vm->playMovie(74);
	_currMap = 10;
	_vm->setATimer();
	_statPlay = 261;
	_vm->_userInput = 0;
}

void Logic::GPL2_261() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(75);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 0x428:
		inventoryDel(0);
		_vm->_aTimer = 0;
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		_vm->setATimer();
		_vm->_userInput = 0;
		break;
	case 0x42A:
		inventoryDel(2);
		_vm->_aTimer = 0;
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		_vm->setATimer();
		_vm->_userInput = 0;
		break;
	case 0x42C:
		_vm->_aTimer = 0;
		enableUIButtons();
		_vm->playMovie(76);
		_nodes[26] = 1;
		_vm->_userInput = 0;
		_statPlay = 290;
		_lastObs = true;
		_lastObstacle = 29;
		_vm->_loopFlag = true;
		break;
	case 0x42D:
		if (wound()) {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(77);
			_vm->playSound(34);
			inventoryDel(5);
			_statPlay = 300;
			_vm->_loopFlag = true;
		} else {
			disableUIButtons();
			_vm->playMovie(78);
			_statPlay = 992;
			_vm->_loopFlag = true;
		}
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_270() {
	_nodeNum = 27;
	_vm->drawLocation();
	if (_nodes[27] == 1)
		_nodes[27] = 9;
	if (_nodes[27] == 9 || _nodes[28] != 0) {
		if (_vm->_wizard)
			_vm->_sound = true;
	} else {
		_vm->_sound = false;
		_nodes[27] = 1;
	}
	_vm->playMovie(80);
	_vm->playSound(18);
	_currMap = 10;
	_statPlay = 271;
	_nodes[27] = 1;
	_vm->_userInput = 0;
}

void Logic::GPL2_271() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 272;
		_vm->_zoom = 2;
		_vm->processMap(27, _vm->_zoom);
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

void Logic::GPL2_272() {
	_vm->processMapInput(27);
}

void Logic::GPL2_280() {
	_nodeNum = 28;
	_vm->drawLocation();
	_vm->_userInput = 0;
	disableUIButtons();
	_vm->_fstFwd = false;
	_vm->playMovie(81);
	_currMap = 10;
	_nodes[28] = _nodes[38] = 1;
	_statPlay = _robberyNode * 10;
	enableUIButtons();
	_vm->_loopFlag = true;
}

void Logic::GPL2_290() {
	_oldNode = _nodeNum;
	if (!_vm->_wizard) {
		_nodes[26] = 9;
		_nodes[29] = 8;
	}
	if (_nodes[26] == 0) {
		_statPlay = 260;
		_vm->_loopFlag = true;
		_nextNode = 29;
	} else {
		if (_nodes[26] == 2)
			_vm->playMovie(79);
		_nodeNum = 29;
		if (_nodes[26] <= 2 || !chkDesertObstacles()) {
			_vm->drawLocation();
			_nodes[26]++;
			_vm->_userInput = 0;
			switch (_nodes[29]) {
			case 0:
				_vm->_sound = false;
				_vm->playMovie(82);
				_currMap = 89;
				_nodes[29] = 1;
				break;
			case 1:
				_vm->_sound = true;
				_vm->playMovie(82);
				_currMap = 89;
				_nodes[29] = 0;
				break;
			case 8:
				_vm->_sound = false;
				_vm->playMovie(85);
				_currMap = 10;
				_nodes[29] = 9;
				_vm->_tsIconOnly = true;
				break;
			default:
				_vm->_sound = true;
				_vm->playMovie(85);
				_currMap = 10;
				_vm->_tsIconOnly = true;
				break;
			}
			_vm->playSound(17);
			_statPlay = 291;
		}
	}

}

void Logic::GPL2_291() {
	switch(_vm->_userInput) {
	case 0x431:
		_vm->_tsIconOnly = true;
		_vm->playMovie(83);
		_vm->playSound(30);
		inventoryAdd(10);
		_nodes[29] = 8;
		_vm->_userInput = 0;
		_currMap = 10;
		break;
	case 0x432:
		if (_nodes[29] >= 8)
			_vm->displayIcon(140);
		else {
			disableUIButtons();
			_vm->playMovie(84);
			_statPlay = 992;
			_vm->_loopFlag = true;
		}
		break;
	case 0x43A:
		_statPlay = 292;
		_vm->_zoom = 2;
		_vm->processMap(29, _vm->_zoom);
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

void Logic::GPL2_292() {
	_vm->processMapInput(29);
}

void Logic::GPL2_300() {
	if (_nodes[24] == 0) {
		_vm->_loopFlag = true;
		_statPlay = 250;
	} else {
		_nodeNum = 30;
		if (!chkDesertObstacles()) {
			_vm->drawLocation();
			_vm->playMovie(86);
			_vm->playSound(18);
			_currMap = 10;
			_statPlay = 301;
		}
	}
}

void Logic::GPL2_301() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 302;
		_vm->_zoom = 2;
		_vm->processMap(30, _vm->_zoom);
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

void Logic::GPL2_302() {
	_vm->processMapInput(30);
}

void Logic::GPL2_310() {
	_oldNode = _nodeNum;
	_nodeNum = 31;
	if (!chkDesertObstacles()) {
		_vm->drawLocation();
		_vm->playMovie(67);
		_vm->playSound(18);
		_currMap = 90;
		_statPlay = 311;
		_nodes[31] = 1;
		_vm->_aTimer = 0;
		_vm->_userInput = 0;
	}
}

void Logic::GPL2_311() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 312;
		_vm->_zoom = 2;
		_vm->processMap(31, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x449:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(68);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_312() {
	_vm->processMapInput(31);
}

void Logic::GPL2_320() {
	_oldNode = _nodeNum;
	_nodeNum = 32;
	_vm->drawLocation();
	_vm->_userInput = 0;
	increaseHealth();
	if (_nodes[32] != 9 && _vm->_wizard) {
		_vm->_sound = false;
		_nodes[32] = 9;
	} else {
		_vm->_sound = true;
		_nodes[32] = 1;
	}
	_vm->playMovie(87);
	_vm->playSound(18);
	_currMap = 10;
	_statPlay = 321;
}

void Logic::GPL2_321() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 322;
		_vm->_zoom = 2;
		_vm->processMap(32, _vm->_zoom);
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

void Logic::GPL2_322() {
	_vm->processMapInput(32);
}

void Logic::GPL2_330() {
	_oldNode = _nodeNum;
	_nodeNum = 33;
	if (!chkDesertObstacles()) {
		_vm->drawLocation();
		_vm->_userInput = 0;
		if (_nodes[31] == 0)
			_vm->_sound = false;
		else
			_vm->_sound = true;
		_vm->playMovie(88);
		_vm->playSound(18);
		_currMap = 10;
		_statPlay = 331;
	}
}

void Logic::GPL2_331() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 332;
		_vm->_zoom = 2;
		_vm->processMap(33, _vm->_zoom);
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

void Logic::GPL2_332() {
	_vm->processMapInput(33);
}

void Logic::GPL2_340() {
	if (_nodes[35] == 0) {
		_statPlay = 350;
		_vm->_loopFlag = true;
		_nextNode = 34;
	} else {
		_oldNode = _nodeNum;
		_nodeNum = 34;
		if (!chkDesertObstacles()) {
			_vm->drawLocation();
			_vm->_userInput = 0;
			if (_nodes[34] == 9 || _nodes[40] == 0 || !_vm->_wizard) {
				_vm->playMovie(91);
				_currMap = 92;
			} else {
				_vm->playMovie(89);
				_vm->showPic(389);
				_currMap = 91;
			}
			if (_nodes[34] == 9)
				_vm->_tsIconOnly = true;
			_vm->playSound(26);
			_statPlay = 341;
		}
	}
}

void Logic::GPL2_341() {
	switch(_vm->_userInput) {
	case 0x431:
		if (_nodes[34] == 9)
			_vm->displayIcon(139);
		else {
			_vm->playMovie(90);
			inventoryAdd(9);
			_nodes[34] = 9;
			_currMap = 93;
		}
		break;
	case 0x43A:
		_statPlay = 342;
		_vm->_zoom = 2;
		_vm->processMap(34, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		if (_vm->_pMovie == 89)
			_vm->showPic(389);
		break;
	case 0x445:
		_statPlay = 410;
		_vm->_userInput = 0;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x446:
		_statPlay = 400;
		_vm->_userInput = 0;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_342() {
	_vm->processMapInput(34);
}

void Logic::GPL2_350() {
	if (!_vm->_wizard)
		_nodes[35] = 1;
	if (_nodes[35] == 1) {
		_statPlay = _nextNode * 10;
		_vm->_loopFlag = true;
	} else {
		_nodeNum = 35;
		_vm->drawLocation();
		disableUIButtons();
		_vm->playMovie(92);
		_currMap = 94;
		_vm->setATimer();
		_vm->_userInput = 0;
		_statPlay = 351;
	}
}

void Logic::GPL2_351() {
	switch(_vm->_userInput) {
	case 0x428:
	case 0x429:
		inventoryDel(_vm->_userInput - 0x428);
		_vm->_aTimer = 0;
		_vm->playMovie(178);
		_vm->_userInput = 0x2F1;
		_vm->_loopFlag = true;
		// fallthrough
	case 0x2F1:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(93);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 0x42A:
		_vm->_aTimer = 0;
		enableUIButtons();
		_vm->playMovie(179);
		inventoryDel(2);
		_nodes[35] = 1;
		_vm->_userInput = 0;
		_statPlay = _nextNode * 10;
		_lastObs = true;
		_lastObstacle = _nextNode;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_360() {
	_nodeNum = 36;
	_vm->drawLocation();
	if (_nodes[36] == 0)
		_vm->_sound = false;
	else
		_vm->_sound = true;
	_vm->_frameStop = 185;
	disableUIButtons();
	_vm->playMovie(94);
	_currMap = 95;
	_vm->setATimer();
	_vm->_userInput = 0;
	_statPlay = 361;
}

void Logic::GPL2_361() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(95);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 0x428:
		_vm->_aTimer = 0;
		enableUIButtons();
		_vm->playMovie(97);
		inventoryDel(0);
		_vm->playSound(5);
		_nodes[36] = 1;
		_vm->_userInput = 0;
		_currMap = 10;
		if (_nodes[42] == 0) {
			_statPlay = 420;
			_oldNode = _nodeNum;
			_nodeNum = 42;
			GPLogic2_SubSP361();
		} else {
			_vm->_tsIconOnly = true;
		}
		break;
	case 0x42A:
		_vm->_aTimer = 0;
		enableUIButtons();
		_vm->playMovie(98);
		inventoryDel(2);
		_vm->playSound(5);
		_nodes[36] = 1;
		_vm->_userInput = 0;
		_currMap = 10;
		_vm->_tsIconOnly = true;
		break;
	case 0x42E:
		_vm->_aTimer = 0;
		enableUIButtons();
		_vm->playMovie(100);
		inventoryDel(6);
		_vm->playSound(5);
		_nodes[36] = 1;
		_vm->_userInput = 0;
		_currMap = 10;
		_vm->_tsIconOnly = true;
		break;
	case 0x431:
		_vm->_aTimer = 0;
		enableUIButtons();
		_vm->playMovie(96);
		_vm->playSound(5);
		_nodes[36] = 9;
		_vm->_userInput = 0;
		_currMap = 10;
		_statPlay = 420;
		_oldNode = _nodeNum;
		_nodeNum = 42;
		GPLogic2_SubSP361();
		break;
	case 0x437:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(99);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 0x43A:
		_statPlay = 362;
		_vm->_zoom = 2;
		_vm->processMap(36, _vm->_zoom);
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

void Logic::GPL2_362() {
	_vm->processMapInput(36);
}

void Logic::GPL2_370() {
	_nodeNum = 37;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_nodes[37] = 0;
	_vm->playMovie(102);
	_vm->playSound(20);
	_currMap = 109;
	_statPlay = 371;
}

void Logic::GPL2_371() {
	switch(_vm->_userInput) {
	case 0x413:
		_statPlay = 440;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x43A:
		_statPlay = 372;
		_vm->_zoom = 2;
		_vm->processMap(37, _vm->_zoom);
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

void Logic::GPL2_372() {
	_vm->processMapInput(37);
}

void Logic::GPL2_380() {
	_oldNode = _nodeNum;
	_nodeNum = 38;
	if (!chkDesertObstacles()) {
		_vm->drawLocation();
		_vm->_userInput = 0;
		_vm->playMovie(159);
		_currMap = 10;
		_vm->playSound(7);
		_nodes[38] = 1;
		_statPlay = 381;
	}
}

void Logic::GPL2_381() {
	switch(_vm->_userInput) {
	case 0x428:
	case 0x429:
	case 0x42A:
		if (_vm->_wizard && _nodes[38] != 9) {
			inventoryDel(_vm->_userInput - 0x428);
			_vm->saveAS();
			_vm->playMovie(178);
			_vm->restoreAS();
		}
		_vm->_userInput = 0;
		break;
	case 0x430:
		if (_vm->_wizard && _nodes[38] != 9) {
			disableUIButtons();
			_vm->playMovie(162);
			_statPlay = 992;
			_vm->_loopFlag = true;
		}
		break;
	case 0x433:
		if (_vm->_wizard && _nodes[38] != 9) {
			_vm->playMovie(161);
			inventoryDel(11);
			_nodes[50] = 0;
			_vm->_userInput = 0;
		}
		_vm->_userInput = 0;
		break;
	case 0x435:
		if (_vm->_wizard && _nodes[38] != 9) {
			_vm->playMovie(160);
			inventoryDel(13);
			_nodes[38] = 9;
			_nodes[58] = 0;
			_vm->_userInput = 0;
			_vm->_tsIconOnly = true;
			_currMap = 108;
		}
		break;
	case 0x437:
		if (_vm->_wizard && _nodes[38] != 9) {
			_vm->playMovie(163);
			inventoryDel(15);
			_nodes[12] = 0;
		}
		_vm->_userInput = 0;
		break;
	case 0x43A:
		_statPlay = 382;
		_vm->_zoom = 2;
		_vm->processMap(38, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x456:
		_statPlay = 370;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_382() {
	if (_vm->_wizard && (_nodes[38] != 9) && (_vm->_userInput == 0x413))
		_vm->_userInput = 0;

	_vm->processMapInput(38);
}

void Logic::GPL2_390() {
	_nodeNum = 39;
	_vm->drawLocation();
	_vm->_aTimer = 0;
	disableUIButtons();
	_vm->playMovie(104);
	_vm->playSound(34);
	_statPlay = 992;
	_vm->_loopFlag = true;
}

void Logic::GPL2_400() {
	_nodeNum = 40;
	if (!_vm->_wizard)
		_nodes[40] = 2;
	if (_nodes[40]) {
		if (_nodes[40] == 1)
			_nodes[40] = 2;
		else {
			_nodes[40] = 1;
			_vm->_sound = true;
		}
	} else {
		disableUIButtons();
		_vm->playMovie(105);
		_currMap = 10;
		_vm->setATimer();
	}

	_vm->_userInput = 0;
	_statPlay = 401;
}

void Logic::GPL2_401() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(106);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 0x428:
	case 0x429:
	case 0x42A:
		if (!_nodes[40]) {
			inventoryDel(_vm->_userInput - 0x428);
			_vm->_aTimer = 0;
			_vm->playMovie(178);
			_vm->_userInput = 753;
			_vm->_loopFlag = true;
			_currMap = 10;
		}
		break;
	case 0x42C:
		if (!_nodes[40]) {
			_vm->_aTimer = 0;
			disableUIButtons();
			_vm->playMovie(108);
			_vm->_userInput = 0;
			_statPlay = 992;
			_vm->_loopFlag = true;
		}
		break;
	case 0x42E:
		if (!_nodes[40]) {
			_vm->_aTimer = 0;
			_vm->playMovie(109);
			inventoryDel(6);
			_nodes[42] = 0;
			_vm->_userInput = 0;
			_vm->_aTimer = 136;
		}
		break;
	case 0x437:
		if (!_nodes[40]) {
			_vm->_aTimer = 0;
			_help = true;
			_replay = true;
			_pouch = true;
			_vm->_fstFwd = true;
			_vm->playMovie(107);
			_vm->playSound(26);
			_nodes[40] = 1;
			_vm->_userInput = 0;
			_currMap = 96;
			_vm->_tsIconOnly = true;
			_vm->_loopFlag = true;
		}
		break;
	case 0x43E:
		_vm->_aTimer = 0;
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x441:
		_statPlay = 460;
		_currMap = 10;
		_vm->_userInput = 0;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_402() {
	_vm->_aTimer = 0;
	_vm->processMapInput(40);
}

void Logic::GPL2_410() {
	_nodeNum = 41;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[40])
		_vm->_sound = true;

	_eye = false;
	_vm->playMovie(111);
	_vm->playSound(26);
	_currMap = 97;
	_statPlay = 411;
}

void Logic::GPL2_411() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		if (_vm->_pMovie == 120) {
			_vm->saveAS();
			_vm->_frameStop = 50;
			_vm->playMovie(120);
			_vm->_bTimer = 34;
			while(_vm->_bTimer != 0) {
				_vm->checkTimers();
				_vm->refreshSound();
				_vm->checkMainScreen();
			}
			_vm->restoreAS();
		}
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_vm->_userInput = 0;
		_vm->_loopFlag = true;
		_currMap = 10;
		_statPlay = 450;
		break;
	case 0x446:
		_vm->_userInput = 0;
		_vm->_loopFlag = true;
		_currMap = 10;
		_statPlay = 460;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_420() {
	if (_nodes[36] == 9 || _nodes[28] == 1 || !_vm->_wizard) {
		_oldNode = _nodeNum;
		_nodeNum = 42;
		if (!chkDesertObstacles()) {
			_vm->drawLocation();
			GPLogic2_SubSP361();
		}
	} else {
		_statPlay = 360;
		_vm->_loopFlag = true;
	}
}

void Logic::GPL2_421() {
	switch(_vm->_userInput) {
	case 0x428:
	case 0x429:
	case 0x42A:
		inventoryDel(_vm->_userInput - 0x428);
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		_vm->_userInput = 0;
		break;
	case 0x42E:
		if (_nodes[42])
			_vm->displayIcon(136);
		else {
			if (_nodes[34] == 9) {
				_vm->_sound = false;
			} else {
				_vm->_sound = true;
			}
			_vm->playMovie(113);
			inventoryAdd(6);
			_nodes[42] = 9;
			_currMap = 10;
		}
		_vm->_tsIconOnly = true;
		break;
	case 0x43A:
		_statPlay = 422;
		_vm->_zoom = 2;
		_vm->processMap(42, _vm->_zoom);
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

void Logic::GPL2_422() {
	_vm->processMapInput(42);
}

void Logic::GPL2_430() {
	_nodeNum = 43;
	_vm->drawLocation();
	_eye = false;
	_vm->_userInput = 0;
	if (_nodes[43] == 9) {
		_vm->_tsIconOnly = true;
		_vm->playMovie(118);
		_currMap = 80;
	} else if (_inventory[12] <= 0) {
		_vm->playMovie(115);
		_vm->showPic(415);
		_currMap = 78;
		if (!_vm->_skylarTimer)
			_vm->_skylarTimer = 5400;
	} else {
		_vm->_skylarTimer = 0;
		_vm->playMovie(116);
		_vm->playSound(25);
		inventoryDel(12);
		inventoryAdd(16);
		_nodes[43] = 9;
		_currMap = 79;
		_vm->_tsIconOnly = true;
	}
	_statPlay = 431;
	_vm->playSound(20);
	_eye = false;
}

void Logic::GPL2_431() {
	switch(_vm->_userInput) {
	case 0x40A:
		_vm->playMovie(10);
		_statPlay = 200;
		if (_nodes[43] != 9)
			_nodes[20] = 1;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x414:
		_vm->playMovie(10);
		_statPlay = 390;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x41D:
		_vm->playMovie(10);
		_statPlay = 500;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x422:
		_vm->playMovie(10);
		_statPlay = 380;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x428:
	case 0x429:
	case 0x42A:
		if (_nodes[43] != 9) {
			inventoryDel(_vm->_userInput - 0x428);
			_vm->saveAS();
			_vm->playMovie(178);
			_vm->restoreAS();
		}
		break;
	case 0x438:
		if (_nodes[43] == 9) {
			_vm->displayIcon(146);
		} else {
			disableUIButtons();
			_vm->playMovie(117);
			_statPlay = 992;
			_vm->_loopFlag = true;
		}
		break;
	case 0x43E:
		// TODO: CHECKME
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_440() {
	_nodeNum = 44;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_inventory[12] > 0 || _nodes[43] == 9)
		_vm->_sound = true;

	_vm->playMovie(119);
	_vm->playSound(20);
	_currMap = 100;
	_statPlay = 441;
}

void Logic::GPL2_441() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 442;
		_vm->_zoom = 2;
		_vm->processMap(44, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x452:
		_statPlay = 430;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_442() {
	_vm->processMapInput(44);
}

void Logic::GPL2_450() {
	_nodeNum = 45;
	_vm->_userInput = 0;
	if (!_nodes[40]) {
		disableUIButtons();
		_vm->playMovie(120);
		_statPlay = 992;
		_currMap = 10;
		_vm->_loopFlag = true;
	} else {
		_vm->_fstFwd = false;
		_vm->_frameStop = 50;
		_vm->playMovie(120);
		_currMap = 17;
		_statPlay = 451;
	}
}

void Logic::GPL2_451() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->_fstFwd = false;
		_vm->_frameStop = 50;
		_vm->playMovie(_vm->_pMovie);
		_currMap = 17;
		_vm->playSound(26);
		break;
	case 0x441:
		_statPlay = 410;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_460() {
	_eye = false;
	_nodeNum = 46;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[46] && _vm->_wizard)
		_nodes[46] = 1;
	else {
		_nodes[46] = 0;
		_vm->_sound = true;
	}
	_vm->playMovie(121);
	_vm->playSound(26);
	_currMap = 107;
	_statPlay = 461;
}

void Logic::GPL2_461() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_statPlay = 340;
		_vm->_userInput = 0;
		_vm->_loopFlag = true;
		_currMap = 10;
		_lastObs = true;
		break;
	case 0x446:
		_statPlay = 400;
		_vm->_userInput = 0;
		_vm->_loopFlag = true;
		_currMap = 10;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL2_462() {
	_vm->processMapInput(46);
}

void Logic::GPL2_480() {
	_nodeNum = 48;
	_vm->_userInput = 0;
	_vm->playMovie(114);
	_currMap = 10;
	_statPlay = 481;
	_nodes[48] = 1;
}

void Logic::GPL2_481() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 482;
		_vm->_zoom = 2;
		_vm->processMap(48, _vm->_zoom);
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

void Logic::GPL2_482() {
	_vm->processMapInput(48);
}

void Logic::GPL2_490() {
	_nodeNum = 49;
	_vm->drawLocation();
	disableUIButtons();
	_vm->playMovie(180);
	_vm->playSound(18);
	_currMap = 101;
	_statPlay = 491;
	_vm->_aTimer = 0;
	_vm->_userInput = 0;
}

void Logic::GPL2_491() {
	switch(_vm->_userInput) {
	case 0x428:
	case 0x42A:
	case 0x42B:
		inventoryDel(_vm->_userInput - 0x428);
		_vm->_aTimer = 0;
		_vm->playMovie(178);
		_vm->_userInput = 753;
		_vm->_loopFlag = true;
		_currMap = 10;
		break;
	case 0x442:
		_vm->playMovie(182);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 0x44A:
		_currMap = 10;
		_vm->_userInput = 0;
		_vm->_aTimer = 0;
		enableUIButtons();
		_vm->playMovie(181);
		_nodes[28] = 9;
		_nodes[48] = 0;
		_statPlay = _nextNode * 10;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPLogic2_SubSP361() {
	enableUIButtons();
	_vm->_userInput = 0;
	_vm->playSound(19);
	if (_nodes[28] == 1) {
		_statPlay = 480;
		_vm->_loopFlag = true;
		_currMap = 10;
	} else {
		if (_inventory[6] == 1 || !_vm->_wizard) {
			if (_nodes[42] != 1 || _vm->_wizard)
				_nodes[42] = 1;
			else {
				_vm->_sound = true;
				_nodes[42] = 2;
			}
			_vm->playMovie(122);
			_currMap = 10;
			_vm->_tsIconOnly = true;
			_statPlay = 421;
		} else {
			_vm->playMovie(112);
			_currMap = 98;
			_statPlay = 421;
		}
	}
}

} // NameSpace
