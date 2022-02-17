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
#include "chewy/globals.h"
#include "chewy/sound.h"

namespace Chewy {

#define ZOBJ_ANI_DETAIL 1
#define ZOBJ_STATIC_DETAIL 2
#define ZOBJ_INVENTAR 3
#define ZOBJ_AUTO_OBJ 4
#define ZOBJ_PROGANI 5
#define ZOBJ_CHEWY 6
#define ZOBJ_HOWARD 7
#define ZOBJ_NICHELLE 8

static const int16 SPZ_ANI_PH[][2] = {
	{   0,   12 },
	{  12,    6 },
	{  18,    6 },
	{  24,    6 },
	{  30,    6 },
	{  36,    6 },
	{  42,    6 },
	{  48,    5 },
	{  53,    3 },
	{  56,   14 },
	{  70,    7 },
	{  77,    6 },
	{  83,    5 },
	{  88,    8 },
	{  96,    5 },
	{   5,    7 },
	{  24,   12 },
	{ 101,   19 },
	{ 120,    8 },
	{ 128,    3 },
	{ 128,    8 },
	{ 136,    8 },
	{ 465,    8 },
	{ 473,    3 },
	{ 473,    8 },
	{ 481,    8 },
	{ 144,    6 },
	{ 150,    5 },
	{ 155,    9 },
	{ 155,    3 },
	{ 164,    3 },
	{ 167,    8 },
	{ 175,    7 },
	{ 182,   10 },
	{ 192,   15 },
	{ 214,    8 },
	{ 207,    3 },
	{ 210,    4 },
	{ 214,    8 },
	{ 559,    8 },
	{ 552,    3 },
	{ 555,    4 },
	{ 559,    8 },
	{ 222,    7 },
	{ 229,    4 },
	{ 233,    9 },
	{ 242,    4 },
	{ 270,    8 },
	{ 246,    8 },
	{ 297,    3 },
	{ 297,    8 },
	{ 262,    8 },
	{ 591,    8 },
	{ 642,    3 },
	{ 642,    8 },
	{ 254,    8 },
	{ 288,    3 },
	{ 288,    9 },
	{ 607,    8 },
	{ 599,    8 },
	{ 633,    3 },
	{ 633,    9 },
	{ 305,   18 },
	{ 278,   10 },
	{ 323,    2 },
	{ 325,    6 },
	{ 331,    2 },
	{ 333,    8 },
	{ 341,    4 }
};


void sprite_engine() {
	int16 zmin;
	int16 min_zeiger = 0;
	int16 i;
	int16 j;
	int16 nr;
	int16 spr_nr;
	int16 *Cxy;
	int16 x, y;
	int16 p_nr;
	ObjMov detmov;
	TafSeqInfo *ts_info;
	calc_z_ebene();
	calc_person_ani();

	for (i = 0; i < _G(z_count); i++) {
		zmin = 3000;
		for (j = 0; j < _G(z_count); j++) {
			if (_G(z_obj_sort)[j].ObjZ != 3000 && _G(z_obj_sort)[j].ObjZ < zmin) {
				zmin = _G(z_obj_sort)[j].ObjZ;
				min_zeiger = j;
			}
		}
		nr = (int16)_G(z_obj_sort)[min_zeiger].ObjNr;

		switch (_G(z_obj_sort)[min_zeiger].ObjArt) {
		case ZOBJ_ANI_DETAIL:
			if (_G(Adi)[nr].zoom) {

				y = _G(Adi)[nr].y;
				calc_zoom(y, (int16)_G(room)->_roomInfo->ZoomFak, (int16)_G(room)->_roomInfo->ZoomFak, &detmov);
			} else {
				detmov.Xzoom = 0;
				detmov.Yzoom = 0;
			}
			_G(det)->plot_ani_details(_G(spieler).scrollx, _G(spieler).scrolly, nr, nr,
			                       detmov.Xzoom, detmov.Yzoom);
			break;

		case ZOBJ_STATIC_DETAIL:
			_G(det)->plot_static_details(_G(spieler).scrollx, _G(spieler).scrolly, nr, nr);
			break;

		case ZOBJ_INVENTAR:
			_G(out)->sprite_set(_G(inv_spr)[nr],
			                 _G(spieler).room_m_obj[nr].X - _G(spieler).scrollx,
			                 _G(spieler).room_m_obj[nr].Y - _G(spieler).scrolly, 0);
			break;

		case ZOBJ_CHEWY:
			if (!_G(spieler).PersonHide[P_CHEWY]) {
				if (!_G(spz_ani)[P_CHEWY]) {
					spr_nr = _G(chewy_ph)[_G(spieler_vector)[P_CHEWY].Phase * 8 + _G(spieler_vector)[P_CHEWY].PhNr];
					x = _G(spieler_mi)[P_CHEWY].XyzStart[0] + _G(chewy_kor)[spr_nr * 2] - _G(spieler).scrollx;
					y = _G(spieler_mi)[P_CHEWY].XyzStart[1] + _G(chewy_kor)[spr_nr * 2 + 1] - _G(spieler).scrolly;
					calc_zoom(_G(spieler_mi)[P_CHEWY].XyzStart[1], (int16)_G(room)->_roomInfo->ZoomFak,
					          (int16)_G(room)->_roomInfo->ZoomFak, &_G(spieler_vector)[P_CHEWY]);

					_G(out)->scale_set(_G(chewy)->image[spr_nr], x, y,
					                _G(spieler_vector)[P_CHEWY].Xzoom,
					                _G(spieler_vector)[P_CHEWY].Yzoom,
					                _G(scr_width));
				} else {
					spr_nr = _G(spz_spr_nr)[_G(spieler_vector)[P_CHEWY].PhNr];
					x = _G(spieler_mi)[P_CHEWY].XyzStart[0] + _G(spz_tinfo)->korrektur[spr_nr * 2] -
					    _G(spieler).scrollx;
					y = _G(spieler_mi)[P_CHEWY].XyzStart[1] + _G(spz_tinfo)->korrektur[spr_nr * 2 + 1] -
					    _G(spieler).scrolly;
					calc_zoom(_G(spieler_mi)[P_CHEWY].XyzStart[1],
					          (int16)_G(room)->_roomInfo->ZoomFak,
					          (int16)_G(room)->_roomInfo->ZoomFak,
					          &_G(spieler_vector)[P_CHEWY]);

					_G(out)->scale_set(_G(spz_tinfo)->image[spr_nr], x, y,
					                _G(spieler_vector)[P_CHEWY].Xzoom,
					                _G(spieler_vector)[P_CHEWY].Yzoom,
					                _G(scr_width));
				}
			}
			break;

		case ZOBJ_HOWARD:
		case ZOBJ_NICHELLE:
			p_nr = _G(z_obj_sort)[min_zeiger].ObjArt - 6;
			if (!_G(spieler).PersonHide[p_nr]) {
				if (!_G(spz_ani)[p_nr]) {
					ts_info = _G(PersonTaf)[p_nr];
					spr_nr = _G(PersonSpr)[p_nr][_G(spieler_vector)[p_nr].PhNr];
				} else {
					ts_info = _G(spz_tinfo);
					spr_nr = _G(spz_spr_nr)[_G(spieler_vector)[p_nr].PhNr];
				}

				x = _G(spieler_mi)[p_nr].XyzStart[0] +
				    ts_info->korrektur[spr_nr * 2] -
				    _G(spieler).scrollx;
				y = _G(spieler_mi)[p_nr].XyzStart[1] +
				    ts_info->korrektur[spr_nr * 2 + 1] -
				    _G(spieler).scrolly;
				calc_zoom(_G(spieler_mi)[p_nr].XyzStart[1],
				          _G(spieler).ZoomXy[p_nr][0],
				          _G(spieler).ZoomXy[p_nr][1],
				          &_G(spieler_vector)[p_nr]);
				_G(out)->scale_set(ts_info->image[spr_nr], x, y,
				                _G(spieler_vector)[p_nr].Xzoom,
				                _G(spieler_vector)[p_nr].Yzoom,
				                _G(scr_width));
			}
			break;
		case ZOBJ_PROGANI:
			_G(out)->sprite_set(_G(spr_info)[nr].Image,
			                 _G(spr_info)[nr].X - _G(spieler).scrollx,
			                 _G(spr_info)[nr].Y - _G(spieler).scrolly, 0);
			break;

		case ZOBJ_AUTO_OBJ:
			spr_nr = _G(mov_phasen)[nr].Phase[_G(auto_mov_vector)[nr].Phase][0] +
			         _G(auto_mov_vector)[nr].PhNr;
			Cxy = _G(room_blk).DetKorrekt + (spr_nr << 1);
			calc_zoom(_G(auto_mov_vector)[nr].Xypos[1],
			          _G(mov_phasen)[nr].ZoomFak,
			          _G(mov_phasen)[nr].ZoomFak,
			          &_G(auto_mov_vector)[nr]);
			_G(out)->scale_set(_G(room_blk).DetImage[spr_nr],
			                _G(auto_mov_vector)[nr].Xypos[0] + Cxy[0] - _G(spieler).scrollx,
			                _G(auto_mov_vector)[nr].Xypos[1] + Cxy[1] - _G(spieler).scrolly,
			                _G(auto_mov_vector)[nr].Xzoom,
			                _G(auto_mov_vector)[nr].Yzoom, _G(scr_width));
			break;

		default:
			break;
		}
		_G(z_obj_sort)[min_zeiger].ObjZ = 3000;
	}
}

void calc_z_ebene() {
	int16 i;
	_G(z_count) = 0;

	for (i = 0; i < MAX_PERSON; i++) {
		if (_G(spieler).PersonRoomNr[P_CHEWY + i] == _G(spieler).PersonRoomNr[P_CHEWY] &&
		        _G(spieler_mi)[P_CHEWY + i].Id != NO_MOV_OBJ) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_CHEWY + i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(spieler_vector)[P_CHEWY + i].Xypos[1] +
			                           _G(spieler_mi)[P_CHEWY + i].HotMovY
			                           - abs(_G(spieler_vector)[P_CHEWY + i].Yzoom);
			++_G(z_count);
		}
	}

