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

#include "engines/chewy/defines.h"
#include "engines/chewy/global.h"

#ifdef ICM
#include "engines/chewy/ch_icm.h"
int16 maus_rect_first;
int16 koordinate[4] = {0};
int16 last_debug_ani;
int16 p_nr = P_CHEWY;
void grab();
void get_inv();
void del_inv();
char no_sav[] = {"Es wurde noch nicht gespeichert\0"
                 "Speichern Ja/Nein ??\0"
                };

void draw_ice_win() {
	int16 x;
	int16 y;
	int16 i;
	int16 str_anz;

	char c[] = {" Inline I.C.M Adventure Engine \0"
	            "F1 ..... Einen neuen Raum anwählen\0"
	            "F2 ..... Raum Info\0"
	            "F3 ..... Zoom Werte eingeben\0"
	            "F4 ..... Gehe zu Auto Mov Punkt\0"
	            "F5 ..... Rechteck festlegen\0"
	            "F6 ..... Auto Move Punkt ermitteln\0"
	            "F7 ..... Scroll Werte anzeigen\0"
	            "F8 ..... Letzte Animation(F7) abspielen\0"
	            "F9 ..... Z Ebene eingeben\0"
	            "F10 ..... Person setzen\0"
	            "F11 ..... NeoGrab\0"
	            "F12 ..... Inventar holen\0"
	            "ALT + F12 Inventar löschen\0"
	           };

	x = 160;
	y = 100;
	str_anz = 14;
	auto_menu(&x, &y, str_anz, 10, c, 0);
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	for (i = 0; i < str_anz; i++)
		out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
}

void grab() {
	set_up_screen(DO_SETUP);
	in ->alter_kb_handler();
	while (getch() != 'a');
	in ->neuer_kb_handler(&kbinfo);
}

void get_room_nr() {
	short tmp;
	int16 abfrage;
	int16 x;
	int16 y;
	int16 i;
	char c[] = {"Bitte Raum Nr eingeben zulässig 0 bis %d\0"
	            "Ihre Wahl : \0"
	           };
	x = 160;
	y = 100;
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	auto_menu(&x, &y, 2, 10, c, 0);
	in ->alter_kb_handler();
	tmp = 0;
	for (i = 0; i < 2; i++)
		out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i), MAX_RAUM);
	abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
	if (abfrage != 27) {
		if (tmp <= MAX_RAUM) {
			flags.IcmEntry = true;
			exit_room(-1);
			spieler.PersonRoomNr[P_CHEWY] = tmp;
			room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
			ERROR
			out->set_palette(pal);

			if (spieler.AkInvent != -1)
				spieler.room_m_obj[spieler.AkInvent].RoomNr = -1;
			obj->sort();

			if (spieler.AkInvent != -1)
				spieler.room_m_obj[spieler.AkInvent].RoomNr = 255;
			auto_obj = 0;
			enter_room(-1);
			spieler_vector[P_CHEWY].Count = 0;
			flags.IcmEntry = false;
		} else
			BELL
		}
	in ->neuer_kb_handler(&kbinfo);
}

void get_zoom_value() {
	int16 tmp;
	int16 abfrage;
	int16 x;
	int16 y;
	int16 i;
	char c[] = {"Bitte Zoom Faktor eingeben \0"
	            "Ihre Wahl : \0"
	           };
	char c1[] = {"Bitte Zoom Horizont eingeben \0"
	             "Ihre Wahl : \0"
	            };
	x = 160;
	y = 100;
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	auto_menu(&x, &y, 2, 10, c, 0);
	in ->alter_kb_handler();
	for (i = 0; i < 2; i++)
		out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
	tmp = room->room_info->ZoomFak;
	abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
	if (abfrage != 27) {
		room->set_zoom(tmp);
	}
	x = 160;
	y = 100;
	auto_menu(&x, &y, 2, 10, c1, 0);
	in ->alter_kb_handler();
	for (i = 0; i < 2; i++)
		out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c1, i));
	tmp = zoom_horizont;
	abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
	if (abfrage != 27) {
		zoom_horizont = tmp;
	}
	in ->neuer_kb_handler(&kbinfo);
}

void room_info() {
	int16 str_anz;
	int16 x, y;
	int16 i;
	int16 value[255];
	char c[] = {" Allgemeine Raum Info \0"
	            "Ged Seiten Anzahl : %d\0"
	            "Auto Objekte : %d\0"
	            "Zoom Horizont : %d\0"
	            "Zoom Faktor : %d\0"
	            "Freie Handles : %d\0"
	           };
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	x = 160;
	y = 100;
	str_anz = 6;
	value[0] = 0;
	value[1] = room->GedInfo[room_blk.AkAblage].Ebenen;
	value[2] = auto_obj;
	value[3] = zoom_horizont;
	value[4] = (int16)room->room_info->ZoomFak;
	value[5] = get_max_handle();
	auto_menu(&x, &y, str_anz, 10, c, 0);
	for (i = 0; i < str_anz; i++)
		out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i), value[i]);
	while (kbinfo.key_code != ESC && kbinfo.key_code != ENTER);
	while (kbinfo.key_code != 0);
}

