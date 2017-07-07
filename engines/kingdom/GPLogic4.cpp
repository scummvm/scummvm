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
		if (_aTimer > 0) {
			PlaySound(0);
			_aTimer = 133;
		}
		if (_asMode) {
			SwitchMtoA();
			_mapStat = 0;
			_oldStatPlay--;
			_asMode = false;
		}
		SaveAS();
		fadeToBlack2();

		if (_health == 10 || _health == 6 || _health == 2) {
			ShowPic(121);
			_bTimer = 38;
			ReadMouse();
			while (_bTimer && !_mouseButton) {
				checkTimers();
				RefreshSound();
				ReadMouse();
			}
			
			if (_aTimer > 0)
				SetATimer();
			
			_userInput = 0;
			GPLogic4_SP901();
		} else {
			if (_aTimer > 0) {
				SetATimer();
				_aTimer = 133;
			}
			ShowPic(120);
			_statPlay = 901;
		}
		break;
	case 901:
		GPLogic4_SP901();
		break;
	case 991:
		_currMap = 10;
		_userInput = 0;
		PlaySound(0);
		if (_health == 10 || _health == 6 || _health == 2)
			_health -= 2;
		else
			_health -= 4;
		
		if (_health == 0) {
			PlayMovie(194);
			_noIFScreen = true;
			FShowPic(107);
			_currMap = 3;
			DsAll();
			_pouch = false;
			PlaySound(1);
			memset(_inventory, -1, 19);
			_statPlay = 994;
		} else {
			_bTimer = 36;
			while(_bTimer) {
				checkTimers();
				RefreshSound();
			}
			
			_loopFlag = true;
			if (_nodes[29] == 1) {
				_nodes[29] = 0;
				_nodes[49] = 0;
			}

			_itemInhibit = false;
			_tideCntl = false;
			DrawPic(179);
			_skylarTimer = 0;
			_skylarTimerFlag = false;
			EnAll();
			_mapEx = true;
			_nodeNum = 5;
			DrawLocation();
			PlaySound(0);
			if (_nodes[5] > 0) {
				PlayMovie(197);
				_resurrect = true;
			}
			InventoryDel(12);
			_statPlay = 50;
		}
		break;
	case 992:
		_currMap = 10;
		_userInput = 0;
		PlaySound(0);
		if (_health == 10 || _health == 6 || _health == 2)
			_health -= 2;
		else
			_health -= 4;
		
		if (_health == 0) {
			PlayMovie(194);
			_noIFScreen = true;
			FShowPic(107);
			_currMap = 3;
			DsAll();
			_pouch = false;
			PlaySound(1);
			memset(_inventory, -1, 19);
			_statPlay = 994;
		} else {
			_bTimer = 36;
			while(_bTimer != 0) {
				checkTimers();
				RefreshSound();
			}

			_loopFlag = true;
			if (_nodes[29] == 1) {
				_nodes[29] = 0;
				_nodes[49] = 0;
			}

			_itemInhibit = false;
			_tideCntl = false;
			DrawPic(179);
			_skylarTimer = 0;
			_skylarTimerFlag = false;
			EnAll();
			_mapEx = true;
			_nodeNum = 27;
			DrawLocation();
			_resurrect = true;
			PlaySound(0);
			PlayMovie(195);
			InventoryDel(12);
			_statPlay = 270;
		}
		break;
	case 993:
		_currMap = 10;
		_userInput = 0;
		PlaySound(0);
		if (_health == 10 || _health == 6 || _health == 2)
			_health -= 2;
		else
			_health -= 4;
		
		if (_health == 0) {
			PlayMovie(194);
			_noIFScreen = true;
			FShowPic(107);
			_currMap = 3;
			DsAll();
			_pouch = false;
			PlaySound(1);
			memset(_inventory, -1, 19);
			_statPlay = 994;
		} else {
			_bTimer = 36;
			while (_bTimer != 0) {
				checkTimers();
				RefreshSound();
			}
			
			_loopFlag = true;
			if (_nodes[29] == 1) {
				_nodes[29] = 0;
				_nodes[49] = 0;
			}
			if (_nodes[68] == 1)
				_nodes[68] = 0;

			_itemInhibit = false;
			_tideCntl = false;
			DrawPic(179);
			_skylarTimer = 0;
			_skylarTimerFlag = false;
			EnAll();
			_mapEx = true;
			_nodeNum = 52;
			DrawLocation();
			_resurrect = true;
			PlaySound(0);
			PlayMovie(196);
			InventoryDel(12);
			_statPlay = 520;
		}
		break;
	case 994:
		switch(_userInput) {
		case 0x190:
			initPlay();
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
	fadeToBlack2();
	ShowPic(106);
	DrawIcon(4, 0, 12 - _healthOld);
	if (_tideCntl)
		DrawPic(178);
	else
		DrawPic(179);
	
	_iconRedraw = true;
	if (_userInput == 0x2F1) {
		_asMode = false;
		RestoreAS();
		_statPlay = _oldStatPlay;
		_loopFlag = true;
	} else {
		if (_health == 10 || _health == 6 || _health == 2 || _userInput < 0x400 || _userInput > 0x427) {
			_asMode = false;
			RestoreAS();
			_statPlay = _oldStatPlay;
		} else {
			_statPlay = _mapExit[_userInput - 0x400];
			_userInput = 0;
			Wound();
			PlayMovie(10);
			InventoryDel(3);
			_tsIconOnly = false;
			EnAll();
			SwitchAS();
			_mapStat = 0;
		}
		_userInput = 0;
		_loopFlag = true;
	}
}

} // Namespace
