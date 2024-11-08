/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room97.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

int Room97::_word18DB2E;
bool Room97::_slimeThrown;
bool Room97::_bool18DB32;
int Room97::_word18DB34;
int Room97::_word18DB36;
int Room97::_word18DB38;
bool Room97::_bool18DB3A;

void Room97::entry() {
	_G(det)->playSound(0, 0);
	_G(SetUpScreenFunc) = setup_func;
	_G(zoom_horizont) = 0;

	_G(spieler_mi)[P_HOWARD].Mode = true;
	_G(spieler_mi)[P_NICHELLE].Mode = true;
	_G(gameState).ZoomXy[P_NICHELLE][0] = 24;
	_G(gameState).ZoomXy[P_NICHELLE][1] = 43;
	_G(gameState).ZoomXy[P_HOWARD][0] = 24;
	_G(gameState).ZoomXy[P_HOWARD][1] = 42;

	_bool18DB32 = false;
	_G(gameState)._personRoomNr[P_NICHELLE] = 97;
	_G(zoom_mov_fak) = 0;
	_G(gameState).ScrollxStep = 2;
	_G(det)->hideStaticSpr(14);

	if (_G(gameState).flags36_2) {
		_G(det)->hideStaticSpr(14);
		_G(det)->hideStaticSpr(18);

		if (!_G(gameState).flags36_4)
			_G(det)->showStaticSpr(17);
	}
	
	if (_G(gameState).flags36_10) {
		_G(det)->startDetail(11, 255, ANI_FRONT);
		_G(det)->stopDetail(5);
		_G(det)->startDetail(6, 255, ANI_FRONT);
		_G(det)->startDetail(12, 255, ANI_FRONT);
	}

	if (_G(gameState).flags36_40) {
		_G(det)->startDetail(14, 255, ANI_FRONT);
		_G(det)->showStaticSpr(13);
		_G(atds)->delControlBit(543, ATS_ACTIVE_BIT);
	} else {
		_G(atds)->setControlBit(543, ATS_ACTIVE_BIT);
	}

	if (_G(gameState).flags36_20) {
		_G(det)->setDetailPos(27, 272, 110);
		for (int i = 0; i < 2; ++i) {
			_G(det)->stopDetail(23 + i);
			_G(det)->startDetail(27 + i, 255, ANI_FRONT);
		}
		_G(det)->stopSound(0);
		_G(det)->stopSound(0);
		_G(det)->stopSound(0);
	}

	if (_G(gameState).flags37_1)
		_G(det)->showStaticSpr(19);

	if (_G(gameState).flags37_4)
		_G(det)->showStaticSpr(20);

	_word18DB2E = 0;
	_word18DB34 = 332;
	_word18DB36 = 110;
	_word18DB38 = 132;

	if (_G(flags).LoadGame)
		return;

	if (_G(gameState).flags37_8) {
		hideCur();
		_G(out)->setPointer(nullptr);
		_G(out)->cls();

		_G(gameState)._personRoomNr[P_HOWARD] = _G(gameState)._personRoomNr[P_NICHELLE] = 89;
		_G(gameState).SVal2 = 0;
		_G(gameState).flags35_4 = true;
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(19, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		showCur();
		switchRoom(89);
	} else {
		_G(gameState).scrollx = 800;
		setPersonPos(1067, 98, P_CHEWY, P_LEFT);
		setPersonPos(1092, 83, P_HOWARD, P_LEFT);
		setPersonPos(130, 29, P_NICHELLE, P_RIGHT);
	}
}

void Room97::xit() {
	_G(gameState).ScrollxStep = 1;
}

void Room97::gedAction(int index) {
	switch (index) {
	case 50:
		proc2();
		break;
	case 51:
		proc3();
		break;
	case 52:
		proc13();
		break;
	case 53:
		proc12();
		break;
	case 54:
		proc4();
		break;
	case 55:
		if (_G(gameState).flags36_20)
			auto_scroll(268, 0);
		break;
	case 56:
		sensorAnim();
		break;
	default:
		break;
	}
}

void Room97::setup_func() {
	_G(spieler_mi)[P_CHEWY].Vorschub = 4;
	_G(spieler_mi)[P_HOWARD].Vorschub = 6;
	_G(spieler_mi)[P_NICHELLE].Vorschub = 4;

	if (_G(gameState).flags35_80 && _G(moveState)->Xypos[P_CHEWY] > 830)
		setPersonPos(830, 98, P_CHEWY, P_RIGHT);

	if (_word18DB2E)
		--_word18DB2E;
	else {
		_word18DB2E = _G(gameState).DelaySpeed / 2;
		if (_word18DB34 != 710)
			_word18DB34 += 2;
		else if (_word18DB36 < 200) {
			_word18DB36 += 2;
			_word18DB38 += 2;
		} else {
			if (!_G(gameState).flags36_8) {
				_word18DB34 = 332;
				_word18DB36 = _G(gameState).flags36_10 ? 106 : 110;
			}
			_bool18DB3A = true;
		}

		if (_bool18DB3A) {
			if (_word18DB38 <= 132)
				_bool18DB3A = false;
			else
				_word18DB38 -= 2;
		}

		_G(det)->setStaticPos(16, 699, _word18DB38, false, false);
		if (_G(gameState).flags36_10) {
			_G(det)->setDetailPos(12, _word18DB34, _word18DB36);
		} else {
			_G(det)->setStaticPos(14, _word18DB34, _word18DB36, false, false);
		}
	}

	if (_G(gameState).flags37_1 && _G(menu_item) == CUR_WALK) {
		if (_G(gameState).scrollx + g_events->_mousePos.x >= 487 && _G(gameState).scrollx + g_events->_mousePos.x <= 522 && g_events->_mousePos.y >= 23 && g_events->_mousePos.y <= 59)
			cursorChoice(CUR_EXIT_TOP);
		else
			cursorChoice(CUR_WALK);
	}

	calc_person_look();

	const int chewyPosX = _G(moveState)[P_CHEWY].Xypos[0];
	if (!_G(gameState).flags36_80) {
		int destX, destY;

		if (chewyPosX > 980) {
			destX = 1080;
			destY = 91;
		} else if (chewyPosX > 880) {
			destX = 994;
			destY = 98;
		} else if (chewyPosX > 780) {
			destX = 824;
			destY = 80;
		} else if (chewyPosX > 650) {
			destX = 758;
			destY = 121;
		} else if (chewyPosX > 420) {
			destX = 590;
			destY = 111;
		} else {
			destX = 412;
			destY = 112;
		}

		if (_G(HowardMov) != 1)
			goAutoXy(destX, destY, P_HOWARD, ANI_GO);
	}

	if (!_bool18DB32)
		return;

	int destX = -1;
	int destY = -1;
	const int nichellePosX = _G(moveState)[P_NICHELLE].Xypos[0];
	if (chewyPosX > 250 && nichellePosX < 232) {
		destX = 232;
		destY = 27;
	} else if (nichellePosX >= 249) {
		destX = 370;
		destY = 20;
	} else if (nichellePosX >= 232) {
		destX = 249;
		destY = 20;
	}	
	
	if (destX != -1)
		goAutoXy(destX, destY, P_NICHELLE, ANI_GO);
}

void Room97::proc2() {
	if (_G(flags).AutoAniPlay)
		return;

	_G(flags).AutoAniPlay = true;
	stopPerson(P_CHEWY);
	hideCur();
	_G(Sdi)[6].z_ebene = 6;
	_G(Sdi)[24].z_ebene = 6;
	_G(gameState).flags36_1 = false;
	_G(spieler_mi)[P_CHEWY].Mode = true;
	goAutoXy(455, 59, P_CHEWY, ANI_WAIT);
	goAutoXy(496, 99, P_CHEWY, ANI_WAIT);
	goAutoXy(530, 99, P_CHEWY, ANI_WAIT);
	goAutoXy(572, 139, P_CHEWY, ANI_WAIT);
	goAutoXy(588, 129, P_CHEWY, ANI_WAIT);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	_G(Sdi)[6].z_ebene = 166;
	_G(Sdi)[24].z_ebene = 157;
	showCur();
	_G(flags).AutoAniPlay = false;
}

void Room97::proc3() {
	if (_G(flags).AutoAniPlay)
		return;

	_G(flags).AutoAniPlay = true;
	hideCur();
	stopPerson(P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	
	if (!_G(gameState).flags36_20) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		startAadWait(557);
		goAutoXy(588, 129, P_CHEWY, ANI_WAIT);
	} else if (!_G(gameState).flags36_40) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		startAadWait(568);
		goAutoXy(588, 129, P_CHEWY, ANI_WAIT);
	} else {
		_G(gameState).flags36_1 = true;
		_G(HowardMov) = 1;
		goAutoXy(572, 139, P_CHEWY, ANI_WAIT);
		_G(Sdi)[6].z_ebene = 6;
		_G(Sdi)[24].z_ebene = 6;

		goAutoXy(530, 99, P_CHEWY, ANI_WAIT);
		goAutoXy(496, 99, P_CHEWY, ANI_WAIT);
		goAutoXy(455, 59, P_CHEWY, ANI_WAIT);
		goAutoXy(440, 52, P_CHEWY, ANI_WAIT);
		if (!_G(gameState).flags36_80) {
			goAutoXy(572, 122, P_HOWARD, ANI_WAIT);
			goAutoXy(526, 83, P_HOWARD, ANI_WAIT);
			goAutoXy(500, 83, P_HOWARD, ANI_WAIT);
		}

		autoMove(6, P_CHEWY);
		if (!_G(gameState).flags36_80) {
			goAutoXy(442, 35, P_HOWARD, ANI_WAIT);
			goAutoXy(497, 31, P_HOWARD, ANI_WAIT);
			startAadWait(562);
		}
		_G(spieler_mi)[P_CHEWY].Mode = false;
		_G(Sdi)[6].z_ebene = 166;
		_G(Sdi)[24].z_ebene = 157;
		_G(gameState).flags36_80 = true;
	}
	showCur();
	_G(flags).AutoAniPlay = false;
}

