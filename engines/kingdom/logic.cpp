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
	initVariables();
}

void Logic::initVariables() {
	_healthOld = 0;
	_health = 0;
	_tideCntl = false;
	_statPlay = 0;
	_spell3 = false;
	_spell2 = false;
	_spell1 = false;
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
	_lastObs = false;
	_lastObstacle = 0;
	_help = false;
	_eye = false;
	_fstFwd = false;

	for (int i = 0; i < 99; i++)
		_nodes[i] = 0;
	_currMap = 0;
}

void Logic::initPlay() {
	for (int i = 0; i != 19; ++i)
		_inventory[i] = -1;

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
	enableUIButtons();
	_pouch = true;

	initOpcodes();
}

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

	_opCodes[250] = &Logic::GPL2_250;
	_opCodes[251] = &Logic::GPL2_251;
	_opCodes[252] = &Logic::GPL2_252;

	_opCodes[900] = &Logic::GPL4_900;
	_opCodes[901] = &Logic::GPL4_901;
	_opCodes[991] = &Logic::GPL4_991;
	_opCodes[994] = &Logic::GPL4_994;

	if (_vm->isDemo())
		initOpcodesDemo();
	else
		initOpcodesFull();
}

void Logic::initOpcodesDemo() {
	_opCodes[992] = &Logic::GPL4_992_demo;
	_opCodes[993] = &Logic::GPL4_993_demo;
}

void Logic::initOpcodesFull() {
	_opCodes[260] = &Logic::GPL2_260;
	_opCodes[261] = &Logic::GPL2_261;
	_opCodes[270] = &Logic::GPL2_270;
	_opCodes[271] = &Logic::GPL2_271;
	_opCodes[272] = &Logic::GPL2_272;
	_opCodes[280] = &Logic::GPL2_280;
	_opCodes[290] = &Logic::GPL2_290;
	_opCodes[291] = &Logic::GPL2_291;
	_opCodes[292] = &Logic::GPL2_292;
	_opCodes[300] = &Logic::GPL2_300;
	_opCodes[301] = &Logic::GPL2_301;
	_opCodes[302] = &Logic::GPL2_302;
	_opCodes[310] = &Logic::GPL2_310;
	_opCodes[311] = &Logic::GPL2_311;
	_opCodes[312] = &Logic::GPL2_312;
	_opCodes[320] = &Logic::GPL2_320;
	_opCodes[321] = &Logic::GPL2_321;
	_opCodes[322] = &Logic::GPL2_322;
	_opCodes[330] = &Logic::GPL2_330;
	_opCodes[331] = &Logic::GPL2_331;
	_opCodes[332] = &Logic::GPL2_332;
	_opCodes[340] = &Logic::GPL2_340;
	_opCodes[341] = &Logic::GPL2_341;
	_opCodes[342] = &Logic::GPL2_342;
	_opCodes[350] = &Logic::GPL2_350;
	_opCodes[351] = &Logic::GPL2_351;
	_opCodes[360] = &Logic::GPL2_360;
	_opCodes[361] = &Logic::GPL2_361;
	_opCodes[362] = &Logic::GPL2_362;
	_opCodes[370] = &Logic::GPL2_370;
	_opCodes[371] = &Logic::GPL2_371;
	_opCodes[372] = &Logic::GPL2_372;
	_opCodes[380] = &Logic::GPL2_380;
	_opCodes[381] = &Logic::GPL2_381;
	_opCodes[382] = &Logic::GPL2_382;
	_opCodes[390] = &Logic::GPL2_390;
	_opCodes[400] = &Logic::GPL2_400;
	_opCodes[401] = &Logic::GPL2_401;
	_opCodes[402] = &Logic::GPL2_402;
	_opCodes[410] = &Logic::GPL2_410;
	_opCodes[411] = &Logic::GPL2_411;
	_opCodes[420] = &Logic::GPL2_420;
	_opCodes[421] = &Logic::GPL2_421;
	_opCodes[422] = &Logic::GPL2_422;
	_opCodes[430] = &Logic::GPL2_430;
	_opCodes[431] = &Logic::GPL2_431;
	_opCodes[440] = &Logic::GPL2_440;
	_opCodes[441] = &Logic::GPL2_441;
	_opCodes[442] = &Logic::GPL2_442;
	_opCodes[450] = &Logic::GPL2_450;
	_opCodes[451] = &Logic::GPL2_451;
	_opCodes[460] = &Logic::GPL2_460;
	_opCodes[461] = &Logic::GPL2_461;
	_opCodes[462] = &Logic::GPL2_462;
	_opCodes[480] = &Logic::GPL2_480;
	_opCodes[481] = &Logic::GPL2_481;
	_opCodes[482] = &Logic::GPL2_482;
	_opCodes[490] = &Logic::GPL2_490;
	_opCodes[491] = &Logic::GPL2_491;

	_opCodes[500] = &Logic::GPL3_500;
	_opCodes[501] = &Logic::GPL3_501;
	_opCodes[502] = &Logic::GPL3_502;
	_opCodes[510] = &Logic::GPL3_510;
	_opCodes[511] = &Logic::GPL3_511;
	_opCodes[512] = &Logic::GPL3_512;
	_opCodes[520] = &Logic::GPL3_520;
	_opCodes[521] = &Logic::GPL3_521;
	_opCodes[522] = &Logic::GPL3_522;
	_opCodes[530] = &Logic::GPL3_530;
	_opCodes[531] = &Logic::GPL3_531;
	_opCodes[540] = &Logic::GPL3_540;
	_opCodes[541] = &Logic::GPL3_541;
	_opCodes[542] = &Logic::GPL3_542;
	_opCodes[550] = &Logic::GPL3_550;
	_opCodes[551] = &Logic::GPL3_551;
	_opCodes[552] = &Logic::GPL3_552;
	_opCodes[560] = &Logic::GPL3_560;
	_opCodes[561] = &Logic::GPL3_561;
	_opCodes[562] = &Logic::GPL3_562;
	_opCodes[570] = &Logic::GPL3_570;
	_opCodes[571] = &Logic::GPL3_571;
	_opCodes[572] = &Logic::GPL3_572;
	_opCodes[580] = &Logic::GPL3_580;
	_opCodes[581] = &Logic::GPL3_581;
	_opCodes[582] = &Logic::GPL3_582;
	_opCodes[590] = &Logic::GPL3_590;
	_opCodes[591] = &Logic::GPL3_591;
	_opCodes[592] = &Logic::GPL3_592;
	_opCodes[600] = &Logic::GPL3_600;
	_opCodes[610] = &Logic::GPL3_610;
	_opCodes[611] = &Logic::GPL3_611;
	_opCodes[620] = &Logic::GPL3_620;
	_opCodes[621] = &Logic::GPL3_621;
	_opCodes[630] = &Logic::GPL3_630;
	_opCodes[631] = &Logic::GPL3_631;
	_opCodes[640] = &Logic::GPL3_640;
	_opCodes[641] = &Logic::GPL3_641;
	_opCodes[642] = &Logic::GPL3_642;
	_opCodes[650] = &Logic::GPL3_650;
	_opCodes[651] = &Logic::GPL3_651;
	_opCodes[660] = &Logic::GPL3_660;
	_opCodes[661] = &Logic::GPL3_661;
	_opCodes[670] = &Logic::GPL3_670;
	_opCodes[671] = &Logic::GPL3_671;
	_opCodes[680] = &Logic::GPL3_680;
	_opCodes[690] = &Logic::GPL3_690;
	_opCodes[691] = &Logic::GPL3_691;
	_opCodes[692] = &Logic::GPL3_692;
	_opCodes[700] = &Logic::GPL3_700;
	_opCodes[701] = &Logic::GPL3_701;
	_opCodes[710] = &Logic::GPL3_710;
	_opCodes[711] = &Logic::GPL3_711;
	_opCodes[712] = &Logic::GPL3_712;
	_opCodes[720] = &Logic::GPL3_720;
	_opCodes[721] = &Logic::GPL3_721;
	_opCodes[730] = &Logic::GPL3_730;
	_opCodes[731] = &Logic::GPL3_731;
	_opCodes[740] = &Logic::GPL3_740;
	_opCodes[741] = &Logic::GPL3_741;
	_opCodes[760] = &Logic::GPL3_760;
	_opCodes[761] = &Logic::GPL3_761;
	_opCodes[780] = &Logic::GPL3_780;
	_opCodes[781] = &Logic::GPL3_781;
	_opCodes[790] = &Logic::GPL3_790;
	_opCodes[791] = &Logic::GPL3_791;

	_opCodes[992] = &Logic::GPL4_992;
	_opCodes[993] = &Logic::GPL4_993;
}

