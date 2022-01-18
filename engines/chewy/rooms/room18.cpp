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
#include "chewy/global.h"
#include "chewy/ani_dat.h"
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
	{ 5, 3, ANI_VOR, ANI_GO, 0 },
	{ 6, 3, ANI_VOR, ANI_GO, 0 },
	{ 7, 3, ANI_VOR, ANI_WAIT, 0 }
};


static const AniBlock ABLOCK21[2] = {
	{  8, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 23, 255, ANI_RUECK, ANI_GO, 0 },
};

static const AniBlock ABLOCK22[2] = {
	{ 13, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 14, 13, ANI_VOR, ANI_GO, 0 },
};

static const AniBlock ABLOCK23[2] = {
	{ 11, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 12, 1, ANI_VOR, ANI_GO, 0 },
};

static const AniBlock ABLOCK24[2] = {
	{ 15, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 16, 7, ANI_VOR, ANI_GO, 0 },
};


void Room18::entry() {
	int16 i;

	_G(spieler).R18MoniSwitch = false;
	atds->set_ats_str(151, TXT_MARK_LOOK, 0, ATS_DATEI);
	_G(spieler).ScrollxStep = 2;

	if (_G(spieler).R18CartTerminal)
		det->show_static_spr(23);
	if (!_G(spieler).R18SurimyWurf) {
		init_borks();
	} else {
		for (i = 0; i < 5; i++)
			det->hide_static_spr(BORK_SPR[i]);
		for (i = 0; i < (4 - _G(spieler).R18Krone ? 1 : 0); i++)
			det->show_static_spr(BORK_SPR1[i]);
	}

	if (_G(spieler).R16F5Exit)
		det->hide_static_spr(19);
	if (_G(spieler).R17EnergieOut) {
		det->stop_detail(0);
		atds->set_ats_str(150, TXT_MARK_LOOK, 1, ATS_DATEI);
	} else {
		atds->set_ats_str(150, TXT_MARK_LOOK, 0, ATS_DATEI);
	}

	if (!_G(spieler).R18FirstEntry && !_G(spieler).R18Gitter) {
		start_aad_wait(39, -1);
		_G(spieler).R18FirstEntry = true;
	}
	if (_G(spieler).R18Gitter)
		_G(spieler).scrolly = 0;
}

bool Room18::timer(int16 t_nr, int16 ani_nr) {
	if (!_G(spieler).R18SurimyWurf) {
		if (!flags.AutoAniPlay) {
			flags.AutoAniPlay = true;

			if (t_nr == _G(timer_nr)[0]) {
				det->hide_static_spr(16);
				start_detail_wait(10, 1, ANI_VOR);
				det->show_static_spr(16);
				uhr->reset_timer(_G(timer_nr)[0], 10);
			} else if (t_nr == _G(timer_nr)[1]) {
				hide_cur();
				det->hide_static_spr(17);
				start_detail_wait(9, 1, ANI_VOR);
				det->show_static_spr(17);

				if (!_G(spieler).R18SondeMoni) {
					_G(spieler).R18SondeMoni = true;
					start_detail_wait(2, 1, ANI_VOR);
					det->show_static_spr(9);
					start_detail_wait(4, 1, ANI_VOR);
					det->show_static_spr(11);
					wait_show_screen(50);
					det->hide_static_spr(9);
					det->hide_static_spr(11);
				} else {
					monitor();
				}

				show_cur();
				uhr->reset_timer(_G(timer_nr)[1], 15);
			}

			flags.AutoAniPlay = false;
		}
	}

	return false;
}

void Room18::gedAction(int index) {
	if (!index && !_G(spieler).R18SurimyWurf) {
		stop_person(P_CHEWY);
		auto_move(1, P_CHEWY);
		start_aad(40, 0);
	}
}

void Room18::init_borks() {
	int16 i;

	for (i = 0; i < 5; i++)
		det->show_static_spr(BORK_SPR[i]);
	for (i = 0; i < 4; i++)
		det->hide_static_spr(BORK_SPR1[i]);

	_G(timer_nr)[0] = room->set_timer(255, 10);
	_G(timer_nr)[1] = room->set_timer(255, 15);
	_G(spieler).scrollx = 276;
	_G(spieler).scrolly = 0;
	flags.NoScroll = true;
}

void Room18::monitor() {
	int16 nr;
	nr = 0;
	_G(spieler).R18MoniSwitch ^= 1;

	if (_G(spieler).R18MoniSwitch) {
		start_ani_block(2, ABLOCK21);
		if (_G(spieler).R17EnergieOut)
			nr = 2;
		else
			nr = 1;
	} else {
		det->stop_detail(23);
		atds->set_ats_str(41, TXT_MARK_LOOK, 1, ATS_DATEI);
	}

	atds->set_ats_str(151, TXT_MARK_LOOK, nr, ATS_DATEI);
}

int16 Room18::sonden_moni() {
	int16 i;
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !_G(spieler).R18Gitter) {
		action_flag = true;

		hide_cur();
		auto_move(8, P_CHEWY);
		start_detail_wait(3, 1, ANI_VOR);
		det->show_static_spr(10);
		start_ani_block(3, ABLOCK20);

		for (i = 0; i < 3; i++)
			det->show_static_spr(i + 12);

		wait_show_screen(40);

		for (i = 0; i < 5; i++)
			det->hide_static_spr(i + 10);

		show_cur();
	}
	return action_flag;
}

