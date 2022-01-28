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
#include "chewy/global.h"

namespace Chewy {

#define ZOBJ_ANI_DETAIL 1
#define ZOBJ_STATIC_DETAIL 2
#define ZOBJ_INVENTAR 3
#define ZOBJ_AUTO_OBJ 4
#define ZOBJ_PROGANI 5
#define ZOBJ_CHEWY 6
#define ZOBJ_HOWARD 7
#define ZOBJ_NICHELLE 8
#define MAX_ZOBJ 60

struct ZObjSort {
	uint8 ObjArt;
	uint8 ObjNr;
	int16 ObjZ;
};

int16 z_count;
ZObjSort z_obj_sort[MAX_ZOBJ];
char new_vector = false;
int16 tmp_maus_links;

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
	taf_seq_info *ts_info;
	calc_z_ebene();
	calc_person_ani();

	for (i = 0; i < z_count; i++) {
		zmin = 3000;
		for (j = 0; j < z_count; j++) {
			if (z_obj_sort[j].ObjZ != 3000 && z_obj_sort[j].ObjZ < zmin) {
				zmin = z_obj_sort[j].ObjZ;
				min_zeiger = j;
			}
		}
		nr = (int16)z_obj_sort[min_zeiger].ObjNr;

		switch (z_obj_sort[min_zeiger].ObjArt) {
		case ZOBJ_ANI_DETAIL:
			if (Adi[nr].zoom) {

				y = Adi[nr].y;
				calc_zoom(y, (int16)room->room_info->ZoomFak, (int16)room->room_info->ZoomFak, &detmov);
			} else {
				detmov.Xzoom = 0;
				detmov.Yzoom = 0;
			}
			det->plot_ani_details(_G(spieler).scrollx, _G(spieler).scrolly, nr, nr,
			                       detmov.Xzoom, detmov.Yzoom);
			break;

		case ZOBJ_STATIC_DETAIL:
			det->plot_static_details(_G(spieler).scrollx, _G(spieler).scrolly, nr, nr);
			break;

		case ZOBJ_INVENTAR:
			out->sprite_set(inv_spr[nr],
			                 _G(spieler).room_m_obj[nr].X - _G(spieler).scrollx,
			                 _G(spieler).room_m_obj[nr].Y - _G(spieler).scrolly, 0);
			break;

		case ZOBJ_CHEWY:
			if (!_G(spieler).PersonHide[P_CHEWY]) {
				if (!spz_ani[P_CHEWY]) {
					spr_nr = chewy_ph[spieler_vector[P_CHEWY].Phase * 8 + spieler_vector[P_CHEWY].PhNr];
					x = spieler_mi[P_CHEWY].XyzStart[0] + chewy_kor[spr_nr * 2] - _G(spieler).scrollx;
					y = spieler_mi[P_CHEWY].XyzStart[1] + chewy_kor[spr_nr * 2 + 1] - _G(spieler).scrolly;
					calc_zoom(spieler_mi[P_CHEWY].XyzStart[1], (int16)room->room_info->ZoomFak,
					          (int16)room->room_info->ZoomFak, &spieler_vector[P_CHEWY]);

					out->scale_set(chewy->image[spr_nr], x, y,
					                spieler_vector[P_CHEWY].Xzoom,
					                spieler_vector[P_CHEWY].Yzoom,
					                scr_width);
				} else {
					spr_nr = spz_spr_nr[spieler_vector[P_CHEWY].PhNr];
					x = spieler_mi[P_CHEWY].XyzStart[0] + spz_tinfo->korrektur[spr_nr * 2] -
					    _G(spieler).scrollx;
					y = spieler_mi[P_CHEWY].XyzStart[1] + spz_tinfo->korrektur[spr_nr * 2 + 1] -
					    _G(spieler).scrolly;
					calc_zoom(spieler_mi[P_CHEWY].XyzStart[1],
					          (int16)room->room_info->ZoomFak,
					          (int16)room->room_info->ZoomFak,
					          &spieler_vector[P_CHEWY]);

					out->scale_set(spz_tinfo->image[spr_nr], x, y,
					                spieler_vector[P_CHEWY].Xzoom,
					                spieler_vector[P_CHEWY].Yzoom,
					                scr_width);
				}
			}
			break;

		case ZOBJ_HOWARD:
		case ZOBJ_NICHELLE:
			p_nr = z_obj_sort[min_zeiger].ObjArt - 6;
			if (!_G(spieler).PersonHide[p_nr]) {
				if (!spz_ani[p_nr]) {
					ts_info = PersonTaf[p_nr];
					spr_nr = PersonSpr[p_nr][spieler_vector[p_nr].PhNr];
				} else {
					ts_info = spz_tinfo;
					spr_nr = spz_spr_nr[spieler_vector[p_nr].PhNr];
				}

				x = spieler_mi[p_nr].XyzStart[0] +
				    ts_info->korrektur[spr_nr * 2] -
				    _G(spieler).scrollx;
				y = spieler_mi[p_nr].XyzStart[1] +
				    ts_info->korrektur[spr_nr * 2 + 1] -
				    _G(spieler).scrolly;
				calc_zoom(spieler_mi[p_nr].XyzStart[1],
				          _G(spieler).ZoomXy[p_nr][0],
				          _G(spieler).ZoomXy[p_nr][1],
				          &spieler_vector[p_nr]);
				out->scale_set(ts_info->image[spr_nr], x, y,
				                spieler_vector[p_nr].Xzoom,
				                spieler_vector[p_nr].Yzoom,
				                scr_width);
			}
			break;
		case ZOBJ_PROGANI:
			out->sprite_set(spr_info[nr].Image,
			                 spr_info[nr].X - _G(spieler).scrollx,
			                 spr_info[nr].Y - _G(spieler).scrolly, 0);
			break;

		case ZOBJ_AUTO_OBJ:
			spr_nr = mov_phasen[nr].Phase[auto_mov_vector[nr].Phase][0] +
			         auto_mov_vector[nr].PhNr;
			Cxy = room_blk.DetKorrekt + (spr_nr << 1);
			calc_zoom(auto_mov_vector[nr].Xypos[1],
			          mov_phasen[nr].ZoomFak,
			          mov_phasen[nr].ZoomFak,
			          &auto_mov_vector[nr]);
			out->scale_set(room_blk.DetImage[spr_nr],
			                auto_mov_vector[nr].Xypos[0] + Cxy[0] - _G(spieler).scrollx,
			                auto_mov_vector[nr].Xypos[1] + Cxy[1] - _G(spieler).scrolly,
			                auto_mov_vector[nr].Xzoom,
			                auto_mov_vector[nr].Yzoom, scr_width);
			break;

		default:
			break;
		}
		z_obj_sort[min_zeiger].ObjZ = 3000;
	}
}

