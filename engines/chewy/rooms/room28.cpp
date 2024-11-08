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
#include "chewy/ani_dat.h"
#include "chewy/rooms/room28.h"
#include "chewy/sound.h"

namespace Chewy {
namespace Rooms {

void Room28::entry(int16 eib_nr) {
	_G(zoom_horizont) = 140;
	_G(flags).ZoomMov = true;
	_G(zoom_mov_fak) = 3;
	_G(gameState).ScrollxStep = 2;

	if (_G(gameState).R28RKuerbis)
		_G(det)->showStaticSpr(6);

	if (_G(gameState).R28LetterBox) {
		_G(det)->showStaticSpr(8);
		_G(det)->showStaticSpr(9);
	} else {
		_G(det)->showStaticSpr(7);
	}

	if (_G(gameState)._personRoomNr[P_HOWARD] == 28) {
		_G(gameState).ZoomXy[P_HOWARD][0] = 40;
		_G(gameState).ZoomXy[P_HOWARD][1] = 40;
		_G(gameState)._personHide[P_HOWARD] = false;
		hideCur();
		_G(SetUpScreenFunc) = setup_func;

		if (_G(gameState).R40Wettbewerb) {
			_G(gameState).scrollx = 320;
			_G(gameState)._personHide[P_CHEWY] = false;
			haendler();
			_G(gameState).R40Wettbewerb = false;
		} else {
			switch (eib_nr) {
			case 55:
				if (!_G(gameState).R28EntryHaus) {
					_G(gameState).R28EntryHaus = true;
					setPersonPos(232, 100, P_HOWARD, P_RIGHT);
					autoMove(3, P_HOWARD);
					goAutoXy(320, _G(moveState)[P_HOWARD].Xypos[1], P_HOWARD, ANI_WAIT);
					startAadWait(176);
				} else if (_G(gameState).R31SurFurz && !_G(gameState).R28RKuerbis) {
					_G(gameState).R28RKuerbis = true;
					_G(gameState).ScrollxStep = 2;
					autoMove(6, P_CHEWY);
					waitShowScreen(10);
					startAadWait(194);
					_G(gameState).room_e_obj[62].Attribut = 255;
					_G(atds)->set_all_ats_str(208, 1, ATS_DATA);
					_G(flags).NoScroll = true;

					auto_scroll(0, 0);
					flic_cut(FCUT_064);
					_G(det)->showStaticSpr(6);

					if (!_G(gameState).R40TeilKarte) {
						_G(out)->fadeOut();
						_G(out)->set_partialpalette(_G(pal), 255, 1);
						startAadWait(607);
						_G(fx_blend) = BLEND3;
					}

					_G(flags).NoScroll = false;

					if (_G(gameState).R40TeilKarte) {
						haendler();
					} else {
						waitShowScreen(60);
						startAadWait(195);
					}

					_G(gameState).ScrollxStep = 2;
				}
				break;

			case 70:
			case 77:
				if (_G(gameState).R28PostCar) {
					_G(gameState).R28PostCar = false;
					_G(out)->setPointer(nullptr);
					_G(out)->cls();
					flic_cut(FCUT_063);
					_G(gameState).R28ChewyPump = false;
					invent_2_slot(K_MASKE_INV);
					_G(atds)->set_all_ats_str(209, 0, ATS_DATA);
					_G(gameState).room_e_obj[69].Attribut = 255;
					load_chewy_taf(CHEWY_NORMAL);

					setPersonPos(480, 118, P_HOWARD, P_LEFT);
					setPersonPos(440, 146, P_CHEWY, P_RIGHT);
					_G(fx_blend) = BLEND_NONE;
					_G(flags).ExitMov = false;
					_G(spieler_mi)[P_HOWARD].Mode = true;
					goAutoXy(420, 113, P_HOWARD, ANI_WAIT);
					_G(spieler_mi)[P_HOWARD].Mode = false;
					setPersonPos(440, 146, P_CHEWY, P_LEFT);
					startAadWait(193);
				} else {
					setPersonPos(480, 100, P_HOWARD, P_LEFT);
					goAutoXy(420, 113, P_HOWARD, ANI_GO);
					_G(flags).ExitMov = false;
					autoMove(6, P_CHEWY);
				}
				break;

			default:
				break;
			}
		}

		showCur();
	}
}

void Room28::xit(int16 eib_nr) {
	_G(gameState).R28PostCar = false;
	_G(gameState).ScrollxStep = 1;
	hideCur();

	if (_G(gameState)._personRoomNr[P_HOWARD] == 28 && eib_nr == 69) {
		_G(SetUpScreenFunc) = nullptr;
		if (!_G(gameState).R28ExitTown) {
			startAadWait(178);
			_G(gameState).R28ExitTown = true;
		}

		_G(flags).ExitMov = false;
		autoMove(6, P_HOWARD);
		_G(gameState)._personRoomNr[P_HOWARD] = 40;
	}

	showCur();
}

void Room28::gedAction(int index) {
	if (!index)
		get_pump();
}

void Room28::haendler() {
	hideCur();
	_G(SetUpScreenFunc) = nullptr;
	_G(gameState).R28ChewyPump = true;
	delInventory(K_MASKE_INV);
	_G(atds)->set_all_ats_str(209, 1, ATS_DATA);
	_G(gameState).room_e_obj[69].Attribut = EXIT_TOP;
	load_chewy_taf(CHEWY_PUMPKIN);

	setPersonPos(480, 113, P_HOWARD, P_RIGHT);
	setPersonPos(490, 146, P_CHEWY, P_RIGHT);
	_G(det)->set_static_ani(2, -1);

	while (_G(gameState).scrollx < 300) {
		SHOULD_QUIT_RETURN;
		setupScreen(DO_SETUP);
	}

	startAadWait(196);
	_G(det)->del_static_ani(2);
	_G(det)->set_static_ani(3, -1);
	startAadWait(197);
	_G(det)->del_static_ani(3);
	_G(det)->set_static_ani(4, -1);
	startAadWait(198);
	_G(SetUpScreenFunc) = setup_func;
	autoMove(4, P_CHEWY);
	hideCur();
	autoMove(3, P_CHEWY);
	setPersonSpr(P_RIGHT, P_CHEWY);
	_G(det)->del_static_ani(4);
	waitShowScreen(28);
	startAadWait(199);
	invent_2_slot(DOLLAR175_INV);
}

void Room28::setup_func() {
	calc_person_look();

	int16 x = (_G(moveState)[P_CHEWY].Xypos[0] > 350) ? 420 : 320;
	goAutoXy(x, 113, P_HOWARD, ANI_GO);
}

void Room28::use_surimy() {
	int16 dia_nr;
	int16 ani_nr;

	if (!_G(gameState).R28SurimyCar) {
		hideCur();
		_G(flags).NoScroll = true;
		auto_scroll(0, 0);
		_G(gameState).R28SurimyCar = true;
		start_spz_wait(CH_LGET_U, 1, false, P_CHEWY);
		flic_cut(FCUT_055);
		flic_cut(FCUT_056);
		ani_nr = CH_TALK3;
		dia_nr = 140;
		_G(atds)->set_all_ats_str(205, 1, ATS_DATA);
		_G(atds)->set_all_ats_str(222, 1, ATS_DATA);

	} else {
		ani_nr = CH_TALK5;
		dia_nr = 139;
		_G(obj)->calc_rsi_flip_flop(SIB_AUTO_R28);
	}

	_G(obj)->calc_all_static_detail();
	start_spz(ani_nr, 255, ANI_FRONT, P_CHEWY);
	startAadWait(dia_nr);
	_G(flags).NoScroll = false;
	showCur();
}

void Room28::set_pump() {
	hideCur();

	if (_G(gameState)._personRoomNr[P_CHEWY] == 28) {
		if (!_G(flags).AutoAniPlay) {
			_G(flags).AutoAniPlay = true;
			if (_G(moveState)[P_CHEWY].Xypos[0] < 380)
				autoMove(5, P_CHEWY);

			_G(gameState)._personHide[P_CHEWY] = true;
			int16 tmp = (_G(person_end_phase)[P_CHEWY] == P_RIGHT) ? 1 : 0;
			delInventory(K_MASKE_INV);
			_G(det)->setDetailPos(tmp, _G(moveState)[P_CHEWY].Xypos[0], _G(moveState)[P_CHEWY].Xypos[1]);

			if (_G(gameState).R28PumpTxt1 < 3) {
				start_aad(137);
				++_G(gameState).R28PumpTxt1;
			}

			startSetAILWait(tmp, 1, ANI_FRONT);
			_G(gameState)._personHide[P_CHEWY] = false;

			load_chewy_taf(CHEWY_PUMPKIN);
			_G(gameState).R28ChewyPump = true;

			if (_G(gameState).R39TranslatorUsed) {
				_G(gameState).room_e_obj[69].Attribut = EXIT_TOP;
			} else {
				_G(atds)->set_all_ats_str(209, 1, ATS_DATA);
			}

			if (_G(gameState)._personRoomNr[P_HOWARD] == 28 && _G(gameState).R28PumpTxt < 3) {
				stopPerson(P_HOWARD);
				_G(SetUpScreenFunc) = nullptr;
				g_engine->_sound->waitForSpeechToFinish();

				startAadWait(177);
				_G(SetUpScreenFunc) = setup_func;
				++_G(gameState).R28PumpTxt;
			}

			_G(flags).AutoAniPlay = false;
		}
	} else {
		startAtsWait(20, TXT_MARK_USE, 14, INV_USE_DEF);
	}

	showCur();
}

void Room28::get_pump() {

	if (_G(gameState).R28ChewyPump) {
		hideCur();

		stopPerson(P_CHEWY);
		_G(gameState).R28ChewyPump = false;
		_G(gameState)._personHide[P_CHEWY] = true;
		int16 tmp = (_G(person_end_phase)[P_CHEWY] == P_RIGHT) ? 1 : 0;
		_G(det)->setDetailPos(tmp, _G(moveState)[P_CHEWY].Xypos[0], _G(moveState)[P_CHEWY].Xypos[1]);
		startSetAILWait(tmp, 1, ANI_BACK);

		invent_2_slot(K_MASKE_INV);
		_G(atds)->set_all_ats_str(209, 0, ATS_DATA);
		_G(gameState).room_e_obj[69].Attribut = 255;
		_G(gameState)._personHide[P_CHEWY] = false;
		load_chewy_taf(CHEWY_NORMAL);

		if (_G(gameState).R28PumpTxt1 < 3) {
			start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			startAadWait(138);
		}

		showCur();
	}
}

int16 Room28::use_breifkasten() {
	int16 action_flag = false;

	if (_G(gameState).R28LetterBox && !_G(cur)->usingInventoryCursor()) {
		action_flag = true;
		hideCur();
		_G(gameState).R28LetterBox = false;
		autoMove(7, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(det)->hideStaticSpr(8);
		_G(det)->hideStaticSpr(9);
		_G(det)->showStaticSpr(7);
		autoMove(8, P_CHEWY);
		start_spz(CH_LGET_O, 1, ANI_FRONT, P_CHEWY);
		startAadWait(179);
		_G(atds)->set_all_ats_str(206, 0, ATS_DATA);
		showCur();
		invent_2_slot(MANUSKRIPT_INV);
		_G(gameState).R28Manuskript = true;
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
