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
		_NodeNum = 50;
		DrawLocation();
		_UserInput = 0;
		_Resurrect = false;
		if (!_Wizard)
			_Nodes[51] = 1;
		switch(_Nodes[51]) {
		case 0:
			PlayMovie(123);
			_CurrMap = 102;
			break;
		case 1:
			_Nodes[51] = 2;
			PlayMovie(126);
			_CurrMap = 10;
			_TSIconOnly = true;
			break;
		default:
			_Nodes[51] = 1;
			_sound = true;
			PlayMovie(126);
			_CurrMap = 10;
			_TSIconOnly = true;
			break;
		}
		PlaySound(29);
		_statPlay = 501;
		break;
	case 501:
		switch(_UserInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			if (!_Nodes[51]) {
				InventoryDel(_UserInput - 0x428);
				SaveAS();
				PlayMovie(178);
				RestoreAS();
				_UserInput = 0;
			}
			break;
		case 0x433:
			if (!_Nodes[51]) {
				DsAll();
				PlayMovie(125);
				_statPlay = 993;
				_loopFlag = true;
			} else
				DisplayIcon(141);

			break;
		case 0x439:
			if (!_Nodes[51]) {
				PlayMovie(124);
				_Nodes[51] = 1;
				InventoryAdd(11);
				_UserInput = 0;
				_CurrMap = 10;
			}
			break;
		case 0x43A:
			_statPlay = 502;
			_Zoom = 2;
			ProcessMap(50, _Zoom);
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
	case 502:
		ProcessMapInput(50);
		break;
	case 510:
		_NodeNum = 51;
		DrawLocation();
		_UserInput = 0;
		PlayMovie(129);
		_CurrMap = 10;
		PlaySound(6);
		_statPlay = 511;
		break;
	case 511:
		switch(_UserInput) {
		case 0x434:
			if (_Inventory[6] >= 1)
				DisplayIcon(142);
			else {
				PlayMovie(128);
				_Nodes[52] = 9;
				InventoryAdd(12);
				_UserInput = 0;
				_CurrMap = 10;
				PlaySound(30);
			}
			break;
		case 0x43A:
			_statPlay = 512;
			_Zoom = 2;
			ProcessMap(51, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			PlayMovie(_PMovie);
			break;
		case 0x457:
			_TideCntl = false;
			DrawPic(179);
			PlayMovie(127);
			_CurrMap = 103;
			_UserInput = 0;
			PlaySound(0);
			PlaySound(6);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 512:
		ProcessMapInput(51);
		break;
	case 520:
		_NodeNum = 52;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[53] == 0)
			_Nodes[53] = 1;
		else {
			_sound = true;
			_Nodes[53] = false;
		}
		PlayMovie(130);
		_CurrMap = 111;
		_statPlay = 521;
		PlaySound(31);
		break;
	case 521:
		switch(_UserInput) {
		case 0x404:
			_statPlay = 550;
			_loopFlag = true;
			break;
		case 0x43A:
			_statPlay = 522;
			_Zoom = 2;
			ProcessMap(52, _Zoom);
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
	case 522:
		ProcessMapInput(52);
		break;
	case 530:
		_NodeNum = 53;
		DrawLocation();
		_UserInput = 0;
		_fstFwd = false;
		if (_Inventory[7] <= 0 && _Wizard)
			_TSIconOnly = false;
		else {
			_FrameStop = 24;
			_TSIconOnly = true;
		}
		_Eye = false;
		PlayMovie(70);
		_CurrMap = 99;
		PlaySound(8);
		_statPlay = 531;
		break;
	case 531:
		switch(_UserInput) {
		case 0x428:
			if (_Nodes[54] == 0) {
				PlayMovie(71);
				InventoryAdd(14);
				InventoryDel(0);
				_TSIconOnly = true;
				PlaySound(25);
			}
			_UserInput = 0;
			break;
		case 0x429:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			SaveAS();
			PlayMovie(178);
			RestoreAS();
			_UserInput = 0;
			break;
		case 0x43E:
			_fstFwd = false;
			if (_Inventory[7] > 0 && _PMovie == 70)
				_FrameStop = 24;
			if (!_Wizard && _PMovie == 70)
				_FrameStop = 24;
			PlayMovie(_PMovie);
			break;
		case 0x445:
			_NodeNum = 56;
			_UserInput = 0;
			_FrameStop = 50;
			EnAll();
			PlayMovie(135);
			_CurrMap = 106;
			_Nodes[57] = 1;
			PlaySound(8);
			_TSIconOnly = true;
			_statPlay = 561;
			break;
		case 0x452:
			_statPlay = 630;
			_loopFlag = true;
			_UserInput = 0;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 540:
		_NodeNum = 54;
		DrawLocation();
		_UserInput = 0;
		if (!_Wizard) {
			_sound = true;
			_CurrMap = 55;
		} else {
			if (!_Nodes[55])
				_Nodes[55] = 1;
			else {
				_sound = true;
				_Nodes[55] = 0;
			}
			_CurrMap = 112;
		}

		PlayMovie(132);
		ShowPic(432);
		_statPlay = 541;
		PlaySound(22);
		break;
	case 541:
		switch(_UserInput) {
		case 0x404:
			_statPlay = 550;
			_loopFlag = true;
			_UserInput = 0;
			break;
		case 0x43A:
			_statPlay = 542;
			_Zoom = 2;
			ProcessMap(54, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			ShowPic(432);
			break;
		case 0x453:
			_sound = _LastSound;
			PlayMovie(131);
			PlaySound(32);
			FShowPic(432);
			// Useless, but present in the original
			// _CurrMap = 10;
			_CurrMap = 55;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 542:
		ProcessMapInput(54);
		break;
	case 550:
		_NodeNum = 55;
		DrawLocation();
		_UserInput = 0;
		PlayMovie(133);
		if (_Nodes[56] != 9)
			_Nodes[56] = 0;
		_TSIconOnly = true;
		_CurrMap = 104;
		_statPlay = 551;
		PlaySound(8);
		break;
	case 551:
		switch(_UserInput) {
		case 0x2F1:
			_ATimer = 0;
			DsAll();
			PlayMovie(134);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x42C:
			_ATimer = 0;
			if (_Nodes[56] == 1) {
				EnAll();
				PlayMovie(183);
				FShowPic(433);
				PlaySound(8);
				_Nodes[56] = 9;
				_TSIconOnly = true;
			}
			break;
		case 0x43A:
			if (_Nodes[56] == 1) {
				DsAll();
				PlayMovie(134);
				_statPlay = 993;
				_loopFlag = true;
			} else {
				_statPlay = 552;
				_Zoom = 2;
				ProcessMap(55, _Zoom);
				_UserInput = 0;
			}
			break;
		case 0x43E:
			_ATimer = 0;
			PlayMovie(_PMovie);
			if (_PMovie == 183)
				FShowPic(433);
			break;
		case 0x452:
			_ATimer = 0;
			switch(_Nodes[56]) {
			case 0:
				DsAll();
				PlayMovie(69);
				_TSIconOnly = false;
				_Nodes[56] = 1;
				SetATimer();
				break;
			case 1:
				DsAll();
				PlayMovie(134);
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
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 552:
		ProcessMapInput(55);
		if (!_Nodes[56] && _statPlay == 560) {
			_statPlay = 551;
			_UserInput = 0x190;
			_loopFlag = true;
		}
		break;
	case 560:
		_NodeNum = 56;
		DrawLocation();
		_UserInput = 0;
		if (_Wizard) {
			_FrameStop = 50;
			EnAll();
			PlayMovie(135);
			ShowPic(435);
			_Nodes[57] = 1;
			PlaySound(8);
			_statPlay = 561;
			_CurrMap = 106;
			_TSIconOnly = true;
		} else {
			DsAll();
			PlayMovie(135);
			_CurrMap = 10;
			SetATimer();
			_Nodes[57] = 0;
			_statPlay = 561;
		}
		break;
	case 561:
		switch(_UserInput) {
		case 0x2F1:
			_ATimer = 0;
			DsAll();
			PlayMovie(136);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			_ATimer = 0;
			if (!_Nodes[57]) {
				InventoryDel(_UserInput - 0x428);
				SaveAS();
				PlayMovie(178);
				RestoreAS();
				_UserInput = 753;
				_loopFlag = true;
			}
			break;
		case 0x42C:
			_ATimer = 0;
			if (!_Nodes[57]) {
				_fstFwd = false;
				_Replay = true;
				PlayMovie(138);
				EnAll();
				PlaySound(8);
				_Nodes[57] = 1;
				InventoryDel(4);
				_Nodes[22] = 0;
				_CurrMap = 88;
				_TSIconOnly = true;
			}
			break;
		case 0x438:
			_ATimer = 0;
			if (!_Nodes[57]) {
				EnAll();
				_fstFwd = false;
				_Replay = true;
				PlayMovie(137);
				EnAll();
				PlaySound(8);
				_Nodes[57] = 1;
				_CurrMap = 106;
				_TSIconOnly = true;
			}
			break;
		case 0x43A:
			_statPlay = 562;
			_Zoom = 2;
			ProcessMap(56, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_fstFwd = false;
			if (_PMovie == 135 && _Nodes[57]) {
				_FrameStop = 50;
				PlayMovie(135);
			} else if (_Nodes[57])
				PlayMovie(_PMovie);
			else {
				_UserInput = 753;
				_ATimer = 0;
				_loopFlag = true;
			}
			break;
		case 0x445:
			_statPlay = 530;
			_loopFlag = true;
			_UserInput = 0;
			break;
		case 0x446:
			_statPlay = 620;
			_loopFlag = true;
			_UserInput = 0;
			break;
		case 0x452:
			_statPlay = 550;
			_loopFlag = true;
			_UserInput = 0;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 562:
		ProcessMapInput(56);
		break;
	case 570:
		if (_Nodes[58]) {
			if (_RtnNode != 34)
				_sound = true;
			_NodeNum = 57;
			DrawLocation();
			_Nodes[58] = 0;
			_UserInput = 0;
			PlayMovie(139);
			PlaySound(27);
			_CurrMap = 10;
			_statPlay = 571;
		} else {
			_statPlay = 740;
			_loopFlag = true;
		}
		break;
	case 571:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 572;
			_Zoom = 2;
			ProcessMap(57, _Zoom);
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
	case 572:
		ProcessMapInput(57);
		break;
	case 580:
		_NodeNum = 58;
		DrawLocation();
		_IconsClosed = true;
		_UserInput = 0;
		switch(_Nodes[59]) {
		case 0:
			_sound = true;
			PlayMovie(140);
			_CurrMap = 113;
			_Nodes[59] = 1;
			break;
		case 1:
			PlayMovie(140);
			_CurrMap = 113;
			_Nodes[59] = 0;
			break;
		case 8:
			_TSIconOnly = true;
			PlayMovie(143);
			_CurrMap = 10;
			_Nodes[59] = 9;
			break;
		case 9:
			_TSIconOnly = true;
			_sound = true;
			PlayMovie(143);
			_CurrMap = 10;
			break;
		}
		PlaySound(27);
		_statPlay = 581;
		break;
	case 581:
		switch(_UserInput) {
		case 0x430:
			PlayMovie(141);
			_Nodes[59] = 8;
			InventoryAdd(13);
			_TSIconOnly = true;
			_CurrMap = 10;
			PlaySound(30);
		// No break on purpose
		case 0x428:
		case 0x429:
		case 0x42A:
			if (_Nodes[59] < 8) {
				InventoryDel(_UserInput - 0x428);
				SaveAS();
				PlayMovie(178);
				RestoreAS();
			}
			_UserInput = 0;
			break;
		case 0x435:
			if (_Nodes[59] >= 8)
				DisplayIcon(143);
			else {
				DsAll();
				PlayMovie(142);
				_statPlay = 993;
				_loopFlag = true;
			}
		case 0x43A:
			_statPlay = 582;
			_Zoom = 2;
			ProcessMap(58, _Zoom);
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
	case 582:
		ProcessMapInput(58);
		break;
	case 590:
		_NodeNum = 59;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[60] == 0)
			_Nodes[60] = 1;
		else {
			_sound = true;
			_Nodes[60] = 0;
		}
		PlayMovie(144);
		ShowPic(144);
		_CurrMap = 114;
		PlaySound(21);
		_statPlay = 591;
		break;
	case 591:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 592;
			_Zoom = 2;
			ProcessMap(59, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			ShowPic(444);
			break;
		case 0x445:
			_statPlay = 660;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 592:
		ProcessMapInput(59);
		break;
	case 600:
		SetMouse();
		EraseCursor();
		FadeToBlack2();
		ShowPic(106);
		DrawIcon(4, 0, 12 - _HealthOld);
		_ASMode = false;
		_NodeNum = 60;
		DrawLocation();
		_UserInput = 0;
		DsAll();
		PlayMovie(145);
		_statPlay = 992;
		_loopFlag = true;
		break;
	case 610:
		_NodeNum = 61;
		DrawLocation();
		_UserInput = 0;
		_Nodes[73] = 0;
		_Nodes[74] = 0;
		if (_Nodes[62] == 0)
			_Nodes[62] = 1;
		else
			_sound = true;

		PlayMovie(146);
		if (_Nodes[70] == 9) {
			_ItemInhibit = true;
			_TSIconOnly = true;
		}
		
		_CurrMap = 115;
		PlaySound(36);
		_statPlay = 611;
		_Eye = false;
		break;
	case 611:
		switch(_UserInput) {
		case 0x428:
		case 0x429:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			DsAll();
			PlayMovie(148);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x432:
			PlayMovie(147);
			_Inventory[5] = -1;
			_Nodes[30] = 0;
			_CurrMap = 116;
			break;
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		case 0x441:
			// Cascade on purpose
		case 0x448:
			_CurrMap = 10;
			_ItemInhibit = true;
			_statPlay = 730;
			_loopFlag = true;
			break;
		case 0x44B:
			_CurrMap = 10;
			_statPlay = 680;
			_loopFlag = true;
			break;
		case 0x451:
			_CurrMap = 10;
			_statPlay = 690;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 620:
		_NodeNum = 62;
		DrawLocation();
		_UserInput = 0;
		_Eye = false;
		if (_Nodes[63] == 0) {
			PlayMovie(149);
			_CurrMap = 122;
		} else {
			_FrameStop = 24;
			PlayMovie(202);
			_CurrMap = 121;
			_Nodes[63] = 9;
		}
		_TSIconOnly = true;
		PlaySound(8);
		_statPlay = 621;
		break;
	case 621:
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
		case 0x430:
			PlayMovie(150);
			InventoryAdd(8);
			_Nodes[63] = 1;
			PlaySound(25);
			_CurrMap = 121;
			_UserInput = 0;
			break;
		case 0x43E:
			switch (_Nodes[63]) {
			case 0:
				PlayMovie(149);
				break;
			case 1:
				PlayMovie(150);
				break;
			case 9:
				_FrameStop = 24;
				PlayMovie(202);
				_CurrMap = 121;
				break;
			}
			break;
		case 0x445:
			_NodeNum = 56;
			_UserInput = 0;
			_FrameStop = 50;
			EnAll();
			PlayMovie(135);
			_Nodes[57] = 1;
			PlaySound(8);
			_statPlay = 561;
			_CurrMap = 106;
			if (_Inventory[16] < 1 && _Wizard)
				_CurrMap = 88;
			break;
		case 0x452:
			_statPlay = 630;
			_loopFlag = true;
			_UserInput = 0;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 630:
		_NodeNum = 63;
		DrawLocation();
		_UserInput = 0;
		_Eye = false;
		if (_Nodes[64] == 1 || !_Wizard) {
			_Nodes[64] = 0;
			_sound = true;
		} else
			_Nodes[64] = true;

		PlayMovie(151);
		_CurrMap = 72;
		PlaySound(40);
		_statPlay = 631;
		break;
	case 631:
		switch(_UserInput) {
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		case 0x445:
			PlayMovie(10);
			_statPlay = 50;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 640:
		_NodeNum = 64;
		DrawLocation();
		_UserInput = 0;
		if (_Nodes[65] || _Wizard) {
			_sound = true;
			_Nodes[65] = 0;
		} else
			_Nodes[65] = 1;

		PlayMovie(152);
		_CurrMap = 10;
		_statPlay = 641;
		PlaySound(28);
		break;
	case 641:
		switch(_UserInput) {
		case 0x43A:
			_statPlay = 642;
			_Zoom = 2;
			ProcessMap(64, _Zoom);
			_UserInput = 0;
			break;
		case 0x43E:
			_sound= _LastSound;
			PlayMovie(_PMovie);
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 642:
		ProcessMapInput(64);
		break;
	case 650:
		_NodeNum = 65;
		_Eye = false;
		_UserInput = 0;
		if (_Nodes[66] == 0)
			_Nodes[66] = 1;
		else {
			_sound = true;
			_Nodes[66] = 0;
		}
		PlayMovie(153);
		ShowPic(453);
		_CurrMap = 73;
		PlaySound(37);
		_statPlay = 651;
		break;
	case 651:
		switch(_UserInput) {
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			ShowPic(453);
			break;
		case 0x445:
			_statPlay = 700;
			_loopFlag = true;
			break;
		case 0x446:
			if (_Nodes[68] == 1) {
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
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 660:
		_NodeNum = 66;
		_Eye = false;
		_UserInput = 0;
		if (_Nodes[67] == 0)
			_Nodes[67] = 1;
		else {
			_sound = true;
			_Nodes[67] = 0;
		}
		PlayMovie(154);
		_CurrMap = 70;
		PlaySound(40);
		_statPlay = 661;
		break;
	case 661:
		switch(_UserInput) {
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		case 0x445:
			_statPlay = 700;
			_loopFlag = true;
			break;
		case 0x446:
			_loopFlag = true;
			_statPlay = (_Nodes[68] == 1) ? 650 : 790;
			break;
		case 0x447:
			if (!_Wizard || _Nodes[68])
				_statPlay = 650;
			else
				_statPlay = 670;

			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 670:
		_NodeNum = 67;
		_UserInput = 0;
		_Eye = false;
		_Nodes[68] = 1;
		_fstFwd = false;
		PlayMovie(155);
		_CurrMap = 127;
		_statPlay = 671;
		break;
	case 671:
		switch(_UserInput) {
		case 0x43E:
			_sound = _LastSound;
			_fstFwd = false;
			PlayMovie(_PMovie);
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
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 680:
		_NodeNum = 68;
		_UserInput = 0;
		DsAll();
		PlayMovie(156);
		_statPlay = 993;
		_loopFlag = true;
		break;
	case 690:
		_ItemInhibit = false;
		EnAll();
		if (_Nodes[70] == 9) {
			_statPlay = 640;
			_Nodes[73] = 0;
			_loopFlag = true;
			_UserInput = 0;
		} else {
			_NodeNum = 69;
			DrawLocation();
			_UserInput = 0;
			_CurrMap = 119;
			_FrameStop = 40;
			PlayMovie(157);
			PlaySound(12);
			_statPlay = 691;
		}
		break;
	case 691:
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
		case 0x439:
			if (_Nodes[70] == 9)
				DisplayIcon(147);
			else {
				_FrameStop = 108;
				PlayMovie(158);
				_ItemInhibit = false;
				InventoryAdd(17);
				_Nodes[70] = 9;
				PlaySound(23);
				_UserInput = 0;
				_CurrMap = 120;
			}
			break;
		case 0x43E:
			if (_PMovie == 157)
				_FrameStop = 40;
			else if (_PMovie == 158)
				_FrameStop = 108;
			
			if (_PMovie == 178) {
				SaveAS();
				PlayMovie(178);
				RestoreAS();
			} else
				PlayMovie(_PMovie);

			break;
		case 0x441:
			_ItemInhibit = false;
			_statPlay = 640;
			_Nodes[73] = 0;
			_loopFlag = true;
			_UserInput = 0;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 692:
		ProcessMapInput(69);
		break;
	case 700:
		_NodeNum = 70;
		_UserInput = 0;
		_Eye = false;
		if (_Nodes[71] && _Wizard) {
			_Nodes[71] = 0;
			_sound = false;
		} else
			_Nodes[71] = 1;

		PlayMovie(164);
		_CurrMap = 74;
		PlaySound(10);
		_statPlay = 701;
		break;
	case 701:
		switch(_UserInput) {
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
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
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 710:
		_NodeNum = 71;
		_UserInput = 0;
		_Eye = false;
		if (_Inventory[8] && _Inventory[14])
			_sound = true;
		DsAll();
		PlayMovie(165);
		_CurrMap = 10;
		SetATimer();
		_statPlay = _Wizard ? 712 : 711;
		break;
	case 711:
		switch(_UserInput) {
		case 0x2F1:
			_Replay = false;
			PlayMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			_Replay = false;
			_ATimer = 0;
			PlayMovie(178);
			PlayMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x430:
			_ATimer = 0;
			_NodeNum = 0;
			_IconsClosed = true;
			_fstFwd = false;
			PlayMovie(167);
			_fstFwd = false;
			PlayMovie(170);
			PlaySound(25);
			EndCredits();
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 712:
		switch(_UserInput) {
		case 0x2F1:
			_Replay = false;
			PlayMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			_Replay = false;
			_ATimer = 0;
			PlayMovie(178);
			PlayMovie(166);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x430:
			_ATimer = 0;
			if (_Nodes[72] != 2) {
				PlayMovie(57);
				_statPlay = 993;
			} else {
				_NodeNum = 0;
				_IconsClosed = true;
				_fstFwd = false;
				PlayMovie(55);
				_fstFwd = false;
				PlayMovie(184);
				PlaySound(25);
				EndCredits();
			}
			_loopFlag = true;
			break;
		case 0x432:
			_ATimer = 0;
			_Replay = false;
			PlayMovie(168);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x433:
			if ((_Inventory[8] > 0 && _Inventory[14] > 0) || _Nodes[72] == 2) {
				_Replay = false;
				PlayMovie(166);
				_statPlay = 993;
			} else {
				_ATimer = 0;
				EnAll();
				PlayMovie(169);
				_statPlay = 580;
				InventoryDel(11);
				_Nodes[51] = 0;
			}
			_loopFlag = true;
			break;
		case 0x436:
			_ATimer = 0;
			PlaySound(0);
			PlayMovie(193);
			ShowPic(128);
			InventoryDel(14);
			_Nodes[72] = 2;
			SetATimer();
			break;

		case 0x437:
			_ATimer = 0;
			if (_Nodes[72] == 2) {
				_NodeNum = 0;
				_IconsClosed = true;
				_fstFwd = false;
				PlayMovie(56);
				_fstFwd = false;
				PlayMovie(184);
				PlaySound(25);
				EndCredits();
				_quit = true;
//--				_QuitFlag = true;
			} else {
				PlayMovie(58);
				_statPlay = 993;
			}
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 720:
		_NodeNum = 72;
		_UserInput = 0;
		_Eye = false;
		switch (_Nodes[73]) {
		case 0:
			_Nodes[73] = 1;
			break;
		case 1:
			_FrameStop = 24;
			_Nodes[73] = 2;
			break;
		default:
			_FrameStop = 24;
			_Nodes[73] = 3;
			break;
		}
		PlayMovie(175);
		_CurrMap = 118;
		PlaySound(12);
		_statPlay = 721;
		break;
	case 721:
		switch(_UserInput) {
		case 0x43E:
			if (_Nodes[73] > 1)
				_FrameStop = 24;
			PlayMovie(_PMovie);
			break;
		case 0x441:
			if (_Nodes[73] == 1) {
				_ItemInhibit = false;
				_statPlay = 640;
			} else
				_statPlay = 680;

			_CurrMap = 10;
			_loopFlag = true;
			break;
		case 0x448:
			_statPlay = (_Nodes[73] == 2) ? 730 : 680;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		case 0x44B:
			_statPlay = (_Nodes[73] == 1) ? 730 : 680;
			_CurrMap = 10;
			_loopFlag = true;
			break;
		case 0x44C:
			if (_Nodes[73] == 1) {
				_statPlay = 730;
				_Nodes[74] = 3;
			} else
				_statPlay = 680;

			_CurrMap = 10;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 730:
		_NodeNum = 73;
		_UserInput = 0;
		_Eye = false;
		switch(_Nodes[74]) {
		case 0:
			_Nodes[74] = 1;
			break;
		case 1:
			_sound = true;
			_Nodes[74] = 2;
			break;
		case 2:
			_FrameStop = 23;
			_Nodes[74] = 3;
			break;
		default:
			_FrameStop = 23;
			_Nodes[74] = 4;
			break;
		}
		PlayMovie(176);
		_CurrMap = 117;
		PlaySound(36);
		_statPlay = 731;
		break;
	case 731:
		switch(_UserInput) {
		case 0x43E:
			if (_Nodes[74] > 2)
				_FrameStop = 23;
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		case 0x441:
			switch (_Nodes[74]) {
			case 1:
			case 2:
				_statPlay = 720;
				break;
			case 3:
				_statPlay = 720;
				_Nodes[73] = 1;
				break;
			default:
				_statPlay = 680;
				break;
			}
			_CurrMap = 10;
			_loopFlag = true;
			break;
		case 0x448:
			switch(_Nodes[74]) {
			case 3:
				_statPlay = 690;
				break;
			case 4:
				_statPlay = 720;
				_Nodes[73] = 1;
				break;
			default:
				_statPlay = 680;
				break;
			}
			break;
		case 0x44B:
			switch(_Nodes[74]) {
			case 1:
				_statPlay = 720;
				break;
			case 3:
			case 4:
				_statPlay = 720;
				_Nodes[73] = 1;
				break;
			default:
				_statPlay= 680;
				break;
			}

			_CurrMap = 10;
			_loopFlag = true;
			break;
		case 0x44C:
			switch(_Nodes[74]) {
			case 1:
			case 2:
				_statPlay = 720;
				break;
			case 4:
				_statPlay = 720;
				_Nodes[73] = 1;
				break;
			default:
				_statPlay = 680;
				break;
			}
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 740:
		_RtnNode = _NodeNum;
		_NodeNum = 74;
		DrawLocation();
		DsAll();
		PlayMovie(185);
		_CurrMap = 10;
		SetATimer();
		_statPlay = 741;
		_UserInput = 0;
		break;
	case 741:
		switch(_UserInput) {
		case 0x2F1:
			DsAll();
			PlayMovie(188);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x428:
		case 0x429:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			_ATimer = 0;
			PlayMovie(178);
			_loopFlag = true;
			_UserInput = 753;
			break;
		case 0x432:
			if (_Nodes[75]) {
				_sound = false;
				_Nodes[75] = false;
			} else
				_Nodes[75] = true;
			break;
		case 0x437:
			_loopFlag = true;
			if (Wound()) {
				_FrameStop = 88;
				PlayMovie(187);
				_statPlay = 570;
				_Nodes[58] = 1;
			} else {
				_FrameStop = 78;
				PlayMovie(187);
				_statPlay = 993;
			}
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 760:
		_NodeNum = 76;
		_Eye = false;
		_UserInput = 0;
		PlayMovie(189);
		_CurrMap = 126;
		PlaySound(21);
		_statPlay = 761;
		break;
	case 761:
		switch(_UserInput) {
		case 0x43E:
			_sound = _LastSound;
			PlayMovie(_PMovie);
			break;
		case 0x445:
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x44D:
			PlayMovie(191);
			_Nodes[68] = 2;
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x44E:
			PlayMovie(192);
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x458:
			PlayMovie(190);
			_statPlay = 660;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 780:
		_NodeNum = 78;
		DrawLocation();
		_UserInput = 0;
		_Eye = false;
		PlayMovie(171);
		_CurrMap = 124;
		PlaySound(40);
		_statPlay = 781;
		break;
	case 781:
		switch(_UserInput) {
		case 0x428:
		case 0x42A:
			InventoryDel(_UserInput - 0x428);
			_FrameStop = 64;
			PlayMovie(173);
			ShowPic(471);
			_CurrMap = 124;
			break;
		case 0x429:
			if (_Wizard) {
				_FrameStop = 64;
				PlayMovie(173);
				InventoryDel(1);
			} else {
				_FrameStop = 66;
				_fstFwd = false;
				PlayMovie(172);
				_fstFwd = true;
				PlayMovie(201);
			}

			FShowPic(471);
			_CurrMap = 124;
			break;
		case 0x439:
			_FrameStop = 59;
			_fstFwd = false;
			PlayMovie(174);
			_fstFwd = true;
			PlayMovie(201);
			FShowPic(471);
			_CurrMap = 124;
			break;
		case 0x43E:
			_sound = _LastSound;
			if (_PMovie == 173)
				_FrameStop = 64;
			if (_PMovie == 201) {
				_FrameStop = 59;
				_fstFwd = false;
				PlayMovie(174);
				_fstFwd = true;
				PlayMovie(201);
			} else {
				PlayMovie(_PMovie);
			}
			if (_PMovie != 171) {
				FShowPic(471);
				_CurrMap = 124;
			}
			break;
		case 0x445:
			_statPlay = 660;
			_loopFlag = true;
			break;
		case 0x446:
			_statPlay = (_Nodes[68] == 1) ? 760 : 660;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	case 790:
		_NodeNum = 79;
		DrawLocation();
		DsAll();
		PlayMovie(4);
		_CurrMap = 8;
		_statPlay = 791;
		SetATimer();
		_UserInput = 0;
		break;
	case 791:
		switch(_UserInput) {
		case 0x2F1:
			DsAll();
			PlayMovie(204);
			_statPlay = 993;
			_loopFlag = true;
			break;
		case 0x445:
			EnAll();
			PlayMovie(10);
			_statPlay = 50;
			_loopFlag = true;
			break;
		case 0x446:
			EnAll();
			PlayMovie(10);
			_statPlay = 520;
			_loopFlag = true;
			break;
		default:
			if (_UserInput)
				debug("Skipped UserInput %d(0x%04X) for _StatPlay %d", _UserInput, _UserInput, _statPlay);
			break;
		}
		break;
	}
}

void KingdomGame::EndCredits() {
	_NodeNum = 0;
	_BTimer = 190;
	ReadMouse();

	while(_BTimer != 0 && _MouseButton == 0) {
		checkTimers();
		RefreshSound();
		CheckMainScreen();
		ReadMouse();
	}

	FadeToBlack1();
	DrawRect(4, 17, 228, 161, 0);
	PlaySound(0);
	DsAll();
	_IconsClosed = true;
	PlayMovie(199);
	DsAll();
	PlayMovie(205);
	DsAll();
	FadeToBlack1();
	DrawRect(4, 17, 228, 161, 0);
	PlayMovie(201);
	FShowPic(125);
	_CurrMap = 3;
	DsAll();
	_Pouch = false;
	memset(_Inventory, 0xFF, 19);
	_statPlay = 994;
	_loopFlag = true;
}
} // NameSpace
