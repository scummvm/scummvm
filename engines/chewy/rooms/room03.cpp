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
#include "chewy/rooms/room02.h"
#include "chewy/rooms/room03.h"
#include "chewy/rooms/room04.h"
#include "chewy/sound.h"

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

#define SONDE_OBJ 0
#define SONDE_OBJ1 1

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
	{ 6, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 7, 3, ANI_FRONT, ANI_WAIT, 0 },
};


static const AniBlock ABLOCK1[2] = {
	{ 10, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 13, 1, ANI_FRONT, ANI_WAIT, 0 },
};

static const AniBlock ABLOCK2[2] = {
	{ 12, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ GITTER_BLITZEN, 1, ANI_FRONT, ANI_GO, 0 }
};

static const AniBlock ABLOCK3[2] = {
	{  4, 2, ANI_FRONT, ANI_GO, 0 },
	{ 11, 255, ANI_FRONT, ANI_GO, 0 }
};

static const AniBlock ablock5[3] = {
	{ 0, 1, ANI_FRONT, ANI_WAIT, 0 },
	{ 8, 9, ANI_FRONT, ANI_GO, 0 },
	{ 1, 1, ANI_FRONT, ANI_WAIT, 0 },

};


void Room3::entry() {
	init_sonde();
}

void Room3::terminal() {
	showCur();
	_G(auto_obj) = 0;
	switch (Room4::sonde_comp()) {
	case 0:
		probeTransfer();
		_G(spr_info)[0].Image = _G(room_blk).DetImage[120];
		_G(spr_info)[0].X = 250;
		_G(spr_info)[0].Y = 2;
		_G(spr_info)[0].ZEbene = 0;

		if (!_G(spieler).R2ElectrocutedBork) {
			_G(det)->stop_detail(5);
			startAniBlock(2, ABLOCK3);
			if (_G(spieler).R2FussSchleim) {
				_G(spieler).R2FussSchleim = false;
				_G(spieler).room_s_obj[SIB_SCHLEIM].ZustandFlipFlop = 1;
			}
			_G(spieler).PersonHide[P_CHEWY] = true;
			startSetailWait(8, 1, ANI_FRONT);
			setupScreen(DO_SETUP);
			_G(det)->stop_detail(6);
			clear_prog_ani();
			switch_room(1);
			startAniBlock(2, ABLOCK0);
			setPersonPos(92, 131, P_CHEWY, P_LEFT);
			_G(spieler).PersonHide[P_CHEWY] = false;
		} else if (!_G(spieler).R2FussSchleim) {
			startAniBlock(2, ABLOCK2);
			Room2::jump_out_r1(9);
			startAadWait(45, -1);
		} else {
			_G(spieler).PersonHide[P_CHEWY] = true;
			startAniBlock(2, ABLOCK1);
			_G(spieler).PersonRoomNr[P_CHEWY] = 5;
			clear_prog_ani();
			_G(auto_obj) = 0;
			_G(room)->load_room(&_G(room_blk), _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
			_G(fx_blend) = BLEND1;
			startAniBlock(3, ablock5);
			setPersonPos(91, 107, P_CHEWY, P_LEFT);
			_G(spieler).PersonHide[P_CHEWY] = false;
		}
		clear_prog_ani();
		break;

	case 1:
		_G(auto_obj) = 1;
		_G(maus_links_click) = false;
		_G(minfo).button = 0;
		stopPerson(P_CHEWY);
		start_spz(15, 255, false, P_CHEWY);
		startAadWait(51, -1);
		setupScreen(DO_SETUP);
		break;

	case 2:
		sonde_knarre();
		break;

	default:
		break;
	}
}

void Room3::init_sonde() {
	_G(auto_obj) = 1;
	_G(mov_phasen)[SONDE_OBJ].AtsText = 24;
	_G(mov_phasen)[SONDE_OBJ].Lines = 3;
	_G(mov_phasen)[SONDE_OBJ].Repeat = 255;
	_G(mov_phasen)[SONDE_OBJ].ZoomFak = 20;
	_G(auto_mov_obj)[SONDE_OBJ].Id = AUTO_OBJ0;
	_G(auto_mov_vector)[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
	_G(auto_mov_obj)[SONDE_OBJ].Mode = true;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], 3, (const MovLine *)SONDE_MPKT);
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
	hideCur();
	RoomDetailInfo *rdi = _G(det)->getRoomDetailInfo();

	_G(det)->load_taf_seq(162, 17, nullptr);
	int16 tmp = _G(zoom_horizont);
	_G(zoom_horizont) = 100;
	_G(auto_obj) = 1;
	_G(mov_phasen)[SONDE_OBJ].Lines = 3;
	_G(mov_phasen)[SONDE_OBJ].Repeat = 1;
	_G(mov_phasen)[SONDE_OBJ].ZoomFak = 20;
	_G(auto_mov_obj)[SONDE_OBJ].Id = AUTO_OBJ0;
	_G(auto_mov_vector)[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
	_G(auto_mov_obj)[SONDE_OBJ].Mode = true;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], 3, (MovLine*)sonde_mpkt1);
	_G(flags).AniUserAction = false;
	while (_G(mov_phasen)[SONDE_OBJ].Repeat != -1)
		set_ani_screen();

	_G(det)->startDetail(SONDE_SHOOT, 1, ANI_FRONT);
	int16 ende = 0;
	while (!ende) {
		clear_prog_ani();
		if (rdi->Ainfo[SONDE_SHOOT].ani_count == 170) {
			_G(spieler).PersonHide[P_CHEWY] = true;
			_G(det)->startDetail(KOPF_SCHUSS, 1, ANI_FRONT);
		}
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 0;
		if (rdi->Ainfo[KOPF_SCHUSS].ani_count >= 13 && rdi->Ainfo[KOPF_SCHUSS].ani_count <= 21) {
			_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, KOPF_SCHUSS, 21, ANI_HIDE);
			_G(spr_info)[1].ZEbene = 190;
		} else if (rdi->Ainfo[KOPF_SCHUSS].ani_count > 21 && _G(det)->get_ani_status(SONDE_RET) == 0) {
			_G(spr_info)[2] = _G(det)->plot_detail_sprite(0, 0, SONDE_RET, 173, ANI_HIDE);
			_G(spr_info)[2].ZEbene = 190;
		}
		
		if (rdi->Ainfo[SONDE_SHOOT].ani_count == 178)
			_G(det)->startDetail(SONDE_RET, 3, ANI_BACK);
		
		if (rdi->Ainfo[KOPF_SCHUSS].ani_count == 28)
			ende = 1;

		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	clear_prog_ani();
	_G(det)->startDetail(SONDE_RAUCH, 3, ANI_FRONT);
	ende = 0;
	while (_G(det)->get_ani_status(SONDE_RAUCH)) {

		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 0;

		_G(spr_info)[1] = _G(det)->plot_detail_sprite(0, 0, SONDE_RET, 173, ANI_HIDE);
		_G(spr_info)[1].ZEbene = 190;
		++ende;
		if (ende == 3)
			_G(spieler).PersonHide[P_CHEWY] = false;
		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	start_spz(15, 255, false, P_CHEWY);
	startAadWait(53, -1);
	clear_prog_ani();
	_G(det)->startDetail(SONDE_REIN, 1, ANI_BACK);
	while (_G(det)->get_ani_status(SONDE_REIN)) {

		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, SONDE_ANI, SONDE_SPR_R, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 0;
		set_ani_screen();
	}

	clear_prog_ani();
	_G(mov_phasen)[SONDE_OBJ].Lines = 2;
	_G(mov_phasen)[SONDE_OBJ].Repeat = 1;
	_G(mov_phasen)[SONDE_OBJ].ZoomFak = 28;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], 2, (MovLine*)sonde_mpkt2);

	while (_G(mov_phasen)[SONDE_OBJ].Repeat != -1) {
		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	_G(det)->del_taf_tbl(162, 17, nullptr);
	_G(zoom_horizont) = tmp;
	init_sonde();

	showCur();
}

