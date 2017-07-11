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
