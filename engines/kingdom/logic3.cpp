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

void Logic::GPL3_500() {
	_nodeNum = 50;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_resurrect = false;
	if (!_vm->_wizard)
		_nodes[50] = 1;
	switch(_nodes[50]) {
	case 0:
		_vm->playMovie(123);
		_currMap = 102;
		break;
	case 1:
		_nodes[50] = 2;
		_vm->playMovie(126);
		_currMap = 10;
		_vm->_tsIconOnly = true;
		break;
	default:
		_nodes[50] = 1;
		_vm->_sound = true;
		_vm->playMovie(126);
		_currMap = 10;
		_vm->_tsIconOnly = true;
		break;
	}
	_vm->playSound(29);
	_statPlay = 501;
}

void Logic::GPL3_501() {
	switch(_vm->_userInput) {
	case 0x428:
	case 0x429:
	case 0x42A:
		if (!_nodes[50]) {
			inventoryDel(_vm->_userInput - 0x428);
			_vm->saveAS();
			_vm->playMovie(178);
			_vm->restoreAS();
			_vm->_userInput = 0;
		}
		break;
	case 0x433:
		if (!_nodes[50]) {
			disableUIButtons();
			_vm->playMovie(125);
			_statPlay = 993;
			_vm->_loopFlag = true;
		} else
			_vm->displayIcon(141);

		break;
	case 0x439:
		if (!_nodes[50]) {
			_vm->playMovie(124);
			_nodes[50] = 1;
			inventoryAdd(11);
			_vm->_userInput = 0;
			_currMap = 10;
		}
		break;
	case 0x43A:
		_statPlay = 502;
		_vm->_zoom = 2;
		_vm->processMap(50, _vm->_zoom);
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

void Logic::GPL3_502() {
	_vm->processMapInput(50);
}

void Logic::GPL3_510() {
	_nodeNum = 51;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_vm->playMovie(129);
	_currMap = 10;
	_vm->playSound(6);
	_statPlay = 511;
}

void Logic::GPL3_511() {
	switch(_vm->_userInput) {
	case 0x434:
		if (_inventory[6] >= 1)
			_vm->displayIcon(142);
		else {
			_vm->playMovie(128);
			_nodes[51] = 9;
			inventoryAdd(12);
			_vm->_userInput = 0;
			_currMap = 10;
			_vm->playSound(30);
		}
		break;
	case 0x43A:
		_statPlay = 512;
		_vm->_zoom = 2;
		_vm->processMap(51, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x457:
		_tideCntl = false;
		_vm->drawPic(179);
		_vm->playMovie(127);
		_currMap = 103;
		_vm->_userInput = 0;
		_vm->playSound(0);
		_vm->playSound(6);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_512() {
	_vm->processMapInput(51);
}

void Logic::GPL3_520() {
	_nodeNum = 52;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[52] == 0)
		_nodes[52] = 1;
	else {
		_vm->_sound = true;
		_nodes[52] = false;
	}
	_vm->playMovie(130);
	_currMap = 111;
	_statPlay = 521;
	_vm->playSound(31);
}

void Logic::GPL3_521() {
	switch(_vm->_userInput) {
	case 0x404:
		_statPlay = 550;
		_vm->_loopFlag = true;
		break;
	case 0x43A:
		_statPlay = 522;
		_vm->_zoom = 2;
		_vm->processMap(52, _vm->_zoom);
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

void Logic::GPL3_522() {
	_vm->processMapInput(52);
}

void Logic::GPL3_530() {
	_nodeNum = 53;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_vm->_fstFwd = false;
	if (_inventory[7] <= 0 && _vm->_wizard)
		_vm->_tsIconOnly = false;
	else {
		_vm->_frameStop = 24;
		_vm->_tsIconOnly = true;
	}
	_eye = false;
	_vm->playMovie(70);
	_currMap = 99;
	_vm->playSound(8);
	_statPlay = 531;
}

void Logic::GPL3_531() {
	switch(_vm->_userInput) {
	case 0x428:
		if (_nodes[53] == 0) {
			_vm->playMovie(71);
			inventoryAdd(14);
			inventoryDel(0);
			_vm->_tsIconOnly = true;
			_vm->playSound(25);
		}
		_vm->_userInput = 0;
		break;
	case 0x429:
	case 0x42A:
		inventoryDel(_vm->_userInput - 0x428);
		_vm->saveAS();
		_vm->playMovie(178);
		_vm->restoreAS();
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_fstFwd = false;
		if (_inventory[7] > 0 && _vm->_pMovie == 70)
			_vm->_frameStop = 24;
		if (!_vm->_wizard && _vm->_pMovie == 70)
			_vm->_frameStop = 24;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_nodeNum = 56;
		_vm->_userInput = 0;
		_vm->_frameStop = 50;
		enableUIButtons();
		_vm->playMovie(135);
		_currMap = 106;
		_nodes[56] = 1;
		_vm->playSound(8);
		_vm->_tsIconOnly = true;
		_statPlay = 561;
		break;
	case 0x452:
		_statPlay = 630;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_540() {
	_nodeNum = 54;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (!_vm->_wizard) {
		_vm->_sound = true;
		_currMap = 55;
	} else {
		if (!_nodes[54])
			_nodes[54] = 1;
		else {
			_vm->_sound = true;
			_nodes[54] = 0;
		}
		_currMap = 112;
	}

	_vm->playMovie(132);
	_vm->showPic(432);
	_statPlay = 541;
	_vm->playSound(22);
}

void Logic::GPL3_541() {
	switch(_vm->_userInput) {
	case 0x404:
		_statPlay = 550;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	case 0x43A:
		_statPlay = 542;
		_vm->_zoom = 2;
		_vm->processMap(54, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		_vm->showPic(432);
		break;
	case 0x453:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(131);
		_vm->playSound(32);
		_vm->fShowPic(432);
		// Useless, but present in the original
		// _CurrMap = 10;
		_currMap = 55;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_542() {
	_vm->processMapInput(54);
}

void Logic::GPL3_550() {
	_nodeNum = 55;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_vm->playMovie(133);
	if (_nodes[55] != 9)
		_nodes[55] = 0;
	_vm->_tsIconOnly = true;
	_currMap = 104;
	_statPlay = 551;
	_vm->playSound(8);
}

void Logic::GPL3_551() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(134);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x42C:
		_vm->_aTimer = 0;
		if (_nodes[55] == 1) {
			enableUIButtons();
			_vm->playMovie(183);
			_vm->fShowPic(433);
			_vm->playSound(8);
			_nodes[55] = 9;
			_vm->_tsIconOnly = true;
		}
		break;
	case 0x43A:
		if (_nodes[55] == 1) {
			disableUIButtons();
			_vm->playMovie(134);
			_statPlay = 993;
			_vm->_loopFlag = true;
		} else {
			_statPlay = 552;
			_vm->_zoom = 2;
			_vm->processMap(55, _vm->_zoom);
			_vm->_userInput = 0;
		}
		break;
	case 0x43E:
		_vm->_aTimer = 0;
		_vm->playMovie(_vm->_pMovie);
		if (_vm->_pMovie == 183)
			_vm->fShowPic(433);
		break;
	case 0x452:
		_vm->_aTimer = 0;
		switch(_nodes[55]) {
		case 0:
			disableUIButtons();
			_vm->playMovie(69);
			_vm->_tsIconOnly = false;
			_nodes[55] = 1;
			_vm->setATimer();
			break;
		case 1:
			disableUIButtons();
			_vm->playMovie(134);
			_statPlay = 993;
			_vm->_loopFlag = true;
			break;
		case 9:
			_statPlay = 560;
			_vm->_loopFlag = true;
			break;
		default:
			break;
		}
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_552() {
	_vm->processMapInput(55);
	if (!_nodes[55] && _statPlay == 560) {
		_statPlay = 551;
		_vm->_userInput = 0x190;
		_vm->_loopFlag = true;
	}
}

void Logic::GPL3_560() {
	_nodeNum = 56;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (!_vm->_wizard) {
		_vm->_frameStop = 50;
		enableUIButtons();
		_vm->playMovie(135);
		_vm->showPic(435);
		_nodes[56] = 1;
		_vm->playSound(8);
		_statPlay = 561;
		_currMap = 106;
		_vm->_tsIconOnly = true;
	} else {
		disableUIButtons();
		_vm->playMovie(135);
		_currMap = 10;
		_vm->setATimer();
		_nodes[56] = 0;
		_statPlay = 561;
	}
}

void Logic::GPL3_561() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_vm->_aTimer = 0;
		disableUIButtons();
		_vm->playMovie(136);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x428:
	case 0x429:
	case 0x42A:
		_vm->_aTimer = 0;
		if (!_nodes[56]) {
			inventoryDel(_vm->_userInput - 0x428);
			_vm->saveAS();
			_vm->playMovie(178);
			_vm->restoreAS();
			_vm->_userInput = 753;
			_vm->_loopFlag = true;
		}
		break;
	case 0x42C:
		_vm->_aTimer = 0;
		if (!_nodes[56]) {
			_vm->_fstFwd = false;
			_replay = true;
			_vm->playMovie(138);
			enableUIButtons();
			_vm->playSound(8);
			_nodes[56] = 1;
			inventoryDel(4);
			_nodes[22] = 0;
			_currMap = 88;
			_vm->_tsIconOnly = true;
		}
		break;
	case 0x438:
		_vm->_aTimer = 0;
		if (!_nodes[56]) {
			enableUIButtons();
			_vm->_fstFwd = false;
			_replay = true;
			_vm->playMovie(137);
			enableUIButtons();
			_vm->playSound(8);
			_nodes[56] = 1;
			_currMap = 106;
			_vm->_tsIconOnly = true;
		}
		break;
	case 0x43A:
		_statPlay = 562;
		_vm->_zoom = 2;
		_vm->processMap(56, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_fstFwd = false;
		if (_vm->_pMovie == 135 && _nodes[56]) {
			_vm->_frameStop = 50;
			_vm->playMovie(135);
		} else if (_nodes[56])
			_vm->playMovie(_vm->_pMovie);
		else {
			_vm->_userInput = 753;
			_vm->_aTimer = 0;
			_vm->_loopFlag = true;
		}
		break;
	case 0x445:
		_statPlay = 530;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	case 0x446:
		_statPlay = 620;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	case 0x452:
		_statPlay = 550;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_562() {
	_vm->processMapInput(56);
}

void Logic::GPL3_570() {
	if (_nodes[57]) {
		if (_rtnNode != 34)
			_vm->_sound = true;
		_nodeNum = 57;
		_vm->drawLocation();
		_nodes[57] = 0;
		_vm->_userInput = 0;
		_vm->playMovie(139);
		_vm->playSound(27);
		_currMap = 10;
		_statPlay = 571;
	} else {
		_statPlay = 740;
		_vm->_loopFlag = true;
	}
}

void Logic::GPL3_571() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 572;
		_vm->_zoom = 2;
		_vm->processMap(57, _vm->_zoom);
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

void Logic::GPL3_572() {
	_vm->processMapInput(57);
}

void Logic::GPL3_580() {
	_nodeNum = 58;
	_vm->drawLocation();
	_vm->_iconsClosed = true;
	_vm->_userInput = 0;
	switch(_nodes[58]) {
	case 0:
		_vm->_sound = true;
		_vm->playMovie(140);
		_currMap = 113;
		_nodes[58] = 1;
		break;
	case 1:
		_vm->playMovie(140);
		_currMap = 113;
		_nodes[58] = 0;
		break;
	case 8:
		_vm->_tsIconOnly = true;
		_vm->playMovie(143);
		_currMap = 10;
		_nodes[58] = 9;
		break;
	case 9:
		_vm->_tsIconOnly = true;
		_vm->_sound = true;
		_vm->playMovie(143);
		_currMap = 10;
		break;
	default:
		break;
	}
	_vm->playSound(27);
	_statPlay = 581;
}

void Logic::GPL3_581() {
	switch(_vm->_userInput) {
	case 0x430:
		_vm->playMovie(141);
		_nodes[58] = 8;
		inventoryAdd(13);
		_vm->_tsIconOnly = true;
		_currMap = 10;
		_vm->playSound(30);
		// fallthrough
	case 0x428:
	case 0x429:
	case 0x42A:
		if (_nodes[58] < 8) {
			inventoryDel(_vm->_userInput - 0x428);
			_vm->saveAS();
			_vm->playMovie(178);
			_vm->restoreAS();
		}
		_vm->_userInput = 0;
		break;
	case 0x435:
		if (_nodes[58] >= 8)
			_vm->displayIcon(143);
		else {
			disableUIButtons();
			_vm->playMovie(142);
			_statPlay = 993;
			_vm->_loopFlag = true;
		}
		break;
	case 0x43A:
		_statPlay = 582;
		_vm->_zoom = 2;
		_vm->processMap(58, _vm->_zoom);
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

void Logic::GPL3_582() {
	_vm->processMapInput(58);
}

void Logic::GPL3_590() {
	_nodeNum = 59;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[59] == 0)
		_nodes[59] = 1;
	else {
		_vm->_sound = true;
		_nodes[59] = 0;
	}
	_vm->playMovie(144);
	_currMap = 114;
	_vm->playSound(21);
	_statPlay = 591;
}

void Logic::GPL3_591() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 592;
		_vm->_zoom = 2;
		_vm->processMap(59, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		_vm->showPic(444);
		break;
	case 0x445:
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_592() {
	_vm->processMapInput(59);
}

void Logic::GPL3_600() {
	_vm->setMouse();
	_vm->eraseCursor();
	_vm->fadeToBlack2();
	_vm->showPic(106);
	_vm->drawIcon(4, 0, 12 - _healthOld);
	_vm->_asMode = false;
	_nodeNum = 60;
	_vm->drawLocation();
	_vm->_userInput = 0;
	disableUIButtons();
	_vm->playMovie(145);
	_statPlay = 992;
	_vm->_loopFlag = true;
}

void Logic::GPL3_610() {
	_nodeNum = 61;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_nodes[72] = 0;
	_nodes[73] = 0;
	if (_nodes[61] == 0)
		_nodes[61] = 1;
	else
		_vm->_sound = true;

	_vm->playMovie(146);
	if (_nodes[69] == 9) {
		_vm->_itemInhibit = true;
		_vm->_tsIconOnly = true;
	}

	_currMap = 115;
	_vm->playSound(36);
	_statPlay = 611;
	_eye = false;
}

void Logic::GPL3_611() {
	switch(_vm->_userInput) {
	case 0x428:
	case 0x429:
	case 0x42A:
		inventoryDel(_vm->_userInput - 0x428);
		disableUIButtons();
		_vm->playMovie(148);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x432:
		_vm->playMovie(147);
		_inventory[5] = -1;
		_nodes[29] = 0;
		_currMap = 116;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x441:
		// Cascade on purpose
	case 0x448:
		_currMap = 10;
		_vm->_itemInhibit = true;
		_statPlay = 730;
		_vm->_loopFlag = true;
		break;
	case 0x44B:
		_currMap = 10;
		_statPlay = 680;
		_vm->_loopFlag = true;
		break;
	case 0x451:
		_currMap = 10;
		_statPlay = 690;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_620() {
	_nodeNum = 62;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_eye = false;
	if (_nodes[62] == 0) {
		_vm->playMovie(149);
		_currMap = 122;
	} else {
		_vm->_frameStop = 24;
		_vm->playMovie(202);
		_currMap = 121;
		_nodes[62] = 9;
	}
	_vm->_tsIconOnly = true;
	_vm->playSound(8);
	_statPlay = 621;
}

void Logic::GPL3_621() {
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
	case 0x430:
		_vm->playMovie(150);
		inventoryAdd(8);
		_nodes[62] = 1;
		_vm->playSound(25);
		_currMap = 121;
		_vm->_userInput = 0;
		break;
	case 0x43E:
		switch (_nodes[62]) {
		case 0:
			_vm->playMovie(149);
			break;
		case 1:
			_vm->playMovie(150);
			break;
		case 9:
			_vm->_frameStop = 24;
			_vm->playMovie(202);
			_currMap = 121;
			break;
		default:
			break;
		}
		break;
	case 0x445:
		_nodeNum = 56;
		_vm->_userInput = 0;
		_vm->_frameStop = 50;
		enableUIButtons();
		_vm->playMovie(135);
		_nodes[56] = 1;
		_vm->playSound(8);
		_statPlay = 561;
		_currMap = 106;
		if (_inventory[16] < 1 && _vm->_wizard)
			_currMap = 88;
		break;
	case 0x452:
		_statPlay = 630;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_630() {
	_nodeNum = 63;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_eye = false;
	if (_nodes[63] == 1 || !_vm->_wizard) {
		_nodes[63] = 0;
		_vm->_sound = true;
	} else
		_nodes[63] = true;

	_vm->playMovie(151);
	_currMap = 72;
	_vm->playSound(40);
	_statPlay = 631;
}

void Logic::GPL3_631() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_vm->playMovie(10);
		_statPlay = 50;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_640() {
	_nodeNum = 64;
	_vm->drawLocation();
	_vm->_userInput = 0;
	if (_nodes[64] || _vm->_wizard) {
		_vm->_sound = true;
		_nodes[64] = 0;
	} else
		_nodes[64] = 1;

	_vm->playMovie(152);
	_currMap = 10;
	_statPlay = 641;
	_vm->playSound(28);
}

void Logic::GPL3_641() {
	switch(_vm->_userInput) {
	case 0x43A:
		_statPlay = 642;
		_vm->_zoom = 2;
		_vm->processMap(64, _vm->_zoom);
		_vm->_userInput = 0;
		break;
	case 0x43E:
		_vm->_sound= _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_642() {
	_vm->processMapInput(64);
}

void Logic::GPL3_650() {
	_nodeNum = 65;
	_eye = false;
	_vm->_userInput = 0;
	if (_nodes[65] == 0)
		_nodes[65] = 1;
	else {
		_vm->_sound = true;
		_nodes[65] = 0;
	}
	_vm->playMovie(153);
	_vm->showPic(453);
	_currMap = 73;
	_vm->playSound(37);
	_statPlay = 651;
}

void Logic::GPL3_651() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		_vm->showPic(453);
		break;
	case 0x445:
		_statPlay = 700;
		_vm->_loopFlag = true;
		break;
	case 0x446:
		if (_nodes[67] == 1) {
			_statPlay = 660;
			_vm->_loopFlag = true;
		} else {
			_statPlay = 790;
			_vm->_loopFlag = true;
		}
		break;
	case 0x447:
		_statPlay = 710;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_660() {
	_nodeNum = 66;
	_eye = false;
	_vm->_userInput = 0;
	if (_nodes[66] == 0)
		_nodes[66] = 1;
	else {
		_vm->_sound = true;
		_nodes[66] = 0;
	}
	_vm->playMovie(154);
	_currMap = 70;
	_vm->playSound(40);
	_statPlay = 661;
}

void Logic::GPL3_661() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_statPlay = 700;
		_vm->_loopFlag = true;
		break;
	case 0x446:
		_vm->_loopFlag = true;
		_statPlay = (_nodes[67] == 1) ? 650 : 790;
		break;
	case 0x447:
		if (!_vm->_wizard || _nodes[67])
			_statPlay = 650;
		else
			_statPlay = 670;

		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_670() {
	_nodeNum = 67;
	_vm->_userInput = 0;
	_eye = false;
	_nodes[67] = 1;
	_vm->_fstFwd = false;
	_vm->playMovie(155);
	_currMap = 127;
	_statPlay = 671;
}

void Logic::GPL3_671() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->_fstFwd = false;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_statPlay = 650;
		_vm->_loopFlag = true;
		break;
	case 0x446:
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_680() {
	_nodeNum = 68;
	_vm->_userInput = 0;
	disableUIButtons();
	_vm->playMovie(156);
	_statPlay = 993;
	_vm->_loopFlag = true;
}

void Logic::GPL3_690() {
	_vm->_itemInhibit = false;
	enableUIButtons();
	if (_nodes[69] == 9) {
		_statPlay = 640;
		_nodes[72] = 0;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
	} else {
		_nodeNum = 69;
		_vm->drawLocation();
		_vm->_userInput = 0;
		_currMap = 119;
		_vm->_frameStop = 40;
		_vm->playMovie(157);
		_vm->playSound(12);
		_statPlay = 691;
	}
}

void Logic::GPL3_691() {
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
	case 0x439:
		if (_nodes[69] == 9)
			_vm->displayIcon(147);
		else {
			_vm->_frameStop = 108;
			_vm->playMovie(158);
			_vm->_itemInhibit = false;
			inventoryAdd(17);
			_nodes[69] = 9;
			_vm->playSound(23);
			_vm->_userInput = 0;
			_currMap = 120;
		}
		break;
	case 0x43E:
		if (_vm->_pMovie == 157)
			_vm->_frameStop = 40;
		else if (_vm->_pMovie == 158)
			_vm->_frameStop = 108;

		if (_vm->_pMovie == 178) {
			_vm->saveAS();
			_vm->playMovie(178);
			_vm->restoreAS();
		} else
			_vm->playMovie(_vm->_pMovie);

		break;
	case 0x441:
		_vm->_itemInhibit = false;
		_statPlay = 640;
		_nodes[72] = 0;
		_vm->_loopFlag = true;
		_vm->_userInput = 0;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_692() {
	_vm->processMapInput(69);
}

void Logic::GPL3_700() {
	_nodeNum = 70;
	_vm->_userInput = 0;
	_eye = false;
	if (_nodes[70] && _vm->_wizard) {
		_nodes[70] = 0;
		_vm->_sound = false;
	} else
		_nodes[70] = 1;

	_vm->playMovie(164);
	_currMap = 74;
	_vm->playSound(10);
	_statPlay = 701;
}

void Logic::GPL3_701() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_statPlay = 650;
		_vm->_loopFlag = true;
		break;
	case 0x446:
		_statPlay = 780;
		_vm->_loopFlag = true;
		break;
	case 0x447:
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_710() {
	_nodeNum = 71;
	_vm->_userInput = 0;
	_eye = false;
	if (_inventory[8] && _inventory[14])
		_vm->_sound = true;
	disableUIButtons();
	_vm->playMovie(165);
	_currMap = 10;
	_vm->setATimer();
	_statPlay = _vm->_wizard ? 712 : 711;
}

void Logic::GPL3_711() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_replay = false;
		_vm->playMovie(166);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x428:
	case 0x429:
	case 0x42A:
		inventoryDel(_vm->_userInput - 0x428);
		_replay = false;
		_vm->_aTimer = 0;
		_vm->playMovie(178);
		_vm->playMovie(166);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x430:
		_vm->_aTimer = 0;
		_nodeNum = 0;
		_vm->_iconsClosed = true;
		_vm->_fstFwd = false;
		_vm->playMovie(167);
		_vm->_fstFwd = false;
		_vm->playMovie(170);
		_vm->playSound(25);
		endCredits();
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_712() {
	switch(_vm->_userInput) {
	case 0x2F1:
		_replay = false;
		_vm->playMovie(166);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x428:
	case 0x429:
	case 0x42A:
		inventoryDel(_vm->_userInput - 0x428);
		_replay = false;
		_vm->_aTimer = 0;
		_vm->playMovie(178);
		_vm->playMovie(166);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x430:
		_vm->_aTimer = 0;
		if (_nodes[71] != 2) {
			_vm->playMovie(57);
			_statPlay = 993;
		} else {
			_nodeNum = 0;
			_vm->_iconsClosed = true;
			_vm->_fstFwd = false;
			_vm->playMovie(55);
			_vm->_fstFwd = false;
			_vm->playMovie(184);
			_vm->playSound(25);
			endCredits();
		}
		_vm->_loopFlag = true;
		break;
	case 0x432:
		_vm->_aTimer = 0;
		_replay = false;
		_vm->playMovie(168);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x433:
		if ((_inventory[8] > 0 && _inventory[14] > 0) || _nodes[71] == 2) {
			_replay = false;
			_vm->playMovie(166);
			_statPlay = 993;
		} else {
			_vm->_aTimer = 0;
			enableUIButtons();
			_vm->playMovie(169);
			_statPlay = 580;
			inventoryDel(11);
			_nodes[50] = 0;
		}
		_vm->_loopFlag = true;
		break;
	case 0x436:
		_vm->_aTimer = 0;
		_vm->playSound(0);
		_vm->playMovie(193);
		_vm->showPic(128);
		inventoryDel(14);
		_nodes[71] = 2;
		_vm->setATimer();
		break;

	case 0x437:
		_vm->_aTimer = 0;
		if (_nodes[71] == 2) {
			_nodeNum = 0;
			_vm->_iconsClosed = true;
			_vm->_fstFwd = false;
			_vm->playMovie(56);
			_vm->_fstFwd = false;
			_vm->playMovie(184);
			_vm->playSound(25);
			endCredits();
			_vm->_quit = true;
			//--				_vm->_quitFlag = true;
		} else {
			_vm->playMovie(58);
			_statPlay = 993;
		}
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_720() {
	_nodeNum = 72;
	_vm->_userInput = 0;
	_eye = false;
	switch (_nodes[72]) {
	case 0:
		_nodes[72] = 1;
		break;
	case 1:
		_vm->_frameStop = 24;
		_nodes[72] = 2;
		break;
	default:
		_vm->_frameStop = 24;
		_nodes[72] = 3;
		break;
	}
	_vm->playMovie(175);
	_currMap = 118;
	_vm->playSound(12);
	_statPlay = 721;
}

void Logic::GPL3_721() {
	switch(_vm->_userInput) {
	case 0x43E:
		if (_nodes[72] > 1)
			_vm->_frameStop = 24;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x441:
		if (_nodes[72] == 1) {
			_vm->_itemInhibit = false;
			_statPlay = 640;
		} else
			_statPlay = 680;

		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x448:
		_statPlay = (_nodes[72] == 2) ? 730 : 680;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x44B:
		_statPlay = (_nodes[72] == 1) ? 730 : 680;
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x44C:
		if (_nodes[72] == 1) {
			_statPlay = 730;
			_nodes[73] = 3;
		} else
			_statPlay = 680;

		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_730() {
	_nodeNum = 73;
	_vm->_userInput = 0;
	_eye = false;
	switch(_nodes[73]) {
	case 0:
		_nodes[73] = 1;
		break;
	case 1:
		_vm->_sound = true;
		_nodes[73] = 2;
		break;
	case 2:
		_vm->_frameStop = 23;
		_nodes[73] = 3;
		break;
	default:
		_vm->_frameStop = 23;
		_nodes[73] = 4;
		break;
	}
	_vm->playMovie(176);
	_currMap = 117;
	_vm->playSound(36);
	_statPlay = 731;
}

void Logic::GPL3_731() {
	switch(_vm->_userInput) {
	case 0x43E:
		if (_nodes[73] > 2)
			_vm->_frameStop = 23;
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x441:
		switch (_nodes[73]) {
		case 1:
		case 2:
			_statPlay = 720;
			break;
		case 3:
			_statPlay = 720;
			_nodes[72] = 1;
			break;
		default:
			_statPlay = 680;
			break;
		}
		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x448:
		switch(_nodes[73]) {
		case 3:
			_statPlay = 690;
			break;
		case 4:
			_statPlay = 720;
			_nodes[72] = 1;
			break;
		default:
			_statPlay = 680;
			break;
		}
		break;
	case 0x44B:
		switch(_nodes[73]) {
		case 1:
			_statPlay = 720;
			break;
		case 3:
		case 4:
			_statPlay = 720;
			_nodes[72] = 1;
			break;
		default:
			_statPlay= 680;
			break;
		}

		_currMap = 10;
		_vm->_loopFlag = true;
		break;
	case 0x44C:
		switch(_nodes[73]) {
		case 1:
		case 2:
			_statPlay = 720;
			break;
		case 4:
			_statPlay = 720;
			_nodes[72] = 1;
			break;
		default:
			_statPlay = 680;
			break;
		}
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_740() {
	_rtnNode = _nodeNum;
	_nodeNum = 74;
	_vm->drawLocation();
	disableUIButtons();
	_vm->playMovie(185);
	_currMap = 10;
	_vm->setATimer();
	_statPlay = 741;
	_vm->_userInput = 0;
}

void Logic::GPL3_741() {
	switch(_vm->_userInput) {
	case 0x2F1:
		disableUIButtons();
		_vm->playMovie(188);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x428:
	case 0x429:
	case 0x42A:
		inventoryDel(_vm->_userInput - 0x428);
		_vm->_aTimer = 0;
		_vm->playMovie(178);
		_vm->_loopFlag = true;
		_vm->_userInput = 753;
		break;
	case 0x432:
		if (_nodes[74]) {
			_vm->_sound = false;
			_nodes[74] = false;
		} else
			_nodes[74] = true;
		break;
	case 0x437:
		_vm->_loopFlag = true;
		if (wound()) {
			_vm->_frameStop = 88;
			_vm->playMovie(187);
			_statPlay = 570;
			_nodes[57] = 1;
		} else {
			_vm->_frameStop = 78;
			_vm->playMovie(187);
			_statPlay = 993;
		}
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_760() {
	_nodeNum = 76;
	_eye = false;
	_vm->_userInput = 0;
	_vm->playMovie(189);
	_currMap = 126;
	_vm->playSound(21);
	_statPlay = 761;
}

void Logic::GPL3_761() {
	switch(_vm->_userInput) {
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		_vm->playMovie(_vm->_pMovie);
		break;
	case 0x445:
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	case 0x44D:
		_vm->playMovie(191);
		_nodes[67] = 2;
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	case 0x44E:
		_vm->playMovie(192);
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	case 0x458:
		_vm->playMovie(190);
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_780() {
	_nodeNum = 78;
	_vm->drawLocation();
	_vm->_userInput = 0;
	_eye = false;
	_vm->playMovie(171);
	_currMap = 124;
	_vm->playSound(40);
	_statPlay = 781;
}

void Logic::GPL3_781() {
	switch(_vm->_userInput) {
	case 0x428:
	case 0x42A:
		inventoryDel(_vm->_userInput - 0x428);
		_vm->_frameStop = 64;
		_vm->playMovie(173);
		_vm->showPic(471);
		_currMap = 124;
		break;
	case 0x429:
		if (_vm->_wizard) {
			_vm->_frameStop = 64;
			_vm->playMovie(173);
		} else {
			_vm->_frameStop = 66;
			_vm->_fstFwd = false;
			_vm->playMovie(172);
			_vm->_fstFwd = true;
			_vm->playMovie(215);
		}

		inventoryDel(1);
		_vm->fShowPic(471);
		_currMap = 124;
		break;
	case 0x439:
		_vm->_frameStop = 59;
		_vm->_fstFwd = false;
		_vm->playMovie(174);
		_vm->_fstFwd = true;
		_vm->playMovie(201);
		_vm->fShowPic(471);
		_currMap = 124;
		break;
	case 0x43E:
		_vm->_sound = _vm->_lastSound;
		if (_vm->_pMovie == 173)
			_vm->_frameStop = 64;
		if (_vm->_pMovie == 201) {
			_vm->_frameStop = 59;
			_vm->_fstFwd = false;
			_vm->playMovie(174);
			_vm->_fstFwd = true;
			_vm->playMovie(201);
		} else {
			_vm->playMovie(_vm->_pMovie);
		}
		if (_vm->_pMovie != 171) {
			_vm->fShowPic(471);
			_currMap = 124;
		}
		break;
	case 0x445:
		_statPlay = 660;
		_vm->_loopFlag = true;
		break;
	case 0x446:
		_statPlay = (_nodes[67] == 1) ? 760 : 660;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

void Logic::GPL3_790() {
	_nodeNum = 79;
	_vm->drawLocation();
	disableUIButtons();
	_vm->playMovie(4);
	_currMap = 8;
	_statPlay = 791;
	_vm->setATimer();
	_vm->_userInput = 0;
}

void Logic::GPL3_791() {
	switch(_vm->_userInput) {
	case 0x2F1:
		disableUIButtons();
		_vm->playMovie(204);
		_statPlay = 993;
		_vm->_loopFlag = true;
		break;
	case 0x445:
		enableUIButtons();
		_vm->playMovie(10);
		_statPlay = 50;
		_vm->_loopFlag = true;
		break;
	case 0x446:
		enableUIButtons();
		_vm->playMovie(10);
		_statPlay = 520;
		_vm->_loopFlag = true;
		break;
	default:
		if (_vm->_userInput)
			warning("Skipped UserInput %d(0x%04X) for _StatPlay %d", _vm->_userInput, _vm->_userInput, _statPlay);
		break;
	}
}

} // NameSpace
