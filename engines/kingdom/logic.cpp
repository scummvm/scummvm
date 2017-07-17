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

Logic::Logic(KingdomGame* vm) : _vm(vm) {
}

void Logic::initVariables() {
	_healthOld = 0;
	_health = 0;
	_tideCntl = 0;
	_statPlay = 0;
	_spell3 = 0;
	_spell2 = 0;
	_spell1 = 0;
	_robberyNode = 0;
	_rtnNode = 0;
	_replay = false;
	_resurrect = false;
	_pouch = false;
	_oldPouch = false;
	_oldHelp = false;
	_oldEye = false;
	_oldStatPlay = 0;
	_oldNode = 0;
	_nodeNum = 0;
	_nextNode = 0;
	_mapStat = 0;
	_lastObs = 0;
	_lastObstacle = 0;
	_help = false;
	_eye = false;
	for (int i = 0; i < 99; i++)
		_nodes[i] = 0;
	_currMap = 0;
}

void Logic::initPlay() {
	memset(_inventory, 0xFF, 19);

	for (int i = 0; i < 99; i++)
		_nodes[i] = 0;

	_statPlay = 10;
	_spell1 = false;
	_spell2 = false;
	_spell3 = false;
	_tideCntl = false;
	_health = 12;
	_healthOld = 1;
	_lastObs = false;
	enAll();
	_pouch = true;

	initOpcodes();
}

#define OPCODE(i, x)   _opcodes_opcodesGob[i]._OPCODEGOB(OPCODEVER, x)
void Logic::initOpcodes() {
	_opCodes[10] = &Logic::GPL1_10;
	_opCodes[11] = &Logic::GPL1_11;
	_opCodes[20] = &Logic::GPL1_20;
	_opCodes[30] = &Logic::GPL1_30;
	_opCodes[31] = &Logic::GPL1_31;
	_opCodes[32] = &Logic::GPL1_32;
	_opCodes[40] = &Logic::GPL1_40;
	_opCodes[41] = &Logic::GPL1_41;
	_opCodes[42] = &Logic::GPL1_42;
	_opCodes[50] = &Logic::GPL1_50;
	_opCodes[51] = &Logic::GPL1_51;
	_opCodes[52] = &Logic::GPL1_52;
	_opCodes[53] = &Logic::GPL1_53;
	_opCodes[60] = &Logic::GPL1_60;
	_opCodes[70] = &Logic::GPL1_70;
	_opCodes[71] = &Logic::GPL1_71;
	_opCodes[72] = &Logic::GPL1_72;
	_opCodes[80] = &Logic::GPL1_80;
	_opCodes[81] = &Logic::GPL1_81;
	_opCodes[82] = &Logic::GPL1_82;
	_opCodes[90] = &Logic::GPL1_90;
	_opCodes[91] = &Logic::GPL1_91;
	_opCodes[92] = &Logic::GPL1_92;
	_opCodes[100] = &Logic::GPL1_100;
	_opCodes[101] = &Logic::GPL1_101;
	_opCodes[102] = &Logic::GPL1_102;
	_opCodes[110] = &Logic::GPL1_110;
	_opCodes[111] = &Logic::GPL1_111;
	_opCodes[112] = &Logic::GPL1_112;
	_opCodes[120] = &Logic::GPL1_120;
	_opCodes[121] = &Logic::GPL1_121;
	_opCodes[130] = &Logic::GPL1_130;
	_opCodes[140] = &Logic::GPL1_140;
	_opCodes[141] = &Logic::GPL1_141;
	_opCodes[142] = &Logic::GPL1_142;
	_opCodes[150] = &Logic::GPL1_150;
	_opCodes[151] = &Logic::GPL1_151;
	_opCodes[160] = &Logic::GPL1_160;
	_opCodes[161] = &Logic::GPL1_161;
	_opCodes[162] = &Logic::GPL1_162;
	_opCodes[170] = &Logic::GPL1_170;
	_opCodes[171] = &Logic::GPL1_171;
	_opCodes[172] = &Logic::GPL1_172;
	_opCodes[180] = &Logic::GPL1_180;
	_opCodes[181] = &Logic::GPL1_181;
	_opCodes[182] = &Logic::GPL1_182;
	_opCodes[190] = &Logic::GPL1_190;
	_opCodes[200] = &Logic::GPL1_200;
	_opCodes[201] = &Logic::GPL1_201;
	_opCodes[202] = &Logic::GPL1_202;
	_opCodes[210] = &Logic::GPL1_210;
	_opCodes[211] = &Logic::GPL1_211;
	_opCodes[212] = &Logic::GPL1_212;
	_opCodes[220] = &Logic::GPL1_220;
	_opCodes[221] = &Logic::GPL1_221;
	_opCodes[222] = &Logic::GPL1_222;
	_opCodes[230] = &Logic::GPL1_230;
	_opCodes[240] = &Logic::GPL1_240;
	_opCodes[241] = &Logic::GPL1_241;
	_opCodes[242] = &Logic::GPL1_242;
}