void Room97::proc4() {
	if (_G(flags).AutoAniPlay)
		return;

	if (!_G(gameState).flags35_80) {
		_G(flags).AutoAniPlay = true;
		hideCur();
		_G(spieler_mi)[P_CHEWY].Mode = true;
		stopPerson(P_CHEWY);
		_G(det)->playSound(9, 0);
		_G(det)->stopSound(1);
		startSetAILWait(9, 1, ANI_FRONT);
		_G(det)->showStaticSpr(21);

		while (_G(moveState)[P_HOWARD].Xypos[0] > 996) {
			setupScreen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(det)->showStaticSpr(2);
		_G(HowardMov) = 1;
		
		goAutoXy(967, 111, P_CHEWY, ANI_WAIT);
		goAutoXy(1008, 93, P_CHEWY, ANI_WAIT);
		goAutoXy(1037, 90, P_CHEWY, ANI_WAIT);

		goAutoXy(995, 77, P_HOWARD, ANI_WAIT);
		goAutoXy(1047, 87, P_HOWARD, ANI_WAIT);

		startSetAILWait(29, 1, ANI_FRONT);
		_G(det)->hideStaticSpr(21);
		_G(det)->playSound(9, 1);
		_G(det)->stopSound(0);
		startSetAILWait(9, 0, ANI_BACK);

		goAutoXy(1008, 93, P_CHEWY, ANI_WAIT);
		goAutoXy(967, 111, P_CHEWY, ANI_WAIT);
		goAutoXy(995, 82, P_HOWARD, ANI_WAIT);
		_G(det)->hideStaticSpr(2);
		_G(spieler_mi)[P_CHEWY].Mode = false;
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		startAadWait(553);
		_G(HowardMov) = 0;
		showCur();
	}
		
	_G(flags).AutoAniPlay = false;
}

int Room97::proc5() {
	if (!isCurInventory(114))
		return 0;

	hideCur();
	autoMove(0, P_CHEWY);
	auto_scroll(800, 0);
	
	while (_G(moveState)[P_HOWARD].Xypos[0] < 1080) {
		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN0;
	}

	flic_cut(FCUT_122);
	register_cutscene(34);

	_G(det)->playSound(0, 0);
	_G(gameState).scrollx = 720;
	setPersonPos(822, 98, P_CHEWY, P_LEFT);
	setPersonPos(861, 81, P_HOWARD, P_LEFT);
	delInventory(_G(cur)->getInventoryCursor());
	_G(det)->showStaticSpr(21);
	_G(gameState).flags35_80 = true;
	startAadWait(546);
	_G(det)->hideStaticSpr(21);
	_G(det)->playSound(9, 1);
	_G(det)->stopSound(0);
	startSetAILWait(9, 0, ANI_GO);
	
	showCur();
	return 1;
}

int Room97::proc6() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(1, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	goAutoXy(298, 120, P_CHEWY, ANI_WAIT);
	setPersonSpr(P_LEFT, P_CHEWY);
	start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
	_G(det)->hideStaticSpr(15);
	startSetAILWait(1, 1, ANI_FRONT);
	_G(det)->showStaticSpr(28);
	autoMove(1, P_CHEWY);
	_G(det)->hideStaticSpr(18);
	startSetAILWait(15, 1, ANI_FRONT);
	_G(det)->showStaticSpr(17);
	start_spz(CH_TALK5, 255, false, P_CHEWY);
	startAadWait(558);
	_G(det)->hideStaticSpr(28);
	_G(det)->stopDetail(5);
	_G(det)->showStaticSpr(15);
	_G(gameState).flags36_2 = true;
	_G(atds)->delControlBit(538, ATS_ACTIVE_BIT);
	_G(atds)->setControlBit(531, ATS_ACTIVE_BIT);
	_G(atds)->set_all_ats_str(530, 1, ATS_DATA);
	_G(atds)->setControlBit(532, ATS_ACTIVE_BIT);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	_G(gameState).flags36_8 = true;

	showCur();
	return 1;
}

int Room97::proc7() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	_G(gameState)._personHide[P_CHEWY] = true;
	_G(det)->hideStaticSpr(17);
	startSetAILWait(22, 1, ANI_FRONT);
	_G(atds)->setControlBit(538, ATS_ACTIVE_BIT);
	_G(atds)->set_all_ats_str(530, 2, ATS_DATA);
	new_invent_2_cur(SLIME_INV);
	_G(gameState).flags36_4 = true;
	setPersonPos(304, 102, P_CHEWY, P_RIGHT);
	_G(gameState)._personHide[P_CHEWY] = false;
	autoMove(1, P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = false;

	showCur();
	return 1;
}

int Room97::proc8() {
	if (!isCurInventory(94))
		return 0;

	hideCur();
	autoMove(2, P_CHEWY);
	int diaNr = -1;
	if (!_G(gameState).flags36_2)
		diaNr = 559;
	else if (!_G(gameState).flags36_4)
		diaNr = 560;
	else {
		start_spz_wait(14, 1, false, P_CHEWY);
		_G(det)->startDetail(11, 255, false);
		delInventory(_G(cur)->getInventoryCursor());
		_G(gameState).flags36_10 = true;
		_G(gameState).flags36_8 = false;
		_G(det)->startDetail(12, 255, false);
		_G(atds)->set_all_ats_str(530, 3, ATS_DATA);
		_G(atds)->delControlBit(532, ATS_ACTIVE_BIT);
		_G(atds)->set_all_ats_str(532, 1, ATS_DATA);
		_G(det)->startDetail(6, 255, false);
		_G(det)->hideStaticSpr(14);
		autoMove(3, P_CHEWY);
		auto_scroll(406, 0);
		waitShowScreen(40);
		_G(det)->stopDetail(24);
		_G(det)->playSound(26, 0);
		startSetAILWait(25, 1, ANI_FRONT);
		_G(det)->startDetail(26, 255, false);
		_G(det)->stopDetail(23);
		_G(det)->startDetail(27, 255, false);
		waitShowScreen(80);
		autoMove(4, P_CHEWY);
		auto_scroll(646, 0);
		startAadWait(566);
		waitShowScreen(60);
		startAadWait(567);
		_G(det)->stopDetail(26);
		_G(det)->stopDetail(27);
		_G(gameState).flags36_20 = true;
		_G(det)->startDetail(28, 255, false);
		_G(det)->setDetailPos(27, 272, 110);
		_G(det)->startDetail(27, 255, false);
		_G(atds)->setControlBit(533, ATS_ACTIVE_BIT);
		_G(atds)->setControlBit(534, ATS_ACTIVE_BIT);
	}

	if (diaNr != -1) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		startAadWait(diaNr);
	}

	showCur();
	return 1;
}