void calc_z_ebene() {
	int16 i;
	z_count = 0;

	for (i = 0; i < MAX_PERSON; i++) {
		if (_G(spieler).PersonRoomNr[P_CHEWY + i] == _G(spieler).PersonRoomNr[P_CHEWY] &&
		        spieler_mi[P_CHEWY + i].Id != NO_MOV_OBJ) {
			z_obj_sort[z_count].ObjArt = ZOBJ_CHEWY + i;
			z_obj_sort[z_count].ObjZ = spieler_vector[P_CHEWY + i].Xypos[1] +
			                           spieler_mi[P_CHEWY + i].HotMovY
			                           - abs(spieler_vector[P_CHEWY + i].Yzoom);
			++z_count;
		}
	}

	for (i = 0; i < MAXDETAILS; i++) {
		if (Sdi[i].SprNr != -1) {
			z_obj_sort[z_count].ObjArt = ZOBJ_STATIC_DETAIL;
			z_obj_sort[z_count].ObjNr = i;
			z_obj_sort[z_count].ObjZ = Sdi[i].z_ebene;
			++z_count;
		}
		if (Adi[i].start_ani != -1) {
			z_obj_sort[z_count].ObjArt = ZOBJ_ANI_DETAIL;
			z_obj_sort[z_count].ObjNr = i;
			z_obj_sort[z_count].ObjZ = Adi[i].z_ebene;
			++z_count;
		}
	}

	for (i = 0; i < obj->mov_obj_room[0]; i++) {

		if (_G(spieler).room_m_obj[obj->mov_obj_room[i + 1]].ZEbene < 2000) {
			z_obj_sort[z_count].ObjArt = ZOBJ_INVENTAR;
			z_obj_sort[z_count].ObjNr = obj->mov_obj_room[i + 1];
			z_obj_sort[z_count].ObjZ = _G(spieler).room_m_obj[obj->mov_obj_room[i + 1]].ZEbene;
			++z_count;
		}
	}

	for (i = 0; i < MAX_PROG_ANI; i++) {
		if (spr_info[i].ZEbene < 200) {
			z_obj_sort[z_count].ObjArt = ZOBJ_PROGANI;
			z_obj_sort[z_count].ObjNr = i;
			z_obj_sort[z_count].ObjZ = spr_info[i].ZEbene;
			++z_count;
		}
	}

	for (i = 0; i < _G(auto_obj); i++) {
		if (auto_mov_vector[i].Xypos[2] < 200) {
			z_obj_sort[z_count].ObjArt = ZOBJ_AUTO_OBJ;
			z_obj_sort[z_count].ObjNr = i;
			z_obj_sort[z_count].ObjZ = auto_mov_vector[i].Xypos[2];
			++z_count;
		}
	}
}