void Logic::enAll() {
	_help = true;
	_eye = true;
	_replay = true;
	_pouch = true;
	_fstFwd = true;
}

void Logic::dsAll() {
	_help = false;
	_eye = false;
	_replay = false;
	_pouch = false;
	_fstFwd = false;
}

void Logic::inventoryDel(int item) {
	if (_inventory[item] > 0)
		_inventory[item]--;
}

void Logic::inventoryAdd(int item) {
	if (item >= 4)
		_inventory[item] = 1;
	else
		_inventory[item] = 3;
}

bool Logic::wound() {
	bool retval = false;
	if (_health == 12 || _health == 8 || _health == 4) {
		_health -= 2;
		retval = true;
	}
	return retval;
}

bool Logic::chkDesertObstacles() {
	if (!_vm->_wizard)
		return false;

	_nextNode = _nodeNum;
	if (_lastObs) {
		_lastObs = false;
		return false;
	}

	if (_nodes[29] || _vm->_rnd->getRandomNumber(6) == 0) {
		if (!_nodes[49] || _robberyNode != _nodeNum) {
			if (_lastObstacle != _nodeNum) {
				if (_vm->_rnd->getRandomNumber(5) == 0) {
					_statPlay = 250;
					_lastObstacle = _nodeNum;
					_lastObs = true;
					_vm->_loopFlag = true;
					return true;
				} else {
					return false;
				}
			} else {
				return false;
			}
		} else {
			_statPlay = 490;
			_vm->_loopFlag = true;
			return true;
		}
	} else {
		_statPlay = 280;
		_robberyNode = _nodeNum;
		_lastObstacle = _nodeNum;
		_lastObs = true;
		_vm->_loopFlag = true;
		return true;
	}
}

void Logic::increaseHealth() {
	if (_health <= 3)
		_health = 4;
	else if (_health <= 7)
		_health = 8;
	else
		_health = 12;
}

void Logic::endCredits() {
	_nodeNum = 0;
	_vm->_bTimer = 190;
	_vm->readMouse();

	while(_vm->_bTimer != 0 && _vm->_mouseButton == 0) {
		_vm->checkTimers();
		_vm->refreshSound();
		_vm->checkMainScreen();
		_vm->readMouse();
	}

	_vm->fadeToBlack1();
	_vm->drawRect(4, 17, 228, 161, 0);
	_vm->playSound(0);
	dsAll();
	_vm->_iconsClosed = true;
	_vm->playMovie(199);
	dsAll();
	_vm->playMovie(205);
	dsAll();
	_vm->fadeToBlack1();
	_vm->drawRect(4, 17, 228, 161, 0);
	_vm->playMovie(201);
	_vm->fShowPic(125);
	_currMap = 3;
	dsAll();
	_pouch = false;
	memset(_inventory, 0xFF, 19);
	_statPlay = 994;
	_vm->_loopFlag = true;
}

void Logic::gameHelp_Sub43C() {
	_vm->fadeToBlack1();
	_currMap = _vm->_asMap;
	_vm->drawRect(4, 17, 228, 161, 0);
	_vm->restoreAS();
	_vm->_userInput = 0;
	_vm->_gameMode = 0;
	_vm->_iconsClosed = false;
	_vm->_treeLeftSta = _vm->_oldTLS;
	_eye = _oldEye;
	_help = _oldHelp;
	_pouch = _oldPouch;
}

