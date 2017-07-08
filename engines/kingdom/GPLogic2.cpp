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

namespace Kingdom {

void KingdomGame::GPLogic2() {
	switch(_statPlay) {
	case 250: {
		bool highNode = (_nodeNum > 25);
		_nodeNum = 25;

		drawLocation();
		dsAll();
		if (_nodes[24] != 0 || highNode) {
			_eye = true;
			playMovie(72);
			_aTimerFlag = false;
			_aTimer = 114;
			playSound(34);
			_userInput = 0;
			_currMap = 10;
			_statPlay = 251;
		} else {
			_frameStop = 67;
			playMovie(72);
			dsAll();
			playMovie(73);
			dsAll();
			_statPlay = 991;
			_loopFlag = true;
		}
		}
		break;
	case 251:
		switch(_userInput) {
		case 0x2F1:
			_aTimer = 0;
			playMovie(73);
			dsAll();
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 252;
			_zoom = 2;
			processMap(30, _zoom);
			setATimer();
			_userInput = 0;
			break;
		}
		break;
	case 252:
		processMapInput(30);
		break;
	case 260:
		_nodeNum = 26;
		drawLocation();
		dsAll();
		playMovie(74);
		_currMap = 10;
		setATimer();
		_statPlay = 261;
		_userInput = 0;
		break;
	case 261:
		switch(_userInput) {
		case 0x2F1:
			_aTimer = 0;
			dsAll();
			playMovie(75);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x428:
			inventoryDel(0);
			_aTimer = 0;
			saveAS();
			playMovie(178);
			restoreAS();
			setATimer();
			_userInput = 0;
			break;
		case 0x42A:
			inventoryDel(2);
			_aTimer = 0;
			saveAS();
			playMovie(178);
			restoreAS();
			setATimer();
			_userInput = 0;
			break;
		case 0x42C:
			_aTimer = 0;
			enAll();
			playMovie(76);
			_nodes[26] = 1;
			_userInput = 0;
			_statPlay = 290;
			_lastObs = true;
			_lastObstacle = 29;
			_loopFlag = true;
			break;
		case 0x42D:
			if (wound()) {
				_aTimer = 0;
				enAll();
				playMovie(77);
				playSound(34);
				inventoryDel(5);
				_statPlay = 300;
				_loopFlag = true;
			} else {
				dsAll();
				playMovie(78);
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		}
		break;
	case 270:
		_nodeNum = 27;
		drawLocation();
		if (_nodes[28] == 1)
			_nodes[28] = 9;
		if (_nodes[28] == 9 || _nodes[29] != 0) {
			if (_wizard)
				_sound = true;
		} else {
			_sound = false;
			_nodes[28] = 1;
		}
		playMovie(80);
		playSound(18);
		_currMap = 10;
		_statPlay = 271;
		_nodes[28] = 1;
		_userInput = 0;
		break;
	case 271:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 272;
			_zoom = 2;
			processMap(27, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 272:
		processMapInput(27);
		break;
	case 280:
		_nodeNum = 28;
		drawLocation();
		_userInput = 0;
		dsAll();
		_fstFwd = false;
		playMovie(81);
		_currMap = 10;
		_nodes[29] = _nodes[39] = 1;
		_statPlay = _robberyNode * 10;
		enAll();
		_loopFlag = true;
		break;
	case 290:
		_oldNode = _nodeNum;
		if (!_wizard) {
			_nodes[26] = 9;
			_nodes[30] = 8;
		}
		if (_nodes[26] == 0) {
			_statPlay = 260;
			_loopFlag = true;
			_nextNode = 29;
		} else {
			if (_nodes[26] == 2)
				playMovie(79);
			_nodeNum = 29;
			if (_nodes[26] <= 2 || !chkDesertObstacles()) {
				drawLocation();
				_nodes[26]++;
				_userInput = 0;
				switch (_nodes[30]) {
				case 0:
					_sound = false;
					playMovie(82);
					_currMap = 89;
					_nodes[30] = 1;
					break;
				case 1:
					_sound = true;
					playMovie(82);
					_currMap = 89;
					_nodes[30] = 0;
					break;
				case 8:
					_sound = false;
					playMovie(85);
					_currMap = 10;
					_nodes[30] = 9;
					_tsIconOnly = true;
					break;
				default:
					_sound = true;
					playMovie(85);
					_currMap = 10;
					_tsIconOnly = true;
					break;
				}
				playSound(17);
				_statPlay = 291;
			}
		}
			
		break;
	case 291:
		switch(_userInput) {
		case 0x431:
			_tsIconOnly = true;
			playMovie(83);
			playSound(30);
			inventoryAdd(10);
			_nodes[30] = 8;
			_userInput = 0;
			_currMap = 10;
			break;
		case 0x432:
			if (_nodes[30] >= 8)
				displayIcon(140);
			else {
				dsAll();
				playMovie(84);
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		case 0x43A:
			_statPlay = 292;
			_zoom = 2;
			processMap(29, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 292:
		processMapInput(29);
		break;
	case 300:
		if (_nodes[24] == 0) {
			_loopFlag = true;
			_statPlay = 250;
		} else {
			_nodeNum = 30;
			if (!chkDesertObstacles()) {
				drawLocation();
				playMovie(86);
				playSound(18);
				_currMap = 10;
				_statPlay = 301;
			}
		}
		break;
	case 301:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 302;
			_zoom = 2;
			processMap(30, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		} 
		break;
	case 302:
		processMapInput(30);
		break;
	case 310:
		_oldNode = _nodeNum;
		_nodeNum = 31;
		if (!chkDesertObstacles()) {
			drawLocation();
			playMovie(67);
			playSound(18);
			_currMap = 90;
			_statPlay = 311;
			_nodes[32] = 1;
			_aTimer = 0;
			_userInput = 0;
		}
		break;
	case 311:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 312;
			_zoom = 2;
			processMap(31, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x449:
			_aTimer = 0;
			dsAll();
			playMovie(68);
			_statPlay = 992;
			_loopFlag = true;
			break;			
		}
		break;
	case 312:
		processMapInput(31);
		break;
	case 320:
		_oldNode = _nodeNum;
		_nodeNum = 32;
		drawLocation();
		_userInput = 0;
		increaseHealth();
		if (_nodes[33] != 9 && _wizard) {
			_sound = false;
			_nodes[33] = 9;
		} else {
			_sound = true;
			_nodes[33] = 1;
		}
		playMovie(87);
		playSound(18);
		_currMap = 10;
		_statPlay = 321;
		break;
	case 321:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 322;
			_zoom = 2;
			processMap(32, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 322:
		processMapInput(32);
		break;
	case 330:
		_oldNode = _nodeNum;
		_nodeNum = 33;
		if (!chkDesertObstacles()) {
			drawLocation();
			_userInput = 0;
			if (_nodes[32] == 0)
				_sound = false;
			else
				_sound = true;
			playMovie(88);
			playSound(18);
			_currMap = 10;
			_statPlay = 331;
		}
		break;
	case 331:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 332;
			_zoom = 2;
			processMap(33, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 332:
		processMapInput(33);
		break;
	case 340:
		if (_nodes[36] == 0) {
			_statPlay = 350;
			_loopFlag = true;
			_nextNode = 34;
		} else {
			_oldNode = _nodeNum;
			_nodeNum = 34;
			if (!chkDesertObstacles()) {
				drawLocation();
				_userInput = 0;
				if (_nodes[35] == 9 || _nodes[41] == 0 || !_wizard) {
					playMovie(91);
					_currMap = 92;
				} else {
					playMovie(89);
					showPic(389);
					_currMap = 91;
				}
				if (_nodes[35] == 9)
					_tsIconOnly = true;
				playSound(26);
				_statPlay = 341;
			}
		}
		break;
	case 341:
		switch(_userInput) {
		case 0x431:
			if (_nodes[35] == 9)
				displayIcon(139);
			else {
				playMovie(90);
				inventoryAdd(9);
				_nodes[35] = 9;
				_currMap = 93;
			}
			break;
		case 0x43A:
			_statPlay = 342;
			_zoom = 2;
			processMap(34, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			if (_pMovie == 89)
				showPic(389);
			break;
		case 0x445:
			_statPlay = 410;
			_userInput = 0;
			_currMap = 10;
			_loopFlag = true;
			break;
		case 0x446:
			_statPlay = 400;
			_userInput = 0;
			_currMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 342:
		processMapInput(34);
		break;
	case 350:
		if (!_wizard)
			_nodes[36] = 1;
		if (_nodes[36] == 1) {
			_statPlay = _nextNode * 10;
			_loopFlag = true;
		} else {
			_nodeNum = 35;
			drawLocation();
			dsAll();
			playMovie(92);
			_currMap = 94;
			setATimer();
			_userInput = 0;
			_statPlay = 351;
		}
		break;
	case 351:
		switch(_userInput) {
		case 0x428:
		case 0x429:
			inventoryDel(_userInput - 0x428);
			_aTimer = 0;
			playMovie(178);
			_userInput = 0x2F1;
			_loopFlag = true;
		// No break on purpose
		case 0x2F1:
			_aTimer = 0;
			dsAll();
			playMovie(93);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x42A:
			_aTimer = 0;
			enAll();
			playMovie(179);
			inventoryDel(2);
			_nodes[36] = 1;
			_userInput = 0;
			_statPlay = _nextNode * 10;
			_lastObs = true;
			_lastObstacle = _nextNode;
			_loopFlag = true;
			break;
		}
		break;
	case 360:
		_nodeNum = 36;
		drawLocation();
		if (_nodes[37] == 0)
			_sound = false;
		else
			_sound = true;
		_frameStop = 185;
		dsAll();
		playMovie(94);
		_currMap = 95;
		setATimer();
		_userInput = 0;
		_statPlay = 361;
		break;
	case 361:
		switch(_userInput) {
		case 0x2F1:
			_aTimer = 0;
			dsAll();
			playMovie(95);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x428:
			_aTimer = 0;
			enAll();
			playMovie(97);
			inventoryDel(0);
			playSound(5);
			_nodes[37] = 1;
			_userInput = 0;
			_currMap = 10;
			if (_nodes[43] == 0) {
				_statPlay = 420;
				_oldNode = _nodeNum;
				_nodeNum = 42;
				GPLogic2_SubSP361();
			} else {
				_tsIconOnly = true;
			}
			break;
		case 0x42A:
			_aTimer = 0;
			enAll();
			playMovie(98);
			inventoryDel(2);
			playSound(5);
			_nodes[37] = 1;
			_userInput = 0;
			_currMap = 10;
			_tsIconOnly = true;
			break;
		case 0x42E:
			_aTimer = 0;
			enAll();
			playMovie(100);
			inventoryDel(6);
			playSound(5);
			_nodes[37] = 1;
			_userInput = 0;
			_currMap = 10;
			_tsIconOnly = true;
			break;
		case 0x431:
			_aTimer = 0;
			enAll();
			playMovie(96);
			playSound(5);
			_nodes[37] = 9;
			_userInput = 0;
			_currMap = 10;
			_statPlay = 420;
			_oldNode = _nodeNum;
			_nodeNum = 42;
			GPLogic2_SubSP361();
			break;
		case 0x437:
			_aTimer = 0;
			dsAll();
			playMovie(99);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 362;
			_zoom = 2;
			processMap(36, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 362:
		processMapInput(36);
		break;
	case 370:
		_nodeNum = 37;
		drawLocation();
		_userInput = 0;
		_nodes[38] = 0;
		playMovie(102);
		playSound(20);
		_currMap = 109;
		_statPlay = 371;
		break;
	case 371:
		switch(_userInput) {
		case 0x413:
			_statPlay = 440;
			_currMap = 10;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 372;
			_zoom = 2;
			processMap(37, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 372:
		processMapInput(37);
		break;
	case 380:
		_oldNode = _nodeNum;
		_nodeNum =38;
		if (!chkDesertObstacles()) {
			drawLocation();
			_userInput = 0;
			playMovie(159);
			_currMap = 10;
			playSound(7);
			_nodes[39] = 1;
			_statPlay = 381;
		}
		break;
	case 381:
		switch(_userInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			if (_wizard && _nodes[39] != 9) {
				inventoryDel(_userInput - 0x428);
				saveAS();
				playMovie(178);
				restoreAS();
			}
			_userInput = 0;
			break;
		case 0x430:
			if (_wizard && _nodes[39] != 9) {
				dsAll();
				playMovie(162);
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		case 0x433:
			if (_wizard && _nodes[39] != 9) {
				playMovie(161);
				inventoryDel(11);
				_nodes[51] = 0;
				_userInput = 0;
			}
			_userInput = 0;
			break;
		case 0x435:
			if (_wizard && _nodes[39] != 9) {
				playMovie(160);
				inventoryDel(13);
				_nodes[39] = 9;
				_nodes[59] = 0;
				_userInput = 0;
				_tsIconOnly = true;
				_currMap = 108;
			}
			break;
		case 0x437:
			if (_wizard && _nodes[39] != 9) {
				playMovie(163);
				inventoryDel(15);
				_nodes[12] = 0;
			}
			_userInput = 0;
			break;
		case 0x43A:
			_statPlay = 382;
			_zoom = 2;
			processMap(38, _zoom);
			_userInput = 0;
		break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x456:
			_statPlay = 370;
			_currMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 382:
		if (_wizard && (_nodes[39] != 9) && (_userInput == 0x413))
			_userInput = 0;

		processMapInput(38);
		break;
	case 390:
		_nodeNum = 39;
		drawLocation();
		_aTimer = 0;
		dsAll();
		playMovie(104);
		playSound(34);
		_statPlay = 992;
		_loopFlag = true;
		break;
	case 400:
		_nodeNum = 40;
		if (!_wizard)
			_nodes[41] = 2;
		if (_nodes[41]) {
			if (_nodes[41] == 1)
				_nodes[41] = 2;
			else {
				_nodes[41] = 1;
				_sound = true;
			}
		} else {
			dsAll();
			playMovie(105);
			_currMap = 10;
			setATimer();
		}
		
		_userInput = 0;
		_statPlay = 401;
		break;
	case 401:
		switch(_userInput) {
		case 0x2F1:
			_aTimer = 0;
			dsAll();
			playMovie(106);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			if (!_nodes[41]) {
				inventoryDel(_userInput - 0x428);
				_aTimer = 0;
				playMovie(178);
				_userInput = 753;
				_loopFlag = true;
				_currMap = 10;
			}
			break;
		case 0x42C:
			if (!_nodes[41]) {
				_aTimer = 0;
				dsAll();
				playMovie(108);
				_userInput = 0;
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		case 0x42E:
			if (!_nodes[41]) {
				_aTimer = 0;
				playMovie(109);
				inventoryDel(6);
				_nodes[43] = 0;
				_userInput = 0;
				_aTimer = 136;
			}
			break;
		case 0x437:
			if (!_nodes[41]) {
				_aTimer = 0;
				_help = true;
				_replay = true;
				_pouch = true;
				_fstFwd = true;
				playMovie(107);
				playSound(26);
				_nodes[41] = 1;
				_userInput = 0;
				_currMap = 96;
				_tsIconOnly = true;
				_loopFlag = true;
			}
			break;
		case 0x43E:
			_aTimer = 0;
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x441:
			_statPlay = 460;
			_currMap = 10;
			_userInput = 0;
			_loopFlag = true;
			break;
		}
		break;
	case 402:
		_aTimer = 0;
		processMapInput(40);
		break;
	case 410:
		_nodeNum = 41;
		drawLocation();
		_userInput = 0;
		if (_nodes[41])
			_sound = true;

		_eye = false;
		playMovie(111);
		playSound(26);
		_currMap = 97;
		_statPlay = 411;
		break;
	case 411:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			if (_pMovie == 120) {
				saveAS();
				_frameStop = 50;
				playMovie(120);
				_bTimer = 34;
				while(_bTimer != 0) {
					checkTimers();
					refreshSound();
					checkMainScreen();
				}
				restoreAS();
			}
			playMovie(_pMovie);
			break;
		case 0x445:
			_userInput = 0;
			_loopFlag = true;
			_currMap = 10;
			_statPlay = 450;
			break;
		case 0x446:
			_userInput = 0;
			_loopFlag = true;
			_currMap = 10;
			_statPlay = 460;
			break;
		}
		break;
	case 420:
		if (_nodes[37] == 9 || _nodes[29] == 1 || !_wizard) {
			_oldNode = _nodeNum;
			_nodeNum = 42;
			if (!chkDesertObstacles()) {
				drawLocation();
				GPLogic2_SubSP361();
			}
		} else {
			_statPlay = 360;
			_loopFlag = true;
		}
		break;
	case 421:
		switch(_userInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			inventoryDel(_userInput - 0x428);
			saveAS();
			playMovie(178);
			restoreAS();
			_userInput = 0;
			break;
		case 0x42E:
			if (_nodes[43])
				displayIcon(136);
			else {
				if (_nodes[35] == 9) {
					_sound = false;
				} else {
					_sound = true;
				}
				playMovie(113);
				inventoryAdd(6);
				_nodes[43] = 9;
				_currMap = 10;
			}
			_tsIconOnly = true;
			break;
		case 0x43A:
			_statPlay = 422;
			_zoom = 2;
			processMap(42, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		
		}
		break;
	case 422:
		processMapInput(42);
		break;
	case 430:
		_nodeNum = 43;
		drawLocation();
		_eye = false;
		_userInput = 0;
		if (_nodes[44] == 9) {
			_tsIconOnly = true;
			playMovie(118);
			_currMap = 80;
		} else if (_inventory[12] <= 0) {
			playMovie(115);
			showPic(415);
			_currMap = 78;
			if (!_skylarTimer)
				_skylarTimer = 5400;
		} else {
			_skylarTimer = 0;
			playMovie(116);
			playSound(25);
			inventoryDel(12);
			inventoryAdd(16);
			_nodes[44] = 9;
			_currMap = 79;
			_tsIconOnly = true;
		}
		_statPlay = 431;
		playSound(20);
		_eye = false;
		break;
	case 431:
		switch(_userInput) {
			case 0x40A:
				playMovie(10);
				_statPlay = 200;
				if (_nodes[44] != 9)
					_nodes[20] = 1;
				_currMap = 10;
				_loopFlag = true;
				break;
			case 0x414:
				playMovie(10);
				_statPlay = 390;
				_currMap = 10;
				_loopFlag = true;
				break;
			case 0x41D:
				playMovie(10);
				_statPlay = 500;
				_currMap = 10;
				_loopFlag = true;
				break;
			case 0x422:
				playMovie(10);
				_statPlay = 380;
				_currMap = 10;
				_loopFlag = true;
				break;
			case 0x428:
			case 0x429:
			case 0x42A:
				if (_nodes[44] != 9) {
					inventoryDel(_userInput - 0x428);
					saveAS();
					playMovie(178);
					restoreAS();
				}
				break;
			case 0x438:
				if (_nodes[44] == 9) {
					displayIcon(146);
				} else {
					dsAll();
					playMovie(117);
					_statPlay = 992;
					_loopFlag = true;
				}
				break;
			case 0x43E:
				break;
		}
		break;
	case 440:
		_nodeNum = 44;
		drawLocation();
		_userInput = 0;
		if (_inventory[12] > 0 || _nodes[44] == 9)
			_sound = true;

		playMovie(119);
		playSound(20);
		_currMap = 100;
		_statPlay = 441;
		break;
	case 441:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 442;
			_zoom = 2;
			processMap(44, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x452:
			_statPlay = 430;
			_currMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 442:
		processMapInput(44);
		break;
	case 450:
		_nodeNum = 45;
		_userInput = 0;
		if (!_nodes[41]) {
			dsAll();
			playMovie(120);
			_statPlay = 992;
			_currMap = 10;
			_loopFlag = true;
		} else {
			_fstFwd = false;
			_frameStop = 50;
			playMovie(120);
			_currMap = 17;
			_statPlay = 451;
		}
		break;
	case 451:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			_fstFwd = false;
			_frameStop = 50;
			playMovie(_pMovie);
			_currMap = 17;
			playSound(26);
			break;
		case 0x441:
			_statPlay = 410;
			_currMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 460:
		_eye = false;
		_nodeNum = 46;
		drawLocation();
		_userInput = 0;
		if (_nodes[47] && _wizard)
			_nodes[47] = 1;
		else {
			_nodes[47] = 0;
			_sound = true;
		}
		playMovie(121);
		playSound(26);
		_currMap = 107;
		_statPlay = 461;
		break;
	case 461:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x445:
			_statPlay = 340;
			_userInput = 0;
			_loopFlag = true;
			_currMap = 10;
			_lastObs = true;
			break;
		case 0x446:
			_statPlay = 400;
			_userInput = 0;
			_loopFlag = true;
			_currMap = 10;
			break;
		}
		break;
	case 462:
		processMapInput(46);
		break;
	case 480:
		_nodeNum = 48;
		_userInput = 0;
		playMovie(114);
		_currMap = 10;
		_statPlay = 481;
		_nodes[49] = 1;
		break;
	case 481:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 482;
			_zoom = 2;
			processMap(48, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 482:
		processMapInput(48);
		break;
	case 490:
		_nodeNum = 49;
		drawLocation();
		dsAll();
		playMovie(180);
		playSound(18);
		_currMap = 101;
		_statPlay = 491;
		_aTimer = 0;
		_userInput = 0;
		break;
	case 491:
		switch(_userInput) {
		case 0x428:
		case 0x42A:
		case 0x42B:
			inventoryDel(_userInput - 0x428);
			_aTimer = 0;
			playMovie(178);
			_userInput = 753;
			_loopFlag = true;
			_currMap = 10;
			break;
		case 0x442:
			playMovie(182);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x44A:
			_currMap = 10;
			_userInput = 0;
			_aTimer = 0;
			enAll();
			playMovie(181);
			_nodes[29] = 9;
			_nodes[49] = 0;
			_statPlay = _nextNode * 10;
			_loopFlag = true;
			break;
		}
		break;
	}
}

void KingdomGame::GPLogic2_SubSP361() {
	enAll();
	_userInput = 0;
	playSound(19);
	if (_nodes[29] == 1) {
		_statPlay = 480;
		_loopFlag = true;
		_currMap = 10;
	} else {
		if (_inventory[6] == 1 || !_wizard) {
			if (_nodes[43] != 1 || _wizard)
				_nodes[43] = 1;
			else {
				_sound = true;
				_nodes[43] = 2;
			}
			playMovie(122);
			_currMap = 10;
			_tsIconOnly = true;
			_statPlay = 421;
		} else {
			playMovie(112);
			_currMap = 98;
			_statPlay = 421;
		}
	}
}

} // NameSpace
