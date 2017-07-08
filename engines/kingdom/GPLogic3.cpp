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

void KingdomGame::GPLogic3() {
	switch(_statPlay) {
	case 500:
		_nodeNum = 50;
		drawLocation();
		_userInput = 0;
		_resurrect = false;
		if (!_wizard)
			_nodes[51] = 1;
		switch(_nodes[51]) {
		case 0:
			playMovie(123);
			_currMap = 102;
			break;
		case 1:
			_nodes[51] = 2;
			playMovie(126);
			_currMap = 10;
			_tsIconOnly = true;
			break;
		default:
			_nodes[51] = 1;
			_sound = true;
			playMovie(126);
			_currMap = 10;
			_tsIconOnly = true;
			break;
		}
		playSound(29);
		_statPlay = 501;
		break;
	case 501:
		switch(_userInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			if (!_nodes[51]) {
				inventoryDel(_userInput - 0x428);
				saveAS();
				playMovie(178);
				restoreAS();
				_userInput = 0;
			}
			break;
		case 0x433:
			if (!_nodes[51]) {
				dsAll();
				playMovie(125);
				_statPlay = 993;
				_loopFlag = true;
			} else
				displayIcon(141);

			break;
		case 0x439:
			if (!_nodes[51]) {
				playMovie(124);
				_nodes[51] = 1;
				inventoryAdd(11);
				_userInput = 0;
				_currMap = 10;
			}
			break;
		case 0x43A:
			_statPlay = 502;
			_zoom = 2;
			processMap(50, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 502:
		processMapInput(50);
		break;
	case 510:
		_nodeNum = 51;
		drawLocation();
		_userInput = 0;
		playMovie(129);
		_currMap = 10;
		playSound(6);
		_statPlay = 511;
		break;
	case 511:
		switch(_userInput) {
		case 0x434:
			if (_inventory[6] >= 1)
				displayIcon(142);
			else {
				playMovie(128);
				_nodes[52] = 9;
				inventoryAdd(12);
				_userInput = 0;
				_currMap = 10;
				playSound(30);
			}
			break;
		case 0x43A:
			_statPlay = 512;
			_zoom = 2;
			processMap(51, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			playMovie(_pMovie);
			break;
		case 0x457:
			_tideCntl = false;
			drawPic(179);
			playMovie(127);
			_currMap = 103;
			_userInput = 0;
			playSound(0);
			playSound(6);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 512:
		processMapInput(51);
		break;
	case 520:
		_nodeNum = 52;
		drawLocation();
		_userInput = 0;
		if (_nodes[53] == 0)
			_nodes[53] = 1;
		else {
			_sound = true;
			_nodes[53] = false;
		}
		playMovie(130);
		_currMap = 111;
		_statPlay = 521;
		playSound(31);
		break;
	case 521:
		switch(_userInput) {
		case 0x404:
			_statPlay = 550;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 522;
			_zoom = 2;
			processMap(52, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 522:
		processMapInput(52);
		break;
	case 530:
		_nodeNum = 53;
		drawLocation();
		_userInput = 0;
		_fstFwd = false;
		if (_inventory[7] <= 0 && _wizard)
			_tsIconOnly = false;
		else {
			_frameStop = 24;
			_tsIconOnly = true;
		}
		_eye = false;
		playMovie(70);
		_currMap = 99;
		playSound(8);
		_statPlay = 531;
		break;
	case 531:
		switch(_userInput) {
		case 0x428:
			if (_nodes[54] == 0) {
				playMovie(71);
				inventoryAdd(14);
				inventoryDel(0);
				_tsIconOnly = true;
				playSound(25);
			}
			_userInput = 0;
			break;
		case 0x429:
		case 0x42A:
			inventoryDel(_userInput - 0x428);
			saveAS();
			playMovie(178);
			restoreAS();
			_userInput = 0;
			break;
		case 0x43E:
			_fstFwd = false;
			if (_inventory[7] > 0 && _pMovie == 70)
				_frameStop = 24;
			if (!_wizard && _pMovie == 70)
				_frameStop = 24;
			playMovie(_pMovie);
			break;
		case 0x445:
			_nodeNum = 56;
			_userInput = 0;
			_frameStop = 50;
			enAll();
			playMovie(135);
			_currMap = 106;
			_nodes[57] = 1;
			playSound(8);
			_tsIconOnly = true;
			_statPlay = 561;
			break;
		case 0x452:
			_statPlay = 630;
			_loopFlag = true;
			_userInput = 0;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 540:
		_nodeNum = 54;
		drawLocation();
		_userInput = 0;
		if (!_wizard) {
			_sound = true;
			_currMap = 55;
		} else {
			if (!_nodes[55])
				_nodes[55] = 1;
			else {
				_sound = true;
				_nodes[55] = 0;
			}
			_currMap = 112;
		}

		playMovie(132);
		showPic(432);
		_statPlay = 541;
		playSound(22);
		break;
	case 541:
		switch(_userInput) {
		case 0x404:
			_statPlay = 550;
			_loopFlag = true;
			_userInput = 0;
			break;
		case 0x43A:
			_statPlay = 542;
			_zoom = 2;
			processMap(54, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			showPic(432);
			break;
		case 0x453:
			_sound = _lastSound;
			playMovie(131);
			playSound(32);
			fShowPic(432);
			// Useless, but present in the original
			// _CurrMap = 10;
			_currMap = 55;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 542:
		processMapInput(54);
		break;
	case 550:
		_nodeNum = 55;
		drawLocation();
		_userInput = 0;
		playMovie(133);
		if (_nodes[56] != 9)
			_nodes[56] = 0;
		_tsIconOnly = true;
		_currMap = 104;
		_statPlay = 551;
		playSound(8);
		break;
	case 551:
		switch(_userInput) {
		case 0x2F1:
			_aTimer = 0;
			dsAll();
			playMovie(134);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x42C:
			_aTimer = 0;
			if (_nodes[56] == 1) {
				enAll();
				playMovie(183);
				fShowPic(433);
				playSound(8);
				_nodes[56] = 9;
				_tsIconOnly = true;
			}
			break;
		case 0x43A:
			if (_nodes[56] == 1) {
				dsAll();
				playMovie(134);
				_statPlay = 993;
				_loopFlag = true;
			} else {
				_statPlay = 552;
				_zoom = 2;
				processMap(55, _zoom);
				_userInput = 0;
			}
			break;
		case 0x43E:
			_aTimer = 0;
			playMovie(_pMovie);
			if (_pMovie == 183)
				fShowPic(433);
			break;
		case 0x452:
			_aTimer = 0;
			switch(_nodes[56]) {
			case 0:
				dsAll();
				playMovie(69);
				_tsIconOnly = false;
				_nodes[56] = 1;
				setATimer();
				break;
			case 1:
				dsAll();
				playMovie(134);
				_statPlay = 993;
				_loopFlag = true;
				break;
			case 9:
				_statPlay = 560;
				_loopFlag = true;
				break;
			}
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 552:
		processMapInput(55);
		if (!_nodes[56] && _statPlay == 560) {
			_statPlay = 551;
			_userInput = 0x190;
			_loopFlag = true;
		}
		break;
	case 560:
		_nodeNum = 56;
		drawLocation();
		_userInput = 0;
		if (_wizard) {
			_frameStop = 50;
			enAll();
			playMovie(135);
			showPic(435);
			_nodes[57] = 1;
			playSound(8);
			_statPlay = 561;
			_currMap = 106;
			_tsIconOnly = true;
		} else {
			dsAll();
			playMovie(135);
			_currMap = 10;
			setATimer();
			_nodes[57] = 0;
			_statPlay = 561;
		}
		break;
	case 561:
		switch(_userInput) {
		case 0x2F1:
			_aTimer = 0;
			dsAll();
			playMovie(136);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			_aTimer = 0;
			if (!_nodes[57]) {
				inventoryDel(_userInput - 0x428);
				saveAS();
				playMovie(178);
				restoreAS();
				_userInput = 753;
				_loopFlag = true;
			}
			break;
		case 0x42C:
			_aTimer = 0;
			if (!_nodes[57]) {
				_fstFwd = false;
				_replay = true;
				playMovie(138);
				enAll();
				playSound(8);
				_nodes[57] = 1;
				inventoryDel(4);
				_nodes[22] = 0;
				_currMap = 88;
				_tsIconOnly = true;
			}
			break;
		case 0x438:
			_aTimer = 0;
			if (!_nodes[57]) {
				enAll();
				_fstFwd = false;
				_replay = true;
				playMovie(137);
				enAll();
				playSound(8);
				_nodes[57] = 1;
				_currMap = 106;
				_tsIconOnly = true;
			}
			break;
		case 0x43A:
			_statPlay = 562;
			_zoom = 2;
			processMap(56, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_fstFwd = false;
			if (_pMovie == 135 && _nodes[57]) {
				_frameStop = 50;
				playMovie(135);
			} else if (_nodes[57])
				playMovie(_pMovie);
			else {
				_userInput = 753;
				_aTimer = 0;
				_loopFlag = true;
			}
			break;
		case 0x445:
			_statPlay = 530;
			_loopFlag = true;
			_userInput = 0;
			break;
		case 0x446:
			_statPlay = 620;
			_loopFlag = true;
			_userInput = 0;
			break;
		case 0x452:
			_statPlay = 550;
			_loopFlag = true;
			_userInput = 0;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 562:
		processMapInput(56);
		break;
	case 570:
		if (_nodes[58]) {
			if (_rtnNode != 34)
				_sound = true;
			_nodeNum = 57;
			drawLocation();
			_nodes[58] = 0;
			_userInput = 0;
			playMovie(139);
			playSound(27);
			_currMap = 10;
			_statPlay = 571;
		} else {
			_statPlay = 740;
			_loopFlag = true;
		}
		break;
	case 571:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 572;
			_zoom = 2;
			processMap(57, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 572:
		processMapInput(57);
		break;
	case 580:
		_nodeNum = 58;
		drawLocation();
		_iconsClosed = true;
		_userInput = 0;
		switch(_nodes[59]) {
		case 0:
			_sound = true;
			playMovie(140);
			_currMap = 113;
			_nodes[59] = 1;
			break;
		case 1:
			playMovie(140);
			_currMap = 113;
			_nodes[59] = 0;
			break;
		case 8:
			_tsIconOnly = true;
			playMovie(143);
			_currMap = 10;
			_nodes[59] = 9;
			break;
		case 9:
			_tsIconOnly = true;
			_sound = true;
			playMovie(143);
			_currMap = 10;
			break;
		}
		playSound(27);
		_statPlay = 581;
		break;
	case 581:
		switch(_userInput) {
		case 0x430:
			playMovie(141);
			_nodes[59] = 8;
			inventoryAdd(13);
			_tsIconOnly = true;
			_currMap = 10;
			playSound(30);
		// No break on purpose
		case 0x428:
		case 0x429:
		case 0x42A:
			if (_nodes[59] < 8) {
				inventoryDel(_userInput - 0x428);
				saveAS();
				playMovie(178);
				restoreAS();
			}
			_userInput = 0;
			break;
		case 0x435:
			if (_nodes[59] >= 8)
				displayIcon(143);
			else {
				dsAll();
				playMovie(142);
				_statPlay = 993;
				_loopFlag = true;
			}
		case 0x43A:
			_statPlay = 582;
			_zoom = 2;
			processMap(58, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 582:
		processMapInput(58);
		break;
	case 590:
		_nodeNum = 59;
		drawLocation();
		_userInput = 0;
		if (_nodes[60] == 0)
			_nodes[60] = 1;
		else {
			_sound = true;
			_nodes[60] = 0;
		}
		playMovie(144);
		showPic(144);
		_currMap = 114;
		playSound(21);
		_statPlay = 591;
		break;
	case 591:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 592;
			_zoom = 2;
			processMap(59, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			showPic(444);
			break;
		case 0x445:
			_statPlay = 660;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 592:
		processMapInput(59);
		break;
	case 600:
		setMouse();
		eraseCursor();
		fadeToBlack2();
		showPic(106);
		drawIcon(4, 0, 12 - _healthOld);
		_asMode = false;
		_nodeNum = 60;
		drawLocation();
		_userInput = 0;
		dsAll();
		playMovie(145);
		_statPlay = 992;
		_loopFlag = true;
		break;
	case 610:
		_nodeNum = 61;
		drawLocation();
		_userInput = 0;
		_nodes[73] = 0;
		_nodes[74] = 0;
		if (_nodes[62] == 0)
			_nodes[62] = 1;
		else
			_sound = true;

		playMovie(146);
		if (_nodes[70] == 9) {
			_itemInhibit = true;
			_tsIconOnly = true;
		}
		
		_currMap = 115;
		playSound(36);
		_statPlay = 611;
		_eye = false;
		break;
	case 611:
		switch(_userInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			inventoryDel(_userInput - 0x428);
			dsAll();
			playMovie(148);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x432:
			playMovie(147);
			_inventory[5] = -1;
			_nodes[30] = 0;
			_currMap = 116;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x441:
			// Cascade on purpose
		case 0x448:
			_currMap = 10;
			_itemInhibit = true;
			_statPlay = 730;
			_loopFlag = true;
			break;
		case 0x44B:
			_currMap = 10;
			_statPlay = 680;
			_loopFlag = true;
			break;
		case 0x451:
			_currMap = 10;
			_statPlay = 690;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 620:
		_nodeNum = 62;
		drawLocation();
		_userInput = 0;
		_eye = false;
		if (_nodes[63] == 0) {
			playMovie(149);
			_currMap = 122;
		} else {
			_frameStop = 24;
			playMovie(202);
			_currMap = 121;
			_nodes[63] = 9;
		}
		_tsIconOnly = true;
		playSound(8);
		_statPlay = 621;
		break;
	case 621:
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
		case 0x430:
			playMovie(150);
			inventoryAdd(8);
			_nodes[63] = 1;
			playSound(25);
			_currMap = 121;
			_userInput = 0;
			break;
		case 0x43E:
			switch (_nodes[63]) {
			case 0:
				playMovie(149);
				break;
			case 1:
				playMovie(150);
				break;
			case 9:
				_frameStop = 24;
				playMovie(202);
				_currMap = 121;
				break;
			}
			break;
		case 0x445:
			_nodeNum = 56;
			_userInput = 0;
			_frameStop = 50;
			enAll();
			playMovie(135);
			_nodes[57] = 1;
			playSound(8);
			_statPlay = 561;
			_currMap = 106;
			if (_inventory[16] < 1 && _wizard)
				_currMap = 88;
			break;
		case 0x452:
			_statPlay = 630;
			_loopFlag = true;
			_userInput = 0;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 630:
		_nodeNum = 63;
		drawLocation();
		_userInput = 0;
		_eye = false;
		if (_nodes[64] == 1 || !_wizard) {
			_nodes[64] = 0;
			_sound = true;
		} else
			_nodes[64] = true;

		playMovie(151);
		_currMap = 72;
		playSound(40);
		_statPlay = 631;
		break;
	case 631:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x445:
			playMovie(10);
			_statPlay = 50;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 640:
		_nodeNum = 64;
		drawLocation();
		_userInput = 0;
		if (_nodes[65] || _wizard) {
			_sound = true;
			_nodes[65] = 0;
		} else
			_nodes[65] = 1;

		playMovie(152);
		_currMap = 10;
		_statPlay = 641;
		playSound(28);
		break;
	case 641:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 642;
			_zoom = 2;
			processMap(64, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound= _lastSound;
			playMovie(_pMovie);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 642:
		processMapInput(64);
		break;
	case 650:
		_nodeNum = 65;
		_eye = false;
		_userInput = 0;
		if (_nodes[66] == 0)
			_nodes[66] = 1;
		else {
			_sound = true;
			_nodes[66] = 0;
		}
		playMovie(153);
		showPic(453);
		_currMap = 73;
		playSound(37);
		_statPlay = 651;
		break;
	case 651:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			showPic(453);
			break;
		case 0x445:
			_statPlay = 700;
			_loopFlag = true;
			break;
		case 0x446:
			if (_nodes[68] == 1) {
				_statPlay = 660;
				_loopFlag = true;
			} else {
				_statPlay = 790;
				_loopFlag = true;
			}
			break;
		case 0x447:
			_statPlay = 710;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 660:
		_nodeNum = 66;
		_eye = false;
		_userInput = 0;
		if (_nodes[67] == 0)
			_nodes[67] = 1;
		else {
			_sound = true;
			_nodes[67] = 0;
		}
		playMovie(154);
		_currMap = 70;
		playSound(40);
		_statPlay = 661;
		break;
	case 661:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x445:
			_statPlay = 700;
			_loopFlag = true;
			break;
		case 0x446:
			_loopFlag = true;
			_statPlay = (_nodes[68] == 1) ? 650 : 790;
			break;
		case 0x447:
			if (!_wizard || _nodes[68])
				_statPlay = 650;
			else
				_statPlay = 670;

			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 670:
		_nodeNum = 67;
		_userInput = 0;
		_eye = false;
		_nodes[68] = 1;
		_fstFwd = false;
		playMovie(155);
		_currMap = 127;
		_statPlay = 671;
		break;
	case 671:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			_fstFwd = false;
			playMovie(_pMovie);
			break;
		case 0x445:
			_statPlay = 650;
			_loopFlag = true;
			break;
		case 0x446:
			_statPlay = 660;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 680:
		_nodeNum = 68;
		_userInput = 0;
		dsAll();
		playMovie(156);
		_statPlay = 993;
		_loopFlag = true;
		break;
	case 690:
		_itemInhibit = false;
		enAll();
		if (_nodes[70] == 9) {
			_statPlay = 640;
			_nodes[73] = 0;
			_loopFlag = true;
			_userInput = 0;
		} else {
			_nodeNum = 69;
			drawLocation();
			_userInput = 0;
			_currMap = 119;
			_frameStop = 40;
			playMovie(157);
			playSound(12);
			_statPlay = 691;
		}
		break;
	case 691:
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
		case 0x439:
			if (_nodes[70] == 9)
				displayIcon(147);
			else {
				_frameStop = 108;
				playMovie(158);
				_itemInhibit = false;
				inventoryAdd(17);
				_nodes[70] = 9;
				playSound(23);
				_userInput = 0;
				_currMap = 120;
			}
			break;
		case 0x43E:
			if (_pMovie == 157)
				_frameStop = 40;
			else if (_pMovie == 158)
				_frameStop = 108;
			
			if (_pMovie == 178) {
				saveAS();
				playMovie(178);
				restoreAS();
			} else
				playMovie(_pMovie);

			break;
		case 0x441:
			_itemInhibit = false;
			_statPlay = 640;
			_nodes[73] = 0;
			_loopFlag = true;
			_userInput = 0;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 692:
		processMapInput(69);
		break;
	case 700:
		_nodeNum = 70;
		_userInput = 0;
		_eye = false;
		if (_nodes[71] && _wizard) {
			_nodes[71] = 0;
			_sound = false;
		} else
			_nodes[71] = 1;

		playMovie(164);
		_currMap = 74;
		playSound(10);
		_statPlay = 701;
		break;
	case 701:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x445:
			_statPlay = 650;
			_loopFlag = true;
			break;
		case 0x446:
			_statPlay = 780;
			_loopFlag = true;
			break;
		case 0x447:
			_statPlay = 660;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 710:
		_nodeNum = 71;
		_userInput = 0;
		_eye = false;
		if (_inventory[8] && _inventory[14])
			_sound = true;
		dsAll();
		playMovie(165);
		_currMap = 10;
		setATimer();
		_statPlay = _wizard ? 712 : 711;
		break;
	case 711:
		switch(_userInput) {
		case 0x2F1:
			_replay = false;
			playMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			inventoryDel(_userInput - 0x428);
			_replay = false;
			_aTimer = 0;
			playMovie(178);
			playMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x430:
			_aTimer = 0;
			_nodeNum = 0;
			_iconsClosed = true;
			_fstFwd = false;
			playMovie(167);
			_fstFwd = false;
			playMovie(170);
			playSound(25);
			endCredits();
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 712:
		switch(_userInput) {
		case 0x2F1:
			_replay = false;
			playMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			inventoryDel(_userInput - 0x428);
			_replay = false;
			_aTimer = 0;
			playMovie(178);
			playMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x430:
			_aTimer = 0;
			if (_nodes[72] != 2) {
				playMovie(57);
				_statPlay = 993;
			} else {
				_nodeNum = 0;
				_iconsClosed = true;
				_fstFwd = false;
				playMovie(55);
				_fstFwd = false;
				playMovie(184);
				playSound(25);
				endCredits();
			}
			_loopFlag = true;
			break;
		case 0x432:
			_aTimer = 0;
			_replay = false;
			playMovie(168);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x433:
			if ((_inventory[8] > 0 && _inventory[14] > 0) || _nodes[72] == 2) {
				_replay = false;
				playMovie(166);
				_statPlay = 993;
			} else {
				_aTimer = 0;
				enAll();
				playMovie(169);
				_statPlay = 580;
				inventoryDel(11);
				_nodes[51] = 0;
			}
			_loopFlag = true;
			break;
		case 0x436:
			_aTimer = 0;
			playSound(0);
			playMovie(193);
			showPic(128);
			inventoryDel(14);
			_nodes[72] = 2;
			setATimer();
			break;

		case 0x437:
			_aTimer = 0;
			if (_nodes[72] == 2) {
				_nodeNum = 0;
				_iconsClosed = true;
				_fstFwd = false;
				playMovie(56);
				_fstFwd = false;
				playMovie(184);
				playSound(25);
				endCredits();
				_quit = true;
//--				_QuitFlag = true;
			} else {
				playMovie(58);
				_statPlay = 993;
			}
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 720:
		_nodeNum = 72;
		_userInput = 0;
		_eye = false;
		switch (_nodes[73]) {
		case 0:
			_nodes[73] = 1;
			break;
		case 1:
			_frameStop = 24;
			_nodes[73] = 2;
			break;
		default:
			_frameStop = 24;
			_nodes[73] = 3;
			break;
		}
		playMovie(175);
		_currMap = 118;
		playSound(12);
		_statPlay = 721;
		break;
	case 721:
		switch(_userInput) {
		case 0x43E:
			if (_nodes[73] > 1)
				_frameStop = 24;
			playMovie(_pMovie);
			break;
		case 0x441:
			if (_nodes[73] == 1) {
				_itemInhibit = false;
				_statPlay = 640;
			} else
				_statPlay = 680;

			_currMap = 10;
			_loopFlag = true;
			break;
		case 0x448:
			_statPlay = (_nodes[73] == 2) ? 730 : 680;
			_currMap = 10;
			_loopFlag = true;
			break;
		case 0x44B:
			_statPlay = (_nodes[73] == 1) ? 730 : 680;
			_currMap = 10;
			_loopFlag = true;
			break;
		case 0x44C:
			if (_nodes[73] == 1) {
				_statPlay = 730;
				_nodes[74] = 3;
			} else
				_statPlay = 680;

			_currMap = 10;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 730:
		_nodeNum = 73;
		_userInput = 0;
		_eye = false;
		switch(_nodes[74]) {
		case 0:
			_nodes[74] = 1;
			break;
		case 1:
			_sound = true;
			_nodes[74] = 2;
			break;
		case 2:
			_frameStop = 23;
			_nodes[74] = 3;
			break;
		default:
			_frameStop = 23;
			_nodes[74] = 4;
			break;
		}
		playMovie(176);
		_currMap = 117;
		playSound(36);
		_statPlay = 731;
		break;
	case 731:
		switch(_userInput) {
		case 0x43E:
			if (_nodes[74] > 2)
				_frameStop = 23;
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x441:
			switch (_nodes[74]) {
			case 1:
			case 2:
				_statPlay = 720;
				break;
			case 3:
				_statPlay = 720;
				_nodes[73] = 1;
				break;
			default:
				_statPlay = 680;
				break;
			}
			_currMap = 10;
			_loopFlag = true;
			break;
		case 0x448:
			switch(_nodes[74]) {
			case 3:
				_statPlay = 690;
				break;
			case 4:
				_statPlay = 720;
				_nodes[73] = 1;
				break;
			default:
				_statPlay = 680;
				break;
			}
			break;
		case 0x44B:
			switch(_nodes[74]) {
			case 1:
				_statPlay = 720;
				break;
			case 3:
			case 4:
				_statPlay = 720;
				_nodes[73] = 1;
				break;
			default:
				_statPlay= 680;
				break;
			}

			_currMap = 10;
			_loopFlag = true;
			break;
		case 0x44C:
			switch(_nodes[74]) {
			case 1:
			case 2:
				_statPlay = 720;
				break;
			case 4:
				_statPlay = 720;
				_nodes[73] = 1;
				break;
			default:
				_statPlay = 680;
				break;
			}
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 740:
		_rtnNode = _nodeNum;
		_nodeNum = 74;
		drawLocation();
		dsAll();
		playMovie(185);
		_currMap = 10;
		setATimer();
		_statPlay = 741;
		_userInput = 0;
		break;
	case 741:
		switch(_userInput) {
		case 0x2F1:
			dsAll();
			playMovie(188);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			inventoryDel(_userInput - 0x428);
			_aTimer = 0;
			playMovie(178);
			_loopFlag = true;
			_userInput = 753;
			break;
		case 0x432:
			if (_nodes[75]) {
				_sound = false;
				_nodes[75] = false;
			} else
				_nodes[75] = true;
			break;
		case 0x437:
			_loopFlag = true;
			if (wound()) {
				_frameStop = 88;
				playMovie(187);
				_statPlay = 570;
				_nodes[58] = 1;
			} else {
				_frameStop = 78;
				playMovie(187);
				_statPlay = 993;
			}
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 760:
		_nodeNum = 76;
		_eye = false;
		_userInput = 0;
		playMovie(189);
		_currMap = 126;
		playSound(21);
		_statPlay = 761;
		break;
	case 761:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		case 0x445:
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x44D:
			playMovie(191);
			_nodes[68] = 2;
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x44E:
			playMovie(192);
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x458:
			playMovie(190);
			_statPlay = 660;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 780:
		_nodeNum = 78;
		drawLocation();
		_userInput = 0;
		_eye = false;
		playMovie(171);
		_currMap = 124;
		playSound(40);
		_statPlay = 781;
		break;
	case 781:
		switch(_userInput) {
		case 0x428:
		case 0x42A:
			inventoryDel(_userInput - 0x428);
			_frameStop = 64;
			playMovie(173);
			showPic(471);
			_currMap = 124;
			break;
		case 0x429:
			if (_wizard) {
				_frameStop = 64;
				playMovie(173);
				inventoryDel(1);
			} else {
				_frameStop = 66;
				_fstFwd = false;
				playMovie(172);
				_fstFwd = true;
				playMovie(201);
			}

			fShowPic(471);
			_currMap = 124;
			break;
		case 0x439:
			_frameStop = 59;
			_fstFwd = false;
			playMovie(174);
			_fstFwd = true;
			playMovie(201);
			fShowPic(471);
			_currMap = 124;
			break;
		case 0x43E:
			_sound = _lastSound;
			if (_pMovie == 173)
				_frameStop = 64;
			if (_pMovie == 201) {
				_frameStop = 59;
				_fstFwd = false;
				playMovie(174);
				_fstFwd = true;
				playMovie(201);
			} else {
				playMovie(_pMovie);
			}
			if (_pMovie != 171) {
				fShowPic(471);
				_currMap = 124;
			}
			break;
		case 0x445:
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x446:
			_statPlay = (_nodes[68] == 1) ? 760 : 660;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 790:
		_nodeNum = 79;
		drawLocation();
		dsAll();
		playMovie(4);
		_currMap = 8;
		_statPlay = 791;
		setATimer();
		_userInput = 0;
		break;
	case 791:
		switch(_userInput) {
		case 0x2F1:
			dsAll();
			playMovie(204);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x445:
			enAll();
			playMovie(10);
			_statPlay = 50;
			_loopFlag = true;
			break;
		case 0x446:
			enAll();
			playMovie(10);
			_statPlay = 520;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	}
}

void KingdomGame::endCredits() {
	_nodeNum = 0;
	_bTimer = 190;
	readMouse();

	while(_bTimer != 0 && _mouseButton == 0) {
		checkTimers();
		refreshSound();
		checkMainScreen();
		readMouse();
	}

	fadeToBlack1();
	drawRect(4, 17, 228, 161, 0);
	playSound(0);
	dsAll();
	_iconsClosed = true;
	playMovie(199);
	dsAll();
	playMovie(205);
	dsAll();
	fadeToBlack1();
	drawRect(4, 17, 228, 161, 0);
	playMovie(201);
	fShowPic(125);
	_currMap = 3;
	dsAll();
	_pouch = false;
	memset(_inventory, 0xFF, 19);
	_statPlay = 994;
	_loopFlag = true;
}
} // NameSpace