	for (i = 0; i < MAXDETAILS; i++) {
		if (_G(Sdi)[i].SprNr != -1) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_STATIC_DETAIL;
			_G(z_obj_sort)[_G(z_count)].ObjNr = i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(Sdi)[i].z_ebene;
			++_G(z_count);
		}
		if (_G(Adi)[i].start_ani != -1) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_ANI_DETAIL;
			_G(z_obj_sort)[_G(z_count)].ObjNr = i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(Adi)[i].z_ebene;
			++_G(z_count);
		}
	}

	for (i = 0; i < _G(obj)->mov_obj_room[0]; i++) {

		if (_G(spieler).room_m_obj[_G(obj)->mov_obj_room[i + 1]].ZEbene < 2000) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_INVENTAR;
			_G(z_obj_sort)[_G(z_count)].ObjNr = _G(obj)->mov_obj_room[i + 1];
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(spieler).room_m_obj[_G(obj)->mov_obj_room[i + 1]].ZEbene;
			++_G(z_count);
		}
	}

	for (i = 0; i < MAX_PROG_ANI; i++) {
		if (_G(spr_info)[i].ZEbene < 200) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_PROGANI;
			_G(z_obj_sort)[_G(z_count)].ObjNr = i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(spr_info)[i].ZEbene;
			++_G(z_count);
		}
	}

	for (i = 0; i < _G(auto_obj); i++) {
		if (_G(auto_mov_vector)[i].Xypos[2] < 200) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_AUTO_OBJ;
			_G(z_obj_sort)[_G(z_count)].ObjNr = i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(auto_mov_vector)[i].Xypos[2];
			++_G(z_count);
		}
	}
}

