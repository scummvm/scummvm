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
#include "chewy/rooms/room2.h"
#include "chewy/rooms/room3.h"
#include "chewy/rooms/room4.h"

namespace Chewy {
namespace Rooms {

#define KOPF_SCHUSS 0
#define SONDE_ANI 2
#define SONDE_SHOOT 3
#define SONDE_RET 4
#define SONDE_RAUCH 5
#define SONDE_REIN 6
#define SONDE_GREIF 7
#define SONDE_GREIF1 8

#define SONDE_SPR_R 118
#define SONDE_SPR_L 120

#define GITTER_BLITZEN 7

static const MovLine SONDE_MPKT[3] = {
	{ {  13, 45,  75 }, 1, 2 },
	{ { 237, 52, 160 }, 1, 2 },
	{ {   4, 83, 180 }, 0, 2 }
};

static const int16 SONDE_PHASEN[4][2] = {
	{ 120, 120 },
	{ 118, 118 },
	{ 141, 141 },
	{ 119, 119 }
};

static const AniBlock ABLOCK0[5] = {
	{ 6, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 7, 3, ANI_VOR, ANI_WAIT, 0 },
};


static const AniBlock ABLOCK1[2] = {
	{ 10, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 13, 1, ANI_VOR, ANI_WAIT, 0 },
};

static const AniBlock ABLOCK2[2] = {
	{ 12, 1, ANI_VOR, ANI_WAIT, 0 },
	{ GITTER_BLITZEN, 1, ANI_VOR, ANI_GO, 0 }
};

static const AniBlock ABLOCK3[2] = {
	{  4, 2, ANI_VOR, ANI_GO, 0 },
	{ 11, 255, ANI_VOR, ANI_GO, 0 }
};

static const AniBlock ablock5[3] = {
	{ 0, 1, ANI_VOR, ANI_WAIT, 0 },
	{ 8, 9, ANI_VOR, ANI_GO, 0 },
	{ 1, 1, ANI_VOR, ANI_WAIT, 0 },

};


void Room3::entry() {
	init_sonde();
}

void Room3::terminal() {
	show_cur();
	_G(auto_obj) = 0;
	switch (Room4::sonde_comp()) {
	case 0:
		probeTransfer();
		spr_info[0].Image = room_blk.DetImage[120];
		spr_info[0].X = 250;
		spr_info[0].Y = 2;
		spr_info[0].ZEbene = 0;

		if (!_G(spieler).R2ElectrocutedBork) {
			det->stop_detail(5);
			start_ani_block(2, ABLOCK3);
			if (_G(spieler).R2FussSchleim) {
				_G(spieler).R2FussSchleim = 0;

				_G(spieler).room_s_obj[SIB_SCHLEIM].ZustandFlipFlop = 1;
			}
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_detail_wait(8, 1, ANI_VOR);
			set_up_screen(DO_SETUP);
			det->stop_detail(6);
			clear_prog_ani();
			switch_room(1);
			start_ani_block(2, ABLOCK0);
			set_person_pos(92, 131, P_CHEWY, P_LEFT);
			_G(spieler).PersonHide[P_CHEWY] = false;
		} else if (!_G(spieler).R2FussSchleim) {
			start_ani_block(2, ABLOCK2);
			Room2::jump_out_r1(9);
			start_aad_wait(45, -1);
		} else {
			_G(spieler).PersonHide[P_CHEWY] = true;
			start_ani_block(2, ABLOCK1);
			_G(spieler).PersonRoomNr[P_CHEWY] = 5;
			clear_prog_ani();
			_G(auto_obj) = 0;
			room->load_room(&room_blk, _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
			ERROR
				fx_blend = BLEND1;
			start_ani_block(3, ablock5);
			set_person_pos(91, 107, P_CHEWY, P_LEFT);
			_G(spieler).PersonHide[P_CHEWY] = false;
		}
		clear_prog_ani();
		break;

	case 1:
		_G(auto_obj) = 1;
		_G(maus_links_click) = false;
		minfo.button = 0;
		stop_person(P_CHEWY);
		start_aad_wait(51, -1);
		set_up_screen(DO_SETUP);
		break;

	case 2:
		sonde_knarre();
		break;

	}
}

void Room3::init_sonde() {
#define SONDE_OBJ 0
#define SONDE_OBJ1 1
	_G(auto_obj) = 1;
	mov_phasen[SONDE_OBJ].AtsText = 24;
	mov_phasen[SONDE_OBJ].Lines = 3;
	mov_phasen[SONDE_OBJ].Repeat = 255;
	mov_phasen[SONDE_OBJ].ZoomFak = 20;
	auto_mov_obj[SONDE_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
	auto_mov_obj[SONDE_OBJ].Mode = true;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], mov_phasen[SONDE_OBJ].Lines, (const MovLine *)SONDE_MPKT);
}

void Room3::sonde_knarre() {
	MovLine sonde_mpkt1[3] = {
		{ { 237,  52, 160 }, 0, 2 },
		{ {  13,  45,  75 }, 0, 4 },
		{ {   4, 100, 180 }, 1, 2 }
	};
	MovLine sonde_mpkt2[2] = {
		{ {   4, 100, 180 }, 1, 3 },
		{ {  13,  45,  75 }, 1, 3 }
	};
	room_detail_info *rdi;
	int16 tmp;
	int16 ende;
	hide_cur();
	rdi = det->get_room_detail_info();

	det->load_taf_seq(162, 17, 0);
	tmp = _G(zoom_horizont);
	_G(zoom_horizont) = 100;
	_G(auto_obj) = 1;
	mov_phasen[SONDE_OBJ].Lines = 3;
	mov_phasen[SONDE_OBJ].Repeat = 1;
	mov_phasen[SONDE_OBJ].ZoomFak = 20;
	auto_mov_obj[SONDE_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
	auto_mov_obj[SONDE_OBJ].Mode = true;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], mov_phasen[SONDE_OBJ].Lines, (MovLine
		*)sonde_mpkt1);
	flags.AniUserAction = false;
	while (mov_phasen[SONDE_OBJ].Repeat != -1)
		set_ani_screen();

