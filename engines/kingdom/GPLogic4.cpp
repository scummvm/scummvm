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
			playSound(0);
			_aTimer = 133;
		}
		if (_asMode) {
			switchMtoA();
			_mapStat = 0;
			_oldStatPlay--;
			_asMode = false;
		}
		saveAS();
		fadeToBlack2();

		if (_health == 10 || _health == 6 || _health == 2) {
			showPic(121);
			_bTimer = 38;
			readMouse();
			while (_bTimer && !_mouseButton) {
				checkTimers();
				refreshSound();
				readMouse();
			}
			
			if (_aTimer > 0)
				setATimer();
			
			_userInput = 0;
			GPLogic4_SP901();
		} else {
			if (_aTimer > 0) {
				setATimer();
				_aTimer = 133;
			}
			showPic(120);
			_statPlay = 901;
		}
		break;
	case 901:
		GPLogic4_SP901();
		break;
	case 991:
		_currMap = 10;
		_userInput = 0;
		playSound(0);
		if (_health == 10 || _health == 6 || _health == 2)
			_health -= 2;
		else
			_health -= 4;
		
		if (_health == 0) {
			playMovie(194);
			_noIFScreen = true;
			fShowPic(107);
			_currMap = 3;
			dsAll();
			_pouch = false;
			playSound(1);
			memset(_inventory, -1, 19);
			_statPlay = 994;
		} else {
			_bTimer = 36;
			while(_bTimer) {
				checkTimers();
				refreshSound();
			}
			
			_loopFlag = true;
			if (_nodes[29] == 1) {
				_nodes[29] = 0;
				_nodes[49] = 0;
			}

			_itemInhibit = false;
			_tideCntl = false;
			drawPic(179);
			_skylarTimer = 0;
			_skylarTimerFlag = false;
			enAll();
			_mapEx = true;
			_nodeNum = 5;
			drawLocation();
			playSound(0);
			if (_nodes[5] > 0) {
				playMovie(197);
				_resurrect = true;
			}
			inventoryDel(12);
			_statPlay = 50;
		}
		break;
	case 992:
		_currMap = 10;
		_userInput = 0;
		playSound(0);
		if (_health == 10 || _health == 6 || _health == 2)
			_health -= 2;
		else
			_health -= 4;
		
		if (_health == 0) {
			playMovie(194);
			_noIFScreen = true;
			fShowPic(107);
			_currMap = 3;
			dsAll();
			_pouch = false;
			playSound(1);
			memset(_inventory, -1, 19);
			_statPlay = 994;
		} else {
			_bTimer = 36;
			while(_bTimer != 0) {
				checkTimers();
				refreshSound();
			}

			_loopFlag = true;
			if (_nodes[29] == 1) {
				_nodes[29] = 0;
				_nodes[49] = 0;
			}

			_itemInhibit = false;
			_tideCntl = false;
			drawPic(179);
			_skylarTimer = 0;
			_skylarTimerFlag = false;
			enAll();
			_mapEx = true;
			_nodeNum = 27;
			drawLocation();
			_resurrect = true;
			playSound(0);
			playMovie(195);
			inventoryDel(12);
			_statPlay = 270;
		}
		break;
	case 993:
		_currMap = 10;
		_userInput = 0;
		playSound(0);
		if (_health == 10 || _health == 6 || _health == 2)
			_health -= 2;
		else
			_health -= 4;
		
		if (_health == 0) {
			playMovie(194);
			_noIFScreen = true;
			fShowPic(107);
			_currMap = 3;
			dsAll();
			_pouch = false;
			playSound(1);
			memset(_inventory, -1, 19);
			_statPlay = 994;
		} else {
			_bTimer = 36;
			while (_bTimer != 0) {
				checkTimers();
				refreshSound();
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
			drawPic(179);
			_skylarTimer = 0;
			_skylarTimerFlag = false;
			enAll();
			_mapEx = true;
			_nodeNum = 52;
			drawLocation();
			_resurrect = true;
			playSound(0);
			playMovie(196);
			inventoryDel(12);
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
	setMouse();
	eraseCursor();
	fadeToBlack2();
	showPic(106);
	drawIcon(4, 0, 12 - _healthOld);
	if (_tideCntl)
		drawPic(178);
	else
		drawPic(179);
	
	_iconRedraw = true;
	if (_userInput == 0x2F1) {
		_asMode = false;
		restoreAS();
		_statPlay = _oldStatPlay;
		_loopFlag = true;
	} else {
		if (_health == 10 || _health == 6 || _health == 2 || _userInput < 0x400 || _userInput > 0x427) {
			_asMode = false;
			restoreAS();
			_statPlay = _oldStatPlay;
		} else {
			_statPlay = _mapExit[_userInput - 0x400];
			_userInput = 0;
			wound();
			playMovie(10);
			inventoryDel(3);
			_tsIconOnly = false;
			enAll();
			switchAS();
			_mapStat = 0;
		}
		_userInput = 0;
		_loopFlag = true;
	}
}

} // Namespace