int16 mouse_on_prog_ani() {
	int16 i;
	int16 ani_nr;
	ani_nr = -1;
	for (i = 0; i < MAX_PROG_ANI && ani_nr == -1; i++) {

		if (_G(minfo).x >= _G(spr_info)[i].X && _G(minfo).x <= _G(spr_info)[i].X1 &&
		        _G(minfo).y >= _G(spr_info)[i].Y && _G(minfo).y <= _G(spr_info)[i].Y1) {
			ani_nr = i;
		}
	}

	return ani_nr;
}

void set_person_pos(int16 x, int16 y, int16 p_nr, int16 richtung) {
	int16 u_index;
	int16 tmp_nr;
	if (richtung != -1)
		set_person_spr(richtung, p_nr);
	_G(spieler_vector)[p_nr].Xypos[0] = x;
	_G(spieler_vector)[p_nr].Xypos[1] = y;
	_G(spieler_mi)[p_nr].XyzStart[0] = x;
	_G(spieler_mi)[p_nr].XyzStart[1] = y;
	_G(spieler_vector)[p_nr].Count = 0;
	_G(spieler_vector)[p_nr].Delay = _G(spieler).DelaySpeed;
	_G(spieler_vector)[p_nr].DelayCount = 0;
	calc_zoom(_G(spieler_mi)[p_nr].XyzStart[1],
	          _G(spieler).ZoomXy[p_nr][0],
	          _G(spieler).ZoomXy[p_nr][1],
	          &_G(spieler_vector)[p_nr]);
	tmp_nr = p_nr;
	if (p_nr >= P_NICHELLE) {
		++tmp_nr;
	}
	_G(ssi)[tmp_nr].X = _G(spieler_vector)[p_nr].Xypos[0] - _G(spieler).scrollx + _G(spieler_mi)[p_nr].HotX;
	_G(ssi)[tmp_nr].Y = _G(spieler_vector)[p_nr].Xypos[1] - _G(spieler).scrolly;
	if (!_G(flags).ExitMov) {
		if (p_nr == P_CHEWY) {
			u_index = _G(ged)->ged_idx(x + _G(spieler_mi)[p_nr].HotX, y + _G(spieler_mi)[p_nr].HotY,
			                        _G(room)->_gedXAnz[_G(room_blk).AkAblage],
			                        _G(ged_mem)[_G(room_blk).AkAblage]);
			check_shad(u_index, 1);
		}
	}
}

void set_person_spr(int16 nr, int16 p_nr) {

	if (nr == 5 || nr == 4)
		nr = 0;
	else if (nr == 10 || nr == 7)
		nr = 1;

	switch (p_nr) {
	case P_CHEWY:
		switch (nr) {
		case P_LEFT:
			_G(spieler_vector)[P_CHEWY].Phase = CH_L_STEHEN;
			_G(spieler_vector)[P_CHEWY].PhNr = 0;
			_G(spieler_vector)[P_CHEWY].PhAnz = _G(chewy_ph_anz)[CH_L_STEHEN];
			_G(person_end_phase)[P_CHEWY] = P_LEFT;
			break;

		case P_RIGHT:
			_G(spieler_vector)[P_CHEWY].Phase = CH_R_STEHEN;
			_G(spieler_vector)[P_CHEWY].PhNr = 0;
			_G(spieler_vector)[P_CHEWY].PhAnz = _G(chewy_ph_anz)[CH_R_STEHEN];
			_G(person_end_phase)[P_CHEWY] = P_RIGHT;
			break;

		}
		break;

	case P_HOWARD:
	case P_NICHELLE:
#define HO_L_STEHEN 0
#define HO_R_STEHEN 0
		_G(spieler_vector)[p_nr].PhNr = 0;
		_G(spieler_vector)[p_nr].PhAnz = 8;
		_G(person_end_phase)[p_nr] = P_LEFT;
		switch (nr) {
		case P_LEFT:
			_G(spieler_vector)[p_nr].Phase = HO_L_STEHEN;
			_G(spieler_vector)[p_nr].PhNr = 0;
			_G(spieler_vector)[p_nr].PhAnz = 8;
			_G(person_end_phase)[p_nr] = P_LEFT;
			break;

		case P_RIGHT:
			_G(spieler_vector)[p_nr].Phase = HO_R_STEHEN;
			_G(spieler_vector)[p_nr].PhNr = 0;
			_G(spieler_vector)[p_nr].PhAnz = 8;
			_G(person_end_phase)[p_nr] = P_RIGHT;
			break;

		}
		break;

	default:
		break;
	}
}

void stop_person(int16 p_nr) {
	_G(mov)->stop_auto_go();
	_G(spieler_vector)[p_nr].Count = 0;

}