	det->start_detail(SONDE_SHOOT, 1, ANI_VOR);
	ende = 0;
	while (!ende) {
		clear_prog_ani();
		if (rdi->Ainfo[SONDE_SHOOT].ani_count == 170) {
			_G(spieler).PersonHide[P_CHEWY] = true;
			det->start_detail(KOPF_SCHUSS, 1, ANI_VOR);
		}
		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		spr_info[0].ZEbene = 0;
		if (rdi->Ainfo[KOPF_SCHUSS].ani_count >= 13 &&
			rdi->Ainfo[KOPF_SCHUSS].ani_count <= 21) {
			spr_info[1] = det->plot_detail_sprite(0, 0, KOPF_SCHUSS, 21, ANI_HIDE);
			spr_info[1].ZEbene = 190;
		} else if (rdi->Ainfo[KOPF_SCHUSS].ani_count > 21 &&
			det->get_ani_status(SONDE_RET) == 0) {

			spr_info[2] = det->plot_detail_sprite(0, 0, SONDE_RET, 173, ANI_HIDE);
			spr_info[2].ZEbene = 190;
		}
		if (rdi->Ainfo[SONDE_SHOOT].ani_count == 178) {
			det->start_detail(SONDE_RET, 3, ANI_RUECK);
		}
		if (rdi->Ainfo[KOPF_SCHUSS].ani_count == 28) {
			ende = 1;
		}
		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	clear_prog_ani();
	det->start_detail(SONDE_RAUCH, 3, ANI_VOR);
	ende = 0;
	while (det->get_ani_status(SONDE_RAUCH)) {

		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		spr_info[0].ZEbene = 0;

		spr_info[1] = det->plot_detail_sprite(0, 0, SONDE_RET, 173, ANI_HIDE);
		spr_info[1].ZEbene = 190;
		++ende;
		if (ende == 3)
			_G(spieler).PersonHide[P_CHEWY] = false;
		set_ani_screen();
	}
	start_aad_wait(53, -1);

	clear_prog_ani();
	det->start_detail(SONDE_REIN, 1, ANI_RUECK);
	while (det->get_ani_status(SONDE_REIN)) {

		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		spr_info[0].ZEbene = 0;
		set_ani_screen();
	}
	clear_prog_ani();

	mov_phasen[SONDE_OBJ].Lines = 2;
	mov_phasen[SONDE_OBJ].Repeat = 1;
	mov_phasen[SONDE_OBJ].ZoomFak = 28;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], mov_phasen[SONDE_OBJ].Lines, (MovLine
		*)sonde_mpkt2);
	while (mov_phasen[SONDE_OBJ].Repeat != -1)
		set_ani_screen();
	det->del_taf_tbl(162, 17, 0);
	_G(zoom_horizont) = tmp;
	init_sonde();

	show_cur();
}