void plot_auto_mov(int16 mode) {
	int16 i;
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	if (!room->room_info->AutoMovAnz) {
		out->printxy(0, 0, 255, 0, scr_width, "Kein Auto Mov Punkt vorhanden!!\0");

	} else {
		for (i = 0; i < room->room_info->AutoMovAnz; i++) {
			out->box_fill(Rdi->AutoMov[i].X - spieler.scrollx,
			               Rdi->AutoMov[i].Y - spieler.scrolly,
			               Rdi->AutoMov[i].X + 10 - spieler.scrollx,
			               Rdi->AutoMov[i].Y + 13 - spieler.scrolly, 255);
			out->box(Rdi->AutoMov[i].X + 1 - spieler.scrollx,
			          Rdi->AutoMov[i].Y + 1 - spieler.scrolly,
			          Rdi->AutoMov[i].X + 9 - spieler.scrollx,
			          Rdi->AutoMov[i].Y + 11 - spieler.scrolly, 0);
			if (Rdi->AutoMov[i].X + 3 - spieler.scrollx > 0 &&
			        Rdi->AutoMov[i].X + 3 - spieler.scrollx < (319 - 16) &&
			        Rdi->AutoMov[i].Y + 3 - spieler.scrolly > 0 &&
			        Rdi->AutoMov[i].Y + 3 - spieler.scrolly < (199 - 10))
				out->printxy(Rdi->AutoMov[i].X + 3 - spieler.scrollx,
				              Rdi->AutoMov[i].Y + 3 - spieler.scrolly, 0, 300, scr_width, "%d\0", i);
		}
	}
	if (!mode) {
		while (kbinfo.key_code != ESC && kbinfo.key_code != ENTER);
		while (kbinfo.key_code != 0);
	}
}

void go_auto_mov() {
	int16 tmp;
	int16 str_anz;
	int16 i;
	int16 x, y;
	int16 abfrage;
	char c[] = {"Welchen Punkt ansteuern ??\0"
	            "Ihre Wahl : \0"
	           };
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	if (!room->room_info->AutoMovAnz) {
		out->printxy(0, 0, 255, 0, scr_width, "Kein Auto Mov Punkt vorhanden!!\0");
		BELL
	} else {
		str_anz = 2;
		x = 160;
		y = 100;
		auto_menu(&x, &y, str_anz, 10, c, 0);
		in ->alter_kb_handler();
		tmp = 0;
		for (i = 0; i < str_anz; i++)
			out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
		abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]2d\0", &tmp);
		in ->neuer_kb_handler(&kbinfo);
		if (abfrage != 27) {
			if (tmp < room->room_info->AutoMovAnz) {
				auto_move(tmp, P_CHEWY);
			} else {
				out->printxy(0, 0, 255, 0, scr_width, "Nr zu hoch !!\0");
				BELL
				delay(300);
			}
		}
	}
}

void get_rect(char *spr1, int16 x1, int16 y1, char *spr2, int16 x2, int16 y2) {
	int16 ende;
	int16 ok;
	int16 *ScrXy;
	in->rectangle(0, 0, 319, 199);
	maus_rect_first = false;
	ScrXy = (int16 *)ablage[room_blk.AkAblage];
	ende = 0;
	while (!ende) {
		out->setze_zeiger(workptr);
		out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);
		if (spr1 != 0)
			out->sprite_set(spr1, x1 - spieler.scrollx, y1 - spieler.scrolly, 0);
		if (spr2 != 0)
			out->sprite_set(spr2, x2 - spieler.scrollx, y2 - spieler.scrolly, 0);
		calc_mouse_scroll(ScrXy[0], ScrXy[1]);
		switch (in->get_switch_code()) {

		case ESC:
			ende = 1;
			while (kbinfo.key_code != 0);
			break;

		}
		if (minfo.button) {

			if (minfo.button == 1 && maus_rect_first == false) {
				maus_rect_first = true;
				koordinate[0] = minfo.x + spieler.scrollx;
				koordinate[1] = minfo.y + spieler.scrolly;
				ok = 0;
				while (!ok) {
					mouse_aktiv = 0;
					cur_move = 0;
					out->setze_zeiger(workptr);
					calc_mouse_scroll(ScrXy[0], ScrXy[1]);
					out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);
					if (spr1 != 0)
						out->sprite_set(spr1, x1 - spieler.scrollx, y1 - spieler.scrolly, 0);
					if (spr2 != 0)
						out->sprite_set(spr2, x2 - spieler.scrollx, y2 - spieler.scrolly, 0);
					out->printxy(0, 0, 255, 0, 0, "X1 = %4d Y1 = %4d \0", koordinate[0], koordinate[1]);
					out->printxy(0, 10, 255, 0, 0, "X = %d Y = %d ohne scroll Offset\0", minfo.x, minfo.y);
					out->printxy(0, 20, 255, 0, 0, "X2 = %4d Y2 = %4d XOff %3d YOff %3d\0",
					              minfo.x + spieler.scrollx, minfo.y + spieler.scrolly,
					              (minfo.x + spieler.scrollx) - koordinate[0],
					              (minfo.y + spieler.scrolly) - koordinate[1]);
					out->
					box(koordinate[0] - spieler.scrollx, koordinate[1] - spieler.scrolly, minfo.x, minfo.y, 255);
					out->back2screen(workpage);
					if (minfo.button != 1)
						ok = 1;
				}
			}
		}
		maus_rect_first = false;
		out->printxy(0, 0, 255, 0, 0, "X = %d Y = %d mit scroll
		              Offset\0", minfo.x + spieler.scrollx, minfo.y + spieler.scrolly);
		              out->printxy(0, 10, 255, 0, 0, "X = %d Y = %d ohne scroll Offset\0", minfo.x, minfo.y);
		              plot_maus();
		              out->setze_zeiger(screen0);
		              out->back2screen(workpage);
	}
}

void plot_maus() {
	mouse_aktiv = 0;
	cur_move = 0;
	out->linie(0, minfo.y, 320, minfo.y, 255);
	out->linie(minfo.x, 0, minfo.x, 205, 255);
}

