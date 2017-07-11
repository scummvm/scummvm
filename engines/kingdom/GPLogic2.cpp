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

void Logic::GPLogic2() {
	switch(_statPlay) {
	case 250: {
		bool highNode = (_nodeNum > 25);
		_nodeNum = 25;

		_vm->drawLocation();
		dsAll();
		if (_nodes[24] != 0 || highNode) {
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
			dsAll();
			_vm->playMovie(73);
			dsAll();
			_statPlay = 991;
			_vm->_loopFlag = true;
		}
		}
		break;
	case 251:
		switch(_vm->_userInput) {
		case 0x2F1:
			_vm->_aTimer = 0;
			_vm->playMovie(73);
			dsAll();
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
		}
		break;
	case 252:
		_vm->processMapInput(30);
		break;
	case 260:
		_nodeNum = 26;
		_vm->drawLocation();
		dsAll();
		_vm->playMovie(74);
		_currMap = 10;
		_vm->setATimer();
		_statPlay = 261;
		_vm->_userInput = 0;
		break;
	case 261:
		switch(_vm->_userInput) {
		case 0x2F1:
			_vm->_aTimer = 0;
			dsAll();
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
			enAll();
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
				enAll();
				_vm->playMovie(77);
				_vm->playSound(34);
				inventoryDel(5);
				_statPlay = 300;
				_vm->_loopFlag = true;
			} else {
				dsAll();
				_vm->playMovie(78);
				_statPlay = 992;
				_vm->_loopFlag = true;
			}
			break;
		}
		break;
	case 270:
		_nodeNum = 27;
		_vm->drawLocation();
		if (_nodes[28] == 1)
			_nodes[28] = 9;
		if (_nodes[28] == 9 || _nodes[29] != 0) {
			if (_vm->_wizard)
				_vm->_sound = true;
		} else {
			_vm->_sound = false;
			_nodes[28] = 1;
		}
		_vm->playMovie(80);
		_vm->playSound(18);
		_currMap = 10;
		_statPlay = 271;
		_nodes[28] = 1;
		_vm->_userInput = 0;
		break;
	case 271:
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
		}
		break;
	case 272:
		_vm->processMapInput(27);
		break;
	case 280:
		_nodeNum = 28;
		_vm->drawLocation();
		_vm->_userInput = 0;
		dsAll();
		_vm->_fstFwd = false;
		_vm->playMovie(81);
		_currMap = 10;
		_nodes[29] = _nodes[39] = 1;
		_statPlay = _robberyNode * 10;
		enAll();
		_vm->_loopFlag = true;
		break;
	case 290:
		_oldNode = _nodeNum;
		if (!_vm->_wizard) {
			_nodes[26] = 9;
			_nodes[30] = 8;
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
				switch (_nodes[30]) {
				case 0:
					_vm->_sound = false;
					_vm->playMovie(82);
					_currMap = 89;
					_nodes[30] = 1;
					break;
				case 1:
					_vm->_sound = true;
					_vm->playMovie(82);
					_currMap = 89;
					_nodes[30] = 0;
					break;
				case 8:
					_vm->_sound = false;
					_vm->playMovie(85);
					_currMap = 10;
					_nodes[30] = 9;
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
			
		break;
	case 291:
		switch(_vm->_userInput) {
		case 0x431:
			_vm->_tsIconOnly = true;
			_vm->playMovie(83);
			_vm->playSound(30);
			inventoryAdd(10);
			_nodes[30] = 8;
			_vm->_userInput = 0;
			_currMap = 10;
			break;
		case 0x432:
			if (_nodes[30] >= 8)
				_vm->displayIcon(140);
			else {
				dsAll();
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
		}
		break;
	case 292:
		_vm->processMapInput(29);
		break;
	case 300:
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
		break;
	case 301:
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
		} 
		break;
	case 302:
		_vm->processMapInput(30);
		break;
	case 310:
		_oldNode = _nodeNum;
		_nodeNum = 31;
		if (!chkDesertObstacles()) {
			_vm->drawLocation();
			_vm->playMovie(67);
			_vm->playSound(18);
			_currMap = 90;
			_statPlay = 311;
			_nodes[32] = 1;
			_vm->_aTimer = 0;
			_vm->_userInput = 0;
		}
		break;
	case 311:
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
			dsAll();
			_vm->playMovie(68);
			_statPlay = 992;
			_vm->_loopFlag = true;
			break;			
		}
		break;
	case 312:
		_vm->processMapInput(31);
		break;
	case 320:
		_oldNode = _nodeNum;
		_nodeNum = 32;
		_vm->drawLocation();
		_vm->_userInput = 0;
		increaseHealth();
		if (_nodes[33] != 9 && _vm->_wizard) {
			_vm->_sound = false;
			_nodes[33] = 9;
		} else {
			_vm->_sound = true;
			_nodes[33] = 1;
		}
		_vm->playMovie(87);
		_vm->playSound(18);
		_currMap = 10;
		_statPlay = 321;
		break;
	case 321:
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
		}
		break;
	case 322:
		_vm->processMapInput(32);
		break;
	case 330:
		_oldNode = _nodeNum;
		_nodeNum = 33;
		if (!chkDesertObstacles()) {
			_vm->drawLocation();
			_vm->_userInput = 0;
			if (_nodes[32] == 0)
				_vm->_sound = false;
			else
				_vm->_sound = true;
			_vm->playMovie(88);
			_vm->playSound(18);
			_currMap = 10;
			_statPlay = 331;
		}
		break;
	case 331:
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
		}
		break;
	case 332:
		_vm->processMapInput(33);
		break;
	case 340:
		if (_nodes[36] == 0) {
			_statPlay = 350;
			_vm->_loopFlag = true;
			_nextNode = 34;
		} else {
			_oldNode = _nodeNum;
			_nodeNum = 34;
			if (!chkDesertObstacles()) {
				_vm->drawLocation();
				_vm->_userInput = 0;
				if (_nodes[35] == 9 || _nodes[41] == 0 || !_vm->_wizard) {
					_vm->playMovie(91);
					_currMap = 92;
				} else {
					_vm->playMovie(89);
					_vm->showPic(389);
					_currMap = 91;
				}
				if (_nodes[35] == 9)
					_vm->_tsIconOnly = true;
				_vm->playSound(26);
				_statPlay = 341;
			}
		}
		break;
	case 341:
		switch(_vm->_userInput) {
		case 0x431:
			if (_nodes[35] == 9)
				_vm->displayIcon(139);
			else {
				_vm->playMovie(90);
				inventoryAdd(9);
				_nodes[35] = 9;
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
		}
		break;
	case 342:
		_vm->processMapInput(34);
		break;
	case 350:
		if (!_vm->_wizard)
			_nodes[36] = 1;
		if (_nodes[36] == 1) {
			_statPlay = _nextNode * 10;
			_vm->_loopFlag = true;
		} else {
			_nodeNum = 35;
			_vm->drawLocation();
			dsAll();
			_vm->playMovie(92);
			_currMap = 94;
			_vm->setATimer();
			_vm->_userInput = 0;
			_statPlay = 351;
		}
		break;
	case 351:
		switch(_vm->_userInput) {
		case 0x428:
		case 0x429:
			inventoryDel(_vm->_userInput - 0x428);
			_vm->_aTimer = 0;
			_vm->playMovie(178);
			_vm->_userInput = 0x2F1;
			_vm->_loopFlag = true;
		// No break on purpose
		case 0x2F1:
			_vm->_aTimer = 0;
			dsAll();
			_vm->playMovie(93);
			_statPlay = 992;
			_vm->_loopFlag = true;
			break;
		case 0x42A:
			_vm->_aTimer = 0;
			enAll();
			_vm->playMovie(179);
			inventoryDel(2);
			_nodes[36] = 1;
			_vm->_userInput = 0;
			_statPlay = _nextNode * 10;
			_lastObs = true;
			_lastObstacle = _nextNode;
			_vm->_loopFlag = true;
			break;
		}
		break;
	case 360:
		_nodeNum = 36;
		_vm->drawLocation();
		if (_nodes[37] == 0)
			_vm->_sound = false;
		else
			_vm->_sound = true;
		_vm->_frameStop = 185;
		dsAll();
		_vm->playMovie(94);
		_currMap = 95;
		_vm->setATimer();
		_vm->_userInput = 0;
		_statPlay = 361;
		break;
	case 361:
		switch(_vm->_userInput) {
		case 0x2F1:
			_vm->_aTimer = 0;
			dsAll();
			_vm->playMovie(95);
			_statPlay = 992;
			_vm->_loopFlag = true;
			break;
		case 0x428:
			_vm->_aTimer = 0;
			enAll();
			_vm->playMovie(97);
			inventoryDel(0);
			_vm->playSound(5);
			_nodes[37] = 1;
			_vm->_userInput = 0;
			_currMap = 10;
			if (_nodes[43] == 0) {
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
			enAll();
			_vm->playMovie(98);
			inventoryDel(2);
			_vm->playSound(5);
			_nodes[37] = 1;
			_vm->_userInput = 0;
			_currMap = 10;
			_vm->_tsIconOnly = true;
			break;
		case 0x42E:
			_vm->_aTimer = 0;
			enAll();
			_vm->playMovie(100);
			inventoryDel(6);
			_vm->playSound(5);
			_nodes[37] = 1;
			_vm->_userInput = 0;
			_currMap = 10;
			_vm->_tsIconOnly = true;
			break;
		case 0x431:
			_vm->_aTimer = 0;
			enAll();
			_vm->playMovie(96);
			_vm->playSound(5);
			_nodes[37] = 9;
			_vm->_userInput = 0;
			_currMap = 10;
			_statPlay = 420;
			_oldNode = _nodeNum;
			_nodeNum = 42;
			GPLogic2_SubSP361();
			break;
		case 0x437:
			_vm->_aTimer = 0;
			dsAll();
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
		}
		break;
	case 362:
		_vm->processMapInput(36);
		break;
	case 370:
		_nodeNum = 37;
		_vm->drawLocation();
		_vm->_userInput = 0;
		_nodes[38] = 0;
		_vm->playMovie(102);
		_vm->playSound(20);
		_currMap = 109;
		_statPlay = 371;
		break;
	case 371:
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
		}
		break;
	case 372:
		_vm->processMapInput(37);
		break;
	case 380:
		_oldNode = _nodeNum;
		_nodeNum =38;
		if (!chkDesertObstacles()) {
			_vm->drawLocation();
			_vm->_userInput = 0;
			_vm->playMovie(159);
			_currMap = 10;
			_vm->playSound(7);
			_nodes[39] = 1;
			_statPlay = 381;
		}
		break;
	case 381:
		switch(_vm->_userInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			if (_vm->_wizard && _nodes[39] != 9) {
				inventoryDel(_vm->_userInput - 0x428);
				_vm->saveAS();
				_vm->playMovie(178);
				_vm->restoreAS();
			}
			_vm->_userInput = 0;
			break;
		case 0x430:
			if (_vm->_wizard && _nodes[39] != 9) {
				dsAll();
				_vm->playMovie(162);
				_statPlay = 992;
				_vm->_loopFlag = true;
			}
			break;
		case 0x433:
			if (_vm->_wizard && _nodes[39] != 9) {
				_vm->playMovie(161);
				inventoryDel(11);
				_nodes[51] = 0;
				_vm->_userInput = 0;
			}
			_vm->_userInput = 0;
			break;
		case 0x435:
			if (_vm->_wizard && _nodes[39] != 9) {
				_vm->playMovie(160);
				inventoryDel(13);
				_nodes[39] = 9;
				_nodes[59] = 0;
				_vm->_userInput = 0;
				_vm->_tsIconOnly = true;
				_currMap = 108;
			}
			break;
		case 0x437:
			if (_vm->_wizard && _nodes[39] != 9) {
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
		}
		break;
	case 382:
		if (_vm->_wizard && (_nodes[39] != 9) && (_vm->_userInput == 0x413))
			_vm->_userInput = 0;

		_vm->processMapInput(38);
		break;
	case 390:
		_nodeNum = 39;
		_vm->drawLocation();
		_vm->_aTimer = 0;
		dsAll();
		_vm->playMovie(104);
		_vm->playSound(34);
		_statPlay = 992;
		_vm->_loopFlag = true;
		break;
	case 400:
		_nodeNum = 40;
		if (!_vm->_wizard)
			_nodes[41] = 2;
		if (_nodes[41]) {
			if (_nodes[41] == 1)
				_nodes[41] = 2;
			else {
				_nodes[41] = 1;
				_vm->_sound = true;
			}
		} else {
			dsAll();
			_vm->playMovie(105);
			_currMap = 10;
			_vm->setATimer();
		}
		
		_vm->_userInput = 0;
		_statPlay = 401;
		break;
	case 401:
		switch(_vm->_userInput) {
		case 0x2F1:
			_vm->_aTimer = 0;
			dsAll();
			_vm->playMovie(106);
			_statPlay = 992;
			_vm->_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			if (!_nodes[41]) {
				inventoryDel(_vm->_userInput - 0x428);
				_vm->_aTimer = 0;
				_vm->playMovie(178);
				_vm->_userInput = 753;
				_vm->_loopFlag = true;
				_currMap = 10;
			}
			break;
		case 0x42C:
			if (!_nodes[41]) {
				_vm->_aTimer = 0;
				dsAll();
				_vm->playMovie(108);
				_vm->_userInput = 0;
				_statPlay = 992;
				_vm->_loopFlag = true;
			}
			break;
		case 0x42E:
			if (!_nodes[41]) {
				_vm->_aTimer = 0;
				_vm->playMovie(109);
				inventoryDel(6);
				_nodes[43] = 0;
				_vm->_userInput = 0;
				_vm->_aTimer = 136;
			}
			break;
		case 0x437:
			if (!_nodes[41]) {
				_vm->_aTimer = 0;
				_help = true;
				_replay = true;
				_pouch = true;
				_vm->_fstFwd = true;
				_vm->playMovie(107);
				_vm->playSound(26);
				_nodes[41] = 1;
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
		}
		break;
	case 402:
		_vm->_aTimer = 0;
		_vm->processMapInput(40);
		break;
	case 410:
		_nodeNum = 41;
		_vm->drawLocation();
		_vm->_userInput = 0;
		if (_nodes[41])
			_vm->_sound = true;

		_eye = false;
		_vm->playMovie(111);
		_vm->playSound(26);
		_currMap = 97;
		_statPlay = 411;
		break;
	case 411:
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
		}
		break;
	case 420:
		if (_nodes[37] == 9 || _nodes[29] == 1 || !_vm->_wizard) {
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
		break;
	case 421:
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
			if (_nodes[43])
				_vm->displayIcon(136);
			else {
				if (_nodes[35] == 9) {
					_vm->_sound = false;
				} else {
					_vm->_sound = true;
				}
				_vm->playMovie(113);
				inventoryAdd(6);
				_nodes[43] = 9;
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
		
		}
		break;
	case 422:
		_vm->processMapInput(42);
		break;
	case 430:
		_nodeNum = 43;
		_vm->drawLocation();
		_eye = false;
		_vm->_userInput = 0;
		if (_nodes[44] == 9) {
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
			_nodes[44] = 9;
			_currMap = 79;
			_vm->_tsIconOnly = true;
		}
		_statPlay = 431;
		_vm->playSound(20);
		_eye = false;
		break;
	case 431:
		switch(_vm->_userInput) {
			case 0x40A:
				_vm->playMovie(10);
				_statPlay = 200;
				if (_nodes[44] != 9)
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
				if (_nodes[44] != 9) {
					inventoryDel(_vm->_userInput - 0x428);
					_vm->saveAS();
					_vm->playMovie(178);
					_vm->restoreAS();
				}
				break;
			case 0x438:
				if (_nodes[44] == 9) {
					_vm->displayIcon(146);
				} else {
					dsAll();
					_vm->playMovie(117);
					_statPlay = 992;
					_vm->_loopFlag = true;
				}
				break;
			case 0x43E:
				break;
		}
		break;
	case 440:
		_nodeNum = 44;
		_vm->drawLocation();
		_vm->_userInput = 0;
		if (_inventory[12] > 0 || _nodes[44] == 9)
			_vm->_sound = true;

		_vm->playMovie(119);
		_vm->playSound(20);
		_currMap = 100;
		_statPlay = 441;
		break;
	case 441:
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
		}
		break;
	case 442:
		_vm->processMapInput(44);
		break;
	case 450:
		_nodeNum = 45;
		_vm->_userInput = 0;
		if (!_nodes[41]) {
			dsAll();
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
		break;
	case 451:
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
		}
		break;
	case 460:
		_eye = false;
		_nodeNum = 46;
		_vm->drawLocation();
		_vm->_userInput = 0;
		if (_nodes[47] && _vm->_wizard)
			_nodes[47] = 1;
		else {
			_nodes[47] = 0;
			_vm->_sound = true;
		}
		_vm->playMovie(121);
		_vm->playSound(26);
		_currMap = 107;
		_statPlay = 461;
		break;
	case 461:
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
		}
		break;
	case 462:
		_vm->processMapInput(46);
		break;
	case 480:
		_nodeNum = 48;
		_vm->_userInput = 0;
		_vm->playMovie(114);
		_currMap = 10;
		_statPlay = 481;
		_nodes[49] = 1;
		break;
	case 481:
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
		}
		break;
	case 482:
		_vm->processMapInput(48);
		break;
	case 490:
		_nodeNum = 49;
		_vm->drawLocation();
		dsAll();
		_vm->playMovie(180);
		_vm->playSound(18);
		_currMap = 101;
		_statPlay = 491;
		_vm->_aTimer = 0;
		_vm->_userInput = 0;
		break;
	case 491:
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
			enAll();
			_vm->playMovie(181);
			_nodes[29] = 9;
			_nodes[49] = 0;
			_statPlay = _nextNode * 10;
			_vm->_loopFlag = true;
			break;
		}
		break;
	}
}

void Logic::GPLogic2_SubSP361() {
	enAll();
	_vm->_userInput = 0;
	_vm->playSound(19);
	if (_nodes[29] == 1) {
		_statPlay = 480;
		_vm->_loopFlag = true;
		_currMap = 10;
	} else {
		if (_inventory[6] == 1 || !_vm->_wizard) {
			if (_nodes[43] != 1 || _vm->_wizard)
				_nodes[43] = 1;
			else {
				_vm->_sound = true;
				_nodes[43] = 2;
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
