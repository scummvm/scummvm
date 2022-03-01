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

#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/room.h"
#include "chewy/rooms/room18.h"

namespace Chewy {
namespace Rooms {

static const int16 BORK_SPR[5] = { 15, 16, 17, 24, 25 };
static const int16 BORK_SPR1[4] = { 20, 21, 18, 19 };

static const int16 SURIMY_PHASEN[4][2] = {
	{ 245, 252 },
	{ 0, 0 },
	{ 0, 0 },
	{ 0, 0 }
};

static const MovLine SURIMY_MPKT[2] = {
	{ { 453, 170, 190 }, 0, 6 },
	{ { 392, 170, 190 }, 0, 6 }
};

static const MovLine SURIMY_MPKT1[2] = {
	{ { 392, 170, 190 }, 0, 6 },
	{ { 143, 170, 190 }, 0, 6 }
};

static const MovLine SURIMY_MPKT2[2] = {
	{ { 143, 170, 190 }, 0, 6 },
	{ {   0, 170, 190 }, 0, 6 }
};

static const MovLine SURIMY_MPKT3[2] = {
	{ { 500, 100, 190 }, 0, 12 },
	{ { 392, 170, 190 }, 0, 12 }
};

static const AniBlock ABLOCK20[3] = {
	{ 5, 3, ANI_FRONT, ANI_GO, 0 },
	{ 6, 3, ANI_FRONT, ANI_GO, 0 },
	{ 7, 3, ANI_FRONT, ANI_WAIT, 0 }
};


static const AniBlock ABLOCK21[2] = {
	{  8, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 23, 255, ANI_BACK, ANI_GO, 0 },
};

static const AniBlock ABLOCK22[2] = {
	{ 13, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 14, 13, ANI_FRONT, ANI_GO, 0 },
};

static const AniBlock ABLOCK23[2] = {
	{ 11, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 12, 1, ANI_FRONT, ANI_GO, 0 },
};

static const AniBlock ABLOCK24[2] = {
	{ 15, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 16, 7, ANI_FRONT, ANI_GO, 0 },
};


void Room18::entry() {
	_G(spieler).R18MoniSwitch = false;
	_G(atds)->set_ats_str(151, TXT_MARK_LOOK, 0, ATS_DATA);
	_G(spieler).ScrollxStep = 2;

	if (_G(spieler).R18CartTerminal)
		_G(det)->showStaticSpr(23);

	if (!_G(spieler).R18SurimyWurf) {
		init_borks();
	} else {
		for (int16 i = 0; i < 5; i++)
			_G(det)->hideStaticSpr(BORK_SPR[i]);

		for (int16 i = 0; i < (4 - (_G(spieler).R18Krone ? 1 : 0)); i++)
			_G(det)->showStaticSpr(BORK_SPR1[i]);
	}

	if (_G(spieler).R16F5Exit)
		_G(det)->hideStaticSpr(19);
	
	if (_G(spieler).R17EnergieOut) {
		_G(det)->stop_detail(0);
		_G(atds)->set_ats_str(150, TXT_MARK_LOOK, 1, ATS_DATA);
	} else {
		_G(atds)->set_ats_str(150, TXT_MARK_LOOK, 0, ATS_DATA);
	}

	if (!_G(spieler).R18FirstEntry && !_G(spieler).R18Gitter) {
		startAadWait(39);
		_G(spieler).R18FirstEntry = true;
	}

	if (_G(spieler).R18Gitter)
		_G(spieler).scrolly = 0;
}

bool Room18::timer(int16 t_nr, int16 ani_nr) {
	if (!_G(spieler).R18SurimyWurf && !_G(flags).AutoAniPlay) {
		_G(flags).AutoAniPlay = true;

		if (t_nr == _G(timer_nr)[0]) {
			_G(det)->hideStaticSpr(16);
			startSetailWait(10, 1, ANI_FRONT);
			_G(det)->showStaticSpr(16);
			_G(uhr)->resetTimer(_G(timer_nr)[0], 10);
		} else if (t_nr == _G(timer_nr)[1]) {
			hideCur();
			_G(det)->hideStaticSpr(17);
			startSetailWait(9, 1, ANI_FRONT);
			_G(det)->showStaticSpr(17);

			if (!_G(spieler).R18SondeMoni) {
				_G(spieler).R18SondeMoni = true;
				startSetailWait(2, 1, ANI_FRONT);
				_G(det)->showStaticSpr(9);
				startSetailWait(4, 1, ANI_FRONT);
				_G(det)->showStaticSpr(11);
				waitShowScreen(50);
				_G(det)->hideStaticSpr(9);
				_G(det)->hideStaticSpr(11);
			} else {
				monitor();
			}

			showCur();
			_G(uhr)->resetTimer(_G(timer_nr)[1], 15);
		}

		_G(flags).AutoAniPlay = false;
	}

	return false;
}

void Room18::gedAction(int index) {
	if (!index && !_G(spieler).R18SurimyWurf) {
		stopPerson(P_CHEWY);
		autoMove(1, P_CHEWY);
		start_aad(40, 0);
	}
}

void Room18::init_borks() {
	for (int16 i = 0; i < 5; i++)
		_G(det)->showStaticSpr(BORK_SPR[i]);
	
	for (int16 i = 0; i < 4; i++)
		_G(det)->hideStaticSpr(BORK_SPR1[i]);

	_G(timer_nr)[0] = _G(room)->set_timer(255, 10);
	_G(timer_nr)[1] = _G(room)->set_timer(255, 15);
	_G(spieler).scrollx = 276;
	_G(spieler).scrolly = 0;
	_G(flags).NoScroll = true;
}

void Room18::monitor() {
	_G(spieler).R18MoniSwitch ^= 1;

	int16 nr = 0;
	if (_G(spieler).R18MoniSwitch) {
		startAniBlock(2, ABLOCK21);
		nr = (_G(spieler).R17EnergieOut) ? 2 : 1;
	} else {
		_G(det)->stop_detail(23);
		_G(atds)->set_ats_str(41, TXT_MARK_LOOK, 1, ATS_DATA);
	}

	_G(atds)->set_ats_str(151, TXT_MARK_LOOK, nr, ATS_DATA);
}

int16 Room18::sonden_moni() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !_G(spieler).R18Gitter) {
		action_flag = true;

		hideCur();
		autoMove(8, P_CHEWY);
		startSetailWait(3, 1, ANI_FRONT);
		_G(det)->showStaticSpr(10);
		startAniBlock(3, ABLOCK20);

		for (int16 i = 0; i < 3; i++)
			_G(det)->showStaticSpr(i + 12);

		waitShowScreen(40);

		for (int16 i = 0; i < 5; i++)
			_G(det)->hideStaticSpr(i + 10);

		showCur();
	}
	return action_flag;
}