void Room3::probeTransfer() {
	int16 tmp;
	int16 i;
	int16 spr_nr;
	int16 anistart;

	static const MovLine SONDE_MPKT1[2] = {
		{ { 237,  52, 160 }, 0, 2 },
		{ { 144, 100, 180 }, 0, 2 }
	};
	static const MovLine SONDE_MPKT_[4][2] = {
		{ { { 144, 100, 180 }, 0, 2 },
		  { { 110, 100, 180 }, 0, 2 } },

		{ { { 110, 101, 180 }, 0, 3 },
		  { { -55,  50, 180 }, 0, 3 } },

		{ { { 310,  20, 180 }, 0, 3 },
		  { { -55,  20, 180 }, 0, 3 } },

		{ { { 310,   2, 180 }, 0, 3 },
		  { { 250,   2, 180 }, 0, 3 } }
	};

	flags.AniUserAction = false;
	hide_cur();
	/*rdi = */(void)det->get_room_detail_info();
	tmp = _G(zoom_horizont);
	_G(zoom_horizont) = 100;
	anistart = false;
	_G(auto_obj) = 1;
	mov_phasen[SONDE_OBJ].Lines = 2;
	mov_phasen[SONDE_OBJ].Repeat = 1;
	mov_phasen[SONDE_OBJ].ZoomFak = 20;
	auto_mov_obj[SONDE_OBJ].Id = AUTO_OBJ0;
	auto_mov_vector[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
	auto_mov_obj[SONDE_OBJ].Mode = true;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], mov_phasen[SONDE_OBJ].Lines,
		(const MovLine *)SONDE_MPKT1);
	while (mov_phasen[SONDE_OBJ].Repeat != -1)
		set_ani_screen();

	det->start_detail(SONDE_GREIF, 1, ANI_VOR);
	while (det->get_ani_status(SONDE_GREIF)) {
		clear_prog_ani();
		spr_info[0] = det->plot_detail_sprite(0, 0, SONDE_GREIF, SONDE_SPR_L, ANI_HIDE);
		spr_info[0].ZEbene = 146;
		set_ani_screen();

		SHOULD_QUIT_RETURN;
	}

	clear_prog_ani();
	_G(auto_obj) = 2;

	spr_nr = 140;
	for (i = 0; i < 4 && !SHOULD_QUIT; i++) {
		mov_phasen[SONDE_OBJ].Lines = 2;
		mov_phasen[SONDE_OBJ].Repeat = 1;
		mov_phasen[SONDE_OBJ].ZoomFak = 0;
		auto_mov_obj[SONDE_OBJ].Id = AUTO_OBJ0;
		auto_mov_obj[SONDE_OBJ].Mode = true;
		auto_mov_vector[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
		init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], mov_phasen[SONDE_OBJ].Lines, SONDE_MPKT_[i]);
		mov_phasen[SONDE_OBJ1].Lines = 2;
		mov_phasen[SONDE_OBJ1].Repeat = 1;
		mov_phasen[SONDE_OBJ1].ZoomFak = 0;
		auto_mov_obj[SONDE_OBJ1].Id = AUTO_OBJ1;
		auto_mov_obj[SONDE_OBJ1].Mode = true;
		auto_mov_vector[SONDE_OBJ1].Delay = _G(spieler).DelaySpeed;
		init_auto_obj(SONDE_OBJ1, &SONDE_PHASEN[0][0], mov_phasen[SONDE_OBJ1].Lines, SONDE_MPKT_[i]);
		mov_phasen[SONDE_OBJ1].Phase[0][0] = spr_nr;
		mov_phasen[SONDE_OBJ1].Phase[0][1] = spr_nr;
		while (mov_phasen[SONDE_OBJ].Repeat != -1) {
			SHOULD_QUIT_RETURN;

			if (i == 2 || i == 1) {
				if (mouse_auto_obj(SONDE_OBJ, 50, 100)) {
					if (minfo.button == 1 || kbinfo.key_code == ENTER) {
						if (is_cur_inventar(SPINAT_INV)) {
							ssi[0].X = 120;
							ssi[0].Y = 100;
							if (_G(spieler).PersonRoomNr[P_CHEWY] == 3)

								start_aad(50);
							else

								start_aad(44);
							del_inventar(_G(spieler).AkInvent);
							_G(spieler).R2FussSchleim = 1;
							mov_phasen[SONDE_OBJ1].Phase[0][0] = 142;
							mov_phasen[SONDE_OBJ1].Phase[0][1] = 149;
							auto_mov_vector[SONDE_OBJ1].PhAnz = 8;
							anistart = true;
						}
					}
				}
			}
			set_ani_screen();

			if (anistart) {
				if (auto_mov_vector[SONDE_OBJ1].PhNr == 7) {
					anistart = false;
					spr_nr = 149;
					mov_phasen[SONDE_OBJ1].Phase[0][0] = spr_nr;
					mov_phasen[SONDE_OBJ1].Phase[0][1] = spr_nr;
					auto_mov_vector[SONDE_OBJ1].PhAnz = 1;
					auto_mov_vector[SONDE_OBJ1].PhNr = 0;
				}
			}
		}

		switch (i) {
		case 0:
			show_cur();
			start_aad(52);
			flags.AniUserAction = true;
			spr_nr = 141;
			_G(spieler).PersonHide[P_CHEWY] = true;
			det->load_taf_seq(142, 8, 0);
			break;

		case 1:
			switch_room(1);
			break;

		case 2:
			det->del_taf_tbl(142, 7, 0);
			flags.AniUserAction = false;
			switch_room(2);
			break;

		default:
			break;
		}
	}

	flags.AniUserAction = false;
	_G(zoom_horizont) = tmp;
	_G(auto_obj) = 0;
}

} // namespace Rooms
} // namespace Chewy