void get_auto_mov() {
	ani_detail_info *adi;
	taf_info *dptr;
	int16 x, y;
	int16 str_anz, str_anz1, str_anz2;
	int16 tmp;
	int16 i;
	int16 ende;
	int16 abfrage;
	int16 *Cxy;
	int16 Dx, Dy;
	int16 det_nr;
	int16 auto_nr;
	int16 info_flag;
	short detail_nr;
	int16 save_flag;
	int16 ani_wahl;
	int16 *ScrXy;

	ObjMov detmov;
	int16 dy;
	char c[] = {"Detail Nummer eingeben\0"
	            "Werte zwischen 0 - 31 sind zulässig\0"
	            "Ihre Wahl : \0"
	           };
	char c1[] = {" HELP AUTO MOV PUNKT\0"
	             "PLUS + MINUS Detail weiterblättern\0"
	             "TAB Chewy Flip/Flop\0"
	             "R Rechteck aufziehen\0"
	             "PAGE_UP/DOWN AutoMov Umschalten\0"
	             "ENTER AutoMov festsetzen\0"
	             "Alt+S AutoMov speichern\0"
	             "N AutoMov neuanlegen\0"
	             "I Info AN/AUS\0"
	             "F2 gehezu AutoMov Punkt\0"
	             "F3 neues Detail wählen\0"
	             "F4 End Sprite Nummer eingeben\0"
	            };
	char c2[] = {"End-Sprite Nummer eingeben wenn Held\0"
	             "Punkt erreicht hat \0"
	             "Ihre Wahl : \0"
	            };
	save_flag = true;
	Rdi = det->get_room_detail_info();
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	str_anz = 3;
	str_anz1 = 12;
	str_anz2 = 3;
	det_nr = -1;
	detail_nr = -1;
	if (!room->room_info->AutoMovAnz)
		auto_nr = -1;
	else
		auto_nr = 0;
	info_flag = true;

	ende = 0;
	load_org();
	ani_wahl = 0;
	ScrXy = (int16 *)ablage[room_blk.AkAblage];
	while (!ende) {
		out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);
		if (minfo.button == 1) {
			spieler_mi[P_CHEWY].XyzStart[0] = spieler_vector[P_CHEWY].Xypos[0];
			spieler_mi[P_CHEWY].XyzStart[1] = spieler_vector[P_CHEWY].Xypos[1];
			spieler_mi[P_CHEWY].XyzEnd[0] = minfo.x - CH_HOT_MOV_X + spieler.scrollx;
			spieler_mi[P_CHEWY].XyzEnd[1] = minfo.y - CH_HOT_MOV_Y + spieler.scrolly;
			mov->get_mov_vector(spieler_mi[P_CHEWY].XyzStart, spieler_mi[P_CHEWY].Vorschub, &spieler_v
			                    ector[P_CHEWY]);
			get_phase(&spieler_vector[P_CHEWY], &spieler_mi[P_CHEWY]);
			holen
			spieler_vector[P_CHEWY].DelayCount = 0;
		}
		switch (in->get_switch_code()) {
		case ESC :
			ende = 1;
			break;

		case CURSOR_UP:
			spieler_vector[P_CHEWY].Xypos[1] -= 1;
			break;

		case CURSOR_DOWN:
			spieler_vector[P_CHEWY].Xypos[1] += 1;
			break;

		case CURSOR_LEFT:
			spieler_vector[P_CHEWY].Xypos[0] -= 1;
			break;

		case CURSOR_RIGHT:
			spieler_vector[P_CHEWY].Xypos[0] += 1;
			break;

		case ALT+CURSOR_LEFT :
			if (spieler.scrollx > 0)
				--spieler.scrollx;
			break;

		case ALT+CURSOR_RIGHT:
			if ((spieler.scrollx + 320) < ScrXy[0])
				++spieler.scrollx;
			break;

		case ALT+CURSOR_UP :
			if (spieler.scrolly > 0)
				--spieler.scrolly;
			break;

		case ALT+CURSOR_DOWN :
			if ((spieler.scrolly + 200) < ScrXy[1])
				++spieler.scrolly;
			break;

		case PLUS:
			if (ani_wahl != 0) {
				if (det_nr < adi->end_ani)
					++det_nr;
				else
					det_nr = adi->start_ani;
				Cxy = dptr->korrektur + (det_nr << 1);
				Dx = adi->x + Cxy[0];
				Dy = adi->y + Cxy[1];
				while (kbinfo.key_code != 0);
			}
			break;

		case MINUS:
			if (ani_wahl != 0) {
				if (det_nr > adi->start_ani)
					--det_nr;
				else
					det_nr = adi->end_ani;
				Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
				Dx = adi->x + Cxy[0];
				Dy = adi->y + Cxy[1];
				while (kbinfo.key_code != 0);
			}
			break;

		case R_KEY:
			if (det_nr != -1)
				get_rect(dptr->image[det_nr], Dx, Dy,
				         chewy->image[spieler_vector[P_CHEWY].Phase * 8],
				         spieler_vector[P_CHEWY].Xypos[0],
				         spieler_vector[P_CHEWY].Xypos[1]);
			else
				get_rect(0, 0, 0, chewy->image[spieler_vector[P_CHEWY].Phase * 8],
				         spieler_vector[P_CHEWY].Xypos[0],
				         spieler_vector[P_CHEWY].Xypos[1]);
			break;

		case I_KEY:
			info_flag ^= 1;
			while (kbinfo.key_code != 0);
			break;

		case TAB:
			if (person_end_phase[P_CHEWY] == P_LEFT)
				person_end_phase[P_CHEWY] = P_RIGHT;
			else
				person_end_phase[P_CHEWY] = P_LEFT;
			while (kbinfo.key_code != 0);
			break;

		case PAGE_UP:

			if (auto_nr < (int16)room->room_info->AutoMovAnz - 1)
				++auto_nr;
			while (kbinfo.key_code != 0);
			break;

		case PAGE_DOWN:

			if (auto_nr > 0)
				--auto_nr;
			while (kbinfo.key_code != 0);
			break;

		case N_KEY:
			if (room->room_info->AutoMovAnz < MAX_AUTO_MOV) {
				auto_nr = room->room_info->AutoMovAnz;
				++room->room_info->AutoMovAnz;
				Rdi->AutoMov[auto_nr].X =
				    spieler_vector[P_CHEWY].Xypos[0] + CH_HOT_MOV_X + spieler.scrollx;
				Rdi->AutoMov[auto_nr].Y =
				    spieler_vector[P_CHEWY].Xypos[1] + CH_HOT_MOV_Y + spieler.scrolly;
			}
			while (kbinfo.key_code != 0);
			break;

		case S_KEY + ALT:
			save_flag = true;
			det->save_detail(DETAILTEST, spieler.PersonRoomNr[P_CHEWY]);
			out->printxy(0, 190, 0, 255, scr_width, " Save ...\0");
			while (kbinfo.key_code != 0);
			while (kbinfo.key_code == 0);
			while (kbinfo.key_code != 0);
			break;

		case ENTER:
			save_flag = false;
			Rdi->AutoMov[auto_nr].X = spieler_vector[P_CHEWY].Xypos[0] + CH_HOT_MOV_X;
			Rdi->AutoMov[auto_nr].Y = spieler_vector[P_CHEWY].Xypos[1] + CH_HOT_MOV_Y;
		case F4_KEY:
			out->setze_zeiger(0);
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz2, 10, c2, 0);
			in ->alter_kb_handler();
			for (i = 0; i < str_anz2; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c2, i));
			tmp = (int16)Rdi->AutoMov[auto_nr].SprNr;
			abfrage = out->scanxy(x + 72, y + 20, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
			in ->neuer_kb_handler(&kbinfo);
			if (abfrage != 27) {
				save_flag = false;
				Rdi->AutoMov[auto_nr].SprNr = (uint8) tmp;
			}
			break;

		case F1_KEY:
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz1, 10, c1, 0);
			for (i = 0; i < str_anz1; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c1, i));
			out->back2screen(workpage);
			while (kbinfo.key_code != 0);
			while (kbinfo.key_code == 0);
			while (kbinfo.key_code != 0);
			out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);
			break;

		case F2_KEY:
			out->setze_zeiger(0);
			go_auto_mov();
			break;

		case F3_KEY:
			out->setze_zeiger(0);
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz, 10, c, 0);
			in ->alter_kb_handler();
			for (i = 0; i < str_anz; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
			tmp = 0;
			abfrage = out->scanxy(x + 72, y + 20, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
			in ->neuer_kb_handler(&kbinfo);
			if (abfrage != 27) {
				ani_wahl = 1;
				adi = det->get_ani_detail(tmp);
				det_nr = adi->start_ani;
				dptr = det->get_taf_info();
				detail_nr = tmp;
				Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
				Dx = adi->x + Cxy[0];
				Dy = adi->y + Cxy[1];
			}
			break;

		}
		calc_mouse_scroll(ScrXy[0], ScrXy[1]);
		set_up_screen(NO_SETUP);
		out->set_fontadr(font6x8);
		out->set_vorschub(fvorx6x8, fvory6x8);
		plot_auto_mov(1);
		if (auto_nr != -1)
			out->box(Rdi->AutoMov[auto_nr].X - 2 - spieler.scrollx,
			          Rdi->AutoMov[auto_nr].Y - 2 - spieler.scrolly,
			          Rdi->AutoMov[auto_nr].X + 12 - spieler.scrollx,
			          Rdi->AutoMov[auto_nr].Y + 14 - spieler.scrolly, 8);
		if (det_nr != -1) {
			if (Adi[det_nr].zoom) {

				dy = adi->y;
				calc_zoom(dy, (int16)room->room_info->ZoomFak,
				          spieler.ZoomXy[P_HOWARD][0],
				          &detmov);
			} else {
				detmov.Xzoom = 0;
				detmov.Yzoom = 0;
			}
			out->scale_set(dptr->image[det_nr], Dx - spieler.scrollx, Dy - spieler.scrolly,
			                detmov.Xzoom, detmov.Yzoom, 0);
		}
		if (info_flag) {
			out->printxy(0, 0, 255, 300, scr_width, "MIT OFFSET x = %3d y =
			              % 3d\0", spieler_vector[P_CHEWY].Xypos[0] + CH_HOT_MOV_X, spieler_vector[P_CHEWY].Xypos[1] + CH
			              _HOT_MOV_Y);
			out->printxy(0, 10, 255, 300, scr_width, "NORMAL x = %3d y =
			              % 3d\0", spieler_vector[P_CHEWY].Xypos[0], spieler_vector[P_CHEWY].Xypos[1]);
			              out->printxy(0, 20, 255, 300, scr_width, "AUTOPUNKT x = %3d y =
			                            % 3d\0", Rdi->AutoMov[auto_nr].X, Rdi->AutoMov[auto_nr].Y);
			                            out->printxy(0, 30, 255, 300, scr_width, "DETAIL NUMMER %d\0", detail_nr);
		}
		              cur->plot_cur();
		out->back2screen(workpage);
	}
	if (save_flag == false) {
		x = 160;
		y = 100;
		str_anz1 = 2;
		out->setze_zeiger(0);
		auto_menu(&x, &y, str_anz1, 10, no_sav, 0);
		for (i = 0; i < str_anz1; i++)
			out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(no_sav, i));
		if (ja_nein() == J_KEY) {
			det->save_detail(DETAILTEST, spieler.PersonRoomNr[P_CHEWY]);
			out->printxy(0, 190, 0, 255, scr_width, " Save ...\0");
		}
	}

}

