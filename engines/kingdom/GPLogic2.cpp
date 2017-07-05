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
		bool highNode = (_NodeNum > 25);
		_NodeNum = 25;

		DrawLocation();
		DsAll();
		if (_Nodes[24] != 0 || highNode) {
			_Eye = true;
			PlayMovie(72);
			_aTimerFlag = false;
			_aTimer = 114;
			PlaySound(34);
			_UserInput = 0;
			_CurrMap = 10;
			_statPlay = 251;
		} else {
			_frameStop = 67;
			PlayMovie(72);
			DsAll();
			PlayMovie(73);
			DsAll();
			_statPlay = 991;
			_loopFlag = true;
		}
		}
		break;
	case 251:
		switch(_UserInput) {
		case 0x2F1:
			_aTimer = 0;
			PlayMovie(73);
			DsAll();
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 252;
			_Zoom = 2;
			ProcessMap(30, _Zoom);
			SetATimer();
			_UserInput = 0;
			break;
		}
		break;
	case 252:
		ProcessMapInput(30);
		break;
	case 260:
		_NodeNum = 26;
		DrawLocation();
		DsAll();
		PlayMovie(74);
		_CurrMap = 10;
		SetATimer();
		_statPlay = 261;
		_UserInput = 0;
		break;
	case 261:
		switch(_UserInput) {
		case 0x2F1:
			_aTimer = 0;
			DsAll();
			PlayMovie(75);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x428:
			InventoryDel(0);
			_aTimer = 0;
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			SetATimer();
			_UserInput = 0;
			break;
		case 0x42A:
			InventoryDel(2);
			_aTimer = 0;
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			SetATimer();
			_UserInput = 0;
			break;
		case 0x42C:
			_aTimer = 0;
			EnAll();
			PlayMovie(76);
			_Nodes[26] = 1;
			_UserInput = 0;
			_statPlay = 290;
			_lastObs = true;
			_LastObstacle = 29;
			_loopFlag = true;
			break;
		case 0x42D:
			if (Wound()) {
				_aTimer = 0;
				EnAll();
				PlayMovie(77);
				PlaySound(34);
				InventoryDel(5);
				_statPlay = 300;
				_loopFlag = true;
			} else {
				DsAll();
				PlayMovie(78);
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		}
		break;
	case 270:
		_NodeNum = 27;
		DrawLocation();
		if (_Nodes[28] == 1)
			_Nodes[28] = 9;
		if (_Nodes[28] == 9 || _Nodes[29] != 0) {
			if (_Wizard)
				_sound = true;
		} else {
			_sound = false;
			_Nodes[28] = 1;
		}
		PlayMovie(80);
		PlaySound(18);
		_CurrMap = 10;
		_statPlay = 271;
		_Nodes[28] = 1;
		_UserInput = 0;
		break;
	case 271:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 272;
			_Zoom = 2;
			ProcessMap(27, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 272:
		ProcessMapInput(27);
		break;
	case 280:
		_NodeNum = 28;
		DrawLocation();
		_UserInput = 0;
		DsAll();
		_fstFwd = false;
		PlayMovie(81);
		_CurrMap = 10;
		_Nodes[29] = _Nodes[39] = 1;
		_statPlay = _RobberyNode * 10;
		EnAll();
		_loopFlag = true;
		break;
	case 290:
		_OldNode = _NodeNum;
		if (!_Wizard) {
			_Nodes[26] = 9;
			_Nodes[30] = 8;
		}
		if (_Nodes[26] == 0) {
			_statPlay = 260;
			_loopFlag = true;
			_NextNode = 29;
		} else {
			if (_Nodes[26] == 2)
				PlayMovie(79);
			_NodeNum = 29;
			if (_Nodes[26] <= 2 || !ChkDesertObstacles()) {
				DrawLocation();
				_Nodes[26]++;
				_UserInput = 0;
				switch (_Nodes[30]) {
				case 0:
					_sound = false;
					PlayMovie(82);
					_CurrMap = 89;
					_Nodes[30] = 1;
					break;
				case 1:
					_sound = true;
					PlayMovie(82);
					_CurrMap = 89;
					_Nodes[30] = 0;
					break;
				case 8:
					_sound = false;
					PlayMovie(85);
					_CurrMap = 10;
					_Nodes[30] = 9;
					_tsIconOnly = true;
					break;
				default:
					_sound = true;
					PlayMovie(85);
					_CurrMap = 10;
					_tsIconOnly = true;
					break;
				}
				PlaySound(17);
				_statPlay = 291;
			}
		}
			
		break;
	case 291:
		switch(_UserInput) {
		case 0x431:
			_tsIconOnly = true;
			PlayMovie(83);
			PlaySound(30);
			InventoryAdd(10);
			_Nodes[30] = 8;
			_UserInput = 0;
			_CurrMap = 10;
			break;
		case 0x432:
			if (_Nodes[30] >= 8)
				DisplayIcon(140);
			else {
				DsAll();
				PlayMovie(84);
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		case 0x43A:
			_statPlay = 292;
			_Zoom = 2;
			ProcessMap(29, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 292:
		ProcessMapInput(29);
		break;
	case 300:
		if (_Nodes[24] == 0) {
			_loopFlag = true;
			_statPlay = 250;
		} else {
			_NodeNum = 30;
			if (!ChkDesertObstacles()) {
				DrawLocation();
				PlayMovie(86);
				PlaySound(18);
				_CurrMap = 10;
				_statPlay = 301;
			}
		}
		break;
	case 301:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 302;
			_Zoom = 2;
			ProcessMap(30, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		} 
		break;
	case 302:
		ProcessMapInput(30);
		break;
	case 310:
		_OldNode = _NodeNum;
		_NodeNum = 31;
		if (!ChkDesertObstacles()) {
			DrawLocation();
			PlayMovie(67);
			PlaySound(18);
			_CurrMap = 90;
			_statPlay = 311;
			_Nodes[32] = 1;
			_aTimer = 0;
			_UserInput = 0;
		}
		break;
	case 311:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 312;
			_Zoom = 2;
			ProcessMap(31, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		case 0x449:
			_aTimer = 0;
			DsAll();
			PlayMovie(68);
			_statPlay = 992;
			_loopFlag = true;
			break;			
		}
		break;
	case 312:
		ProcessMapInput(31);
		break;
	case 320:
		_OldNode = _NodeNum;
		_NodeNum = 32;
		DrawLocation();
		_UserInput = 0;
		IncreaseHealth();
		if (_Nodes[33] != 9 && _Wizard != 0) {
			_sound = false;
			_Nodes[33] = 9;
		} else {
			_sound = true;
			_Nodes[33] = 1;
		}
		PlayMovie(87);
		PlaySound(18);
		_CurrMap = 10;
		_statPlay = 321;
		break;
	case 321:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 322;
			_Zoom = 2;
			ProcessMap(32, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 322:
		ProcessMapInput(32);
		break;
	case 330:
		_OldNode = _NodeNum;
		_NodeNum = 33;
		if (!ChkDesertObstacles()) {
			DrawLocation();
			_UserInput = 0;
			if (_Nodes[32] == 0)
				_sound = false;
			else
				_sound = true;
			PlayMovie(88);
			PlaySound(18);
			_CurrMap = 10;
			_statPlay = 331;
		}
		break;
	case 331:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 332;
			_Zoom = 2;
			ProcessMap(33, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 332:
		ProcessMapInput(33);
		break;
	case 340:
		if (_Nodes[36] == 0) {
			_statPlay = 350;
			_loopFlag = true;
			_NextNode = 34;
		} else {
			_OldNode = _NodeNum;
			_NodeNum = 34;
			if (!ChkDesertObstacles()) {
				DrawLocation();
				_UserInput = 0;
				if (_Nodes[35] == 9 || _Nodes[41] == 0 || _Wizard == 0) {
					PlayMovie(91);
					_CurrMap = 92;
				} else {
					PlayMovie(89);
					ShowPic(389);
					_CurrMap = 91;
				}
				if (_Nodes[35] == 9)
					_tsIconOnly = true;
				PlaySound(26);
				_statPlay = 341;
			}
		}
		break;
	case 341:
		switch(_UserInput) {
		case 0x431:
			if (_Nodes[35] == 9)
				DisplayIcon(139);
			else {
				PlayMovie(90);
				InventoryAdd(9);
				_Nodes[35] = 9;
				_CurrMap = 93;
			}
			break;
		case 0x43A:
			_statPlay = 342;
			_Zoom = 2;
			ProcessMap(34, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			if (_PMovie == 89)
				ShowPic(389);
			break;
		case 0x445:
			_statPlay = 410;
			_UserInput = 0;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		case 0x446:
			_statPlay = 400;
			_UserInput = 0;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 342:
		ProcessMapInput(34);
		break;
	case 350:
		if (_Wizard == 0)
			_Nodes[36] = 1;
		if (_Nodes[36] == 1) {
			_statPlay = _NextNode * 10;
			_loopFlag = true;
		} else {
			_NodeNum = 35;
			DrawLocation();
			DsAll();
			PlayMovie(92);
			_CurrMap = 94;
			SetATimer();
			_UserInput = 0;
			_statPlay = 351;
		}
		break;
	case 351:
		switch(_UserInput) {
		case 0x428:
		case 0x429:
			InventoryDel(_UserInput - 0x428);
			_aTimer = 0;
			PlayMovie(178);
			_UserInput = 0x2F1;
			_loopFlag = true;
		// No break on purpose
		case 0x2F1:
			_aTimer = 0;
			DsAll();
			PlayMovie(93);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x42A:
			_aTimer = 0;
			EnAll();
			PlayMovie(179);
			InventoryDel(2);
			_Nodes[36] = 1;
			_UserInput = 0;
			_statPlay = _NextNode * 10;
			_lastObs = true;
			_LastObstacle = _NextNode;
			_loopFlag = true;
			break;
		}
		break;
	case 360:
		_NodeNum = 36;
		DrawLocation();
		if (_Nodes[37] == 0)
			_sound = false;
		else
			_sound = true;
		_frameStop = 185;
		DsAll();
		PlayMovie(94);
		_CurrMap = 95;
		SetATimer();
		_UserInput = 0;
		_statPlay = 361;
		break;
	case 361:
		switch(_UserInput) {
		case 0x2F1:
			_aTimer = 0;
			DsAll();
			PlayMovie(95);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x428:
			_aTimer = 0;
			EnAll();
			PlayMovie(97);
			InventoryDel(0);
			PlaySound(5);
			_Nodes[37] = 1;
			_UserInput = 0;
			_CurrMap = 10;
			if (_Nodes[43] == 0) {
				_statPlay = 420;
				_OldNode = _NodeNum;
				_NodeNum = 42;
				GPLogic2_SubSP361();
			} else {
				_tsIconOnly = true;
			}
			break;
		case 0x42A:
			_aTimer = 0;
			EnAll();
			PlayMovie(98);
			InventoryDel(2);
			PlaySound(5);
			_Nodes[37] = 1;
			_UserInput = 0;
			_CurrMap = 10;
			_tsIconOnly = true;
			break;
		case 0x42E:
			_aTimer = 0;
			EnAll();
			PlayMovie(100);
			InventoryDel(6);
			PlaySound(5);
			_Nodes[37] = 1;
			_UserInput = 0;
			_CurrMap = 10;
			_tsIconOnly = true;
			break;
		case 0x431:
			_aTimer = 0;
			EnAll();
			PlayMovie(96);
			PlaySound(5);
			_Nodes[37] = 9;
			_UserInput = 0;
			_CurrMap = 10;
			_statPlay = 420;
			_OldNode = _NodeNum;
			_NodeNum = 42;
			GPLogic2_SubSP361();
			break;
		case 0x437:
			_aTimer = 0;
			DsAll();
			PlayMovie(99);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 362;
			_Zoom = 2;
			ProcessMap(36, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 362:
		ProcessMapInput(36);
		break;
	case 370:
		_NodeNum = 37;
		DrawLocation();
		_UserInput = 0;
		_Nodes[38] = 0;
		PlayMovie(102);
		PlaySound(20);
		_CurrMap = 109;
		_statPlay = 371;
		break;
	case 371:
		switch(_UserInput) {
		case 0x413:
			_statPlay = 440;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 372;
			_Zoom = 2;
			ProcessMap(37, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 372:
		ProcessMapInput(37);
		break;
	case 380:
		_OldNode = _NodeNum;
		_NodeNum =38;
		if (!ChkDesertObstacles()) {
			DrawLocation();
			_UserInput = 0;
			PlayMovie(159);
			_CurrMap = 10;
			PlaySound(7);
			_Nodes[39] = 1;
			_statPlay = 381;
		}
		break;
	case 381:
		switch(_UserInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			if (_Wizard && _Nodes[39] != 9) {
				InventoryDel(_UserInput - 0x428);
				SaveAS();
				PlayMovie(178);
				RestoreAS();
			}
			_UserInput = 0;
			break;
		case 0x430:
			if (_Wizard && _Nodes[39] != 9) {
				DsAll();
				PlayMovie(162);
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		case 0x433:
			if (_Wizard && _Nodes[39] != 9) {
				PlayMovie(161);
				InventoryDel(11);
				_Nodes[51] = 0;
				_UserInput = 0;
			}
			_UserInput = 0;
			break;
		case 0x435:
			if (_Wizard && _Nodes[39] != 9) {
				PlayMovie(160);
				InventoryDel(13);
				_Nodes[39] = 9;
				_Nodes[59] = 0;
				_UserInput = 0;
				_tsIconOnly = true;
				_CurrMap = 108;
			}
			break;
		case 0x437:
			if (_Wizard && _Nodes[39] != 9) {
				PlayMovie(163);
				InventoryDel(15);
				_Nodes[12] = 0;
			}
			_UserInput = 0;
			break;
		case 0x43A:
			_statPlay = 382;
			_Zoom = 2;
			ProcessMap(38, _Zoom);
			_UserInput = 0;
		break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		case 0x456:
			_statPlay = 370;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 382:
		if (_Wizard && (_Nodes[39] != 9) && (_UserInput == 0x413))
			_UserInput = 0;

		ProcessMapInput(38);
		break;
	case 390:
		_NodeNum = 39;
		DrawLocation();
		_aTimer = 0;
		DsAll();
		PlayMovie(104);
		PlaySound(34);
		_statPlay = 992;
		_loopFlag = true;
		break;
	case 400:
		_NodeNum = 40;
		if (!_Wizard)
			_Nodes[41] = 2;
		if (_Nodes[41]) {
			if (_Nodes[41] == 1)
				_Nodes[41] = 2;
			else {
				_Nodes[41] = 1;
				_sound = true;
			}
		} else {
			DsAll();
			PlayMovie(105);
			_CurrMap = 10;
			SetATimer();
		}
		
		_UserInput = 0;
		_statPlay = 401;
		break;
	case 401:
		switch(_UserInput) {
		case 0x2F1:
			_aTimer = 0;
			DsAll();
			PlayMovie(106);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			if (!_Nodes[41]) {
				InventoryDel(_UserInput - 0x428);
				_aTimer = 0;
				PlayMovie(178);
				_UserInput = 753;
				_loopFlag = true;
				_CurrMap = 10;
			}
			break;
		case 0x42C:
			if (!_Nodes[41]) {
				_aTimer = 0;
				DsAll();
				PlayMovie(108);
				_UserInput = 0;
				_statPlay = 992;
				_loopFlag = true;
			}
			break;
		case 0x42E:
			if (!_Nodes[41]) {
				_aTimer = 0;
				PlayMovie(109);
				InventoryDel(6);
				_Nodes[43] = 0;
				_UserInput = 0;
				_aTimer = 136;
			}
			break;
		case 0x437:
			if (!_Nodes[41]) {
				_aTimer = 0;
				_Help = 1;
				_Replay = true;
				_Pouch = 1;
				_fstFwd = true;
				PlayMovie(107);
				PlaySound(26);
				_Nodes[41] = 1;
				_UserInput = 0;
				_CurrMap = 96;
				_tsIconOnly = true;
				_loopFlag = true;
			}
			break;
		case 0x43E:
			_aTimer = 0;
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		case 0x441:
			_statPlay = 460;
			_CurrMap = 10;
			_UserInput = 0;
			_loopFlag = true;
			break;
		}
		break;
	case 402:
		_aTimer = 0;
		ProcessMapInput(40);
		break;
	case 410:
		_NodeNum = 41;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[41])
			_sound = true;

		_Eye = false;
		PlayMovie(111);
		PlaySound(26);
		_CurrMap = 97;
		_statPlay = 411;
		break;
	case 411:
		switch(_UserInput) {
		case 0x43E:
			_sound = _lastSound;
			if (_PMovie == 120) {
				SaveAS();
				_frameStop = 50;
				PlayMovie(120);
				_bTimer = 34;
				while(_bTimer != 0) {
					checkTimers();
					RefreshSound();
					CheckMainScreen();
				}
				RestoreAS();
			}
			PlayMovie(_PMovie);
			break;
		case 0x445:
			_UserInput = 0;
			_loopFlag = true;
			_CurrMap = 10;
			_statPlay = 450;
			break;
		case 0x446:
			_UserInput = 0;
			_loopFlag = true;
			_CurrMap = 10;
			_statPlay = 460;
			break;
		}
		break;
	case 420:
		if (_Nodes[37] == 9 || _Nodes[29] == 1 || !_Wizard) {
			_OldNode = _NodeNum;
			_NodeNum = 42;
			if (!ChkDesertObstacles()) {
				DrawLocation();
				GPLogic2_SubSP361();
			}
		} else {
			_statPlay = 360;
			_loopFlag = true;
		}
		break;
	case 421:
		switch(_UserInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			_UserInput = 0;
			break;
		case 0x42E:
			if (_Nodes[43])
				DisplayIcon(136);
			else {
				if (_Nodes[35] == 9) {
					_sound = false;
				} else {
					_sound = true;
				}
				PlayMovie(113);
				InventoryAdd(6);
				_Nodes[43] = 9;
				_CurrMap = 10;
			}
			_tsIconOnly = true;
			break;
		case 0x43A:
			_statPlay = 422;
			_Zoom = 2;
			ProcessMap(42, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		
		}
		break;
	case 422:
		ProcessMapInput(42);
		break;
	case 430:
		_NodeNum = 43;
		DrawLocation();
		_Eye = false;
		_UserInput = 0;
		if (_Nodes[44] == 9) {
			_tsIconOnly = true;
			PlayMovie(118);
			_CurrMap = 80;
		} else if (_Inventory[12] <= 0) {
			PlayMovie(115);
			ShowPic(415);
			_CurrMap = 78;
			if (!_skylarTimer)
				_skylarTimer = 5400;
		} else {
			_skylarTimer = 0;
			PlayMovie(116);
			PlaySound(25);
			InventoryDel(12);
			InventoryAdd(16);
			_Nodes[44] = 9;
			_CurrMap = 79;
			_tsIconOnly = true;
		}
		_statPlay = 431;
		PlaySound(20);
		_Eye = false;
		break;
	case 431:
		switch(_UserInput) {
			case 0x40A:
				PlayMovie(10);
				_statPlay = 200;
				if (_Nodes[44] != 9)
					_Nodes[20] = 1;
				_CurrMap = 10;
				_loopFlag = true;
				break;
			case 0x414:
				PlayMovie(10);
				_statPlay = 390;
				_CurrMap = 10;
				_loopFlag = true;
				break;
			case 0x41D:
				PlayMovie(10);
				_statPlay = 500;
				_CurrMap = 10;
				_loopFlag = true;
				break;
			case 0x422:
				PlayMovie(10);
				_statPlay = 380;
				_CurrMap = 10;
				_loopFlag = true;
				break;
			case 0x428:
			case 0x429:
			case 0x42A:
				if (_Nodes[44] != 9) {
					InventoryDel(_UserInput - 0x428);
					SaveAS();
					PlayMovie(178);
					RestoreAS();
				}
				break;
			case 0x438:
				if (_Nodes[44] == 9) {
					DisplayIcon(146);
				} else {
					DsAll();
					PlayMovie(117);
					_statPlay = 992;
					_loopFlag = true;
				}
				break;
			case 0x43E:
				break;
		}
		break;
	case 440:
		_NodeNum = 44;
		DrawLocation();
		_UserInput = 0;
		if (_Inventory[12] > 0 || _Nodes[44] == 9)
			_sound = true;

		PlayMovie(119);
		PlaySound(20);
		_CurrMap = 100;
		_statPlay = 441;
		break;
	case 441:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 442;
			_Zoom = 2;
			ProcessMap(44, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		case 0x452:
			_statPlay = 430;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 442:
		ProcessMapInput(44);
		break;
	case 450:
		_NodeNum = 45;
		_UserInput = 0;
		if (!_Nodes[41]) {
			DsAll();
			PlayMovie(120);
			_statPlay = 992;
			_CurrMap = 10;
			_loopFlag = true;
		} else {
			_fstFwd = false;
			_frameStop = 50;
			PlayMovie(120);
			_CurrMap = 17;
			_statPlay = 451;
		}
		break;
	case 451:
		switch(_UserInput) {
		case 0x43E:
			_sound = _lastSound;
			_fstFwd = false;
			_frameStop = 50;
			PlayMovie(_PMovie);
			_CurrMap = 17;
			PlaySound(26);
			break;
		case 0x441:
			_statPlay = 410;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		}
		break;
	case 460:
		_Eye = false;
		_NodeNum = 46;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[47] && _Wizard)
			_Nodes[47] = 1;
		else {
			_Nodes[47] = 0;
			_sound = true;
		}
		PlayMovie(121);
		PlaySound(26);
		_CurrMap = 107;
		_statPlay = 461;
		break;
	case 461:
		switch(_UserInput) {
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		case 0x445:
			_statPlay = 340;
			_UserInput = 0;
			_loopFlag = true;
			_CurrMap = 10;
			_lastObs = true;
			break;
		case 0x446:
			_statPlay = 400;
			_UserInput = 0;
			_loopFlag = true;
			_CurrMap = 10;
			break;
		}
		break;
	case 462:
		ProcessMapInput(46);
		break;
	case 480:
		_NodeNum = 48;
		_UserInput = 0;
		PlayMovie(114);
		_CurrMap = 10;
		_statPlay = 481;
		_Nodes[49] = 1;
		break;
	case 481:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 482;
			_Zoom = 2;
			ProcessMap(48, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _lastSound;
			PlayMovie(_PMovie);
			break;
		}
		break;
	case 482:
		ProcessMapInput(48);
		break;
	case 490:
		_NodeNum = 49;
		DrawLocation();
		DsAll();
		PlayMovie(180);
		PlaySound(18);
		_CurrMap = 101;
		_statPlay = 491;
		_aTimer = 0;
		_UserInput = 0;
		break;
	case 491:
		switch(_UserInput) {
		case 0x428:
		case 0x42A:
		case 0x42B:
			InventoryDel(_UserInput - 0x428);
			_aTimer = 0;
			PlayMovie(178);
			_UserInput = 753;
			_loopFlag = true;
			_CurrMap = 10;
			break;
		case 0x442:
			PlayMovie(182);
			_statPlay = 992;
			_loopFlag = true;
			break;
		case 0x44A:
			_CurrMap = 10;
			_UserInput = 0;
			_aTimer = 0;
			EnAll();
			PlayMovie(181);
			_Nodes[29] = 9;
			_Nodes[49] = 0;
			_statPlay = _NextNode * 10;
			_loopFlag = true;
			break;
		}
		break;
	}
}

void KingdomGame::GPLogic2_SubSP361() {
	EnAll();
	_UserInput = 0;
	PlaySound(19);
	if (_Nodes[29] == 1) {
		_statPlay = 480;
		_loopFlag = true;
		_CurrMap = 10;
	} else {
		if (_Inventory[6] == 1 || _Wizard == 0) {
			if (_Nodes[43] != 1 || _Wizard != 0)
				_Nodes[43] = 1;
			else {
				_sound = true;
				_Nodes[43] = 2;
			}
			PlayMovie(122);
			_CurrMap = 10;
			_tsIconOnly = true;
			_statPlay = 421;
		} else {
			PlayMovie(112);
			_CurrMap = 98;
			_statPlay = 421;
		}
	}
}

} // NameSpace