void start_detail_wait(int16 ani_nr, int16 rep, int16 mode) {
	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;
	_G(det)->start_detail(ani_nr, rep, mode);
	while (_G(det)->get_ani_status(ani_nr) && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = _G(tmp_maus_links);
}

void start_detail_frame(int16 ani_nr, int16 rep, int16 mode, int16 frame) {
	AniDetailInfo *adi;
	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;
	_G(det)->start_detail(ani_nr, rep, mode);
	adi = _G(det)->get_ani_detail(ani_nr);
	if (mode == ANI_VOR)
		frame = adi->ani_count + frame;
	else
		frame = adi->ani_count - frame;
	while (_G(det)->get_ani_status(ani_nr) && adi->ani_count != frame && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = _G(tmp_maus_links);
}

void wait_detail(int16 det_nr) {
	while (_G(det)->get_ani_status(det_nr) && !SHOULD_QUIT)
		set_up_screen(DO_SETUP);
}

void wait_show_screen(int16 frames) {
	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;
	if (_G(spieler).DelaySpeed > 0)
		frames *= _G(spieler).DelaySpeed;
	while (--frames > 0 && !SHOULD_QUIT) {
		if (_G(flags).AniUserAction)
			get_user_key(NO_SETUP);
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = _G(tmp_maus_links);
}

void start_ani_block(int16 anz, const AniBlock *ab) {
	int16 i;
	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;
	for (i = 0; i < anz; i++) {
		if (ab[i].Mode == ANI_WAIT)
			start_detail_wait(ab[i].Nr, ab[i].Repeat, ab[i].Dir);
		else
			_G(det)->start_detail(ab[i].Nr, ab[i].Repeat, ab[i].Dir);
	}
	_G(maus_links_click) = _G(tmp_maus_links);
}

void start_aad_wait(int16 dia_nr, int16 str_nr) {

	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;
	_G(talk_start_ani) = -1;
	_G(talk_hide_static) = -1;
	set_ssi_xy();
	_G(atds)->start_aad(dia_nr);
	while (_G(atds)->aad_get_status() != -1 && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = _G(tmp_maus_links);
	if (_G(minfo).button)
		_G(flags).main_maus_flag = 1;
	_G(kbinfo).scan_code = Common::KEYCODE_INVALID;
	stop_spz();
}

void start_aad(int16 dia_nr) {
	g_engine->_sound->waitForSpeechToFinish();
	set_ssi_xy();
	_G(atds)->start_aad(dia_nr);
}

bool start_ats_wait(int16 txt_nr, int16 txt_mode, int16 col, int16 mode) {
	int16 VocNr;
	int16 vocx;
	bool ret = false;
	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;

	if (!_G(flags).AtsText) {
		_G(flags).AtsText = true;
		if (txt_nr != -1) {
			if (_G(menu_item) != CUR_WALK)
				atds_string_start(30000, 0, 0, AAD_STR_START);
			ret = _G(atds)->start_ats(txt_nr, txt_mode, col, mode, &VocNr);
			if (ret) {
				while (_G(atds)->ats_get_status() != false && !SHOULD_QUIT)
					set_up_screen(DO_SETUP);

			// WORKAROUND: There are a few cases in the game with no text,
			// but a voice sample the game would fall back on even in
			// subtitles only mode. Don't allow this in ScummVM
			} else if (VocNr >= 0 && g_engine->_sound->getSpeechSubtitlesMode() != DISPLAY_TXT) {
				ret = true;
				vocx = _G(spieler_vector)[P_CHEWY].Xypos[0] - _G(spieler).scrollx + _G(spieler_mi)[P_CHEWY].HotX;
				g_engine->_sound->setSoundChannelBalance(0, _G(atds)->getStereoPos(vocx));
				g_engine->_sound->playSpeech(VocNr);
				//warning("FIXME - unknown constant SMP_PLAYING");

				set_up_screen(DO_SETUP);
			}
			if (_G(menu_item) != CUR_WALK)
				atds_string_start(30000, 0, 0, AAD_STR_END);
		}
		_G(flags).AtsText = false;
	}
	if (_G(minfo).button)
		_G(flags).main_maus_flag = 1;
	_G(kbinfo).scan_code = Common::KEYCODE_INVALID;
	_G(maus_links_click) = _G(tmp_maus_links);
	return ret;
}

void aad_wait(int16 str_nr) {
	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;
	if (str_nr == -1) {
		while (_G(atds)->aad_get_status() != -1 && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}
	} else {
		while (_G(atds)->aad_get_status() < str_nr && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}
	}
	_G(maus_links_click) = _G(tmp_maus_links);
	if (_G(minfo).button)
		_G(flags).main_maus_flag = 1;
	_G(kbinfo).scan_code = Common::KEYCODE_INVALID;
}

void start_aad(int16 dia_nr, int16 ssi_nr) {
	switch (ssi_nr) {
	case 0:
		_G(ssi)[0].X = _G(spieler_vector)[P_CHEWY].Xypos[0] - _G(spieler).scrollx + _G(spieler_mi)[P_CHEWY].HotX;
		_G(ssi)[0].Y = _G(spieler_vector)[P_CHEWY].Xypos[1] - _G(spieler).scrolly;
		_G(atds)->set_split_win(0, &_G(ssi)[0]);
		break;

	default:
		break;
	}
	_G(atds)->start_aad(dia_nr);
}

void start_ads_wait(int16 dia_nr) {
	if (!_G(flags).AdsDialog) {
		_G(menu_item) = CUR_TALK;
		cursor_wahl(_G(menu_item));
		load_ads_dia(dia_nr);
		while (_G(flags).AdsDialog && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}
	}
}

void wait_auto_obj(int16 nr) {
	_G(tmp_maus_links) = _G(maus_links_click);
	_G(maus_links_click) = false;
	while (_G(mov_phasen)[nr].Repeat != -1 && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = _G(tmp_maus_links);
}

void stop_auto_obj(int16 nr) {
	if (nr < _G(auto_obj)) {
		_G(mov_phasen)[nr].Start = false;
	}
}

void continue_auto_obj(int16 nr, int16 repeat) {
	if (nr < _G(auto_obj)) {
		_G(mov_phasen)[nr].Start = 1;
		if (repeat)
			_G(mov_phasen)[nr].Repeat = repeat;
	}
}

void init_auto_obj(int16 auto_nr, const int16 *phasen, int16 lines, const MovLine *mline) {
	int16 i;
	int16 *tmp;
	MovLine *tmp1;
	const MovLine *tmp2;
	tmp2 = mline;
	tmp = (int16 *)_G(mov_phasen)[auto_nr].Phase;
	for (i = 0; i < 8; i++)
		tmp[i] = phasen[i];
	_G(mov_phasen)[auto_nr].Start = 1;

	tmp1 = _G(mov_line)[auto_nr];
	for (i = 0; i < lines; i++) {
		tmp1->EndXyz[0] = tmp2->EndXyz[0];
		tmp1->EndXyz[1] = tmp2->EndXyz[1];
		tmp1->EndXyz[2] = tmp2->EndXyz[2];
		tmp1->PhNr = tmp2->PhNr;
		tmp1->Vorschub = tmp2->Vorschub;
		++tmp1;
		++tmp2;
	}

	_G(auto_mov_obj)[auto_nr].XyzEnd[0] = mline->EndXyz[0];
	_G(auto_mov_obj)[auto_nr].XyzEnd[1] = mline->EndXyz[1];
	_G(auto_mov_obj)[auto_nr].XyzEnd[2] = mline->EndXyz[2];

	_G(auto_mov_vector)[auto_nr].Count = 0;
	_G(auto_mov_vector)[auto_nr].StNr = 0;
	_G(auto_mov_vector)[auto_nr].DelayCount = 0;
	new_auto_line(auto_nr);
}

void new_auto_line(int16 nr) {

	if (nr < _G(auto_obj)) {
		if (_G(mov_phasen)[nr].Repeat != -1 &&
		        _G(mov_phasen)[nr].Start) {

			if (_G(auto_mov_vector)[nr].StNr < _G(mov_phasen)[nr].Lines) {
				++_G(auto_mov_vector)[nr].StNr;
			} else {

				_G(auto_mov_vector)[nr].StNr = 1;
				if (_G(mov_phasen)[nr].Repeat != 255) {
					--_G(mov_phasen)[nr].Repeat;
				}
			}
			if (!_G(mov_phasen)[nr].Repeat) {
				_G(auto_mov_vector)[nr].Xypos[2] = 201;
				_G(mov_phasen)[nr].Repeat = -1;
				_G(mov_phasen)[nr].Start = 0;
			} else {
				_G(auto_mov_vector)[nr].Phase = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].PhNr;
				_G(auto_mov_vector)[nr].PhNr = 0;

				_G(auto_mov_vector)[nr].PhAnz = (_G(mov_phasen)[nr].Phase[_G(auto_mov_vector)[nr].Phase][1] -
				                             _G(mov_phasen)[nr].Phase[_G(auto_mov_vector)[nr].Phase][0]) + 1;
				_G(auto_mov_obj)[nr].Vorschub = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].Vorschub;
				_G(auto_mov_obj)[nr].XyzStart[0] = _G(auto_mov_obj)[nr].XyzEnd[0];
				_G(auto_mov_obj)[nr].XyzStart[1] = _G(auto_mov_obj)[nr].XyzEnd[1];
				_G(auto_mov_obj)[nr].XyzStart[2] = _G(auto_mov_obj)[nr].XyzEnd[2];
				_G(auto_mov_obj)[nr].XyzEnd[0] = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].EndXyz[0] ;
				_G(auto_mov_obj)[nr].XyzEnd[1] = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].EndXyz[1] ;;
				_G(auto_mov_obj)[nr].XyzEnd[2] = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].EndXyz[2] ;;
				_G(mov)->get_mov_vector((int16 *)_G(auto_mov_obj)[nr].XyzStart,
				                     _G(auto_mov_obj)[nr].Vorschub, &_G(auto_mov_vector)[nr]);
			}
		} else {
			_G(auto_mov_vector)[nr].Xypos[2] = 201;
		}
	}
}