int Room97::proc9() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(5, P_CHEWY);
	
	if (!_G(gameState).flags36_20) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		startAadWait(556);
	} else {
		start_spz_wait(13, 1, false, P_CHEWY);
		startSetAILWait(0, 1, ANI_FRONT);
		_G(det)->showStaticSpr(13);
		startSetAILWait(13, 1, ANI_FRONT);
		_G(det)->startDetail(14, 255, false);
		startAadWait(555);
		_G(gameState).flags36_40 = true;
		_G(atds)->setControlBit(539, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(537, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(535, ATS_ACTIVE_BIT);
		_G(atds)->delControlBit(543, ATS_ACTIVE_BIT);
	}

	showCur();
	return 1;
}

int Room97::proc10() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();
	autoMove(8, P_CHEWY);
	start_spz_wait(13, 1, false, P_CHEWY);
	_G(det)->playSound(7, 0);
	_G(det)->stopSound(1);
	startSetAILWait(7, 1, ANI_FRONT);
	_G(det)->showStaticSpr(19);
	_G(gameState).flags37_1 = true;
	_G(atds)->setControlBit(543, ATS_ACTIVE_BIT);
	_G(menu_item) = CUR_WALK;
	cursorChoice(CUR_WALK);

	showCur();
	return 1;
}