void Room3::probeTransfer() {
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

	_G(flags).AniUserAction = false;
	hideCur();
	_G(det)->getRoomDetailInfo();
	int16 tmp = _G(zoom_horizont);
	_G(zoom_horizont) = 100;
	int16 anistart = false;
	_G(auto_obj) = 1;
	_G(mov_phasen)[SONDE_OBJ].Lines = 2;
	_G(mov_phasen)[SONDE_OBJ].Repeat = 1;
	_G(mov_phasen)[SONDE_OBJ].ZoomFak = 20;
	_G(auto_mov_obj)[SONDE_OBJ].Id = AUTO_OBJ0;
	_G(auto_mov_vector)[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
	_G(auto_mov_obj)[SONDE_OBJ].Mode = true;
	init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], 2, (const MovLine *)SONDE_MPKT1);
	_G(atds)->set_ats_str(24, ATS_AKTIV_BIT, ATS_DATA);
	
	while (_G(mov_phasen)[SONDE_OBJ].Repeat != -1) {
		set_ani_screen();
		SHOULD_QUIT_RETURN;
	}

	_G(det)->startDetail(SONDE_GREIF, 1, ANI_FRONT);
	while (_G(det)->get_ani_status(SONDE_GREIF)) {
		clear_prog_ani();
		_G(spr_info)[0] = _G(det)->plot_detail_sprite(0, 0, SONDE_GREIF, SONDE_SPR_L, ANI_HIDE);
		_G(spr_info)[0].ZEbene = 146;
		set_ani_screen();

		SHOULD_QUIT_RETURN;
	}

	clear_prog_ani();
	_G(auto_obj) = 2;

	int16 spr_nr = 140;
	for (int16 i = 0; i < 4 && !SHOULD_QUIT; i++) {
		_G(mov_phasen)[SONDE_OBJ].Lines = 2;
		_G(mov_phasen)[SONDE_OBJ].Repeat = 1;
		_G(mov_phasen)[SONDE_OBJ].ZoomFak = 0;
		_G(auto_mov_obj)[SONDE_OBJ].Id = AUTO_OBJ0;
		_G(auto_mov_obj)[SONDE_OBJ].Mode = true;
		_G(auto_mov_vector)[SONDE_OBJ].Delay = _G(spieler).DelaySpeed;
		init_auto_obj(SONDE_OBJ, &SONDE_PHASEN[0][0], 2, SONDE_MPKT_[i]);
		_G(mov_phasen)[SONDE_OBJ1].Lines = 2;
		_G(mov_phasen)[SONDE_OBJ1].Repeat = 1;
		_G(mov_phasen)[SONDE_OBJ1].ZoomFak = 0;
		_G(auto_mov_obj)[SONDE_OBJ1].Id = AUTO_OBJ1;
		_G(auto_mov_obj)[SONDE_OBJ1].Mode = true;
		_G(auto_mov_vector)[SONDE_OBJ1].Delay = _G(spieler).DelaySpeed;
		init_auto_obj(SONDE_OBJ1, &SONDE_PHASEN[0][0], 2, SONDE_MPKT_[i]);
		_G(mov_phasen)[SONDE_OBJ1].Phase[0][0] = spr_nr;
		_G(mov_phasen)[SONDE_OBJ1].Phase[0][1] = spr_nr;
		while (_G(mov_phasen)[SONDE_OBJ].Repeat != -1) {
			SHOULD_QUIT_RETURN;

			if (i == 2 || i == 1) {
				if (mouse_auto_obj(SONDE_OBJ, 50, 100)) {
					if (_G(minfo).button == 1 || _G(kbinfo).key_code == Common::KEYCODE_RETURN) {
						if (is_cur_inventar(SPINAT_INV)) {
							_G(ssi)[0].X = 120;
							_G(ssi)[0].Y = 100;
							if (_G(spieler).PersonRoomNr[P_CHEWY] == 3)
								start_aad(50);
							else
								start_aad(44);

							del_inventar(_G(spieler).AkInvent);
							_G(spieler).R2FussSchleim = true;
							_G(mov_phasen)[SONDE_OBJ1].Phase[0][0] = 142;
							_G(mov_phasen)[SONDE_OBJ1].Phase[0][1] = 149;
							_G(auto_mov_vector)[SONDE_OBJ1].PhAnz = 8;
							anistart = true;
						}
					}
				}
			}
			set_ani_screen();

			if (anistart) {
				if (_G(auto_mov_vector)[SONDE_OBJ1].PhNr == 7) {
					anistart = false;
					spr_nr = 149;
					_G(mov_phasen)[SONDE_OBJ1].Phase[0][0] = spr_nr;
					_G(mov_phasen)[SONDE_OBJ1].Phase[0][1] = spr_nr;
					_G(auto_mov_vector)[SONDE_OBJ1].PhAnz = 1;
					_G(auto_mov_vector)[SONDE_OBJ1].PhNr = 0;
				}
			}
		}

		switch (i) {
		case 0:
			showCur();
			start_aad(52);
			_G(flags).AniUserAction = true;
			spr_nr = 141;
			_G(spieler).PersonHide[P_CHEWY] = true;
			_G(det)->load_taf_seq(142, 8, nullptr);
			break;

		case 1:
			g_engine->_sound->waitForSpeechToFinish();
			_G(atds)->set_ats_str(24, 0, 1);
			_G(mov_phasen)[0].AtsText = 544;
			switch_room(1);
			break;

		case 2:
			g_engine->_sound->waitForSpeechToFinish();
			_G(det)->del_taf_tbl(142, 7, nullptr);
			_G(flags).AniUserAction = false;
			_G(mov_phasen)[0].AtsText = 24;
			switch_room(2);
			break;

		default:
			break;
		}
	}

	_G(flags).AniUserAction = false;
	_G(zoom_horizont) = tmp;
	_G(auto_obj) = 0;
}

} // namespace Rooms
} // namespace Chewy
