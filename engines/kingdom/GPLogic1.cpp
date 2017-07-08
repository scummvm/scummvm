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

void KingdomGame::GPLogic1() {
	switch(_statPlay) {
	case 10:
		_nodeNum = 1;
		initPlay();
		_replay = false;
		GPLogic1_SubSP10();
		break;
	case 11:
		switch(_userInput) {
		case 0x190:
			_statPlay = 20;
			_wizard = false;
			_aTimer = 0;
			break;
		case 0x191:
			_statPlay = 20;
			_wizard = true;
			_aTimer = 0;
			break;
		case 0x192:
			_aTimer = 0;
			fadeToBlack2();
			playMovie(1);
			GPLogic1_SubSP10();
			break;
		case 0x193:
			_aTimer = 0;
			_keyActive = false;
			_noIFScreen = true;
			playSound(0);
			while(!_keyActive) {
				fadeToBlack2();
				playMovie(54);
			}
			GPLogic1_SubSP10();
			break;
		case 0x194:
			// CHECKME	_QuitFlag = 2;
			_quit = true;
			break;
		case 0x24A:
			fadeToBlack2();
			setMouse();
			EraseCursor();
			restoreGame();
			break;
		case 0x2F1:
			EraseCursorAsm();
			_cursorDrawn = false;
			fadeToBlack2();
			playSound(0);
			playMovie(54);
			GPLogic1_SubSP10();
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 20:
		initPlay();
		_noIFScreen = false;
		EraseCursorAsm();
		_cursorDrawn = false;
		fadeToBlack2();
		showPic(106);
		_statPlay = 30;
		_userInput = 0;
		_loopFlag = true;
		break;
	case 30:
		_nodeNum = 3;
		_userInput = 0;
		if (_nodes[3] == 0) {
			playMovie(177);
			_nodes[3] = 9;
			if (!_wizard) {
				_statPlay = 50;
				_loopFlag = true;
				break;
			}
		}
		DrawLocation();
		if (_nodes[4] != 0 && _nodes[6] != 1)
			_sound = true;
		else
			_sound = false;
		
		playMovie(7);
		_statPlay = 31;
		_currMap = 10;
		playSound(24);
		_userInput = 0;
		break;
	case 31:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 32;
			_zoom = 2;
			ProcessMap(3, _zoom);
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
	case 32:
		ProcessMapInput(3);
		break;
	case 40:
		_nodeNum = 4;
		DrawLocation();
		_userInput = 0;
		if (_nodes[4] == 0)
			playMovie(62);
		if (_nodes[4] == 9) {
			playMovie(8);
			_tsIconOnly = true;
		}
		_currMap = 10;
		playSound(14);
		_statPlay = 41;
		break;
	case 41:
		switch(_userInput) {
		case 0x429:
			playMovie(178);
			playMovie(63);
			InventoryDel(1);
			_currMap = 10;
			_nodes[4] = 9;
			_statPlay = 41;
			_tsIconOnly = true;
			break;
		case 0x43A:
			_statPlay = 42;
			_zoom = 2;
			ProcessMap(4, _zoom);
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
	case 42:
		ProcessMapInput(4);
		break;
	case 50:
		_nodeNum = 5;
		DrawLocation();
		if (_nodes[5] != 0 && !_resurrect) {
			_frameStop = 38;
			playMovie(5);
		}
		
		if (_nodes[5] == 0)
			playMovie(5);
		
		_resurrect = false;
		playSound(1);
		
		if (_spell1 && _spell2 && _spell3) {
			_sound = true;
			playMovie(2);
			_currMap = 10;
			_statPlay = 51;
			_loopFlag = true;
		} else if (_daelonCntr) {
			playMovie(2);
			_currMap = 10;
			_statPlay = 51;
		} else {
			switch(_nodes[5]) {
			case 0:
				InventoryAdd(1);
				break;
			case 1:
				playMovie(60);
				InventoryAdd(0);
				_spell1 = true;
				break;
			case 2:
				playMovie(59);
				InventoryAdd(2);
				_spell2 = true;
				break;
			case 3:
				_sound = true;
				playMovie(59);
				InventoryAdd(3);
				_spell3 = true;
				break;
			}
			if (_spell1 && _spell2 && _spell3) {
				playMovie(6);
				_currMap = 7;
				_statPlay = 51;
				_loopFlag = true;
			} else {
				_eye = false;
				playMovie(61);
				playSound(1);

				if (!_spell1)
					DrawPic(116);
				
				if (!_spell2)
					DrawPic(117);
				
				if (!_spell3)
					DrawPic(118);
				
				_currMap = 6;
				_statPlay = 53;
				_loopFlag = true;
			}
		}
		break;
	case 51:
		switch(_userInput) {
		case 0x406:
			_statPlay = 70;
			_loopFlag = true;
			break;
		case 0x40A:
			_statPlay = 110;
			_loopFlag = true;
			break;
		case 0x417:
			_statPlay = 30;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 52;
			_zoom = 2;
			ProcessMap(5, _zoom);
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
	case 52:
		ProcessMapInput(5);
		break;
	case 53:
		switch(_userInput) {
		case 0x428:
			_nodes[5] = 1;
			_daelonCntr = 5;
			playMovie(6);
			_currMap = 7;
			_statPlay = 51;
			_eye = true;
			break;
		case 0x42A:
			_nodes[5] = 2;
			_daelonCntr = 5;
			playMovie(6);
			_currMap = 7;
			_statPlay = 51;
			_eye = true;
			break;
		case 0x42B:
			_nodes[5] = 3;
			_daelonCntr = 5;
			playMovie(6);
			_currMap = 7;
			_statPlay = 51;
			_eye = true;
			break;
		case 0x43E:
			if (!_spell1 && !_spell2 && !_spell3 && !_wizard) {
				playMovie(177);
				DrawLocation();
				playMovie(5);
			}

			playSound(1);
			_eye = false;
			playMovie(61);

			if (!_spell1)
				DrawPic(116);

			if (!_spell2)
				DrawPic(117);

			if (!_spell3)
				DrawPic(118);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 60:
		_nodeNum = 6;
		DrawLocation();
		_userInput = 0;
		if (_nodes[4] == 0) {
			dsAll();
			playMovie(9);
			_statPlay = 991;
		} else {
			playMovie(64);
			_statPlay = 200;
		}
		_currMap = 10;
		_loopFlag = true;
		break;
	case 70:
		_nodeNum = 7;
		DrawLocation();
		_userInput = 0;
		if (_nodes[7] == 0) {
			playMovie(11);
			playSound(2);
			_currMap = 9;
		} else {
			_sound = true;
			playMovie(13);
			playSound(43);
			_currMap = 10;
			_tsIconOnly = true;
		}
		_statPlay = 71;
		break;
	case 71:
		switch(_userInput) {
		case 0x42F:
			if (_nodes[7] == 9) {
				DisplayIcon(137);
			} else {
				playMovie(12);
				playSound(30);
				InventoryAdd(7);
				_nodes[7] = 9;
				_currMap = 10;
			}
			_userInput = 0;
			break;
		case 0x43A:
			_statPlay = 72;
			_zoom = 2;
			ProcessMap(7, _zoom);
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
	case 72:
		ProcessMapInput(7);
		break;
	case 80:
		_nodeNum = 8;
		DrawLocation();
		_userInput = 0;
		if (_nodes[8] == 9) {
			playMovie(24);
			playSound(3);
			_currMap = 13;
			_tsIconOnly = true;
		} else {
			_nodes[8] = 0;
			dsAll();
			playMovie(14);
			_currMap = 10;
			SetATimer();
		}
		_statPlay = 81;
		break;
	case 81:
		switch(_userInput) {
		case 0x2F1:
			dsAll();
			playMovie(15);
			_statPlay = 991;
			_loopFlag = true;
			break;
		case 0x429:
			if (!_nodes[8]) {
				_aTimer = 0;
				enAll();
				playMovie(18);
				playSound(3);
				InventoryDel(1);
				_userInput = 0;
				_currMap = 13;
				_nodes[8] = 1;
			}
			break;
		case 0x42A:
			if (!_nodes[8]) {
				_aTimer = 0;
				enAll();
				playMovie(18);
				playSound(3);
				InventoryDel(2);
				_userInput = 0;
				_currMap = 13;
				_nodes[8] = 1;
			}
			break;
		case 0x42C:
			if (!_nodes[8]) {
				_aTimer = 0;
				enAll();
				playMovie(16);
				playSound(3);
				_nodes[8] = 9;
				_tsIconOnly = true;
				_currMap = 13;
			}
			break;
		case 0x42D:
			if (!_nodes[8]) {
				enAll();
				playMovie(17);
				InventoryDel(5);
				playSound(3);
				_aTimer = 0;
				_userInput = 0;
				_currMap = 13;
				_nodes[8] = 1;
			}
			break;
		case 0x43A:
			_statPlay = 82;
			_zoom = 2;
			ProcessMap(8, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			_aTimer = 0;
			break;
		case 0x453:
			if (_nodes[8] == 1) {
				_statPlay = 80;
				_loopFlag = true;
			} else {
				_statPlay = 160;
				_loopFlag = true;
				_userInput = 0;
			}
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 82:
		ProcessMapInput(8);
		break;
	case 90:
		if (_nodeNum == 18 && _nodes[17] == 0) {
			_statPlay = 170;
			_loopFlag = true;
			_nextNode = 90;
		} else {
			_nodeNum = 9;
			DrawLocation();
			_userInput = 0;
			if (_nodes[9] == 0) {
				_sound = false;
				_nodes[9] = 9;
			} else
				_sound = true;
			
			if (!_wizard)
				_frameStop = 90;

			playMovie(19);
			playSound(15);
			if (!_wizard)
				showPic(109);
			else
				showPic(319);

			_currMap = 10;
			_statPlay = 91;
		}
		break;
	case 91:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 92;
			_zoom = 2;
			ProcessMap(9, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			if (!_wizard)
				_frameStop = 90;

			_sound = _lastSound;
			playMovie(_pMovie);
			if (!_wizard)
				showPic(109);
			else
				showPic(319);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 92:
		ProcessMapInput(9);
		break;
	case 100:
		if (_nodeNum == 18 && _nodes[17] == 0) {
			_statPlay = 170;
			_loopFlag = true;
			_nextNode = 100;
		} else {
			_nodeNum = 10;
			DrawLocation();
			_userInput = 0;
			if (_nodes[10] == 1)
				_nodes[10] = 9;

			if (_nodes[10] != 9) {
				_sound = false;
				_nodes[10] = 1;
			} else if (_wizard)
				_sound = true;

			playMovie(20);
			playSound(41);
			_currMap = 10;
			_statPlay = 101;
		}
		break;
	case 101:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 102;
			_zoom = 2;
			ProcessMap(10, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 102:
		ProcessMapInput(10);
		break;
	case 110:
		_nodeNum = 11;
		DrawLocation();
		_userInput = 0;
		if (_nodes[22] == 0)
			_sound = false;
		else
			_sound = true;
		playMovie(21);
		playSound(42);
		_currMap = 10;
		_statPlay = 111;
		break;
	case 111:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 112;
			_zoom = 1;
			ProcessMap(11, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			playMovie(21);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 112:
		ProcessMapInput(11);
		break;
	case 120:
		_nodeNum = 12;
		DrawLocation();
		_userInput = 0;
		if (_nodes[12] == 0) {
			playMovie(22);
			_currMap = 14;
		} else {
			playMovie(25);
			_currMap = 15;
		}
		_eye = false;
		playSound(3);
		_statPlay = 121;
		break;
	case 121:
		switch (_userInput) {
		case 0x445:
			_statPlay = 150;
			_loopFlag = true;
			_eye = true;
			_userInput = 0;
			break;
		case 0x446:
			_statPlay = 80;
			_loopFlag = true;
			_eye = true;
			_userInput = 0;
			break;
		case 0x437:
			if (_nodes[12] == 9) {
				DisplayIcon(145);
				_userInput = 0;
			} else
				GPLogic1_SubSP121();
			break;
		case 0x43E:
			if (_pMovie == 023)
				GPLogic1_SubSP121();
			else {
				_sound = _lastSound;
				playMovie(_pMovie);
				playSound(3);
			}
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 130:
		_nodeNum = 13;
		DrawLocation();
		_userInput = 0;
		dsAll();
		playMovie(4);
		_statPlay = 991;
		_loopFlag = true;
		break;
	case 140:
		_nodeNum = 14;
		DrawLocation();
		_userInput = 0;
		if (Wound()) {
			playMovie(26);
			playSound(35);
			_currMap = 10;
			_statPlay = 141;
		} else {
			dsAll();
			playMovie(27);
			_statPlay = 991;
			_loopFlag = true;
			_userInput = 0;
		}
		break;
	case 141:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 142;
			_zoom = 2;
			ProcessMap(14, _zoom);
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
	case 142:
		ProcessMapInput(14);
		break;
	case 150:
		_nodeNum = 15;
		DrawLocation();
		playMovie(28);
		playSound(3);
		_currMap = 16;
		_statPlay = 151;
		_eye = false;
		break;
	case 151:
		switch(_userInput) {
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			_aTimer = 0;
			break;
		case 0x445:
			_statPlay = 80;
			enAll();
			_loopFlag = true;
			_userInput = 0;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 160:
		_nodeNum = 16;
		DrawLocation();
		_userInput = 0;
		playMovie(29);
		playSound(3);
		_currMap = 85;
		_statPlay = 161;
		break;
	case 161:
		switch(_userInput) {
		case 0x198: {
			fadeToBlack1();
			drawRect(4, 17, 228, 161, 0);
			_replay = true;
			int oldPMovie = _pMovie;
			playMovie(208);
			_pMovie = oldPMovie;
			_asMode = false;
			restoreAS();
			_help = true;
			_eye = true;
			_pouch = true;
			_currMap = 85;
			}
			break;
		case 0x199: {
			fadeToBlack1();
			drawRect(4, 17, 228, 161, 0);
			_replay = true;
			int oldPMovie = _pMovie;
			playMovie(207);
			_pMovie = oldPMovie;
			_asMode = false;
			restoreAS();
			_help = true;
			_eye = true;
			_pouch = true;
			_currMap = 85;
			}
			break;
		case 0x19A:
			fShowPic(110);
			_bTimer = 190;
			readMouse();
			while(_bTimer != 0 && _mouseButton == 0) {
				checkTimers();
				RefreshSound();
				readMouse();
			}
			fShowPic(105);
			break;
		case 0x42E:
			_nodes[15] = 1;
			_nodes[16] = 1;
			_statPlay = 120;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 162;
			_zoom = 2;
			ProcessMap(16, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			if (_pMovie > 206)
				saveAS();
			_sound = _lastSound;
			playMovie(_pMovie);
			if (_pMovie > 206)
				restoreAS();
			break;
		case 0x444:
			saveAS();
			_help = false;
			_eye = false;
			_pouch = false;
			_replay = false;
			fShowPic(105);
			_currMap = 86;
			break;
		case 0x445:
			if (_nodes[16] || !_wizard) {
				_statPlay = 120;
				_loopFlag = true;
			}
			break;
		case 0x446:
			_statPlay = 80;
			_loopFlag = true;
			_userInput = 0;
			break;
		case 0x450:
			saveAS();
			fShowPic(123);
			_bTimer = 76;
			readMouse();

			while(_bTimer != 0 && _mouseButton == 0) {
				checkTimers();
				RefreshSound();
				readMouse();
			}

			fadeToBlack1();
			drawRect(4, 17, 228, 161, 0);
			restoreAS();
			break;	
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 162:
			ProcessMapInput(16);
		break;
	case 170:
		if (_nodes[17] == 9) {
			_statPlay = _nextNode;
			_loopFlag = true;
		} else {
			_nodeNum = 17;
			DrawLocation();
			dsAll();
			playMovie(30);
			_currMap = 10;
			_statPlay = 171;
			SetATimer();
			_userInput = 0;
		}
		break;
	case 171:
		switch(_userInput) {
		case 0x2F1:
			enAll();
			if (Wound()) {
				_aTimer = 0;
				_replay = false;
				playMovie(31);
				_statPlay = _nextNode;
				_loopFlag = true;
			} else {
				dsAll();
				playMovie(32);
				_statPlay = 991;
				_loopFlag = true;
			}
			break;
		case 0x42C:
			_aTimer = 0;
			_replay = false;
			enAll();
			playMovie(34);
			_userInput = 0;
			_nodes[17] = 9;
			_statPlay = _nextNode;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 172:
		if (_userInput == 0x2F1) {
			_statPlay = 171;
			_loopFlag= true;
			SwitchAS();
		} else
			ProcessMapInput(_nodeNum);
		break;
	case 180:
		if ((_nodeNum != 9 && _nodeNum != 10) || (_nodes[17] != 0)) {
			_nodeNum = 18;
			DrawLocation();
			_userInput = 0;
			if (_nodes[18] == 0 || _nodes[18] == 1) {
				_nodes[18] = 0;
				playMovie(33);
				_currMap = 10;
			}
			if (_nodes[18] == 9) {
				playMovie(37);
				_currMap = 19;
				_tsIconOnly = true;
			}
			playSound(16);
			_statPlay = 181;
		} else {
			_statPlay = 170;
			_loopFlag = true;
			_nextNode = 180;
		}
		break;
	case 181:
		switch(_userInput) {
		case 0x407:
			_statPlay = 80;
			_loopFlag = true;
			_userInput = 0;
			break;
		case 0x428:
			saveAS();
			playMovie(178);
			restoreAS();
			InventoryDel(0);
			_userInput = 0;
			break;
		case 0x429:
			if (!_nodes[18]) {
				InventoryDel(1);
				_nodes[18] = true;
			}
			playMovie(35);
			_userInput = 0;
			break;
		case 0x42A:
			saveAS();
			playMovie(178);
			restoreAS();
			InventoryDel(2);
			_userInput = 0;
			break;
		case 0x42C:
			saveAS();
			playMovie(34);
			restoreAS();
			break;
		case 0x42F:
			if (_nodes[18] == 1) {
				playMovie(36);
				InventoryDel(7);
				_nodes[18] = 9;
				_currMap = 19;
				_tsIconOnly = true;
			} else {
				_frameStop = 27;
				_fstFwd = false;
				playMovie(36);
				_fstFwd = true;
			}
			_userInput = 0;
			break;
		case 0x43A:
			_statPlay = 182;
			_zoom = 2;
			if (_nodes[18] == 9)
				ProcessMap(80, _zoom);
			else
				ProcessMap(18, _zoom);

			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			if (_pMovie == 36 && _nodes[18] != 9) {
				_frameStop = 27;
				_fstFwd = false;
				playMovie(36);
				_fstFwd = true;
			} else
				playMovie(_pMovie);
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 182:
		ProcessMapInput(18);
		break;
	case 190:
		_nodeNum = 19;
		DrawLocation();
		_userInput = 0;
		dsAll();
		playMovie(38);
		_statPlay = 991;
		_loopFlag = true;
		break;
	case 200:
		_nodeNum = 20;
		DrawLocation();
		_userInput = 0;
		if (!_nodes[20]) {
			playMovie(39);
			IncreaseHealth();
		} else {
			playMovie(40);
			_tideCntl = true;
			DrawPic(178);
		}
		playSound(42);
		_currMap = 10;
		_statPlay = 201;
		break;
	case 201:
		switch(_userInput) {
		case 0x43A:
			_statPlay = 202;
			_zoom = 0;
			ProcessMap(20, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			playMovie(_pMovie);
			break;
		}
		break;
	case 202:
		ProcessMapInput(20);
		break;
	case 210:
		_nodeNum = 21;
		DrawLocation();
		_userInput = 0;
		if (!_wizard)
			_nodes[21] = 9;

		if (_nodes[21] == 9) {
			_nodes[21] = 9;
			playMovie(209);
			playSound(38);
			_currMap = 81;
		} else {
			dsAll();
			playMovie(45);
			_nodes[21] = 0;
			_currMap = 10;
			SetATimer();
		}
		_statPlay = 211;
		break;
	case 211:
		switch(_userInput) {
		case 0x2F1:
			dsAll();
			playMovie(46);
			_statPlay = 991;
			_loopFlag = true;
			break;
		case 0x428:
			InventoryDel(0);
			if (_nodes[21] == 9) {
				saveAS();
				playMovie(65);
				restoreAS();
				_userInput = 0;
			} else {
				playMovie(178);
				dsAll();
				playMovie(46);
				_statPlay = 991;
				_loopFlag = true;
			}
			break;
		case 0x42A:
			if (!_nodes[21]) {
				_aTimer = 0;
				enAll();
				playMovie(48);
				InventoryDel(2);
				_nodes[21] = 1;
				_tsIconOnly = true;
				playSound(38);
				_userInput = 0;
			}
			break;
		case 0x42D:
			if (!_nodes[21]) {
				_aTimer = 0;
				enAll();
				playMovie(47);
				playSound(38);
				InventoryDel(5);
				_nodes[21] = 9;
				_userInput = 0;
				_currMap = 81;
				_tsIconOnly = true;
			}
			break;
		case 0x43A:
			_statPlay = 212;
			_zoom = 2;
			ProcessMap(21, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			if (_pMovie == 65) {
				saveAS();
				playMovie(65);
				restoreAS();
				_userInput = 0;
			} else {
				_sound = _lastSound;
				playMovie(_pMovie);
			}
			break;
		case 0x441:
			_statPlay = 220;
			_loopFlag = true;
			_userInput = 0;
			break;
		case 0x442:
			if (_tsIconOnly) {
				_aTimer = 0;
				enAll();
				playMovie(49);
				_tsIconOnly = false;
				playSound(38);
				_userInput = 0;
			}
			break;
		case 0x448:
			_statPlay = 140;
			_loopFlag = true;
			_userInput = 0;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		break;
	case 212:
		if (_userInput == 0x2F1) {
			_userInput = 0;
			_aTimer = 0;
			_statPlay = 211;
			_loopFlag = true;
			SwitchAS();
		}
		ProcessMapInput(21);
		break;
	case 220:
		_nodeNum = 22;
		DrawLocation();
		_userInput = 0;
		if (!_nodes[22]) {
			playMovie(41);
			_currMap = 82;
		} else {
			_tsIconOnly = true;
			playMovie(44);
			_currMap = 10;
		}
		playSound(38);
		_statPlay = 221;
		break;
	case 221:
		switch(_userInput) {
		case 0x428:
			saveAS();
			playMovie(178);
			restoreAS();
			InventoryDel(0);
			_userInput = 0;
			break;
		case 0x429:
			saveAS();
			playMovie(178);
			restoreAS();
			InventoryDel(1);
			_userInput = 0;
			break;
		case 0x42A:
			playMovie(42);
			playSound(30);
			if (!_nodes[22]) {
				InventoryDel(2);
				InventoryAdd(4);
				_tsIconOnly = true;
			}
			_userInput = 0;
			_currMap = 10;
			break;
		case 0x42C:
			if (!_nodes[22]) {
				dsAll();
				playMovie(43);
				playSound(34);
				_statPlay = 991;
				_loopFlag = true;
			} else {
				playMovie(42);
				playSound(30);
			}
			break;
		case 0x43A:
			_statPlay = 222;
			_zoom = 2;
			ProcessMap(22, _zoom);
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
	case 222:
		ProcessMapInput(22);
		break;
	case 230:
		_nodeNum = 23;
		DrawLocation();
		_userInput = 0;
		dsAll();
		playMovie(50);
		_statPlay = 991;
		_loopFlag = true;
		break;
	case 240:
		_nodeNum = 24;
		DrawLocation();
		_userInput = 0;
		if (!_nodes[24]) {
			playMovie(51);
			_currMap = 83;
		} else {
			playMovie(53);
			showPic(119);
			_currMap = 84;
			_tsIconOnly = true;
		}
		playSound(4);
		_statPlay = 241;
		break;
	case 241:
		switch(_userInput) {
		case 0x428:
			saveAS();
			playMovie(178);
			restoreAS();
			InventoryDel(0);
			break;
		case 0x429:
			if (_nodes[24] != 9) {
				_tsIconOnly = true;
				playMovie(66);
				InventoryDel(1);
				_currMap = 84;
				_nodes[24] = 9;
			}
			break;
		case 0x42A:
			saveAS();
			playMovie(178);
			restoreAS();
			InventoryDel(2);
			break;
		case 0x42D:
			if (_inventory[5] >= 1)
				DisplayIcon(135);
			else {
				saveAS();
				playMovie(52);
				playSound(30);
				restoreAS();
				InventoryAdd(5);
				_userInput = 0;
			}
			break;
		case 0x43A:
			_statPlay = 242;
			_zoom = 2;
			ProcessMap(24, _zoom);
			_userInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			if (_pMovie == 53) {
				playMovie(53);
				showPic(119);
				_currMap = 84;
			} else
				playMovie(_pMovie);
			break;
		case 0x441:
			_currMap = 10;
			_statPlay = 300;
			_loopFlag = true;
			break;
		default:
			if (_userInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _userInput, _userInput, _statPlay);
			break;
		}
		_userInput = 0;
		break;
	case 242:
		ProcessMapInput(24);
		break;
	}
}

void KingdomGame::GPLogic1_SubSP10() {
	playSound(0);
	playSound(18);
	fadeToBlack2();
	showPic(115);
	showPic(114);

	_currMap = 4;
	_userInput = 0;
	_aTimer = 595;
	_statPlay = 11;
}

void KingdomGame::GPLogic1_SubSP121() {
	playMovie(23);
	playSound(30);
	_bTimer = 190;
	readMouse();
	
	while (_bTimer != 0 && _mouseButton == 0) {
		checkTimers();
		RefreshSound();
		CheckMainScreen();
		readMouse();
	}
	
	playSound(23);
	InventoryAdd(15);
	_currMap = 10;
	fShowPic(150);
	_nodes[12] = 9;
	playSound(3);
	_currMap = 15;
}

} // NameSpace