int16 mouse_on_prog_ani() {
	int16 i;
	int16 ani_nr;
	ani_nr = -1;
	for (i = 0; i < MAX_PROG_ANI && ani_nr == -1; i++) {

		if (minfo.x >= spr_info[i].X && minfo.x <= spr_info[i].X1 &&
		        minfo.y >= spr_info[i].Y && minfo.y <= spr_info[i].Y1) {
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
	spieler_vector[p_nr].Xypos[0] = x;
	spieler_vector[p_nr].Xypos[1] = y;
	spieler_mi[p_nr].XyzStart[0] = x;
	spieler_mi[p_nr].XyzStart[1] = y;
	spieler_vector[p_nr].Count = 0;
	spieler_vector[p_nr].Delay = _G(spieler).DelaySpeed;
	spieler_vector[p_nr].DelayCount = 0;
	calc_zoom(spieler_mi[p_nr].XyzStart[1],
	          _G(spieler).ZoomXy[p_nr][0],
	          _G(spieler).ZoomXy[p_nr][1],
	          &spieler_vector[p_nr]);
	tmp_nr = p_nr;
	if (p_nr >= P_NICHELLE) {
		++tmp_nr;
	}
	ssi[tmp_nr].X = spieler_vector[p_nr].Xypos[0] - _G(spieler).scrollx + spieler_mi[p_nr].HotX;
	ssi[tmp_nr].Y = spieler_vector[p_nr].Xypos[1] - _G(spieler).scrolly;
	if (!flags.ExitMov) {
		if (p_nr == P_CHEWY) {
			u_index = ged->ged_idx(x + spieler_mi[p_nr].HotX, y + spieler_mi[p_nr].HotY,
			                        room->GedXAnz[room_blk.AkAblage],
			                        ged_mem[room_blk.AkAblage]);
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
			spieler_vector[P_CHEWY].Phase = CH_L_STEHEN;
			spieler_vector[P_CHEWY].PhNr = 0;
			spieler_vector[P_CHEWY].PhAnz = chewy_ph_anz[CH_L_STEHEN];
			person_end_phase[P_CHEWY] = P_LEFT;
			break;

		case P_RIGHT:
			spieler_vector[P_CHEWY].Phase = CH_R_STEHEN;
			spieler_vector[P_CHEWY].PhNr = 0;
			spieler_vector[P_CHEWY].PhAnz = chewy_ph_anz[CH_R_STEHEN];
			person_end_phase[P_CHEWY] = P_RIGHT;
			break;

		}
		break;

	case P_HOWARD:
	case P_NICHELLE:
#define HO_L_STEHEN 0
#define HO_R_STEHEN 0
		spieler_vector[p_nr].PhNr = 0;
		spieler_vector[p_nr].PhAnz = 8;
		person_end_phase[p_nr] = P_LEFT;
		switch (nr) {
		case P_LEFT:
			spieler_vector[p_nr].Phase = HO_L_STEHEN;
			spieler_vector[p_nr].PhNr = 0;
			spieler_vector[p_nr].PhAnz = 8;
			person_end_phase[p_nr] = P_LEFT;
			break;

		case P_RIGHT:
			spieler_vector[p_nr].Phase = HO_R_STEHEN;
			spieler_vector[p_nr].PhNr = 0;
			spieler_vector[p_nr].PhAnz = 8;
			person_end_phase[p_nr] = P_RIGHT;
			break;

		}
		break;

	default:
		break;
	}
}

void stop_person(int16 p_nr) {
	mov->stop_auto_go();
	spieler_vector[p_nr].Count = 0;

}

void start_detail_wait(int16 ani_nr, int16 rep, int16 mode) {
	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	det->start_detail(ani_nr, rep, mode);
	while (det->get_ani_status(ani_nr) && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = tmp_maus_links;
}

void start_detail_frame(int16 ani_nr, int16 rep, int16 mode, int16 frame) {
	ani_detail_info *adi;
	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	det->start_detail(ani_nr, rep, mode);
	adi = det->get_ani_detail(ani_nr);
	if (mode == ANI_VOR)
		frame = adi->ani_count + frame;
	else
		frame = adi->ani_count - frame;
	while (det->get_ani_status(ani_nr) && adi->ani_count != frame && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = tmp_maus_links;
}

void wait_detail(int16 det_nr) {
	while (det->get_ani_status(det_nr) && !SHOULD_QUIT)
		set_up_screen(DO_SETUP);
}

void wait_show_screen(int16 frames) {
	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	if (_G(spieler).DelaySpeed > 0)
		frames *= _G(spieler).DelaySpeed;
	while (--frames > 0 && !SHOULD_QUIT) {
		if (flags.AniUserAction)
			get_user_key(NO_SETUP);
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = tmp_maus_links;
}

void start_ani_block(int16 anz, const AniBlock *ab) {
	int16 i;
	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	for (i = 0; i < anz; i++) {
		if (ab[i].Mode == ANI_WAIT)
			start_detail_wait(ab[i].Nr, ab[i].Repeat, ab[i].Dir);
		else
			det->start_detail(ab[i].Nr, ab[i].Repeat, ab[i].Dir);
	}
	_G(maus_links_click) = tmp_maus_links;
}

void start_aad_wait(int16 dia_nr, int16 str_nr) {

	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	talk_start_ani = -1;
	talk_hide_static = -1;
	set_ssi_xy();
	atds->start_aad(dia_nr);
	while (atds->aad_get_status() != -1 && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = tmp_maus_links;
	if (minfo.button)
		flags.main_maus_flag = 1;
	kbinfo.scan_code = Common::KEYCODE_INVALID;
	stop_spz();
}

void start_aad(int16 dia_nr) {
	ailsnd->waitForSpeechToFinish();
	set_ssi_xy();
	atds->start_aad(dia_nr);
}

bool start_ats_wait(int16 txt_nr, int16 txt_mode, int16 col, int16 mode) {
	int16 VocNr;
	int16 vocx;
	bool ret = false;
	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	if (!flags.AtsText) {
		flags.AtsText = true;
		if (txt_nr != -1) {
			if (menu_item != CUR_WALK)
				atds_string_start(30000, 0, 0, AAD_STR_START);
			ret = atds->start_ats(txt_nr, txt_mode, col, mode, &VocNr);
			if (ret) {
				while (atds->ats_get_status() != false && !SHOULD_QUIT)
					set_up_screen(DO_SETUP);
			} else if (VocNr != -1) {
				ret = true;
				mem->file->select_pool_item(speech_handle, VocNr);
				vocx = spieler_vector[P_CHEWY].Xypos[0] - _G(spieler).scrollx + spieler_mi[P_CHEWY].HotX;
				ailsnd->setStereoPos(0, atds->get_stereo_pos(vocx));
				ailsnd->startDbVoc(speech_handle, 0, 63);
				ailsnd->setStereoPos(0, atds->get_stereo_pos(vocx));
				warning("FIXME - unknown constant SMP_PLAYING");

				set_up_screen(DO_SETUP);
			}
			if (menu_item != CUR_WALK)
				atds_string_start(30000, 0, 0, AAD_STR_END);
		}
		flags.AtsText = false;
	}
	if (minfo.button)
		flags.main_maus_flag = 1;
	kbinfo.scan_code = Common::KEYCODE_INVALID;
	_G(maus_links_click) = tmp_maus_links;
	return ret;
}

void aad_wait(int16 str_nr) {
	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	if (str_nr == -1) {
		while (atds->aad_get_status() != -1 && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}
	} else {
		while (atds->aad_get_status() < str_nr && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}
	}
	_G(maus_links_click) = tmp_maus_links;
	if (minfo.button)
		flags.main_maus_flag = 1;
	kbinfo.scan_code = Common::KEYCODE_INVALID;
}

void start_aad(int16 dia_nr, int16 ssi_nr) {
	switch (ssi_nr) {
	case 0:
		ssi[0].X = spieler_vector[P_CHEWY].Xypos[0] - _G(spieler).scrollx + spieler_mi[P_CHEWY].HotX;
		ssi[0].Y = spieler_vector[P_CHEWY].Xypos[1] - _G(spieler).scrolly;
		atds->set_split_win(0, &ssi[0]);
		break;

	default:
		break;
	}
	atds->start_aad(dia_nr);
}

void start_ads_wait(int16 dia_nr) {
	if (!flags.AdsDialog) {
		menu_item = CUR_TALK;
		cursor_wahl(menu_item);
		load_ads_dia(dia_nr);
		while (flags.AdsDialog && !SHOULD_QUIT) {
			set_up_screen(DO_SETUP);
		}
	}
}

void wait_auto_obj(int16 nr) {
	tmp_maus_links = _G(maus_links_click);
	_G(maus_links_click) = false;
	while (mov_phasen[nr].Repeat != -1 && !SHOULD_QUIT) {
		set_up_screen(DO_SETUP);
	}
	_G(maus_links_click) = tmp_maus_links;
}

void stop_auto_obj(int16 nr) {
	if (nr < _G(auto_obj)) {
		mov_phasen[nr].Start = false;
	}
}

void continue_auto_obj(int16 nr, int16 repeat) {
	if (nr < _G(auto_obj)) {
		mov_phasen[nr].Start = 1;
		if (repeat)
			mov_phasen[nr].Repeat = repeat;
	}
}

void init_auto_obj(int16 auto_nr, const int16 *phasen, int16 lines, const MovLine *mline) {
	int16 i;
	int16 *tmp;
	MovLine *tmp1;
	const MovLine *tmp2;
	tmp2 = mline;
	tmp = (int16 *)mov_phasen[auto_nr].Phase;
	for (i = 0; i < 8; i++)
		tmp[i] = phasen[i];
	mov_phasen[auto_nr].Start = 1;

	tmp1 = mov_line[auto_nr];
	for (i = 0; i < lines; i++) {
		tmp1->EndXyz[0] = tmp2->EndXyz[0];
		tmp1->EndXyz[1] = tmp2->EndXyz[1];
		tmp1->EndXyz[2] = tmp2->EndXyz[2];
		tmp1->PhNr = tmp2->PhNr;
		tmp1->Vorschub = tmp2->Vorschub;
		++tmp1;
		++tmp2;
	}

	auto_mov_obj[auto_nr].XyzEnd[0] = mline->EndXyz[0];
	auto_mov_obj[auto_nr].XyzEnd[1] = mline->EndXyz[1];
	auto_mov_obj[auto_nr].XyzEnd[2] = mline->EndXyz[2];

	auto_mov_vector[auto_nr].Count = 0;
	auto_mov_vector[auto_nr].StNr = 0;
	auto_mov_vector[auto_nr].DelayCount = 0;
	new_auto_line(auto_nr);
}

void new_auto_line(int16 nr) {

	if (nr < _G(auto_obj)) {
		if (mov_phasen[nr].Repeat != -1 &&
		        mov_phasen[nr].Start) {

			if (auto_mov_vector[nr].StNr < mov_phasen[nr].Lines) {
				++auto_mov_vector[nr].StNr;
			} else {

				auto_mov_vector[nr].StNr = 1;
				if (mov_phasen[nr].Repeat != 255) {
					--mov_phasen[nr].Repeat;
				}
			}
			if (!mov_phasen[nr].Repeat) {
				auto_mov_vector[nr].Xypos[2] = 201;
				mov_phasen[nr].Repeat = -1;
				mov_phasen[nr].Start = 0;
			} else {
				auto_mov_vector[nr].Phase = mov_line[nr][auto_mov_vector[nr].StNr - 1].PhNr;
				auto_mov_vector[nr].PhNr = 0;

				auto_mov_vector[nr].PhAnz = (mov_phasen[nr].Phase[auto_mov_vector[nr].Phase][1] -
				                             mov_phasen[nr].Phase[auto_mov_vector[nr].Phase][0]) + 1;
				auto_mov_obj[nr].Vorschub = mov_line[nr][auto_mov_vector[nr].StNr - 1].Vorschub;
				auto_mov_obj[nr].XyzStart[0] = auto_mov_obj[nr].XyzEnd[0];
				auto_mov_obj[nr].XyzStart[1] = auto_mov_obj[nr].XyzEnd[1];
				auto_mov_obj[nr].XyzStart[2] = auto_mov_obj[nr].XyzEnd[2];
				auto_mov_obj[nr].XyzEnd[0] = mov_line[nr][auto_mov_vector[nr].StNr - 1].EndXyz[0] ;
				auto_mov_obj[nr].XyzEnd[1] = mov_line[nr][auto_mov_vector[nr].StNr - 1].EndXyz[1] ;;
				auto_mov_obj[nr].XyzEnd[2] = mov_line[nr][auto_mov_vector[nr].StNr - 1].EndXyz[2] ;;
				mov->get_mov_vector((int16 *)auto_mov_obj[nr].XyzStart,
				                     auto_mov_obj[nr].Vorschub, &auto_mov_vector[nr]);
			}
		} else {
			auto_mov_vector[nr].Xypos[2] = 201;
		}
	}
}

int16 mouse_auto_obj(int16 nr, int16 xoff, int16 yoff) {
	int16 ret;
	int16 spr_nr;
	int16 *xy;
	int16 *Cxy;
	ret = false;
	if (mov_phasen[nr].Start == 1) {
		spr_nr = mov_phasen[nr].Phase[auto_mov_vector[nr].Phase][0] +
		         auto_mov_vector[nr].PhNr;
		xy = (int16 *)room_blk.DetImage[spr_nr];
		Cxy = room_blk.DetKorrekt + (spr_nr << 1);

		if (!xoff) {
			xoff = xy ? xy[0] : 0;
			xoff += auto_mov_vector[nr].Xzoom;
		}
		if (!yoff) {
			yoff = xy ? xy[1] : 0;
			yoff += auto_mov_vector[nr].Yzoom;
		}
		if (minfo.x >= auto_mov_vector[nr].Xypos[0] + Cxy[0] - _G(spieler).scrollx &&
		        minfo.x <= auto_mov_vector[nr].Xypos[0] + xoff + Cxy[0] - _G(spieler).scrollx &&
		        minfo.y >= auto_mov_vector[nr].Xypos[1] + Cxy[1] - _G(spieler).scrolly &&
		        minfo.y <= auto_mov_vector[nr].Xypos[1] + yoff + Cxy[1] - _G(spieler).scrolly)
			ret = true;
	}
	return ret;
}

int16 auto_obj_status(int16 nr) {
	int16 status;
	status = false;
	if (nr < _G(auto_obj)) {
		if (mov_phasen[nr].Repeat != -1)
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
				if (!(u_index = ged->ged_idx(om->Xypos[0] + mi->HotX + tmpx,
				                              om->Xypos[1] + mi->HotY + tmpy,
				                              room->GedXAnz[room_blk.AkAblage],
				                              ged_mem[room_blk.AkAblage]))) {

					if (!(u_index = ged->ged_idx(om->Xypos[0] + mi->HotX + tmpx,
					                              om->Xypos[1] + mi->HotY,
					                              room->GedXAnz[room_blk.AkAblage],
					                              ged_mem[room_blk.AkAblage]))) {

						if (!(u_index = ged->ged_idx(om->Xypos[0] + mi->HotX,
						                              om->Xypos[1] + mi->HotY + tmpy,
						                              room->GedXAnz[room_blk.AkAblage],
						                              ged_mem[room_blk.AkAblage]))) {
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
								if (!flags.NoEndPosMovObj) {
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
								mov->get_mov_vector(mi->XyzStart, mi->Vorschub, om);
								if (om->Xyvo[1] != 0) {
									new_vector = true;
									swap_if_l(&om->Xyvo[1], &om->Xyvo[0]);
								}
								get_phase(om, mi);
								if (om->Count == 0 && !flags.NoEndPosMovObj) {
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
							if (!flags.NoEndPosMovObj) {
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
							mov->get_mov_vector(mi->XyzStart, mi->Vorschub, om);
							if (om->Xyvo[0] != 0) {
								new_vector = true;
								swap_if_l(&om->Xyvo[0], &om->Xyvo[1]);

							}
							get_lr_phase(om, mi->Id);
							if (om->Count == 0 && !flags.NoEndPosMovObj) {
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
					if (new_vector) {
						new_vector = false;
						mi->XyzStart[0] = om->Xypos[0];
						mi->XyzStart[1] = om->Xypos[1];
						mov->get_mov_vector(mi->XyzStart, mi->Vorschub, om);
					}
					if (om->Count == 0 && !flags.NoEndPosMovObj) {
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
					u_index = ged->ged_idx(om->Xypos[0] + mi->HotX,
					                        om->Xypos[1] + mi->HotY,
					                        room->GedXAnz[room_blk.AkAblage],
					                        ged_mem[room_blk.AkAblage]);
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
				if (!spz_ani[P_CHEWY])
					calc_person_end_ani(om, P_CHEWY);
				else
					calc_person_spz_ani(om);
				break;

			case HOWARD_OBJ:
				if (!spz_ani[P_HOWARD])
					calc_person_end_ani(om, P_HOWARD);
				else
					calc_person_spz_ani(om);
				break;

			case NICHELLE_OBJ:
				if (!spz_ani[P_NICHELLE])
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
	if (ani_stand_count[p_nr] >= 25 * (_G(spieler).DelaySpeed + p_nr * 2)) {
		ani_stand_count[p_nr] = 0;
		om->PhNr = 0;
		ani_stand_flag[p_nr] = true;
		set_person_spr(person_end_phase[p_nr], p_nr);
	} else if (ani_stand_flag[p_nr]) {
		om->Delay = _G(spieler).DelaySpeed + 1;
		if (om->PhNr < om->PhAnz - 1)
			++om->PhNr;
		else
			ani_stand_flag[p_nr] = false;
	} else {
		set_person_spr(person_end_phase[p_nr], p_nr);
		om->PhNr = 0;
		++ani_stand_count[p_nr];
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
						person_end_phase[p_nr] = P_LEFT;
					else
						person_end_phase[p_nr] = P_RIGHT;
				} else {
					om->Phase = CH_RIGHT_NO;
					person_end_phase[p_nr] = P_RIGHT;
				}
			} else {
				if (om->Xyvo[1] < - (mi->Vorschub - 1)) {
					om->Phase = CH_UP;
					if (om->Xypos[0] - _G(spieler).scrollx > SCREEN_WIDTH / 2)
						person_end_phase[p_nr] = P_LEFT;
					else
						person_end_phase[p_nr] = P_RIGHT;
				} else {
					person_end_phase[p_nr] = P_RIGHT;
					om->Phase = CH_RIGHT_NO;
				}
			}
		} else {
			if (om->Xyvo[1] > 0) {

				if (om->Xyvo[1] > (mi->Vorschub - 1)) {
					om->Phase = CH_DOWN;
					if (om->Xypos[0] - _G(spieler).scrollx > SCREEN_WIDTH / 2)
						person_end_phase[p_nr] = P_LEFT;
					else
						person_end_phase[p_nr] = P_RIGHT;
				} else {
					person_end_phase[p_nr] = P_LEFT;
					om->Phase = CH_LEFT_NO;
				}
			} else {
				if (om->Xyvo[1] < -(mi->Vorschub - 1)) {
					om->Phase = CH_UP;
					if (om->Xypos[0] - _G(spieler).scrollx > SCREEN_WIDTH / 2)
						person_end_phase[p_nr] = P_LEFT;
					else
						person_end_phase[p_nr] = P_RIGHT;
				} else {
					person_end_phase[p_nr] = P_LEFT;
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
	if (flags.ZoomMov) {
		if (om->Xzoom < 0 && _G(zoom_mov_fak) > 0) {
			tmp = om->Xzoom / _G(zoom_mov_fak);
			tmp = abs(tmp);
			mi->Vorschub -= tmp;
			if (mi->Vorschub < 2)
				mi->Vorschub = 2;
		}
		if (tmp_vorschub != mi->Vorschub && om->Count) {
			mov->get_mov_vector((int16 *)mi->XyzStart, mi->Vorschub, om);
			get_phase(om, mi);
		}
	}
}

void start_spz_wait(int16 ani_id, int16 count, bool reverse, int16 p_nr) {
	if (start_spz(ani_id, count, reverse, p_nr)) {
		while (spz_count && !SHOULD_QUIT)
			set_up_screen(DO_SETUP);
	}
}

bool start_spz(int16 ani_id, int16 count, bool reverse, int16 p_nr) {
	int16 i;
	int16 spr_start;
	int16 spr_anz;

	bool ret = false;
	if (!flags.SpzAni) {
		flags.SpzAni = true;
		spz_ani[p_nr] = true;
		spz_p_nr = p_nr;
		spr_start = SPZ_ANI_PH[ani_id][0];
		spr_anz = SPZ_ANI_PH[ani_id][1];
		if (person_end_phase[p_nr] == P_RIGHT) {
			ani_id += 100;
			spr_start += CH_SPZ_OFFSET;
		}

		if (ani_id != spz_akt_id) {
			if (spz_tinfo)
				free((char *)spz_tinfo);
			spz_akt_id = ani_id;
			spz_tinfo = mem->taf_seq_adr(spz_taf_handle, spr_start, spr_anz);
			ERROR
		}

		for (i = 0; i < spr_anz; i++) {
			if (!reverse)
				spz_spr_nr[i] = i;
			else
				spz_spr_nr[i] = spr_anz - i - 1;
		}

		spz_start = spr_start;
		spz_delay[p_nr] = SpzDelay;
		spieler_vector[p_nr].Count = 0;
		spieler_vector[p_nr].PhNr = 0;
		spieler_vector[p_nr].PhAnz = spr_anz;
		spieler_vector[p_nr].Delay = _G(spieler).DelaySpeed + spz_delay[p_nr];
		spieler_vector[p_nr].DelayCount = 0;
		spz_count = count;
		flags.MausLinks = true;
		ret = true;
	}
	return ret;
}

void calc_person_spz_ani(ObjMov *om) {
	if (om->PhNr < om->PhAnz - 1)
		++om->PhNr;
	else {
		--spz_count;
		if (spz_count > 0) {
			om->PhNr = 0;
			om->Delay = _G(spieler).DelaySpeed + spz_delay[spz_p_nr];
		} else {
			if (spz_count != 255)
				stop_spz();
		}
	}
}

void stop_spz() {
	if (flags.SpzAni) {
		flags.SpzAni = false;
		flags.MausLinks = false;
		spz_ani[spz_p_nr] = false;
		spieler_vector[spz_p_nr].Count = 0;
		spieler_vector[spz_p_nr].PhNr = 0;
		set_person_spr(person_end_phase[spz_p_nr], spz_p_nr);
		spz_delay[spz_p_nr] = 0;
	}
}

void set_spz_delay(int16 delay) {
	SpzDelay = delay;
}

void load_person_ani(int16 ani_id, int16 p_nr) {
	int16 ani_start;
	short ani_anz;
	if (PersonAni[p_nr] != ani_id) {
		ani_start = SPZ_ANI_PH[ani_id][0];
		ani_anz = SPZ_ANI_PH[ani_id][1];
		PersonAni[p_nr] = ani_id;
		if (PersonTaf[p_nr])
			free((char *)PersonTaf[p_nr]);
		PersonTaf[p_nr] = mem->taf_seq_adr(spz_taf_handle, ani_start, ani_anz);
		ERROR
		spieler_vector[p_nr].PhNr = 0;
		spieler_vector[p_nr].PhAnz = ani_anz;
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
		if (spieler_mi[p_nr].Id != NO_MOV_OBJ) {
			switch (p_nr) {
			case P_CHEWY:
				break;

			case P_HOWARD:
			case P_NICHELLE:
				if (!spz_ani[p_nr]) {
					for (i = 0; i < 8; i++)
						PersonSpr[p_nr][i] = i;

					if (!spieler_vector[p_nr].Count &&
					        _G(auto_p_nr) != p_nr) {
						ani_nr = (int16)p_ani[p_nr - 1][4] + (person_end_phase[p_nr] * 4);

						spieler_vector[p_nr].PhAnz = 5;
						PersonSpr[p_nr][3] = 1;
						PersonSpr[p_nr][4] = 0;
					} else {
						switch (spieler_vector[p_nr].Phase) {
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