int16 mouse_auto_obj(int16 nr, int16 xoff, int16 yoff) {
	int16 ret;
	int16 spr_nr;
	int16 *xy;
	int16 *Cxy;
	ret = false;
	if (_G(mov_phasen)[nr].Start == 1) {
		spr_nr = _G(mov_phasen)[nr].Phase[_G(auto_mov_vector)[nr].Phase][0] +
		         _G(auto_mov_vector)[nr].PhNr;
		xy = (int16 *)_G(room_blk).DetImage[spr_nr];
		Cxy = _G(room_blk).DetKorrekt + (spr_nr << 1);

		if (!xoff) {
			xoff = xy ? xy[0] : 0;
			xoff += _G(auto_mov_vector)[nr].Xzoom;
		}
		if (!yoff) {
			yoff = xy ? xy[1] : 0;
			yoff += _G(auto_mov_vector)[nr].Yzoom;
		}
		if (_G(minfo).x >= _G(auto_mov_vector)[nr].Xypos[0] + Cxy[0] - _G(spieler).scrollx &&
		        _G(minfo).x <= _G(auto_mov_vector)[nr].Xypos[0] + xoff + Cxy[0] - _G(spieler).scrollx &&
		        _G(minfo).y >= _G(auto_mov_vector)[nr].Xypos[1] + Cxy[1] - _G(spieler).scrolly &&
		        _G(minfo).y <= _G(auto_mov_vector)[nr].Xypos[1] + yoff + Cxy[1] - _G(spieler).scrolly)
			ret = true;
	}
	return ret;
}

int16 auto_obj_status(int16 nr) {
	int16 status;
	status = false;
	if (nr < _G(auto_obj)) {
		if (_G(mov_phasen)[nr].Repeat != -1)
			status = true;
	}

	return status;
}

void calc_zoom(int16 y, int16 zoomfak_x, int16 zoomfak_y, ObjMov *om) {
	float zoom_fak_x;
	float zoom_fak_y;
	zoom_fak_x = (float)zoomfak_x / (float)100.0;
	zoom_fak_x = -zoom_fak_x;
	zoom_fak_y = (float)zoomfak_y / (float)100.0;
	zoom_fak_y = -zoom_fak_y;
	if (!_G(zoom_horizont)) {
		om->Xzoom = -zoomfak_x;
		om->Yzoom = -zoomfak_y;
	} else {
		if (y < _G(zoom_horizont)) {
			om->Xzoom = (_G(zoom_horizont) - y) * zoom_fak_x;
			om->Yzoom = (_G(zoom_horizont) - y) * zoom_fak_y;
		} else {
			om->Xzoom = 0;
			om->Yzoom = 0;
		}
	}
}