void get_scroll_val() {
	out->setze_zeiger(0);
	out->printxy(0, 0, 255, 300, scr_width, "SCROLL X = %3d ", spieler.scrollx);
	out->printxy(0, 10, 255, 300, scr_width, "SCROLL Y = %3d ", spieler.scrolly);
	while (in->get_switch_code() != ESC);
}

void play_last_ani(int16 nr) {
}

void set_z_ebene() {
#define ANI_DETAIL 1
#define STATIC_DETAIL 2
#define INVENT_DETAIL 3
	static_detail_info *sdi;
	ani_detail_info *adi;
	taf_info *dptr;
	int16 ende;
	int16 x, y;
	int16 lx, ly;
	int16 str_anz, str_anz1;
	int16 info_flag;
	int16 i;
	int16 tmp;
	int16 abfrage;
	int16 det_nr;
	int16 detail_nr;
	int16 *Cxy;
	int16 Dx, Dy;
	int16 *xy;
	int16 detail_flag;
	int16 plot_flag;
	short save_flag;
	int16 *ScrXy;
	int16 dy;
	char *image;
	ObjMov detmov;
	char c[] = {"Detail Nummer eingeben\0"
	            "Werte zwischen 0 - 32 sind zulässig\0"
	            "Ihre Wahl : \0"
	           };
	char c1[] = {" HELP Z-EBENE\0"
	             "I Info AN/AUS\0"
	             "P Detail nicht zeichnen\0"
	             "TAB Chewy Flip/Flop\0"
	             "CURSOR + ALT Z-Linie verschieben\0"
	             "F2 neues Ani Detail wählen\0"
	             "F3 neues Static Detail wählen\0"
	             "F4 Inventar wählen\0"
	             "PLUS + MINUS Ani Detail weiterblättern\0"
	             "Alt+S Z-Ebenen speichern\0"
	             "ENTER Z-Ebene festsetzen\0"
	            };
	save_flag = true;
	str_anz1 = 11;
	str_anz = 3;
	ende = 0;
	lx = 0;
	ly = 1;
	det_nr = -1;
	detail_nr = -1;
	detail_flag = false;

	info_flag = true;
	plot_flag = true;
	load_org();
	ScrXy = (int16 *)ablage[room_blk.AkAblage];
	while (!ende) {
		if (minfo.button == 1) {
			spieler_mi[P_CHEWY].XyzStart[0] = spieler_vector[P_CHEWY].Xypos[0];
			Bewegungs
			spieler_mi[P_CHEWY].XyzStart[1] = spieler_vector[P_CHEWY].Xypos[1];
			spieler_mi[P_CHEWY].XyzEnd[0] = minfo.x - CH_HOT_MOV_X + spieler.scrollx;
			spieler_mi[P_CHEWY].XyzEnd[1] = minfo.y - CH_HOT_MOV_Y + spieler.scrolly;
			mov->
			get_mov_vector(spieler_mi[P_CHEWY].XyzStart, spieler_mi[P_CHEWY].Vorschub, &spieler_vector
			               [P_CHEWY]);
			get_phase(&spieler_vector[P_CHEWY], &spieler_mi[P_CHEWY]);
			spieler_vector[P_CHEWY].DelayCount = 0;
		}
		switch (in->get_switch_code()) {
		case I_KEY:
			info_flag ^= 1;
			while (kbinfo.key_code != 0);
			break;

		case P_KEY:
			plot_flag ^= 1;
			while (kbinfo.key_code != 0);
			break;

		case ESC :
			ende = 1;
			break;

		case PAGE_UP:
			if (detail_flag == STATIC_DETAIL) {
				if (detail_nr < MAXDETAILS - 1) {
					++detail_nr;
					sdi = &Rdi->Sinfo[detail_nr];
					det_nr = sdi->SprNr;
					if (det_nr != -1) {
						ly = sdi->z_ebene;
						dptr = det->get_taf_info();
						Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
						Dx = sdi->x;
						Dy = sdi->y;
						image = dptr->image[det_nr];
					}
				}
			} else if (detail_flag == ANI_DETAIL) {
				if (detail_nr < MAXDETAILS - 1) {
					++detail_nr;
					adi = &Rdi->Ainfo[detail_nr];
					det_nr = adi->start_ani;
					if (det_nr != -1) {
						ly = adi->z_ebene;
						dptr = det->get_taf_info();
						Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
						Dx = adi->x + Cxy[0];
						Dy = adi->y + Cxy[1];
						image = dptr->image[det_nr];
					}
				}
			} else if (detail_flag == INVENT_DETAIL) {
				if (detail_nr < obj->mov_obj_room[0] - 1) {
					++detail_nr;
					det_nr = obj->mov_obj_room[detail_nr + 1];
					image = inv_spr[det_nr];
					Dx = spieler.room_m_obj[det_nr].X;
					Dy = spieler.room_m_obj[det_nr].Y;
					ly = spieler.room_m_obj[det_nr].ZEbene;
				}
			}
			while (kbinfo.key_code != 0);
			break;

		case PAGE_DOWN:
			if (detail_flag == STATIC_DETAIL) {
				if (detail_nr > 0) {
					--detail_nr;
					sdi = &Rdi->Sinfo[detail_nr];
					det_nr = sdi->SprNr;
					if (det_nr != -1) {
						ly = sdi->z_ebene;
						dptr = det->get_taf_info();
						Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
						Dx = sdi->x;
						Dy = sdi->y;
						image = dptr->image[det_nr];
					}
				}
			} else if (detail_flag == ANI_DETAIL) {
				if (detail_nr > 0) {
					--detail_nr;
					adi = &Rdi->Ainfo[detail_nr];
					det_nr = adi->start_ani;
					if (det_nr != -1) {
						ly = adi->z_ebene;
						dptr = det->get_taf_info();
						Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
						Dx = adi->x + Cxy[0];
						Dy = adi->y + Cxy[1];
						image = dptr->image[det_nr];
					}
				}
			} else if (detail_flag == INVENT_DETAIL) {
				if (detail_nr > 0) {
					--detail_nr;
					det_nr = obj->mov_obj_room[detail_nr + 1];
					image = inv_spr[det_nr];
					Dx = spieler.room_m_obj[det_nr].X;
					Dy = spieler.room_m_obj[det_nr].Y;
					ly = spieler.room_m_obj[det_nr].ZEbene;
				}
			}
			while (kbinfo.key_code != 0);
			break;

		case CURSOR_UP:
			spieler_vector[P_CHEWY].Xypos[1] -= 1;
			break;

		case CURSOR_DOWN:
			spieler_vector[P_CHEWY].Xypos[1] += 1;
			break;

		case CURSOR_LEFT:
			spieler_vector[P_CHEWY].Xypos[0] -= 1;
			break;

		case CURSOR_RIGHT:
			spieler_vector[P_CHEWY].Xypos[0] += 1;
			break;

		case CURSOR_UP + ALT:
			if (ly > 0)
				--ly;
			break;

		case CURSOR_DOWN + ALT:

			++ly;
			break;

		case TAB:
			if (person_end_phase[P_CHEWY] == P_LEFT)
				person_end_phase[P_CHEWY] = P_RIGHT;
			else
				person_end_phase[P_CHEWY] = P_LEFT;
			while (kbinfo.key_code != 0);
			break;

		case PLUS:
			if (detail_flag == ANI_DETAIL) {
				if (det_nr < adi->end_ani)
					++det_nr;
				else
					det_nr = adi->start_ani;
				Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
				Dx = adi->x + Cxy[0];
				Dy = adi->y + Cxy[1];
				image = dptr->image[det_nr];
				if (adi->zoom) {
					dy = adi->y;
					calc_zoom(dy, (int16)room->room_info->ZoomFak,
					          (int16)room->room_info->ZoomFak, &detmov);
				} else {
					detmov.Xzoom = 0;
					detmov.Yzoom = 0;
				}
				while (kbinfo.key_code != 0);
			}
			break;

		case MINUS:
			if (detail_flag == ANI_DETAIL) {
				if (det_nr > adi->start_ani)
					--det_nr;
				else
					det_nr = adi->end_ani;
				Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
				Dx = adi->x + Cxy[0];
				Dy = adi->y + Cxy[1];
				image = dptr->image[det_nr];
				if (adi->zoom) {
					dy = adi->y;
					calc_zoom(dy, (int16)room->room_info->ZoomFak,
					          (int16)room->room_info->ZoomFak, &detmov);
				} else {
					detmov.Xzoom = 0;
					detmov.Yzoom = 0;
				}
				while (kbinfo.key_code != 0);
			}
			break;

		case S_KEY + ALT:
			save_flag = true;
			det->save_detail(DETAILTEST, spieler.PersonRoomNr[P_CHEWY]);

			save_iib(INVENTAR_IIB);
			out->printxy(0, 190, 0, 255, scr_width, " Save ...\0");
			while (kbinfo.key_code != 0);
			while (kbinfo.key_code == 0);
			while (kbinfo.key_code != 0);
			break;

		case ENTER:
			save_flag = false;
			if (detail_flag == ANI_DETAIL) {
				adi->z_ebene = ly;
			} else if (detail_flag == STATIC_DETAIL) {
				sdi->z_ebene = ly;
			} else if (detail_flag == INVENT_DETAIL) {
				spieler.room_m_obj[det_nr].ZEbene = ly;
			}
			break;

		case F1_KEY:
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz1, 10, c1, 0);
			for (i = 0; i < str_anz1; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c1, i));
			out->back2screen(workpage);
			while (kbinfo.key_code != 0);
			while (kbinfo.key_code == 0);
			while (kbinfo.key_code != 0);
			out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);
			break;

		case F2_KEY:
			out->setze_zeiger(0);
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz, 10, c, 0);
			in ->alter_kb_handler();
			for (i = 0; i < str_anz; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
			tmp = 0;
			abfrage = out->scanxy(x + 72, y + 20, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
			in ->neuer_kb_handler(&kbinfo);
			if (abfrage != 27) {
				detail_nr = tmp;
				detail_flag = ANI_DETAIL;
				adi = det->get_ani_detail(tmp);
				det_nr = adi->start_ani;
				if (det_nr != -1) {
					ly = adi->z_ebene;
					dptr = det->get_taf_info();
					Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
					Dx = adi->x + Cxy[0];
					Dy = adi->y + Cxy[1];
					image = dptr->image[det_nr];
					if (adi->zoom) {
						dy = adi->y ;
						calc_zoom(dy, (int16)room->room_info->ZoomFak,
						          (int16)room->room_info->ZoomFak, &detmov);
					} else {
						detmov.Xzoom = 0;
						detmov.Yzoom = 0;
					}
				}
			}
			break;

		case F3_KEY:
			out->setze_zeiger(0);
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz, 10, c, 0);
			in ->alter_kb_handler();
			for (i = 0; i < str_anz; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
			tmp = 0;
			abfrage = out->scanxy(x + 72, y + 20, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
			in ->neuer_kb_handler(&kbinfo);
			if (abfrage != 27) {
				detail_flag = STATIC_DETAIL;
				detail_nr = tmp;
				sdi = &Rdi->Sinfo[tmp];
				det_nr = sdi->SprNr;
				if (det_nr != -1) {
					detmov.Xzoom = 0;
					detmov.Yzoom = 0;
					ly = sdi->z_ebene;
					dptr = det->get_taf_info();
					Cxy = (int16 *)dptr->korrektur + (det_nr << 1);
					Dx = sdi->x;
					Dy = sdi->y;
					image = dptr->image[det_nr];
				}
			}
			break;

		case F4_KEY:
			if (obj->mov_obj_room[0] != 0) {
				detail_flag = INVENT_DETAIL;
				detail_nr = 0;
				det_nr = obj->mov_obj_room[detail_nr + 1];
				image = inv_spr[det_nr];
				Dx = spieler.room_m_obj[det_nr].X;
				Dy = spieler.room_m_obj[det_nr].Y;
				ly = spieler.room_m_obj[det_nr].ZEbene;
			} else {
				out->setze_zeiger(0);
				out->printxy(0, 190, 0, 255, scr_width, " Kein Inventar vorhanden ...\0");
				while (kbinfo.key_code != 0);
			}
			break;

		}
		calc_mouse_scroll(ScrXy[0], ScrXy[1]);
		set_up_screen(NO_SETUP);
		if (det_nr != -1 && plot_flag) {
			out->scale_set(image, Dx - spieler.scrollx, Dy - spieler.scrolly,
			                detmov.Xzoom, detmov.Yzoom, 0);

			xy = (int16 *)image;
			out->
			box(Dx - spieler.scrollx, Dy - spieler.scrolly, Dx + xy[0] - spieler.scrollx, Dy + xy[1] - spieler.scro
			    lly, 255);
		}
		out->set_fontadr(font6x8);
		out->set_vorschub(fvorx6x8, fvory6x8);
		out->linie(lx, ly - spieler.scrolly, lx + 320, ly - spieler.scrolly, 255);
		if (info_flag) {
			out->printxy(0, 0, 255, 300, scr_width, "CHEWY Z_EBENE %d LINIE Z_EBENE
			              % d\0", spieler_vector[P_CHEWY].Xypos[1] + CH_HOT_MOV_Y - abs(spieler_vector[P_CHEWY].Yzoom), l
			              y);
			if (detail_flag == ANI_DETAIL)
				out->printxy(0, 10, 255, 300, scr_width, "ANI DETAIL NUMMER %d\0", detail_nr);
			if (detail_flag == STATIC_DETAIL)
				out->printxy(0, 10, 255, 300, scr_width, "STATIC DETAIL NUMMER %d\0", detail_nr);
			if (detail_flag == INVENT_DETAIL)
				out->printxy(0, 10, 255, 300, scr_width, "INVENTAR NUMMER %d\0", detail_nr);
		}
		cur->plot_cur();
		out->back2screen(workpage);
	}
	if (save_flag == false) {
		x = 160;
		y = 100;
		str_anz1 = 2;
		out->setze_zeiger(0);
		auto_menu(&x, &y, str_anz1, 10, no_sav, 0);
		for (i = 0; i < str_anz1; i++)
			out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(no_sav, i));
		if (ja_nein() == J_KEY) {
			det->save_detail(DETAILTEST, spieler.PersonRoomNr[P_CHEWY]);
			save_iib(INVENTAR_IIB);
			out->printxy(0, 190, 0, 255, scr_width, " Save ...\0");
		}
	}

}