void Logic::gameHelp() {
	if (!_vm->_gameMode) {
		if (_vm->_userInput == 0x43C) {
			_vm->saveAS();
			_vm->_asMap = _currMap;
			_currMap = 0;
			_vm->fadeToBlack1();
			_vm->drawRect(4, 17, 228, 161, 0);
			_vm->drawHelpScreen();
			_vm->_gameMode = 1;
			_vm->_oldTLS = _vm->_treeLeftSta;
			_vm->_treeLeftSta = 0;
			_vm->_iconsClosed = true;
			_oldEye = _eye;
			_eye = false;
			_oldHelp = _help;
			_oldPouch = _pouch;
			_pouch = false;
			_vm->_userInput = 0;
		}
		if (_vm->_userInput == 0x44F) {
			_vm->saveAS();
			_vm->_asMap = _currMap;
			_vm->_gameMode = 2;
			_vm->_oldTLS = _vm->_treeLeftSta;
			_vm->_treeLeftSta = 0;
			_vm->_iconsClosed = true;
			_oldEye = _eye;
			_eye = false;
			_oldHelp = _help;
			_help = false;
			_oldPouch = _pouch;
			_vm->fadeToBlack1();
			_vm->drawRect(4, 17, 228, 161, 0);
			_vm->drawInventory();

			if (_nodes[68] == 1 || _nodes[29] == 1 || _vm->_itemInhibit)
				_currMap = 10;
			else
				_currMap = 11;

			_vm->_userInput = 0;
		}
	}
	if (_vm->_gameMode == 0)
		return;

	switch(_vm->_userInput) {
	case 0x240:
		_vm->fadeToBlack2();
		//TODO: Set _quitFlag to 1
		break;
	case 0x241:
		gameHelp_Sub43C();
		return;
		break;
	case 0x242:
		if (_vm->_noMusic) {
			_vm->_noMusic = false;
			_vm->playSound(1);
		} else {
			_vm->_noMusic = true;
			_vm->playSound(0);
		}
		_vm->drawHelpScreen();
		break;
	case 0x243: {
		_vm->fadeToBlack2();
		_vm->_keyActive = false;
		_vm->_noIFScreen = true;
		_vm->playSound(0);
		// The demo isn't saving pMovie.
		// It's obviously a bug and this behavior wasn't kept in ScummVM
		int oldPMovie = _vm->_pMovie;
		while(!_vm->_keyActive) {
			_vm->playMovie(54);
			_vm->fadeToBlack2();
		}
		_vm->_pMovie = oldPMovie;
		_vm->_noIFScreen = false;
		_vm->showPic(106);
		_vm->drawHelpScreen();
		_vm->_iconRedraw = true;
		_vm->playSound(1);
				}
				break;
	case 0x244:
		//TODO: Set _quitFlag to 2
		_vm->_quit = true;
		break;
	case 0x245: {
		_vm->fadeToBlack1();
		// The demo isn't saving pMovie.
		// It's obviously a bug and this behavior wasn't kept in ScummVM
		int oldPMovie = _vm->_pMovie;
		_vm->drawRect(4, 17, 228, 161, 0);
		_vm->playMovie(205);
		_vm->fadeToBlack1();
		_vm->drawRect(4, 17, 228, 161, 0);
		_vm->playMovie(199);
		_vm->fadeToBlack1();
		_vm->drawRect(4, 17, 228, 161, 0);
		_vm->drawHelpScreen();
		_vm->_pMovie = oldPMovie;
		}
		break;
	case 0x246:
		_vm->saveGame();
		break;
	case 0x43B:
	case 0x43C:
	case 0x44F:
		gameHelp_Sub43C();
		return;
	}

	if (_vm->_userInput > 0x427 && _vm->_userInput < 0x43A)
		_vm->fShowPic(130 + _vm->_userInput - 0x428);

	if (_vm->_userInput == 0x260) {
		_vm->drawInventory();
		if (_nodes[68] == 1 || _nodes[29] == 1)
			_currMap = 10;
		else
			_currMap = 11;

		_vm->_userInput = 0;
	}
}

void Logic::switchAS() {
	_vm->_asMode = false;
	_currMap = _vm->_asMap;
	_vm->_treeLeftSta = _vm->_oldTLS;
	_vm->_treeRightSta = _vm->_oldTRS;
	_pouch = _oldPouch;
	_help = _oldHelp;
	_vm->_iconsClosed = _vm->_oldIconsClosed;
}

void Logic::synchronize(Common::Serializer &s) {
	s.syncAsSint16LE(_statPlay);
	s.syncAsSint16LE(_health);
	s.syncAsSint16LE(_healthOld);
	s.syncAsSint16LE(_lastObstacle);
	s.syncAsSint16LE(_nextNode);
	s.syncAsSint16LE(_nodeNum);
	s.syncAsSint16LE(_rtnNode);
	s.syncAsSint16LE(_robberyNode);

	for (int i = 0; i < 18; i++)
		s.syncAsSint16LE(_inventory[i]);

	for (int i = 0; i < 99; i++)
		s.syncAsSint16LE(_nodes[i]);

	s.syncAsByte(_oldEye);
	s.syncAsByte(_fstFwd);
	s.syncAsByte(_help);
	s.syncAsByte(_lastObs);
	s.syncAsByte(_oldPouch);
	s.syncAsByte(_replay);
	s.syncAsByte(_spell1);
	s.syncAsByte(_spell2);
	s.syncAsByte(_spell3);
	s.syncAsByte(_tideCntl);
}

} // NameSpace