int16 Room18::calc_surimy() {
	int16 action_flag = false;

	if (is_cur_inventar(SURIMY_INV)) {
		action_flag = true;

		hideCur();
		del_inventar(_G(spieler).AkInvent);
		_G(spieler).R18SurimyWurf = true;
		_G(det)->load_taf_seq(245, 50, nullptr);
		_G(det)->load_taf_seq(116, 55, nullptr);
		_G(auto_obj) = 1;
		_G(mov_phasen)[SURIMY_OBJ].AtsText = 0;
		_G(mov_phasen)[SURIMY_OBJ].Lines = 2;
		_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
		_G(mov_phasen)[SURIMY_OBJ].ZoomFak = 0;
		_G(auto_mov_obj)[SURIMY_OBJ].Id = AUTO_OBJ0;
		_G(auto_mov_vector)[SURIMY_OBJ].Delay = _G(spieler).DelaySpeed;
		_G(auto_mov_obj)[SURIMY_OBJ].Mode = true;

		if (_G(spieler_vector)[P_CHEWY].Xypos[1] < 150) {
			startDetailFrame(18, 1, ANI_FRONT, 8);

			init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT3);
			waitDetail(18);
		} else {
			autoMove(1, P_CHEWY);
			stopPerson(P_CHEWY);
			_G(spieler).PersonHide[P_CHEWY] = true;
			startDetailFrame(17, 1, ANI_FRONT, 12);
			_G(maus_links_click) = false;

			init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT);
			waitDetail(17);
		}

		_G(spieler).PersonHide[P_CHEWY] = false;
		wait_auto_obj(SURIMY_OBJ);

		_G(det)->setSetailPos(21, 392, 170);
		_G(det)->setSetailPos(22, 447, 154);
		startDetailFrame(21, 1, ANI_FRONT, 14);
		_G(det)->startDetail(22, 1, ANI_FRONT);
		waitDetail(21);
		_G(det)->setStaticPos(26, 392, 170, false, true);
		_G(det)->showStaticSpr(26);

		_G(det)->hideStaticSpr(24);
		startAniBlock(2, ABLOCK22);
		_G(det)->showStaticSpr(20);

		for (int16 i = 0; i < 3; i++)
			_G(det)->hideStaticSpr(i + 15);

		startAniBlock(2, ABLOCK23);
		_G(det)->showStaticSpr(18);
		_G(det)->showStaticSpr(19);
		_G(det)->hideStaticSpr(26);

		_G(flags).NoScroll = true;
		_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT1);
		auto_scroll(70, 0);
		wait_auto_obj(SURIMY_OBJ);

		_G(det)->setSetailPos(21, 143, 170);
		_G(det)->setSetailPos(22, 198, 154);
		startDetailFrame(21, 1, ANI_FRONT, 14);
		_G(det)->startDetail(22, 1, ANI_FRONT);
		waitDetail(21);
		_G(det)->setStaticPos(26, 143, 170, false, true);
		_G(det)->showStaticSpr(26);

		_G(det)->hideStaticSpr(25);
		startAniBlock(2, ABLOCK24);
		_G(det)->showStaticSpr(21);

		_G(det)->hideStaticSpr(26);
		_G(mov_phasen)[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], _G(mov_phasen)[SURIMY_OBJ].Lines, (const MovLine *)SURIMY_MPKT2);
		auto_scroll(0, 0);
		wait_auto_obj(SURIMY_OBJ);
		_G(spieler).ScrollxStep = 6;
		_G(flags).NoScroll = false;
		auto_scroll(318, 0);
		_G(spieler).ScrollxStep = 2;
		_G(auto_obj) = 0;

		_G(atds)->set_ats_str(153, 1, ATS_DATA);
		_G(atds)->set_ats_str(149, TXT_MARK_LOOK, 1, ATS_DATA);

		for (int16 i = 0; i < 3; i++)
			_G(atds)->del_steuer_bit(158 + i, ATS_AKTIV_BIT, ATS_DATA);

		_G(atds)->del_steuer_bit(179, ATS_AKTIV_BIT, ATS_DATA);
		showCur();
		_G(det)->del_taf_tbl(245, 50, nullptr);
	}

	return action_flag;
}

