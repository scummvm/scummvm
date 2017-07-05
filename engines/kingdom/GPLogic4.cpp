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

void KingdomGame::GPLogic4() {
	switch(_statPlay) {
	case 900:
		if (_ATimer > 0) {
			PlaySound(0);
			_ATimer = 133;
		}
		if (_ASMode) {
			SwitchMtoA();
			_MapStat = 0;
			_oldStatPlay--;
			_ASMode = false;
		}
		SaveAS();
		FadeToBlack2();

		if (_Health == 10 || _Health == 6 || _Health == 2) {
			ShowPic(121);
			_BTimer = 38;
			ReadMouse();
			while (_BTimer && !_MouseButton) {
				checkTimers();
				RefreshSound();
				ReadMouse();
			}
			
			if (_ATimer > 0)
				SetATimer();
			
			_UserInput = 0;
			GPLogic4_SP901();
		} else {
			if (_ATimer > 0) {
				SetATimer();
				_ATimer = 133;
			}
			ShowPic(120);
			_statPlay = 901;
		}
		break;
	case 901:
		GPLogic4_SP901();
		break;
	case 991:
		_CurrMap = 10;
		_UserInput = 0;
		PlaySound(0);
		if (_Health == 10 || _Health == 6 || _Health == 2)
			_Health -= 2;
		else
			_Health -= 4;
		
		if (_Health == 0) {
			PlayMovie(194);
			_noIFScreen = true;
			FShowPic(107);
			_CurrMap = 3;
			DsAll();
			_Pouch = 0;
			PlaySound(1);
			memset(_Inventory, -1, 19);
			_statPlay = 994;
		} else {
			_BTimer = 36;
			while(_BTimer) {
				checkTimers();
				RefreshSound();
			}
			
			_loopFlag = true;
			if (_Nodes[29] == 1) {
				_Nodes[29] = 0;
				_Nodes[49] = 0;
			}

			_ItemInhibit = false;
			_TideCntl = false;
			DrawPic(179);
			_SkylarTimer = 0;
			_SkylarTimerFlag = false;
			EnAll();
			_MapEx = true;
			_NodeNum = 5;
			DrawLocation();
			PlaySound(0);
			if (_Nodes[5] > 0) {
				PlayMovie(197);
				_Resurrect = 1;
			}
			InventoryDel(12);
			_statPlay = 50;
		}
		break;
	case 992:
		_CurrMap = 10;
		_UserInput = 0;
		PlaySound(0);
		if (_Health == 10 || _Health == 6 || _Health == 2)
			_Health -= 2;
		else
			_Health -= 4;
		
		if (_Health == 0) {
			PlayMovie(194);
			_noIFScreen = true;
			FShowPic(107);
			_CurrMap = 3;
			DsAll();
			_Pouch = 0;
			PlaySound(1);
			memset(_Inventory, -1, 19);
			_statPlay = 994;
		} else {
			_BTimer = 36;
			while(_BTimer != 0) {
				checkTimers();
				RefreshSound();
			}

			_loopFlag = true;
			if (_Nodes[29] == 1) {
				_Nodes[29] = 0;
				_Nodes[49] = 0;
			}

			_ItemInhibit = false;
			_TideCntl = false;
			DrawPic(179);
			_SkylarTimer = 0;
			_SkylarTimerFlag = false;
			EnAll();
			_MapEx = true;
			_NodeNum = 27;
			DrawLocation();
			_Resurrect = 1;
			PlaySound(0);
			PlayMovie(195);
			InventoryDel(12);
			_statPlay = 270;
		}
		break;
	case 993:
		_CurrMap = 10;
		_UserInput = 0;
		PlaySound(0);
		if (_Health == 10 || _Health == 6 || _Health == 2)
			_Health -= 2;
		else
			_Health -= 4;
		
		if (_Health == 0) {
			PlayMovie(194);
			_noIFScreen = true;
			FShowPic(107);
			_CurrMap = 3;
			DsAll();
			_Pouch = 0;
			PlaySound(1);
			memset(_Inventory, -1, 19);
			_statPlay = 994;
		} else {
			_BTimer = 36;
			while (_BTimer != 0) {
				checkTimers();
				RefreshSound();
			}
			
			_loopFlag = true;
			if (_Nodes[29] == 1) {
				_Nodes[29] = 0;
				_Nodes[49] = 0;
			}
			if (_Nodes[68] == 1)
				_Nodes[68] = 0;

			_ItemInhibit = false;
			_TideCntl = false;
			DrawPic(179);
			_SkylarTimer = 0;
			_SkylarTimerFlag = false;
			EnAll();
			_MapEx = true;
			_NodeNum = 52;
			DrawLocation();
			_Resurrect = 1;
			PlaySound(0);
			PlayMovie(196);
			InventoryDel(12);
			_statPlay = 520;
		}
		break;
	case 994:
		switch(_UserInput) {
		case 0x190:
			InitPlay();
			// TODO _QuitFlag = 1;
			_quit = true;
			break;
		case 0x191:
			// TODO _QuitFlag = 2;
			_quit = true;
			break;
		}
		break;
	}
}

void KingdomGame::GPLogic4_SP901() {
	SetMouse();
	EraseCursor();
	FadeToBlack2();
	ShowPic(106);
	DrawIcon(4, 0, 12 - _HealthOld);
	if (_TideCntl)
		DrawPic(178);
	else
		DrawPic(179);
	
	_IconRedraw = true;
	if (_UserInput == 0x2F1) {
		_ASMode = false;
		RestoreAS();
		_statPlay = _oldStatPlay;
		_loopFlag = true;
	} else {
		if (_Health == 10 || _Health == 6 || _Health == 2 || _UserInput < 0x400 || _UserInput > 0x427) {
			_ASMode = false;
			RestoreAS();
			_statPlay = _oldStatPlay;
		} else {
			_statPlay = _mapExit[_UserInput - 0x400];
			_UserInput = 0;
			Wound();
			PlayMovie(10);
			InventoryDel(3);
			_TSIconOnly = 0;
			EnAll();
			SwitchAS();
			_MapStat = 0;
		}
		_UserInput = 0;
		_loopFlag = true;
	}
}

} // Namespace