int16 ja_nein() {
	int16 ende;
	int16 ret;
	ende = 0;
	while (in->get_switch_code());
	while (!ende) {
		switch (in->get_switch_code()) {
		case ESC:
			ret = false;
			ende = 1;
			break;

		case J_KEY:
			ret = kbinfo.scan_code;
			ende = 1;
			break;

		case N_KEY:
			ret = kbinfo.scan_code;
			ende = 1;
			break;

		}
	}
	return (ret);
}

void save_iib(char *fname) {
	FILE *handle;
	modul = 0;
	fcode = 0;
	handle = fopen(fname, "rb+");
	if (handle) {
		if (!fread(&iib_datei_header, sizeof(IibDateiHeader), 1, handle)) {
			fcode = READFEHLER;
			modul = DATEI;
		} else if (!strnicmp(iib_datei_header.Id, "IIB", 3)) {
			fclose(handle);
			handle = fopen(fname, "wb+");
			if (handle) {
				iib_datei_header.Size = (uint32)(sizeof(RoomMovObjekt) * MAX_MOV_OBJ);
				if (!fwrite(&iib_datei_header, sizeof(IibDateiHeader), 1, handle)) {
					modul = DATEI;
					fcode = WRITEFEHLER;
				} else if (!fwrite(spieler.room_m_obj, sizeof(RoomMovObjekt)*MAX_MOV_OBJ, 1, handle)) {
					modul = DATEI;
					fcode = WRITEFEHLER;
				}
			}
		}
		fclose(handle);
	}
}

