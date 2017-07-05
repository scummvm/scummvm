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
		_NodeNum = 1;
		InitPlay();
		_Replay = false;
		GPLogic1_SubSP10();
		break;
	case 11:
		switch(_UserInput) {
		case 0x190:
			_statPlay = 20;
			_Wizard = false;
			_ATimer = 0;
			break;
		case 0x191:
			_statPlay = 20;
			_Wizard = true;
			_ATimer = 0;
			break;
		case 0x192:
			_ATimer = 0;
			FadeToBlack2();
			PlayMovie(1);
			GPLogic1_SubSP10();
			break;
		case 0x193:
			_ATimer = 0;
			_KeyActive = 0;
			_noIFScreen = true;
			PlaySound(0);
			while(!_KeyActive) {
				FadeToBlack2();
				PlayMovie(54);
			}
			GPLogic1_SubSP10();
			break;
		case 0x194:
			// CHECKME	_QuitFlag = 2;
			_quit = true;
			break;
		case 0x24A:
			FadeToBlack2();
			SetMouse();
			EraseCursor();
			restoreGame();
			break;
		case 0x2F1:
			EraseCursorAsm();
			_CursorDrawn = false;
			FadeToBlack2();
			PlaySound(0);
			PlayMovie(54);
			GPLogic1_SubSP10();
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 20:
		InitPlay();
		_noIFScreen = false;
		EraseCursorAsm();
		_CursorDrawn = false;
		FadeToBlack2();
		ShowPic(106);
		_statPlay = 30;
		_UserInput = 0;
		_loopFlag = true;
		break;
	case 30:
		_NodeNum = 3;
		_UserInput = 0;
		if (_Nodes[3] == 0) {
			PlayMovie(177);
			_Nodes[3] = 9;
			if (!_Wizard) {
				_statPlay = 50;
				_loopFlag = true;
				break;
			}
		}
		DrawLocation();
		if (_Nodes[4] != 0 && _Nodes[6] != 1)
			_sound = true;
		else
			_sound = false;
		
		PlayMovie(7);
		_statPlay = 31;
		_CurrMap = 10;
		PlaySound(24);
		_UserInput = 0;
		break;
	case 31:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 32;
			_Zoom = 2;
			ProcessMap(3, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 32:
		ProcessMapInput(3);
		break;
	case 40:
		_NodeNum = 4;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[4] == 0)
			PlayMovie(62);
		if (_Nodes[4] == 9) {
			PlayMovie(8);
			_TSIconOnly = 1;
		}
		_CurrMap = 10;
		PlaySound(14);
		_statPlay = 41;
		break;
	case 41:
		switch(_UserInput) {
		case 0x429:
			PlayMovie(178);
			PlayMovie(63);
			InventoryDel(1);
			_CurrMap = 10;
			_Nodes[4] = 9;
			_statPlay = 41;
			_TSIconOnly = true;
			break;
		case 0x43A:
			_statPlay = 42;
			_Zoom = 2;
			ProcessMap(4, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 42:
		ProcessMapInput(4);
		break;
	case 50:
		_NodeNum = 5;
		DrawLocation();
		if (_Nodes[5] != 0 && !_Resurrect) {
			_FrameStop = 38;
			PlayMovie(5);
		}
		
		if (_Nodes[5] == 0)
			PlayMovie(5);
		
		_Resurrect = false;
		PlaySound(1);
		
		if (_Spell1 && _Spell2 && _Spell3) {
			_sound = true;
			PlayMovie(2);
			_CurrMap = 10;
			_statPlay = 51;
			_loopFlag = true;
		} else if (_DaelonCntr) {
			PlayMovie(2);
			_CurrMap = 10;
			_statPlay = 51;
		} else {
			switch(_Nodes[5]) {
			case 0:
				InventoryAdd(1);
				break;
			case 1:
				PlayMovie(60);
				InventoryAdd(0);
				_Spell1 = true;
				break;
			case 2:
				PlayMovie(59);
				InventoryAdd(2);
				_Spell2 = true;
				break;
			case 3:
				_sound = true;
				PlayMovie(59);
				InventoryAdd(3);
				_Spell3 = true;
				break;
			}
			if (_Spell1 && _Spell2 && _Spell3) {
				PlayMovie(6);
				_CurrMap = 7;
				_statPlay = 51;
				_loopFlag = true;
			} else {
				_Eye = false;
				PlayMovie(61);
				PlaySound(1);

				if (!_Spell1)
					DrawPic(116);
				
				if (!_Spell2)
					DrawPic(117);
				
				if (!_Spell3)
					DrawPic(118);
				
				_CurrMap = 6;
				_statPlay = 53;
				_loopFlag = true;
			}
		}
		break;
	case 51:
		switch(_UserInput) {
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
			_Zoom = 2;
			ProcessMap(5, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 52:
		ProcessMapInput(5);
		break;
	case 53:
		switch(_UserInput) {
		case 0x428:
			_Nodes[5] = 1;
			_DaelonCntr = 5;
			PlayMovie(6);
			_CurrMap = 7;
			_statPlay = 51;
			_Eye = true;
			break;
		case 0x42A:
			_Nodes[5] = 2;
			_DaelonCntr = 5;
			PlayMovie(6);
			_CurrMap = 7;
			_statPlay = 51;
			_Eye = true;
			break;
		case 0x42B:
			_Nodes[5] = 3;
			_DaelonCntr = 5;
			PlayMovie(6);
			_CurrMap = 7;
			_statPlay = 51;
			_Eye = true;
			break;
		case 0x43E:
			if (!_Spell1 && !_Spell2 && !_Spell3 && !_Wizard) {
				PlayMovie(177);
				DrawLocation();
				PlayMovie(5);
			}

			PlaySound(1);
			_Eye = false;
			PlayMovie(61);

			if (!_Spell1)
				DrawPic(116);

			if (!_Spell2)
				DrawPic(117);

			if (!_Spell3)
				DrawPic(118);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 60:
		_NodeNum = 6;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[4] == 0) {
			DsAll();
			PlayMovie(9);
			_statPlay = 991;
		} else {
			PlayMovie(64);
			_statPlay = 200;
		}
		_CurrMap = 10;
		_loopFlag = true;
		break;
	case 70:
		_NodeNum = 7;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[7] == 0) {
			PlayMovie(11);
			PlaySound(2);
			_CurrMap = 9;
		} else {
			_sound = true;
			PlayMovie(13);
			PlaySound(43);
			_CurrMap = 10;
			_TSIconOnly = 1;
		}
		_statPlay = 71;
		break;
	case 71:
		switch(_UserInput) {
		case 0x42F:
			if (_Nodes[7] == 9) {
				DisplayIcon(137);
			} else {
				PlayMovie(12);
				PlaySound(30);
				InventoryAdd(7);
				_Nodes[7] = 9;
				_CurrMap = 10;
			}
			_UserInput = 0;
			break;
		case 0x43A:
			_statPlay = 72;
			_Zoom = 2;
			ProcessMap(7, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 72:
		ProcessMapInput(7);
		break;
	case 80:
		_NodeNum = 8;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[8] == 9) {
			PlayMovie(24);
			PlaySound(3);
			_CurrMap = 13;
			_TSIconOnly = 1;
		} else {
			_Nodes[8] = 0;
			DsAll();
			PlayMovie(14);
			_CurrMap = 10;
			SetATimer();
		}
		_statPlay = 81;
		break;
	case 81:
		switch(_UserInput) {
		case 0x2F1:
			DsAll();
			PlayMovie(15);
			_statPlay = 991;
			_loopFlag = true;
			break;
		case 0x429:
			if (!_Nodes[8]) {
				_ATimer = 0;
				EnAll();
				PlayMovie(18);
				PlaySound(3);
				InventoryDel(1);
				_UserInput = 0;
				_CurrMap = 13;
				_Nodes[8] = 1;
			}
			break;
		case 0x42A:
			if (!_Nodes[8]) {
				_ATimer = 0;
				EnAll();
				PlayMovie(18);
				PlaySound(3);
				InventoryDel(2);
				_UserInput = 0;
				_CurrMap = 13;
				_Nodes[8] = 1;
			}
			break;
		case 0x42C:
			if (!_Nodes[8]) {
				_ATimer = 0;
				EnAll();
				PlayMovie(16);
				PlaySound(3);
				_Nodes[8] = 9;
				_TSIconOnly = 1;
				_CurrMap = 13;
			}
			break;
		case 0x42D:
			if (!_Nodes[8]) {
				EnAll();
				PlayMovie(17);
				InventoryDel(5);
				PlaySound(3);
				_ATimer = 0;
				_UserInput = 0;
				_CurrMap = 13;
				_Nodes[8] = 1;
			}
			break;
		case 0x43A:
			_statPlay = 82;
			_Zoom = 2;
			ProcessMap(8, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			_ATimer = 0;
			break;
		case 0x453:
			if (_Nodes[8] == 1) {
				_statPlay = 80;
				_loopFlag = true;
			} else {
				_statPlay = 160;
				_loopFlag = true;
				_UserInput = 0;
			}
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 82:
		ProcessMapInput(8);
		break;
	case 90:
		if (_NodeNum == 18 && _Nodes[17] == 0) {
			_statPlay = 170;
			_loopFlag = true;
			_NextNode = 90;
		} else {
			_NodeNum = 9;
			DrawLocation();
			_UserInput = 0;
			if (_Nodes[9] == 0) {
				_sound = false;
				_Nodes[9] = 9;
			} else
				_sound = true;
			
			if (!_Wizard)
				_FrameStop = 90;

			PlayMovie(19);
			PlaySound(15);
			if (!_Wizard)
				ShowPic(109);
			else
				ShowPic(319);

			_CurrMap = 10;
			_statPlay = 91;
		}
		break;
	case 91:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 92;
			_Zoom = 2;
			ProcessMap(9, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			if (!_Wizard)
				_FrameStop = 90;

			_sound = _LastSound;
			PlayMovie(_PMovie);
			if (!_Wizard)
				ShowPic(109);
			else
				ShowPic(319);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 92:
		ProcessMapInput(9);
		break;
	case 100:
		if (_NodeNum == 18 && _Nodes[17] == 0) {
			_statPlay = 170;
			_loopFlag = true;
			_NextNode = 100;
		} else {
			_NodeNum = 10;
			DrawLocation();
			_UserInput = 0;
			if (_Nodes[10] == 1)
				_Nodes[10] = 9;

			if (_Nodes[10] != 9) {
				_sound = false;
				_Nodes[10] = 1;
			} else if (_Wizard)
				_sound = true;

			PlayMovie(20);
			PlaySound(41);
			_CurrMap = 10;
			_statPlay = 101;
		}
		break;
	case 101:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 102;
			_Zoom = 2;
			ProcessMap(10, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 102:
		ProcessMapInput(10);
		break;
	case 110:
		_NodeNum = 11;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[22] == 0)
			_sound = false;
		else
			_sound = true;
		PlayMovie(21);
		PlaySound(42);
		_CurrMap = 10;
		_statPlay = 111;
		break;
	case 111:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 112;
			_Zoom = 1;
			ProcessMap(11, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			PlayMovie(21);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 112:
		ProcessMapInput(11);
		break;
	case 120:
		_NodeNum = 12;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[12] == 0) {
			PlayMovie(22);
			_CurrMap = 14;
		} else {
			PlayMovie(25);
			_CurrMap = 15;
		}
		_Eye = false;
		PlaySound(3);
		_statPlay = 121;
		break;
	case 121:
		switch (_UserInput) {
		case 0x445:
			_statPlay = 150;
			_loopFlag = true;
			_Eye = true;
			_UserInput = 0;
			break;
		case 0x446:
			_statPlay = 80;
			_loopFlag = true;
			_Eye = true;
			_UserInput = 0;
			break;
		case 0x437:
			if (_Nodes[12] == 9) {
				DisplayIcon(145);
				_UserInput = 0;
			} else
				GPLogic1_SubSP121();
			break;
		case 0x43E:
			if (_PMovie == 023)
				GPLogic1_SubSP121();
			else {
				_sound = _LastSound;
				PlayMovie(_PMovie);
				PlaySound(3);
			}
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 130:
		_NodeNum = 13;
		DrawLocation();
		_UserInput = 0;
		DsAll();
		PlayMovie(4);
		_statPlay = 991;
		_loopFlag = true;
		break;
	case 140:
		_NodeNum = 14;
		DrawLocation();
		_UserInput = 0;
		if (Wound()) {
			PlayMovie(26);
			PlaySound(35);
			_CurrMap = 10;
			_statPlay = 141;
		} else {
			DsAll();
			PlayMovie(27);
			_statPlay = 991;
			_loopFlag = true;
			_UserInput = 0;
		}
		break;
	case 141:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 142;
			_Zoom = 2;
			ProcessMap(14, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 142:
		ProcessMapInput(14);
		break;
	case 150:
		_NodeNum = 15;
		DrawLocation();
		PlayMovie(28);
		PlaySound(3);
		_CurrMap = 16;
		_statPlay = 151;
		_Eye = false;
		break;
	case 151:
		switch(_UserInput) {
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			_ATimer = 0;
			break;
		case 0x445:
			_statPlay = 80;
			EnAll();
			_loopFlag = true;
			_UserInput = 0;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 160:
		_NodeNum = 16;
		DrawLocation();
		_UserInput = 0;
		PlayMovie(29);
		PlaySound(3);
		_CurrMap = 85;
		_statPlay = 161;
		break;
	case 161:
		switch(_UserInput) {
		case 0x198: {
			FadeToBlack1();
			DrawRect(4, 17, 228, 161, 0);
			_Replay = true;
			int oldPMovie = _PMovie;
			PlayMovie(208);
			_PMovie = oldPMovie;
			_ASMode = false;
			RestoreAS();
			_Help = true;
			_Eye = true;
			_Pouch = true;
			_CurrMap = 85;
			}
			break;
		case 0x199: {
			FadeToBlack1();
			DrawRect(4, 17, 228, 161, 0);
			_Replay = true;
			int oldPMovie = _PMovie;
			PlayMovie(207);
			_PMovie = oldPMovie;
			_ASMode = false;
			RestoreAS();
			_Help = true;
			_Eye = true;
			_Pouch = true;
			_CurrMap = 85;
			}
			break;
		case 0x19A:
			FShowPic(110);
			_BTimer = 190;
			ReadMouse();
			while(_BTimer != 0 && _MouseButton == 0) {
				checkTimers();
				RefreshSound();
				ReadMouse();
			}
			FShowPic(105);
			break;
		case 0x42E:
			_Nodes[15] = 1;
			_Nodes[16] = 1;
			_statPlay = 120;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 162;
			_Zoom = 2;
			ProcessMap(16, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			if (_PMovie > 206)
				SaveAS();
			_sound = _LastSound;
			PlayMovie(_PMovie);
			if (_PMovie > 206)
				RestoreAS();
			break;
		case 0x444:
			SaveAS();
			_Help = false;
			_Eye = false;
			_Pouch = false;
			_Replay = false;
			FShowPic(105);
			_CurrMap = 86;
			break;
		case 0x445:
			if (_Nodes[16] || !_Wizard) {
				_statPlay = 120;
				_loopFlag = true;
			}
			break;
		case 0x446:
			_statPlay = 80;
			_loopFlag = true;
			_UserInput = 0;
			break;
		case 0x450:
			SaveAS();
			FShowPic(123);
			_BTimer = 76;
			ReadMouse();

			while(_BTimer != 0 && _MouseButton == 0) {
				checkTimers();
				RefreshSound();
				ReadMouse();
			}

			FadeToBlack1();
			DrawRect(4, 17, 228, 161, 0);
			RestoreAS();
			break;	
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 162:
			ProcessMapInput(16);
		break;
	case 170:
		if (_Nodes[17] == 9) {
			_statPlay = _NextNode;
			_loopFlag = true;
		} else {
			_NodeNum = 17;
			DrawLocation();
			DsAll();
			PlayMovie(30);
			_CurrMap = 10;
			_statPlay = 171;
			SetATimer();
			_UserInput = 0;
		}
		break;
	case 171:
		switch(_UserInput) {
		case 0x2F1:
			EnAll();
			if (Wound()) {
				_ATimer = 0;
				_Replay = false;
				PlayMovie(31);
				_statPlay = _NextNode;
				_loopFlag = true;
			} else {
				DsAll();
				PlayMovie(32);
				_statPlay = 991;
				_loopFlag = true;
			}
			break;
		case 0x42C:
			_ATimer = 0;
			_Replay = false;
			EnAll();
			PlayMovie(34);
			_UserInput = 0;
			_Nodes[17] = 9;
			_statPlay = _NextNode;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 172:
		if (_UserInput == 0x2F1) {
			_statPlay = 171;
			_loopFlag= true;
			SwitchAS();
		} else
			ProcessMapInput(_NodeNum);
		break;
	case 180:
		if ((_NodeNum != 9 && _NodeNum != 10) || (_Nodes[17] != 0)) {
			_NodeNum = 18;
			DrawLocation();
			_UserInput = 0;
			if (_Nodes[18] == 0 || _Nodes[18] == 1) {
				_Nodes[18] = 0;
				PlayMovie(33);
				_CurrMap = 10;
			}
			if (_Nodes[18] == 9) {
				PlayMovie(37);
				_CurrMap = 19;
				_TSIconOnly = 1;
			}
			PlaySound(16);
			_statPlay = 181;
		} else {
			_statPlay = 170;
			_loopFlag = true;
			_NextNode = 180;
		}
		break;
	case 181:
		switch(_UserInput) {
		case 0x407:
			_statPlay = 80;
			_loopFlag = true;
			_UserInput = 0;
			break;
		case 0x428:
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			InventoryDel(0);
			_UserInput = 0;
			break;
		case 0x429:
			if (!_Nodes[18]) {
				InventoryDel(1);
				_Nodes[18] = true;
			}
			PlayMovie(35);
			_UserInput = 0;
			break;
		case 0x42A:
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			InventoryDel(2);
			_UserInput = 0;
			break;
		case 0x42C:
			SaveAS();
			PlayMovie(34);
			RestoreAS();
			break;
		case 0x42F:
			if (_Nodes[18] == 1) {
				PlayMovie(36);
				InventoryDel(7);
				_Nodes[18] = 9;
				_CurrMap = 19;
				_TSIconOnly = 1;
			} else {
				_FrameStop = 27;
				_fstFwd = false;
				PlayMovie(36);
				_fstFwd = true;
			}
			_UserInput = 0;
			break;
		case 0x43A:
			_statPlay = 182;
			_Zoom = 2;
			if (_Nodes[18] == 9)
				ProcessMap(80, _Zoom);
			else
				ProcessMap(18, _Zoom);

			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			if (_PMovie == 36 && _Nodes[18] != 9) {
				_FrameStop = 27;
				_fstFwd = false;
				PlayMovie(36);
				_fstFwd = true;
			} else
				PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 182:
		ProcessMapInput(18);
		break;
	case 190:
		_NodeNum = 19;
		DrawLocation();
		_UserInput = 0;
		DsAll();
		PlayMovie(38);
		_statPlay = 991;
		_loopFlag = true;
		break;
	case 200:
		_NodeNum = 20;
		DrawLocation();
		_UserInput = 0;
		if (!_Nodes[20]) {
			PlayMovie(39);
			IncreaseHealth();
		} else {
			PlayMovie(40);
			_TideCntl = true;
			DrawPic(178);
		}
		PlaySound(42);
		_CurrMap = 10;
		_statPlay = 201;
		break;
	case 201:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 202;
			_Zoom = 0;
			ProcessMap(20, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 202:
		ProcessMapInput(20);
		break;
	case 210:
		_NodeNum = 21;
		DrawLocation();
		_UserInput = 0;
		if (!_Wizard)
			_Nodes[21] = 9;

		if (_Nodes[21] == 9) {
			_Nodes[21] = 9;
			PlayMovie(209);
			PlaySound(38);
			_CurrMap = 81;
		} else {
			DsAll();
			PlayMovie(45);
			_Nodes[21] = 0;
			_CurrMap = 10;
			SetATimer();
		}
		_statPlay = 211;
		break;
	case 211:
		switch(_UserInput) {
		case 0x2F1:
			DsAll();
			PlayMovie(46);
			_statPlay = 991;
			_loopFlag = true;
			break;
		case 0x428:
			InventoryDel(0);
			if (_Nodes[21] == 9) {
				SaveAS();
				PlayMovie(65);
				RestoreAS();
				_UserInput = 0;
			} else {
				PlayMovie(178);
				DsAll();
				PlayMovie(46);
				_statPlay = 991;
				_loopFlag = true;
			}
			break;
		case 0x42A:
			if (!_Nodes[21]) {
				_ATimer = 0;
				EnAll();
				PlayMovie(48);
				InventoryDel(2);
				_Nodes[21] = 1;
				_TSIconOnly = 1;
				PlaySound(38);
				_UserInput = 0;
			}
			break;
		case 0x42D:
			if (!_Nodes[21]) {
				_ATimer = 0;
				EnAll();
				PlayMovie(47);
				PlaySound(38);
				InventoryDel(5);
				_Nodes[21] = 9;
				_UserInput = 0;
				_CurrMap = 81;
				_TSIconOnly = 1;
			}
			break;
		case 0x43A:
			_statPlay = 212;
			_Zoom = 2;
			ProcessMap(21, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			if (_PMovie == 65) {
				SaveAS();
				PlayMovie(65);
				RestoreAS();
				_UserInput = 0;
			} else {
				_sound = _LastSound;
				PlayMovie(_PMovie);
			}
			break;
		case 0x441:
			_statPlay = 220;
			_loopFlag = true;
			_UserInput = 0;
			break;
		case 0x442:
			if (_TSIconOnly != 0) {
				_ATimer = 0;
				EnAll();
				PlayMovie(49);
				_TSIconOnly = 0;
				PlaySound(38);
				_UserInput = 0;
			}
			break;
		case 0x448:
			_statPlay = 140;
			_loopFlag = true;
			_UserInput = 0;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 212:
		if (_UserInput == 0x2F1) {
			_UserInput = 0;
			_ATimer = 0;
			_statPlay = 211;
			_loopFlag = true;
			SwitchAS();
		}
		ProcessMapInput(21);
		break;
	case 220:
		_NodeNum = 22;
		DrawLocation();
		_UserInput = 0;
		if (!_Nodes[22]) {
			PlayMovie(41);
			_CurrMap = 82;
		} else {
			_TSIconOnly = 1;
			PlayMovie(44);
			_CurrMap = 10;
		}
		PlaySound(38);
		_statPlay = 221;
		break;
	case 221:
		switch(_UserInput) {
		case 0x428:
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			InventoryDel(0);
			_UserInput = 0;
			break;
		case 0x429:
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			InventoryDel(1);
			_UserInput = 0;
			break;
		case 0x42A:
			PlayMovie(42);
			PlaySound(30);
			if (!_Nodes[22]) {
				InventoryDel(2);
				InventoryAdd(4);
				_TSIconOnly = 1;
			}
			_UserInput = 0;
			_CurrMap = 10;
			break;
		case 0x42C:
			if (!_Nodes[22]) {
				DsAll();
				PlayMovie(43);
				PlaySound(34);
				_statPlay = 991;
				_loopFlag = true;
			} else {
				PlayMovie(42);
				PlaySound(30);
			}
			break;
		case 0x43A:
			_statPlay = 222;
			_Zoom = 2;
			ProcessMap(22, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 222:
		ProcessMapInput(22);
		break;
	case 230:
		_NodeNum = 23;
		DrawLocation();
		_UserInput = 0;
		DsAll();
		PlayMovie(50);
		_statPlay = 991;
		_loopFlag = true;
		break;
	case 240:
		_NodeNum = 24;
		DrawLocation();
		_UserInput = 0;
		if (!_Nodes[24]) {
			PlayMovie(51);
			_CurrMap = 83;
		} else {
			PlayMovie(53);
			ShowPic(119);
			_CurrMap = 84;
			_TSIconOnly = 1;
		}
		PlaySound(4);
		_statPlay = 241;
		break;
	case 241:
		switch(_UserInput) {
		case 0x428:
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			InventoryDel(0);
			break;
		case 0x429:
			if (_Nodes[24] != 9) {
				_TSIconOnly = 1;
				PlayMovie(66);
				InventoryDel(1);
				_CurrMap = 84;
				_Nodes[24] = 9;
			}
			break;
		case 0x42A:
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			InventoryDel(2);
			break;
		case 0x42D:
			if (_Inventory[5] >= 1)
				DisplayIcon(135);
			else {
				SaveAS();
				PlayMovie(52);
				PlaySound(30);
				RestoreAS();
				InventoryAdd(5);
				_UserInput = 0;
			}
			break;
		case 0x43A:
			_statPlay = 242;
			_Zoom = 2;
			ProcessMap(24, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			if (_PMovie == 53) {
				PlayMovie(53);
				ShowPic(119);
				_CurrMap = 84;
			} else
				PlayMovie(_PMovie);
			break;
		case 0x441:
			_CurrMap = 10;
			_statPlay = 300;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		_UserInput = 0;
		break;
	case 242:
		ProcessMapInput(24);
		break;
	}
}

void KingdomGame::GPLogic1_SubSP10() {
	PlaySound(0);
	PlaySound(18);
	FadeToBlack2();
	ShowPic(115);
	ShowPic(114);

	_CurrMap = 4;
	_UserInput = 0;
	_ATimer = 595;
	_statPlay = 11;
}

void KingdomGame::GPLogic1_SubSP121() {
	PlayMovie(23);
	PlaySound(30);
	_BTimer = 190;
	ReadMouse();
	
	while (_BTimer != 0 && _MouseButton == 0) {
		checkTimers();
		RefreshSound();
		CheckMainScreen();
		ReadMouse();
	}
	
	PlaySound(23);
	InventoryAdd(15);
	_CurrMap = 10;
	FShowPic(150);
	_Nodes[12] = 9;
	PlaySound(3);
	_CurrMap = 15;
}

} // NameSpace