void mov_objekt(ObjMov *om, MovInfo *mi) {
	int16 tmpx;
	int16 tmpy;
	int16 tmpz;
	int16 u_index;
	u_index = 0;

	if (om->DelayCount > 0)
		--om->DelayCount;
	else {
		om->DelayCount = om->Delay;
		if (om->Count > 0) {

			--om->Count;
			if (om->PhNr < om->PhAnz - 1)
				++om->PhNr;
			else
				om->PhNr = 0;
			tmpx = om->Xyvo[0];
			tmpy = om->Xyvo[1];
			tmpz = om->Xyvo[2];
			om->Xyna[0][0] += om->Xyna[0][1];
			if (om->Xyna[0][1] < 0) {
				if (om->Xyna[0][0] < -1000) {
					om->Xyna[0][0] += 1000;
					--tmpx;
				}
			} else if (om->Xyna[0][0] > 1000) {
				om->Xyna[0][0] -= 1000;
				++tmpx;
			}
			om->Xyna[1][0] += om->Xyna[1][1];
			if (om->Xyna[1][1] < 0) {
				if (om->Xyna[1][0] < -1000) {
					om->Xyna[1][0] += 1000;
					--tmpy;
				}
			} else if (om->Xyna[1][0] > 1000) {
				om->Xyna[1][0] -= 1000;
				++tmpy;
			}
			om->Xyna[2][0] += om->Xyna[2][1];
			if (om->Xyna[2][1] < 0) {
				if (om->Xyna[2][0] < -1000) {
					om->Xyna[2][0] += 1000;
					--tmpz;
				}
			} else if (om->Xyna[2][0] > 1000) {
				om->Xyna[2][0] -= 1000;
				++tmpz;
			}

			if (!mi->Mode) {
				if (!(u_index = _G(ged)->ged_idx(om->Xypos[0] + mi->HotX + tmpx,
				                              om->Xypos[1] + mi->HotY + tmpy,
				                              _G(room)->_gedXAnz[_G(room_blk).AkAblage],
				                              _G(ged_mem)[_G(room_blk).AkAblage]))) {

					if (!(u_index = _G(ged)->ged_idx(om->Xypos[0] + mi->HotX + tmpx,
					                              om->Xypos[1] + mi->HotY,
					                              _G(room)->_gedXAnz[_G(room_blk).AkAblage],
					                              _G(ged_mem)[_G(room_blk).AkAblage]))) {

						if (!(u_index = _G(ged)->ged_idx(om->Xypos[0] + mi->HotX,
						                              om->Xypos[1] + mi->HotY + tmpy,
						                              _G(room)->_gedXAnz[_G(room_blk).AkAblage],
						                              _G(ged_mem)[_G(room_blk).AkAblage]))) {
							om->Count = 0;
						} else {
							if (!tmpy) {
								if (om->Xyna[1][1] < 0)
									tmpy = -1;
								else
									tmpy = 1;
							}
							if (mi->Id == CHEWY_OBJ)
								check_shad(u_index, 1);

							if (abs(om->Xypos[1] - mi->XyzEnd[1]) <= abs(tmpy)) {
								om->Count = 0;
								if (!_G(flags).NoEndPosMovObj) {
									mi->XyzStart[0] = mi->XyzEnd[0];
									mi->XyzStart[1] = mi->XyzEnd[1];
									om->Xypos[0] = mi->XyzEnd[0];
									om->Xypos[1] = mi->XyzEnd[1];
								}
							} else {
								om->Xypos[1] += tmpy;
								om->Xypos[2] += tmpz;
								mi->XyzStart[0] = om->Xypos[0];
								mi->XyzStart[1] = om->Xypos[1];
								_G(mov)->get_mov_vector(mi->XyzStart, mi->Vorschub, om);
								if (om->Xyvo[1] != 0) {
									_G(new_vector) = true;
									swap_if_l(&om->Xyvo[1], &om->Xyvo[0]);
								}
								get_phase(om, mi);
								if (om->Count == 0 && !_G(flags).NoEndPosMovObj) {
									mi->XyzStart[0] = mi->XyzEnd[0];
									mi->XyzStart[1] = mi->XyzEnd[1];
									om->Xypos[0] = mi->XyzEnd[0];
									om->Xypos[1] = mi->XyzEnd[1];
								}
							}
						}
					} else {
						if (!tmpx) {
							if (om->Xyna[0][1] < 0)
								tmpx = -1;
							else
								tmpx = 1;
						}
						if (mi->Id == CHEWY_OBJ)
							check_shad(u_index, 1);

						if (abs(om->Xypos[0] - mi->XyzEnd[0]) <= abs(tmpx)) {
							om->Count = 0;
							if (!_G(flags).NoEndPosMovObj) {
								mi->XyzStart[0] = mi->XyzEnd[0];
								mi->XyzStart[1] = mi->XyzEnd[1];
								om->Xypos[0] = mi->XyzEnd[0];
								om->Xypos[1] = mi->XyzEnd[1];
							}
						} else {
							om->Xypos[0] += tmpx;
							om->Xypos[2] += tmpz;
							mi->XyzStart[0] = om->Xypos[0];
							mi->XyzStart[1] = om->Xypos[1];
							_G(mov)->get_mov_vector(mi->XyzStart, mi->Vorschub, om);
							if (om->Xyvo[0] != 0) {
								_G(new_vector) = true;
								swap_if_l(&om->Xyvo[0], &om->Xyvo[1]);

							}
							get_lr_phase(om, mi->Id);
							if (om->Count == 0 && !_G(flags).NoEndPosMovObj) {
								mi->XyzStart[0] = mi->XyzEnd[0];
								mi->XyzStart[1] = mi->XyzEnd[1];
								om->Xypos[0] = mi->XyzEnd[0];
								om->Xypos[1] = mi->XyzEnd[1];
							}
						}
					}
				} else {
					if (mi->Id == CHEWY_OBJ)
						check_shad(u_index, 1);
					om->Xypos[0] += tmpx;
					om->Xypos[1] += tmpy;
					om->Xypos[2] += tmpz;
					if (_G(new_vector)) {
						_G(new_vector) = false;
						mi->XyzStart[0] = om->Xypos[0];
						mi->XyzStart[1] = om->Xypos[1];
						_G(mov)->get_mov_vector(mi->XyzStart, mi->Vorschub, om);
					}
					if (om->Count == 0 && !_G(flags).NoEndPosMovObj) {
						mi->XyzStart[0] = mi->XyzEnd[0];
						mi->XyzStart[1] = mi->XyzEnd[1];
						om->Xypos[0] = mi->XyzEnd[0];
						om->Xypos[1] = mi->XyzEnd[1];
					}
				}
			} else {

				om->Xypos[0] += tmpx;
				om->Xypos[1] += tmpy;
				om->Xypos[2] += tmpz;
				if (mi->Id == CHEWY_OBJ) {
					u_index = _G(ged)->ged_idx(om->Xypos[0] + mi->HotX,
					                        om->Xypos[1] + mi->HotY,
					                        _G(room)->_gedXAnz[_G(room_blk).AkAblage],
					                        _G(ged_mem)[_G(room_blk).AkAblage]);
					check_shad(u_index, 1);
				}
			}
			if (mi->Id == CHEWY_OBJ) {
				if (u_index >= 50 && u_index < 62)
					check_ged_action(u_index);
			}
		} else {
			switch (mi->Id) {
			case CHEWY_OBJ:
				if (!_G(spz_ani)[P_CHEWY])
					calc_person_end_ani(om, P_CHEWY);
				else
					calc_person_spz_ani(om);
				break;

			case HOWARD_OBJ:
				if (!_G(spz_ani)[P_HOWARD])
					calc_person_end_ani(om, P_HOWARD);
				else
					calc_person_spz_ani(om);
				break;

			case NICHELLE_OBJ:
				if (!_G(spz_ani)[P_NICHELLE])
					calc_person_end_ani(om, P_NICHELLE);
				else
					calc_person_spz_ani(om);
				break;

			case NO_MOV_OBJ:
				break;

			default:
				new_auto_line(mi->Id);
				break;
			}
		}
	}

}