void Logic::enableUIButtons() {
	_help = true;
	_eye = true;
	_replay = true;
	_pouch = true;
	_fstFwd = true;
}

void Logic::disableUIButtons() {
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

	if (_nodes[28] || _vm->_rnd->getRandomNumber(6) == 0) {
		if (!_nodes[48] || _robberyNode != _nodeNum) {
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
	disableUIButtons();
	_vm->_iconsClosed = true;
	_vm->playMovie(199);
	disableUIButtons();
	_vm->playMovie(205);
	disableUIButtons();
	_vm->fadeToBlack1();
	_vm->drawRect(4, 17, 228, 161, 0);
	_vm->playMovie(201);
	_vm->fShowPic(125);

	characterDeath();
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

			if (_nodes[67] == 1 || _nodes[28] == 1 || _vm->_itemInhibit)
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
		// New game
		_vm->fadeToBlack2();
		//TODO: Set _quitFlag to 1
		break;
	case 0x241:
		// Resume game
		gameHelp_Sub43C();
		return;
		break;
	case 0x242:
		// Music On/Off
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
		// Show Demo
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
		// Quit
		//TODO: Set _quitFlag to 2
		_vm->_quit = true;
		break;
	case 0x245: {
		// Show Credits

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
		if (_nodes[67] == 1 || _nodes[28] == 1)
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

void Logic::characterDeath() {
	_currMap = 3;
	disableUIButtons();
	_pouch = false;
	for (int i = 0; i != 19; ++i) {
		_inventory[i] = -1;
	}
	_statPlay = 994;
	_vm->_loopFlag = true;
}

void Logic::executeOpcode() {
	if (_opCodes.contains(_statPlay)) {
		Opcode op = _opCodes[_statPlay];
		(this->*op)();
	} else
		warning("Unknown opcode: %d", _statPlay);
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