int Room97::proc11() {
	if (_G(cur)->usingInventoryCursor())
		return 0;

	hideCur();

	if (!_G(gameState).flags37_2) {
		start_spz(CH_TALK5, 255, false, P_CHEWY);
		startAadWait(569);
	} else {
		autoMove(9, P_CHEWY);
		startAadWait(570);
		start_spz_wait(13, 1, false, P_CHEWY);
		autoMove(7, P_CHEWY);
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		startAadWait(571);
		_G(det)->playSound(4, 0);
		_G(det)->startDetail(4, 1, false);
		autoMove(12, P_CHEWY);
		start_spz_wait(64, 1, false, P_CHEWY);
		start_spz(65, 255, false, P_CHEWY);
		startAadWait(561);
		_G(gameState).SVal1 = 97;
		_G(gameState).SVal2 = 523;
		cur_2_inventory();
		_G(gameState).flags37_8 = true;
		switchRoom(92);
		showCur(); // probably useless, but present in the original
	}

	showCur();
	return 1;
}

void Room97::proc12() {
	if (!_G(gameState).flags37_1) {
		stopPerson(P_CHEWY);
		setPersonPos(491, 42, P_CHEWY, P_RIGHT);
		return;
	}

	if (_G(flags).AutoAniPlay)
		return;

	_G(flags).AutoAniPlay = true;
	hideCur();
	stopPerson(P_CHEWY);
	_G(det)->showStaticSpr(26);
	goAutoXy(481, 39, P_CHEWY, ANI_WAIT);
	_G(gameState)._personHide[P_CHEWY] = true;
	setPersonPos(327, 42, P_CHEWY, P_LEFT);
	waitShowScreen(50);
	setPersonPos(347, 42, P_CHEWY, P_LEFT);
	_G(gameState)._personHide[P_CHEWY] = false;
	_G(det)->hideStaticSpr(26);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	showCur();
	_G(flags).AutoAniPlay = false;
}