void calc_person_end_ani(ObjMov *om, int16 p_nr) {
	if (_G(ani_stand_count)[p_nr] >= 25 * (_G(spieler).DelaySpeed + p_nr * 2)) {
		_G(ani_stand_count)[p_nr] = 0;
		om->PhNr = 0;
		_G(ani_stand_flag)[p_nr] = true;
		set_person_spr(_G(person_end_phase)[p_nr], p_nr);
	} else if (_G(ani_stand_flag)[p_nr]) {
		om->Delay = _G(spieler).DelaySpeed + 1;
		if (om->PhNr < om->PhAnz - 1)
			++om->PhNr;
		else
			_G(ani_stand_flag)[p_nr] = false;
	} else {
		set_person_spr(_G(person_end_phase)[p_nr], p_nr);
		om->PhNr = 0;
		++_G(ani_stand_count)[p_nr];
	}
}

void get_phase(ObjMov *om, MovInfo *mi) {
	int16 p_nr;
	p_nr = 255 - mi->Id;
	if (p_nr >= 0) {
		if (om->Xyvo[0] > 0) {
			if (om->Xyvo[1] > 0) {

				if (om->Xyvo[1] > (mi->Vorschub - 1)) {
					om->Phase = CH_DOWN;
					if (om->Xypos[0] - _G(spieler).scrollx > SCREEN_WIDTH / 2)
						_G(person_end_phase)[p_nr] = P_LEFT;
					else
						_G(person_end_phase)[p_nr] = P_RIGHT;
				} else {
					om->Phase = CH_RIGHT_NO;
					_G(person_end_phase)[p_nr] = P_RIGHT;
				}
			} else {
				if (om->Xyvo[1] < - (mi->Vorschub - 1)) {
					om->Phase = CH_UP;
					if (om->Xypos[0] - _G(spieler).scrollx > SCREEN_WIDTH / 2)
						_G(person_end_phase)[p_nr] = P_LEFT;
					else
						_G(person_end_phase)[p_nr] = P_RIGHT;
				} else {
					_G(person_end_phase)[p_nr] = P_RIGHT;
					om->Phase = CH_RIGHT_NO;
				}
			}
		} else {
			if (om->Xyvo[1] > 0) {

				if (om->Xyvo[1] > (mi->Vorschub - 1)) {
					om->Phase = CH_DOWN;
					if (om->Xypos[0] - _G(spieler).scrollx > SCREEN_WIDTH / 2)
						_G(person_end_phase)[p_nr] = P_LEFT;
					else
						_G(person_end_phase)[p_nr] = P_RIGHT;
				} else {
					_G(person_end_phase)[p_nr] = P_LEFT;
					om->Phase = CH_LEFT_NO;
				}
			} else {
				if (om->Xyvo[1] < -(mi->Vorschub - 1)) {
					om->Phase = CH_UP;
					if (om->Xypos[0] - _G(spieler).scrollx > SCREEN_WIDTH / 2)
						_G(person_end_phase)[p_nr] = P_LEFT;
					else
						_G(person_end_phase)[p_nr] = P_RIGHT;
				} else {
					_G(person_end_phase)[p_nr] = P_LEFT;
					om->Phase = CH_LEFT_NO;
				}
			}
		}
	}
}

void get_lr_phase(ObjMov *om, int16 mode) {
	int16 p_nr;
	p_nr = 255 - mode;
	if (p_nr >= 0) {
		if (om->Xyvo[0] > 0) {
			om->Phase = CH_RIGHT_NO;
		} else {
			om->Phase = CH_LEFT_NO;
		}
	}
}

void zoom_mov_anpass(ObjMov *om, MovInfo *mi) {
	int16 tmp;
	int16 tmp_vorschub;
	tmp_vorschub = mi->Vorschub;
	mi->Vorschub = CH_X_PIX;
	if (_G(flags).ZoomMov) {
		if (om->Xzoom < 0 && _G(zoom_mov_fak) > 0) {
			tmp = om->Xzoom / _G(zoom_mov_fak);
			tmp = abs(tmp);
			mi->Vorschub -= tmp;
			if (mi->Vorschub < 2)
				mi->Vorschub = 2;
		}
		if (tmp_vorschub != mi->Vorschub && om->Count) {
			_G(mov)->get_mov_vector((int16 *)mi->XyzStart, mi->Vorschub, om);
			get_phase(om, mi);
		}
	}
}