void load_org() {
	int16 i;
	det->load_rdi(room_blk.DetFile, spieler.PersonRoomNr[P_CHEWY]);
	obj->load(INVENTAR_IIB, &spieler.room_m_obj[0]);
	ERROR
	obj->load(INVENTAR_SIB, &spieler.room_s_obj[0]);
	ERROR
	obj->sort();
	for (i = 0; i < MAX_MOV_OBJ; i++) {
		spieler.InventSlot[i] = -1;
	}
	for (i = 0; i < obj->spieler_invnr[0]; i++)
		spieler.InventSlot[i] = obj->spieler_invnr[i + 1];
	obj->calc_all_static_detail();
}

void calc_mouse_scroll(int16 scrx, int16 scry) {
	if (minfo.x > 319 - (curblk.xsize + 20)) {
		if ((spieler.scrollx + 320) < scrx)
			++spieler.scrollx;
	}
	if (minfo.x < 10) {
		if (spieler.scrollx > 0)
			--spieler.scrollx;
	}
	if (minfo.y > 199 - (curblk.ysize + 10)) {
		if ((spieler.scrolly + 200) < scry)
			++spieler.scrolly;
	}
	if (minfo.y < 10) {
		if (spieler.scrolly > 0)
			--spieler.scrolly;
	}
}