void Room97::proc13() {
	if (!_G(gameState).flags37_1 || _G(flags).AutoAniPlay)
		return;

	_G(flags).AutoAniPlay = true;
	hideCur();
	stopPerson(P_CHEWY);
	_G(spieler_mi)[P_CHEWY].Mode = true;
	_G(det)->showStaticSpr(26);
	_G(gameState)._personHide[P_CHEWY] = true;
	setPersonPos(508, 41, P_CHEWY, P_LEFT);
	waitShowScreen(50);
	setPersonPos(488, 41, P_CHEWY, P_LEFT);
	_G(gameState)._personHide[P_CHEWY] = false;
	autoMove(6, P_CHEWY);
	_G(det)->hideStaticSpr(26);
	_G(spieler_mi)[P_CHEWY].Mode = false;
	showCur();
	_G(flags).AutoAniPlay = false;
}

int Room97::throwSlime() {
	if (!isCurInventory(SLIME_INV))
		return 0;

	_slimeThrown = true;
	hideCur();
	delInventory(_G(cur)->getInventoryCursor());
	_G(menu_item) = CUR_USE;
	cursorChoice(CUR_USE);
	
	return 1;
}

void Room97::sensorAnim() {
	if (_G(gameState).flags37_4 || _G(flags).AutoAniPlay)
		return;

	_G(flags).AutoAniPlay = true;
	stopPerson(P_CHEWY);
	_G(menu_item) = CUR_USE;
	cursorChoice(CUR_USE);
	setPersonPos(294, 42, P_CHEWY, P_LEFT);
	_G(atds)->delControlBit(541, ATS_ACTIVE_BIT);
	_slimeThrown = false;
	_G(det)->startDetail(16, 1, 0);
	
	while (_G(det)->get_ani_status(16)) {
		get_user_key(NO_SETUP);
		if (_G(minfo).button == 1 || g_events->getSwitchCode() == 28) {
			if (_G(cur)->usingInventoryCursor())
				_G(mouseLeftClick) = true;
		}

		setupScreen(DO_SETUP);
		SHOULD_QUIT_RETURN;
	}

	hideCur();
	if (!_slimeThrown) {
		startSetAILWait(17, 1, ANI_FRONT);
		_G(det)->startDetail(16, 1, true);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(20, 1, false);
		setPersonPos(318, 42, P_CHEWY, P_LEFT);
		_G(gameState)._personHide[P_CHEWY] = false;
	} else {
		_G(gameState).flags37_4 = true;
		delInventory(_G(cur)->getInventoryCursor());
		_G(det)->showStaticSpr(27);
		_G(gameState)._personHide[P_CHEWY] = true;
		startSetAILWait(21, 1, ANI_FRONT);
		_G(gameState)._personHide[P_CHEWY] = false;
		_G(det)->hideStaticSpr(27);
		startSetAILWait(18, 1, ANI_FRONT);

		_G(det)->playSound(8, 0);
		_G(det)->stopSound(1);
		startSetAILWait(8, 1, ANI_FRONT);
		_G(det)->showStaticSpr(20);
		autoMove(10, P_CHEWY);
		auto_scroll(60, 0);
		startAadWait(544);
		_bool18DB32 = true;
		autoMove(11, P_CHEWY);
		_bool18DB32 = false;
		_G(flags).AutoAniPlay = false;
		proc13();
		_G(flags).AutoAniPlay = true;
		autoMove(7, P_CHEWY);

		while (_G(gameState).scrollx < 368) {
			setupScreen(DO_SETUP);
			SHOULD_QUIT_RETURN;
		}

		_G(det)->showStaticSpr(26);
		setPersonPos(482, 24, P_NICHELLE, P_RIGHT);
		goAutoXy(477, 29, P_NICHELLE, ANI_WAIT);
		_G(det)->hideStaticSpr(26);
		_G(gameState).flags37_2 = true;
	}

	_G(atds)->setControlBit(541, ATS_ACTIVE_BIT);
	showCur();
	_G(flags).AutoAniPlay = false;
}

} // namespace Rooms
} // namespace Chewy