void start_spz_wait(int16 ani_id, int16 count, bool reverse, int16 p_nr) {
	if (start_spz(ani_id, count, reverse, p_nr)) {
		while (_G(spz_count) && !SHOULD_QUIT)
			set_up_screen(DO_SETUP);
	}
}

bool start_spz(int16 ani_id, int16 count, bool reverse, int16 p_nr) {
	int16 i;
	int16 spr_start;
	int16 spr_anz;

	bool ret = false;
	if (!_G(flags).SpzAni) {
		_G(flags).SpzAni = true;
		_G(spz_ani)[p_nr] = true;
		_G(spz_p_nr) = p_nr;
		spr_start = SPZ_ANI_PH[ani_id][0];
		spr_anz = SPZ_ANI_PH[ani_id][1];
		if (_G(person_end_phase)[p_nr] == P_RIGHT) {
			ani_id += 100;
			spr_start += CH_SPZ_OFFSET;
		}

		if (ani_id != _G(spz_akt_id)) {
			if (_G(spz_tinfo))
				free((char *)_G(spz_tinfo));
			_G(spz_akt_id) = ani_id;
			_G(spz_tinfo) = _G(mem)->taf_seq_adr(spr_start, spr_anz);
		}

		for (i = 0; i < spr_anz; i++) {
			if (!reverse)
				_G(spz_spr_nr)[i] = i;
			else
				_G(spz_spr_nr)[i] = spr_anz - i - 1;
		}

		_G(spz_start) = spr_start;
		_G(spz_delay)[p_nr] = _G(SpzDelay);
		_G(spieler_vector)[p_nr].Count = 0;
		_G(spieler_vector)[p_nr].PhNr = 0;
		_G(spieler_vector)[p_nr].PhAnz = spr_anz;
		_G(spieler_vector)[p_nr].Delay = _G(spieler).DelaySpeed + _G(spz_delay)[p_nr];
		_G(spieler_vector)[p_nr].DelayCount = 0;
		_G(spz_count) = count;
		_G(flags).MausLinks = true;
		ret = true;
	}
	return ret;
}

void calc_person_spz_ani(ObjMov *om) {
	if (om->PhNr < om->PhAnz - 1)
		++om->PhNr;
	else {
		--_G(spz_count);
		if (_G(spz_count) > 0) {
			om->PhNr = 0;
			om->Delay = _G(spieler).DelaySpeed + _G(spz_delay)[_G(spz_p_nr)];
		} else {
			if (_G(spz_count) != 255)
				stop_spz();
		}
	}
}

void stop_spz() {
	if (_G(flags).SpzAni) {
		_G(flags).SpzAni = false;
		_G(flags).MausLinks = false;
		_G(spz_ani)[_G(spz_p_nr)] = false;
		_G(spieler_vector)[_G(spz_p_nr)].Count = 0;
		_G(spieler_vector)[_G(spz_p_nr)].PhNr = 0;
		set_person_spr(_G(person_end_phase)[_G(spz_p_nr)], _G(spz_p_nr));
		_G(spz_delay)[_G(spz_p_nr)] = 0;
	}
}

void set_spz_delay(int16 delay) {
	_G(SpzDelay) = delay;
}

void load_person_ani(int16 ani_id, int16 p_nr) {
	int16 ani_start;
	short ani_anz;
	if (_G(PersonAni)[p_nr] != ani_id) {
		ani_start = SPZ_ANI_PH[ani_id][0];
		ani_anz = SPZ_ANI_PH[ani_id][1];
		_G(PersonAni)[p_nr] = ani_id;
		if (_G(PersonTaf)[p_nr])
			free((char *)_G(PersonTaf)[p_nr]);
		_G(PersonTaf)[p_nr] = _G(mem)->taf_seq_adr(ani_start, ani_anz);
		_G(spieler_vector)[p_nr].PhNr = 0;
		_G(spieler_vector)[p_nr].PhAnz = ani_anz;
	}
}

uint8 p_ani[MAX_PERSON - 1][5] = {
	{HO_WALK_L, HO_WALK_R, HO_BACK, HO_FRONT, HO_STAND_L},
	{NI_WALK_L, NI_WALK_R, NI_BACK, NI_FRONT, NI_STAND_L}
};

void calc_person_ani() {
	int16 i;
	int16 p_nr;
	int16 ani_nr = 0;
	for (p_nr = 0; p_nr < MAX_PERSON; p_nr++) {
		if (_G(spieler_mi)[p_nr].Id != NO_MOV_OBJ) {
			switch (p_nr) {
			case P_CHEWY:
				break;

			case P_HOWARD:
			case P_NICHELLE:
				if (!_G(spz_ani)[p_nr]) {
					for (i = 0; i < 8; i++)
						_G(PersonSpr)[p_nr][i] = i;

					if (!_G(spieler_vector)[p_nr].Count &&
					        _G(auto_p_nr) != p_nr) {
						ani_nr = (int16)p_ani[p_nr - 1][4] + (_G(person_end_phase)[p_nr] * 4);

						_G(spieler_vector)[p_nr].PhAnz = 5;
						_G(PersonSpr)[p_nr][3] = 1;
						_G(PersonSpr)[p_nr][4] = 0;
					} else {
						switch (_G(spieler_vector)[p_nr].Phase) {
						case CH_LEFT_NO:
							ani_nr = (int16)p_ani[p_nr - 1][0];
							break;

						case CH_RIGHT_NO:
							ani_nr = (int16)p_ani[p_nr - 1][1];
							break;

						case CH_UP:
							ani_nr = (int16)p_ani[p_nr - 1][2];
							break;

						case CH_DOWN:
							ani_nr = (int16)p_ani[p_nr - 1][3];
							break;
						}
					}
					load_person_ani(ani_nr, p_nr);
				}
				break;

			default:
				break;
			}
		}
	}
}

} // namespace Chewy