int16 Room18::calc_surimy() {
	int16 i;
	int16 action_flag = false;

	if (is_cur_inventar(SURIMY_INV)) {
		action_flag = true;

		hide_cur();
		del_inventar(_G(spieler).AkInvent);
		_G(spieler).R18SurimyWurf = true;
		det->load_taf_seq(245, 294 - 245 + 1, 0);
		det->load_taf_seq(116, 170 - 116 + 1, 0);
		_G(auto_obj) = 1;
		mov_phasen[SURIMY_OBJ].AtsText = 0;
		mov_phasen[SURIMY_OBJ].Lines = 2;
		mov_phasen[SURIMY_OBJ].Repeat = 1;
		mov_phasen[SURIMY_OBJ].ZoomFak = 0;
		auto_mov_obj[SURIMY_OBJ].Id = AUTO_OBJ0;
		auto_mov_vector[SURIMY_OBJ].Delay = _G(spieler).DelaySpeed;
		auto_mov_obj[SURIMY_OBJ].Mode = true;

		if (spieler_vector[P_CHEWY].Xypos[1] < 150) {
			start_detail_frame(18, 1, ANI_VOR, 8);

			init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], mov_phasen[SURIMY_OBJ].Lines,
				(const MovLine *)SURIMY_MPKT3);
			wait_detail(18);
		} else {
			auto_move(1, P_CHEWY);
			stop_person(P_CHEWY);
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_detail_frame(17, 1, ANI_VOR, 12);
			_G(maus_links_click) = false;

			init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], mov_phasen[SURIMY_OBJ].Lines,
				(const MovLine *)SURIMY_MPKT);
			wait_detail(17);
		}

		_G(spieler).PersonHide[P_CHEWY] = false;
		wait_auto_obj(SURIMY_OBJ);

		det->set_detail_pos(21, 392, 170);
		det->set_detail_pos(22, 447, 154);
		start_detail_frame(21, 1, ANI_VOR, 14);
		det->start_detail(22, 1, ANI_VOR);
		wait_detail(21);
		det->set_static_pos(26, 392, 170, false, true);
		det->show_static_spr(26);

		det->hide_static_spr(24);
		start_ani_block(2, ABLOCK22);
		det->show_static_spr(20);

		for (i = 0; i < 3; i++)
			det->hide_static_spr(i + 15);

		start_ani_block(2, ABLOCK23);
		det->show_static_spr(18);
		det->show_static_spr(19);
		det->hide_static_spr(26);

		flags.NoScroll = true;
		mov_phasen[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], mov_phasen[SURIMY_OBJ].Lines,
			(const MovLine *)SURIMY_MPKT1);
		auto_scroll(70, 0);
		wait_auto_obj(SURIMY_OBJ);

		det->set_detail_pos(21, 143, 170);
		det->set_detail_pos(22, 198, 154);
		start_detail_frame(21, 1, ANI_VOR, 14);
		det->start_detail(22, 1, ANI_VOR);
		wait_detail(21);
		det->set_static_pos(26, 143, 170, false, true);
		det->show_static_spr(26);

		det->hide_static_spr(25);
		start_ani_block(2, ABLOCK24);
		det->show_static_spr(21);

		det->hide_static_spr(26);
		mov_phasen[SURIMY_OBJ].Repeat = 1;
		init_auto_obj(SURIMY_OBJ, &SURIMY_PHASEN[0][0], mov_phasen[SURIMY_OBJ].Lines,
			(const MovLine *)SURIMY_MPKT2);
		auto_scroll(0, 0);
		wait_auto_obj(SURIMY_OBJ);
		_G(spieler).ScrollxStep = 6;
		flags.NoScroll = false;
		auto_scroll(318, 0);
		_G(spieler).ScrollxStep = 2;
		_G(auto_obj) = 0;

		atds->set_ats_str(153, 1, ATS_DATEI);
		atds->set_ats_str(149, TXT_MARK_LOOK, 1, ATS_DATEI);

		for (i = 0; i < 3; i++)
			atds->del_steuer_bit(158 + i, ATS_AKTIV_BIT, ATS_DATEI);

		atds->del_steuer_bit(179, 0, 1);
		show_cur();
		det->del_taf_tbl(245, 50, 0);
	}

	return action_flag;
}

int16 Room18::calc_schalter() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !_G(spieler).R18Gitter) {
		action_flag = true;

		hide_cur();
		auto_move(6, P_CHEWY);
		monitor();
		show_cur();
	}

	return action_flag;
}

short Room18::use_cart_moni() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur) {
		action_flag = true;

		hide_cur();
		auto_move(9, P_CHEWY);
		set_person_spr(P_LEFT, P_CHEWY);
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		_G(spieler).R18CartTerminal ^= 1;

		if (!_G(spieler).R18CartTerminal) {
			atds->set_ats_str(147, TXT_MARK_LOOK, 0, ATS_DATEI);
			det->hide_static_spr(23);
			start_detail_wait(20, 1, ANI_RUECK);
		} else {
			atds->set_ats_str(147, TXT_MARK_LOOK, 1, ATS_DATEI);
			start_detail_wait(20, 1, ANI_VOR);
			det->show_static_spr(23);

			if (_G(spieler).R18CartFach) {
				_G(spieler).R18CartSave = true;
				atds->set_ats_str(CARTRIDGE_INV, TXT_MARK_LOOK, 1, INV_ATS_DATEI);
				start_aad_wait(120, -1);
			}
		}
	}
	return action_flag;
}

int16 Room18::go_cyberspace() {
	int16 action_flag = false;

	if (!_G(spieler).inv_cur && !_G(spieler).R18Gitter) {
		action_flag = true;

		hide_cur();
		auto_move(7, P_CHEWY);
		cur_2_inventory();
		show_cur();

		switch_room(24);
	}

	return action_flag;
}

} // namespace Rooms
} // namespace Chewy