void set_person() {
	int16 ende;
	int16 x, y;
	int16 str_anz;
	int16 tmp;
	int16 info_flag;
	int16 i;
	int16 abfrage;
	void (*TmpSetUpScreenFunc)(void);
	char c[] = {" HELP PERSON setzen\0"
	            "F2 Person wählen\0"
	            "F3 Zoomwerte eingeben\0"
	            "I Info AN/AUS\0"
	           };
	char c1[] = {"Bitte Zoom Faktor eingeben \0"
	             "X Zoom : \0"
	            };
	char c2[] = {"Bitte Zoom Faktor eingeben \0"
	             "Y Zoom : \0"
	            };
	char c3[] = {"Bitte Zoom Horizont eingeben \0"
	             "Ihre Wahl : \0"
	            };
	char c4[] = {"Person Nummer eingeben\0"
	             "Ihre Wahl : \0"
	            };
	str_anz = 4;
	ende = 0;
	info_flag = true;
	TmpSetUpScreenFunc = SetUpScreenFunc;
	SetUpScreenFunc = 0;
	out->set_fontadr(font6x8);
	out->set_vorschub(fvorx6x8, fvory6x8);
	spieler.ZoomXy[P_CHEWY][0] = room->room_info->ZoomFak;
	spieler.ZoomXy[P_CHEWY][1] = room->room_info->ZoomFak;
	while (!ende) {
		if (minfo.button == 1) {
			spieler_mi[p_nr].XyzStart[0] = spieler_vector[p_nr].Xypos[0];
			spieler_mi[p_nr].XyzStart[1] = spieler_vector[p_nr].Xypos[1];
			spieler_mi[p_nr].XyzEnd[0] = minfo.x - CH_HOT_MOV_X + spieler.scrollx;
			spieler_mi[p_nr].XyzEnd[1] = minfo.y - CH_HOT_MOV_Y + spieler.scrolly;
			mov->
			get_mov_vector(spieler_mi[p_nr].XyzStart, spieler_mi[p_nr].Vorschub, &spieler_vector[p_nr]
			              );
			get_phase(&spieler_vector[p_nr], &spieler_mi[p_nr]);
			spieler_vector[p_nr].DelayCount = 0;
		}
		switch (in->get_switch_code()) {
		case I_KEY:
			info_flag ^= 1;
			while (kbinfo.key_code != 0);
			break;

		case F1_KEY:
			x = 160;
			y = 100;
			auto_menu(&x, &y, str_anz, 10, c, 0);
			for (i = 0; i < str_anz; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
			out->back2screen(workpage);
			while (kbinfo.key_code != 0);
			while (kbinfo.key_code == 0);
			while (kbinfo.key_code != 0);
			out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);
			break;

		case F2_KEY:
			out->setze_zeiger(0);
			x = 160;
			y = 100;
			auto_menu(&x, &y, 2, 10, c, 0);
			in ->alter_kb_handler();
			tmp = p_nr;
			for (i = 0; i < 2; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c4, i));
			abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
			if (abfrage != 27) {
				p_nr = tmp;
			}
			in->neuer_kb_handler(&kbinfo);
			break;

		case F3_KEY:
			out->setze_zeiger(0);
			x = 160;
			y = 100;
			auto_menu(&x, &y, 2, 10, c, 0);
			in ->alter_kb_handler();
			if (p_nr == P_CHEWY) {
				for (i = 0; i < 2; i++)
					out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c1, i));
				tmp = room->room_info->ZoomFak;
				abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
				if (abfrage != 27) {
					room->set_zoom(tmp);
					spieler.ZoomXy[P_CHEWY][0] = room->room_info->ZoomFak;
					spieler.ZoomXy[P_CHEWY][1] = room->room_info->ZoomFak;
				}
			} else {
				for (i = 0; i < 2; i++)
					out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c1, i));
				tmp = spieler.ZoomXy[p_nr][0];
				abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
				if (abfrage != 27) {
					spieler.ZoomXy[p_nr][0] = tmp;
				}
				for (i = 0; i < 2; i++)
					out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c2, i));
				tmp = spieler.ZoomXy[p_nr][1];
				abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
				if (abfrage != 27) {
					spieler.ZoomXy[p_nr][1] = tmp;
				}
			}
			x = 160;
			y = 100;
			auto_menu(&x, &y, 2, 10, c1, 0);
			for (i = 0; i < 2; i++)
				out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c3, i));
			tmp = zoom_horizont;
			abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, "%[0-9]3d\0", &tmp);
			if (abfrage != 27) {
				zoom_horizont = tmp;
			}
			in->neuer_kb_handler(&kbinfo);
			break;

		case ESC:
			ende = 1;
			break;

		case CURSOR_UP:
			spieler_vector[p_nr].Xypos[1] -= 1;
			break;

		case CURSOR_DOWN:
			spieler_vector[p_nr].Xypos[1] += 1;
			break;

		case CURSOR_LEFT:
			spieler_vector[p_nr].Xypos[0] -= 1;
			break;

		case CURSOR_RIGHT:
			spieler_vector[p_nr].Xypos[0] += 1;
			break;

		}
		set_up_screen(NO_SETUP);
		out->set_fontadr(font6x8);
		out->set_vorschub(fvorx6x8, fvory6x8);
		if (info_flag) {
			out->printxy(0, 0, 255, 300, scr_width, "Person x %d Person y %d", spieler_vector[p_nr].Xypos[0], spieler_vector[p_nr].Xypos[1]);
			out->printxy(0, 10, 255, 300, scr_width, "Zoomx %d Zoomy %d", spieler.ZoomXy[p_nr][0], spieler.ZoomXy[p_nr][1]);
			out->printxy(0, 20, 255, 300, scr_width, "ZoomHorizont %d", zoom_horizont);
			out->printxy(0, 30, 255, 300, scr_width, "AK-Zoomx %d AK-Zoomy %d", spieler_vector[p_nr].Xzoom, spieler_vector[p_nr].Yzoom);

		}
		cur->plot_cur();
		out->back2screen(workpage);
	}
	SetUpScreenFunc = TmpSetUpScreenFunc;
}