int16 Room18::calc_schalter() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !_G(spieler).R18Gitter) {
		action_flag = true;

		hideCur();
		autoMove(6, P_CHEWY);
		monitor();
		showCur();
	}

	return action_flag;
}

short Room18::use_cart_moni() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur) {
		action_flag = true;

		hideCur();
		autoMove(9, P_CHEWY);
		setPersonSpr(P_LEFT, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(spieler).R18CartTerminal ^= 1;

		if (!_G(spieler).R18CartTerminal) {
			_G(atds)->set_ats_str(147, TXT_MARK_LOOK, 0, ATS_DATA);
			_G(det)->hideStaticSpr(23);
			startSetailWait(20, 1, ANI_BACK);
		} else {
			_G(atds)->set_ats_str(147, TXT_MARK_LOOK, 1, ATS_DATA);
			startSetailWait(20, 1, ANI_FRONT);
			_G(det)->showStaticSpr(23);

			if (_G(spieler).R18CartFach) {
				_G(spieler).R18CartSave = true;
				_G(atds)->set_ats_str(CARTRIDGE_INV, TXT_MARK_LOOK, 1, INV_ATS_DATA);
				startAadWait(120);
			}
		}

		showCur();
	}

	return action_flag;
}

int16 Room18::go_cyberspace() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !_G(spieler).R18Gitter) {
		action_flag = true;

		hideCur();
		autoMove(7, P_CHEWY);
		cur_2_inventory();
		showCur();

		switch_room(24);
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