void get_inv() {
	int16 x, y;
	int16 abfrage;
	int16 i;
	int16 tmp;
	char c[] = {" INVENTAR holen n\0"
	            "Ihre Wahl :\0"
	           };
	out->setze_zeiger(0);
	x = 160;
	y = 100;
	auto_menu(&x, &y, 2, 10, c, 0);
	in ->alter_kb_handler();
	tmp = 0;
	for (i = 0; i < 2; i++)
		out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
	abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, " % [0 - 9]3d\0", &tmp);
	if (abfrage != 27)
	{
		invent_2_slot(tmp);
	}
	in->neuer_kb_handler(&kbinfo);
}

void del_inv() {
	int16 x, y;
	int16 abfrage;
	int16 i;
	int16 tmp;
	char c[] = {" INVENTAR löschen n\0"
	            "Ihre Wahl :\0"
	           };
	out->setze_zeiger(0);
	x = 160;
	y = 100;
	auto_menu(&x, &y, 2, 10, c, 0);
	in ->alter_kb_handler();
	tmp = 0;
	for (i = 0; i < 2; i++)
		out->printxy(x, y + i * 10, 14, 300, scr_width, txt->str_pos(c, i));
	abfrage = out->scanxy(x + 72, y + 10, 14, 60, 6, scr_width, " % [0 - 9]3d\0", &tmp);
	if (abfrage != 27)
	{
		del_invent_slot(tmp);
	}
	in->neuer_kb_handler(&kbinfo);
}

#endif
