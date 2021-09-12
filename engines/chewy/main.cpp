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

#define FORBIDDEN_SYMBOL_EXCEPTION_rand

#include "chewy/ngshext.h"
#include "chewy/main.h"
#include "chewy/menus.h"

#define AUSGANG_CHECK_PIX 8
#define BLENDE1 1
#define BLENDE2 2
#define BLENDE3 3
int16 menu_flag;
char cur_no_flag;
bool inv_disp_ok;
int16 txt_aus_click;
int16 txt_nummer;
int16 tmp_menu_item;
int16 cur_ausgang_flag;
int16 room_start_nr;
int16 TmpFrameSpeed;

#if 0
fehler *err;
sblaster *snd;
loudness *music;

#endif

maus *in;
memory *mem;
io_game *iog;
mcga_grafik *out;
cursor *cur;
#ifdef AIL
ailclass *ailsnd;
#endif

int16 modul = 0;
int16 fcode = 0;

ChewyFont *_font6;
ChewyFont *_font8;
FontMgr *_fontMgr;

void r39_ok();
void switch_room(int16 nr);

void game_main() {
	_fontMgr = new FontMgr();

	_font8 = new ChewyFont("TXT/8X8.TFF");
	_font6 = new ChewyFont("TXT/6X8.TFF");
	_font6->setDisplaySize(_font6->getDataWidth() - 2, _font6->getDataHeight());
	_font8->setDeltaX(10);

	room_start_nr = 0;
	standard_init();
	out->cls();
	cursor_wahl(CUR_WALK);
	workptr = workpage + 4l;

	test_menu();
	remove(ADSH_TMP);
	tidy();
	out->rest_palette();
	out->restore_mode();
	}

void alloc_buffers() {
	workpage = (char *)calloc(64004l, 1);
	ERROR
	pal = (char *)calloc(768l, 1);
	ERROR
	cur_back = (char *)calloc(16 * 16 + 4, 1);
	ERROR;
	Ci.VirtScreen = workpage;
	Ci.TempArea = (char *)calloc(64004l, 1);
	ERROR;
	det->set_taf_ani_mem(Ci.TempArea);
	Ci.MusicSlot = (char *)calloc(MUSIC_SLOT_SIZE, 1);
	Ci.MaxMusicSize = MUSIC_SLOT_SIZE;
	ERROR;
	Ci.SoundSlot = (char *)calloc(SOUND_SLOT_SIZE, 1);
	Ci.MaxSoundSize = SOUND_SLOT_SIZE;
	ERROR;
	Ci.Fname = fname;
	SpeechBuf[0] = (char *)calloc(SPEECH_HALF_BUF * 2, 1);
	ERROR
	SpeechBuf[1] = SpeechBuf[0] + SPEECH_HALF_BUF;
}

void free_buffers() {
	int16 i;
	det->del_dptr();
	for (i = 0; i < MAX_PERSON; i++)
		free((char *)PersonTaf[i]);
	free((char *)spz_tinfo);
	free(font6x8);
	free(font8x8);
	free(spblende);
	free((char *)menutaf);
	free((char *)chewy);

	free((char *)curtaf);
	free(SpeechBuf[0]);
	free(Ci.SoundSlot);
	free(Ci.MusicSlot);
	free(Ci.TempArea);
	free(cur_back);
	free(pal);
	free(workpage);
}

void cursor_wahl(int16 nr) {
	int16 ok;
	int16 *xy;
	ok = true;
	if (nr != CUR_USER) {
		curblk.sprite = curtaf->image;
		mouse_hot_x = 0;
		mouse_hot_y = 0;
		curani.delay = (1 + spieler.DelaySpeed) * 5;
	}
	switch (nr) {
	case CUR_WALK:
		curani.ani_anf = 0;
		curani.ani_end = 3;
		break;

	case CUR_NO_WALK:
		curani.ani_anf = 8;
		curani.ani_end = 8;
		break;

	case CUR_USE:
		curani.ani_anf = 4;
		curani.ani_end = 7;
		break;

	case CUR_NO_USE:
		curani.ani_anf = 4;
		curani.ani_end = 4;
		break;

	case CUR_NOPE:
		curani.ani_anf = 9;
		curani.ani_end = 12;
		break;

	case CUR_LOOK:
		curani.ani_anf = 13;
		curani.ani_end = 16;
		break;

	case CUR_NO_LOOK:
		curani.ani_anf = 16;
		curani.ani_end = 16;
		break;

	case CUR_TALK:
		curani.ani_anf = 17;
		curani.ani_end = 20;
		break;

	case CUR_NO_TALK:
		curani.ani_anf = 17;
		curani.ani_end = 17;
		break;

	case CUR_INVENT:
		curani.ani_anf = 21;
		curani.ani_end = 24;
		break;

	case CUR_AK_INVENT:
		curani.ani_anf = spieler.AkInvent;
		curani.ani_end = spieler.AkInvent;
		curblk.sprite = &inv_spr[0];
		spieler.inv_cur = true;
		break;

	case CUR_SAVE:
		curani.ani_anf = 25;
		curani.ani_end = 25;
		break;

	case CUR_AUSGANG_LINKS:
		curani.ani_anf = AUSGANG_LINKS_SPR;
		curani.ani_end = AUSGANG_LINKS_SPR;
		break;

	case CUR_AUSGANG_RECHTS:
		curani.ani_anf = AUSGANG_RECHTS_SPR;
		curani.ani_end = AUSGANG_RECHTS_SPR;
		break;

	case CUR_AUSGANG_OBEN:
		curani.ani_anf = AUSGANG_OBEN_SPR;
		curani.ani_end = AUSGANG_OBEN_SPR;
		break;

	case CUR_AUSGANG_UNTEN:
		curani.ani_anf = AUSGANG_UNTEN_SPR;
		curani.ani_end = AUSGANG_UNTEN_SPR;
		break;

	case CUR_DISK:
		curani.ani_anf = 30;
		curani.ani_end = 30;
		break;

	case CUR_HOWARD:
		curani.ani_anf = 31;
		curani.ani_end = 31;
		break;

	case CUR_NICHELLE:
		curani.ani_anf = 37;
		curani.ani_end = 37;
		break;

	case CUR_ZEIGE:
		curani.ani_anf = 9;
		curani.ani_end = 9;
		break;
	case CUR_USER:

		break;

	default:
		ok = false;
		break;
	}

	if (ok) {
		cur_move = 1;
		cur->set_cur_ani(&curani);
		xy = (int16 *)curblk.sprite[curani.ani_anf];
		spieler.CurBreite = xy[0];
		spieler.CurHoehe = xy[1];
		in->rectangle(0, 0, 320 - xy[0], 210 - xy[1]);
	}
}

void hide_cur() {
	if (!cur_hide_flag) {
		cur_hide_flag = true;
		flags.ShowAtsInvTxt = false;
		cur->hide_cur();
		flags.CursorStatus = false;
	}
}

void show_cur() {
	flags.ShowAtsInvTxt = true;
	cur->show_cur();
	flags.CursorStatus = true;
}

int16 invent_display[4][2] = { {5, 0},
	{265, 0},
	{265, 149},
	{5, 149}
};

void test_menu() {
	int16 ende;
	menu_lauflicht = 0;
	inv_disp_ok = false;
	spieler.inv_cur = false;
	menu_display = false;
	cur_display = true;
	cur->show_cur();
	spieler_vector[P_CHEWY].Phase = 6;
	spieler_vector[P_CHEWY].PhAnz = chewy_ph_anz[spieler_vector[P_CHEWY].Phase];
	set_person_pos(160, 80, P_CHEWY, P_RIGHT);
	spieler_vector[P_CHEWY].Count = 0;
	ende = false;
	kbinfo.scan_code = false;
	flags.main_maus_flag = false;
	tmp_menu_item = false;
	maus_links_click = false;
	spieler.PersonHide[P_CHEWY] = false;
	txt_aus_click = false;
	fx_blende = BLENDE3;
	auto_obj = 0;
	flags.MainInput = true;
	flags.ShowAtsInvTxt = true;
	enter_room(-1);
	uhr->reset_timer(0, 0);
	while (ende == 0)
		ende = main_loop(DO_SETUP);
}

void menu_entry() {
	det->freeze_ani();
	uhr->set_all_status(TIMER_FREEZE);
}

void menu_exit() {
	det->unfreeze_ani();
	uhr->set_all_status(TIMER_UNFREEZE);
	uhr->reset_timer(0, 0);
	FrameSpeed = 0;
}

int16 main_loop(int16 mode) {
	int16 ende;
	int16 ret;
	ende = false;

	maus_action();
	if (flags.MainInput) {
		switch (kbinfo.scan_code) {
		case F1_KEY:
			spieler.inv_cur = false;
			menu_item = CUR_WALK;
			cursor_wahl(menu_item);
			if (menu_display == MENU_EINBLENDEN)
				menu_display = MENU_AUSBLENDEN;
			break;

		case F2_KEY:
			spieler.inv_cur = false;
			menu_item = CUR_USE;
			cursor_wahl(menu_item);
			if (menu_display == MENU_EINBLENDEN)
				menu_display = MENU_AUSBLENDEN;
			break;

		case F3_KEY:
			spieler.inv_cur = false;
			menu_item = CUR_LOOK;
			cursor_wahl(menu_item);
			if (menu_display == MENU_EINBLENDEN)
				menu_display = MENU_AUSBLENDEN;
			break;

		case F4_KEY:
			spieler.inv_cur = false;
			menu_item = CUR_TALK;
			cursor_wahl(menu_item);
			if (menu_display == MENU_EINBLENDEN)
				menu_display = MENU_AUSBLENDEN;
			break;

		case F5_KEY:
		case SPACE:
			tmp_menu_item = menu_item;
			maus_old_x = minfo.x;
			maus_old_y = minfo.y;
			menu_item = CUR_USE;
			menu_entry();
			invent_menu();
			menu_exit();
			menu_flag = MENU_AUSBLENDEN;
			menu_display = 0;
			cur_display = true;
			if (spieler.AkInvent == -1) {
				menu_item = tmp_menu_item;
				cursor_wahl(menu_item);
				spieler.inv_cur = false;
			} else {
				menu_item = CUR_USE;
				cursor_wahl(CUR_AK_INVENT);

				get_display_xy(&spieler.DispZx, &spieler.DispZy, spieler.AkInvent);
			}

			kbinfo.key_code = 0;
			break;

		case F6_KEY:
			flags.SaveMenu = true;

			out->setze_zeiger(screen0);
			out->set_fontadr(font6x8);
			out->set_vorschub(fvorx6x8, fvory6x8);
			cursor_wahl(CUR_SAVE);
			if (file_menue() == 1)
				ende = 1;
			if (spieler.inv_cur && spieler.AkInvent != -1 && menu_item == CUR_USE) {
				cursor_wahl(CUR_AK_INVENT);
			} else
				cursor_wahl(menu_item);
			cur_display = true;
			flags.SaveMenu = false;
			cur->show_cur();
			out->setze_zeiger(workptr);
			break;

		case ESC:
			if (menu_display == 0) {
				menu_entry();
				tmp_menu_item = menu_item;
				maus_old_x = minfo.x;
				maus_old_y = minfo.y;
				menu_display = MENU_EINBLENDEN;
				maus_menu_x = (MAUS_MENU_MAX_X / 5) * (menu_item);
				cur_display = false;
				cur->move(maus_menu_x, 100);
			} else {
				menu_exit();
				menu_item = tmp_menu_item;
				menu_display = MENU_AUSBLENDEN;
				if (spieler.inv_cur && spieler.AkInvent != -1 && menu_item == CUR_USE) {
					cursor_wahl(CUR_AK_INVENT);
				} else
					cursor_wahl(menu_item);
			}
			break;

		case ENTER:
			switch (menu_item) {
			case CUR_INVENT:

				menu_item = CUR_USE;
				menu_entry();
				invent_menu();
				menu_exit();
				menu_flag = MENU_AUSBLENDEN;
				menu_display = 0;
				cur_display = true;
				if (spieler.AkInvent == -1) {
					menu_item = tmp_menu_item;
					cursor_wahl(menu_item);
					spieler.inv_cur = false;
				} else {
					menu_item = CUR_USE;
					cursor_wahl(CUR_AK_INVENT);
					get_display_xy(&spieler.DispZx, &spieler.DispZy, spieler.AkInvent);
				}
				break;

			case CUR_SAVE:
				flags.SaveMenu = true;
				menu_display = MENU_EINBLENDEN;
				cur->move(152, 92);
				minfo.x = 152;
				minfo.y = 92;
				out->set_fontadr(font6x8);
				out->set_vorschub(fvorx6x8, fvory6x8);

				out->setze_zeiger(screen0);
				cursor_wahl(CUR_SAVE);
				ret = file_menue();
				if (ret == IOG_END)
					ende = 1;
				out->setze_zeiger(workptr);

				menu_item = tmp_menu_item;
				menu_display = MENU_AUSBLENDEN;
				if (spieler.inv_cur && spieler.AkInvent != -1 && menu_item == CUR_USE) {
					cursor_wahl(CUR_AK_INVENT);
				} else
					cursor_wahl(tmp_menu_item);
				cur_display = true;

				flags.SaveMenu = false;
				cur->show_cur();
				break;

			default:
				if (menu_display != 0) {
					menu_exit();
					menu_flag = MENU_AUSBLENDEN;
					menu_display = 0;
					cur_display = true;
					cur->move(maus_old_x, maus_old_y);
					minfo.x = maus_old_x;
					minfo.y = maus_old_y;
					spieler.inv_cur = false;
					cursor_wahl(menu_item);
				}
				break;

			}
			break;

		case CURSOR_RIGHT:
		case CURSOR_LEFT:
		case CURSOR_UP:
		case CURSOR_DOWN:
			kb_cur_action(kbinfo.scan_code, 0);
			break;

		case X_KEY + ALT:
			ende = 1;
			break;

		case 41:
			spieler.DispFlag ^= 1;
			break;

		case TAB:
			if (menu_display == 0 && spieler.DispFlag) {
				if (spieler.InvDisp < 3)
					++spieler.InvDisp;
				else
					spieler.InvDisp = 0;
			}
			break;

		}
		if (menu_display == MENU_AUSBLENDEN) {
			menu_exit();
			menu_flag = MENU_AUSBLENDEN;
			menu_display = 0;
			cur_display = true;
			cur->move(maus_old_x, maus_old_y);
			minfo.x = maus_old_x;
			minfo.y = maus_old_y;
		}
	}
	kbinfo.scan_code = 0;
	if (mode == DO_SETUP)
		set_up_screen(DO_SETUP);
#ifdef DEMO
	if (spieler.PersonRoomNr[P_CHEWY] > 24)
		ende = 1;
#endif
	return (ende);
}

void set_up_screen(int16 mode) {
	int16 nr;
	int16 tmp;
	int16 i;
	int16 *ScrXy;
	int16 txt_nr;

	if (flags.InitSound && spieler.SpeechSwitch)
		ailsnd->serve_db_samples();
	uhr->calc_timer();

	if (ani_timer[0].TimeFlag) {
		uhr->reset_timer(0, 0);
		spieler.DelaySpeed = FrameSpeed / spieler.FramesPerSecond;
		spieler_vector[P_CHEWY].Delay = spieler.DelaySpeed + spz_delay[P_CHEWY];
		FrameSpeed = 0;
		det->set_global_delay(spieler.DelaySpeed);
	}
	++FrameSpeed;
	out->setze_zeiger(workptr);
	out->map_spr2screen(ablage[room_blk.AkAblage], spieler.scrollx, spieler.scrolly);

	if (SetUpScreenFunc) {
		if (menu_display == 0) {
			SetUpScreenFunc();
			out->setze_zeiger(workptr);
		}
	}
	for (i = 0; i < MAX_PERSON; i++)
		zoom_mov_anpass(&spieler_vector[i], &spieler_mi[i]);

	sprite_engine();
	if (menu_display == MENU_EINBLENDEN) {

		plot_main_menu();
	} else {
		kb_mov(1);
		det->unfreeze_ani();
		check_mouse_ausgang(minfo.x + spieler.scrollx, minfo.y + spieler.scrolly);

		if (!flags.SaveMenu)
			calc_ani_timer();

		if (spieler.AkInvent != -1 && spieler.DispFlag) {
			build_menu(invent_display[spieler.InvDisp][0],
			           invent_display[spieler.InvDisp][1], 3, 3, 60, 0);
			out->sprite_set(inv_spr[spieler.AkInvent],
			                 invent_display[spieler.InvDisp][0] + 1 + spieler.DispZx,
			                 invent_display[spieler.InvDisp][1] + 1 + spieler.DispZy
			                 , scr_width);
		}

		if (flags.AdsDialog)
			ads_menu();
		if (maus_links_click) {
			if (menu_item == CUR_WALK) {
				if (cur_ausgang_flag) {
					calc_ausgang(minfo.x + spieler.scrollx, minfo.y + spieler.scrolly);
				} else {
					if (!flags.ChewyDontGo) {
						gpkt.Dx = minfo.x - spieler_mi[P_CHEWY].HotMovX +
						          spieler.scrollx + spieler_mi[P_CHEWY].HotX;
						gpkt.Dy = minfo.y - spieler_mi[P_CHEWY].HotMovY +
						          spieler.scrolly + spieler_mi[P_CHEWY].HotY;
						gpkt.Sx = spieler_vector[P_CHEWY].Xypos[0] +
						          spieler_mi[P_CHEWY].HotX;
						gpkt.Sy = spieler_vector[P_CHEWY].Xypos[1] +
						          spieler_mi[P_CHEWY].HotY;
						gpkt.Breite = room->GedXAnz[room_blk.AkAblage];
						gpkt.Hoehe = room->GedYAnz[room_blk.AkAblage];
						gpkt.Mem = ged_mem[room_blk.AkAblage];
						gpkt.Ebenen = room->GedInfo[room_blk.AkAblage].Ebenen;
						gpkt.AkMovEbene = ged_mov_ebene;
						mov->goto_xy(&gpkt);
						spieler_mi[P_CHEWY].XyzStart[0] = spieler_vector[P_CHEWY].Xypos[0];
						spieler_mi[P_CHEWY].XyzStart[1] = spieler_vector[P_CHEWY].Xypos[1];
						spieler_mi[P_CHEWY].XyzEnd[0] = gpkt.Dx - spieler_mi[P_CHEWY].HotX;
						spieler_mi[P_CHEWY].XyzEnd[1] = gpkt.Dy - spieler_mi[P_CHEWY].HotY;
						mov->get_mov_vector((int16
						                     *)spieler_mi[P_CHEWY].XyzStart, spieler_mi[P_CHEWY].Vorschub, &spieler_vector[P_CHEWY]);
						get_phase(&spieler_vector[P_CHEWY], &spieler_mi[P_CHEWY]);
						spieler_vector[P_CHEWY].DelayCount = 0;
						auto_p_nr = P_CHEWY;
					}
				}
			}
		}

		calc_auto_go();

		for (i = 0; i < MAX_PERSON; i++) {
			mov_objekt(&spieler_vector[i], &spieler_mi[i]);
			spieler_mi[i].XyzStart[0] = spieler_vector[i].Xypos[0];
			spieler_mi[i].XyzStart[1] = spieler_vector[i].Xypos[1];
		}

		for (i = 0; i < auto_obj && !flags.StopAutoObj; i++)
			mov_objekt(&auto_mov_vector[i], &auto_mov_obj[i]);

		nr = obj->is_iib_mouse(minfo.x + spieler.scrollx, minfo.y + spieler.scrolly);
		if (nr != -1) {
			txt_nr = obj->iib_txt_nr(nr);
			mous_obj_action(nr, mode, INVENTAR_NORMAL, txt_nr);
		} else {
			tmp = calc_maus_txt(minfo.x, minfo.y, mode);
			if (tmp == -1 || tmp == 255) {

				nr = obj->is_sib_mouse(minfo.x + spieler.scrollx, minfo.y + spieler.scrolly);
				if (nr != -1) {
					txt_nr = obj->sib_txt_nr(nr);
					mous_obj_action(nr, mode, INVENTAR_STATIC, txt_nr);
				} else
					calc_mouse_person(minfo.x, minfo.y);
			}
		}
		if (cur_display == true && mode == DO_SETUP) {
			cur->plot_cur();

			if ((spieler.inv_cur) && (flags.CursorStatus == true))
				out->sprite_set(curtaf->image[pfeil_ani + 32], minfo.x, minfo.y,
				                scr_width);
			if (pfeil_delay == 0) {
				pfeil_delay = spieler.DelaySpeed;
				if (pfeil_ani < 4)
					++pfeil_ani;
				else
					pfeil_ani = 0;
			} else
				--pfeil_delay;
		}
	}

	atds->print_aad(spieler.scrollx, spieler.scrolly);
	atds->print_ats(spieler_vector[P_CHEWY].Xypos[0] + CH_HOT_X,
	                spieler_vector[P_CHEWY].Xypos[1], spieler.scrollx, spieler.scrolly);
	maus_links_click = false;
	menu_flag = false;
	if (mode == DO_SETUP) {
		out->setze_zeiger(0);
		switch (fx_blende) {
		case BLENDE1:
			fx->blende1(workptr, screen0, pal, 150, 0, 0);
			break;

		case BLENDE2:
			fx->blende1(workptr, screen0, pal, 150, 1, 0);
			break;

		case BLENDE3:
			fx->rnd_blende(spblende, workptr, screen0, pal, 0, 10);
			break;

		default:
			out->back2screen(workpage);
			break;

		}
		fx_blende = 0;
	}
	cur_hide_flag = false;
	ScrXy = (int16 *)ablage[room_blk.AkAblage];
	if (!menu_display)
		calc_scroll(spieler_vector[P_CHEWY].Xypos[0] + spieler_mi[P_CHEWY].HotX,
		            spieler_vector[P_CHEWY].Xypos[1] + spieler_mi[P_CHEWY].HotY,
		            ScrXy[0], ScrXy[1],
		            &spieler.scrollx, &spieler.scrolly);
}

void mous_obj_action(int16 nr, int16 mode, int16 txt_mode, int16 txt_nr) {
	int16 x;
	int16 y;
	int16 i;
	int16 anz;
	char *str_adr;
	str_adr = 0;

	if (mode == DO_SETUP) {

		if (txt_nr != -1 && flags.ShowAtsInvTxt) {

			switch (txt_mode) {
			case INVENTAR_NORMAL:
			case INVENTAR_STATIC:
				str_adr = atds->ats_get_txt(txt_nr, TXT_MARK_NAME, &anz, ATS_DATEI);
				break;

			}
			if (str_adr) {
				out->set_fontadr(font8x8);
				out->set_vorschub(fvorx8x8, fvory8x8);
				x = minfo.x;
				y = minfo.y;
				calc_txt_xy(&x, &y, str_adr, anz);
				for (i = 0; i < anz; i++)
					print_shad(x, y + i * 10, 255, 300, 0, scr_width, txt->str_pos(str_adr, i));
			}
		}
	}

	if (maus_links_click) {
		if (menu_item != CUR_USE)
			look_invent_screen(txt_mode, txt_nr);
		else {
			if (spieler.inv_cur) {
				obj_auswerten(nr, txt_mode);
			} else {
				if (txt_mode == INVENTAR_NORMAL) {
					if (!flags.ChAutoMov) {
						maus_links_click = false;
						auto_move(spieler.room_m_obj[nr].AutoMov, P_CHEWY);
						look_invent_screen(txt_mode, txt_nr);

						if (spieler.AkInvent != -1)
							spieler.room_m_obj[spieler.AkInvent].RoomNr = -1;
						if (spieler.room_m_obj[nr].AniFlag == 255) {
							invent_2_slot(nr);
						} else {
							spieler.PersonHide[P_CHEWY] = spieler.room_m_obj[nr].HeldHide;
							play_scene_ani(spieler.room_m_obj[nr].AniFlag, ANI_VOR);
							invent_2_slot(nr);

							spieler.PersonHide[P_CHEWY] = false;
						}

						spieler.inv_cur = false;
						menu_item = CUR_WALK;
						cursor_wahl(menu_item);
						spieler_vector[P_CHEWY].DelayCount = 0;

						if (spieler.AkInvent != -1)
							spieler.room_m_obj[spieler.AkInvent].RoomNr = 255;

					}
				} else if (txt_mode == INVENTAR_STATIC) {
					obj_auswerten(nr, STATIC_USE);
				}
			}
		}
	}
}

void kb_mov(int16 mode) {
	int16 ende;
	ende = 0;
	while (!ende) {
		switch (in->get_switch_code()) {
		case CURSOR_RIGHT:
			if (minfo.x < 320 - spieler.CurBreite)
				minfo.x += 2;
			break;

		case CURSOR_LEFT:
			if (minfo.x > 1)
				minfo.x -= 2;
			break;

		case CURSOR_UP:
			if (minfo.y > 1)
				minfo.y -= 2;
			break;

		case CURSOR_DOWN:
			if (minfo.y < 210 - spieler.CurHoehe)
				minfo.y += 2;
			break;

		default:
			ende = 1;
			break;

		}
		cur->move(minfo.x, minfo.y);
		if (mode)
			ende = 1;
		else
			set_up_screen(DO_SETUP);
	}
}

void kb_cur_action(int16 key, int16 mode) {
	switch (key) {
	case CURSOR_RIGHT:
		if (menu_display == MENU_EINBLENDEN) {
			if (menu_item < 5)
				++menu_item;
			else
				menu_item = 0;
			maus_menu_x = (menu_item) * (MAUS_MENU_MAX_X / 5);
			cur->move(maus_menu_x, 100);
		}
		break;

	case CURSOR_LEFT:
		if (menu_display == MENU_EINBLENDEN) {
			if (menu_item > 0)
				--menu_item;
			else
				menu_item = 5;
			maus_menu_x = (menu_item) * (MAUS_MENU_MAX_X / 5);
			cur->move(maus_menu_x, 100);
		}
		break;

	case CURSOR_UP:
		if (menu_display == MENU_EINBLENDEN) {
			if (spieler.MainMenuY > 1)
				spieler.MainMenuY -= 2;
		}
		break;

	case CURSOR_DOWN:
		if (menu_display == MENU_EINBLENDEN) {
			if (spieler.MainMenuY < 163)
				spieler.MainMenuY += 2;
		}
		break;

	}
}

void maus_action() {
	int16 x, y;

	x = minfo.x;
	y = minfo.y;
	if (x > invent_display[spieler.InvDisp][0] &&
	        x < invent_display[spieler.InvDisp][0] + 48 &&
	        y > invent_display[spieler.InvDisp][1] &&
	        y < invent_display[spieler.InvDisp][1] + 48) {
		if (!spieler.inv_cur && !inv_disp_ok && spieler.AkInvent != -1) {
			cursor_wahl(CUR_USE);
		}
		inv_disp_ok = true;
	} else {
		if (!spieler.inv_cur && inv_disp_ok) {
			cursor_wahl(menu_item);
		}
		inv_disp_ok = false;
	}
	if (atds->aad_get_status() == -1) {
		if (minfo.button || kbinfo.key_code == ESC || kbinfo.key_code == ENTER) {

			if (minfo.button == 2 || kbinfo.key_code == ESC) {
				if (!flags.main_maus_flag) {
					kbinfo.scan_code = ESC;
				}
			} else if (minfo.button == 1 || kbinfo.key_code == ENTER) {
				if (!flags.main_maus_flag) {
					if (menu_display == MENU_EINBLENDEN)
						kbinfo.scan_code = ENTER;
					else if (spieler.AkInvent != -1) {
						if (inv_disp_ok) {
							if (spieler.inv_cur) {
								menu_item = CUR_USE;
								cursor_wahl(menu_item);
								spieler.inv_cur = false;
							} else {
								menu_item = CUR_USE;
								cursor_wahl(CUR_AK_INVENT);
								kbinfo.scan_code = 0;
							}
						} else if (!flags.MausLinks)
							maus_links_click = true;
					} else if (!flags.MausLinks)
						maus_links_click = true;
				}
			}
			flags.main_maus_flag = 1;
		} else
			flags.main_maus_flag = 0;
	}
}

void obj_auswerten(int16 test_nr, int16 mode) {
	int16 ret;
	int16 tmp;
	int16 ani_nr;
	int16 txt_nr;
	int16 sib_ret;
	int16 action_flag;
	tmp = spieler.AkInvent;
	ret = NO_ACTION;
	action_flag = false;
	switch (mode) {
	case INVENTAR_NORMAL:
		ret = obj->action_iib_iib(spieler.AkInvent, test_nr);
		if (ret != NO_ACTION) {
			hide_cur();
			if (flags.InventMenu == false) {
				if (spieler.room_m_obj[spieler.AkInvent].AutoMov != 255) {
					maus_links_click = 0;
					auto_move(spieler.room_m_obj[test_nr].AutoMov, P_CHEWY);
				}
				txt_nr = obj->iib_txt_nr(test_nr);
				look_invent_screen(INVENTAR_NORMAL, txt_nr);
				if (spieler.room_m_obj[test_nr].AniFlag != 255) {
					spieler.PersonHide[P_CHEWY] = spieler.room_m_obj[test_nr].HeldHide;
					play_scene_ani(spieler.room_m_obj[test_nr].AniFlag, ANI_VOR);
					spieler.PersonHide[P_CHEWY] = false;
				}
			}
			show_cur();
		}
		break;

	case INVENTAR_STATIC:
		ret = obj->action_iib_sib(spieler.AkInvent, test_nr);
		if (ret != NO_ACTION) {
			maus_links_click = 0;
			hide_cur();
			if (spieler.room_m_obj[spieler.AkInvent].AutoMov != 255) {
				auto_move(spieler.room_s_obj[test_nr].AutoMov, P_CHEWY);
			}
			txt_nr = obj->sib_txt_nr(test_nr);
			look_invent_screen(INVENTAR_STATIC, txt_nr);
			if (spieler.room_s_obj[test_nr].AniFlag != 255) {
				spieler.PersonHide[P_CHEWY] = spieler.room_s_obj[test_nr].HeldHide;
				tmp = get_ani_richtung((int16)spieler.room_s_obj[test_nr].ZustandAk);
				ani_nr = spieler.room_s_obj[test_nr].AniFlag;

				if (ani_nr >= 150) {
					start_spz_wait(ani_nr - 150, 1, ANI_VOR, P_CHEWY);
					ani_nr = -1;
				} else if (ani_nr >= 100) {
					ani_nr -= 100;
					obj->calc_static_detail(test_nr);
				}
				if (ani_nr != -1)
					play_scene_ani(ani_nr, tmp);
				spieler.PersonHide[P_CHEWY] = false;
			}
			menu_item_vorwahl = CUR_WALK;
			show_cur();
			sib_event_inv(test_nr);
			if (!spieler.inv_cur) {
				menu_item = menu_item_vorwahl;
				cursor_wahl(menu_item);
			}
			obj->calc_all_static_detail();
		}
		break;

	case STATIC_USE:
		ret = obj->calc_static_use(test_nr);
		if (ret == OBJEKT_1) {
			maus_links_click = 0;
			hide_cur();
			if (spieler.room_s_obj[test_nr].AutoMov != 255) {

				auto_move(spieler.room_s_obj[test_nr].AutoMov, P_CHEWY);
			}
			txt_nr = obj->sib_txt_nr(test_nr);
			look_invent_screen(INVENTAR_STATIC, txt_nr);
			if (spieler.room_s_obj[test_nr].AniFlag != 255) {
				spieler.PersonHide[P_CHEWY] = spieler.room_s_obj[test_nr].HeldHide;
				tmp = get_ani_richtung((int16)spieler.room_s_obj[test_nr].ZustandAk);

				ani_nr = spieler.room_s_obj[test_nr].AniFlag;

				if (ani_nr >= 150) {
					start_spz_wait(ani_nr - 150, 1, ANI_VOR, P_CHEWY);
					ani_nr = -1;
				} else if (ani_nr >= 100) {
					ani_nr -= 100;
					obj->calc_static_detail(test_nr);
				}
				if (ani_nr != -1)
					play_scene_ani(ani_nr, tmp);
				spieler.PersonHide[P_CHEWY] = false;
			}

			if (spieler.room_s_obj[test_nr].InvNr != -1) {
				invent_2_slot(spieler.room_s_obj[test_nr].InvNr);
				action_flag = true;
			}
			menu_item_vorwahl = CUR_WALK;
			show_cur();
			sib_ret = sib_event_no_inv(test_nr);

			obj->calc_all_static_detail();

			if (!spieler.inv_cur) {

				if (sib_ret || action_flag) {
					menu_item = menu_item_vorwahl;
					cursor_wahl(menu_item);
				}
			}
		} else if (ret == SIB_GET_INV) {
			maus_links_click = 0;
			hide_cur();
			if (spieler.room_s_obj[test_nr].AutoMov != 255) {
				auto_move(spieler.room_s_obj[test_nr].AutoMov, P_CHEWY);
			}
			txt_nr = obj->sib_txt_nr(test_nr);
			look_invent_screen(INVENTAR_STATIC, txt_nr);
			if (spieler.room_s_obj[test_nr].AniFlag != 255) {
				spieler.PersonHide[P_CHEWY] = spieler.room_s_obj[test_nr].HeldHide;
				tmp = get_ani_richtung((int16)spieler.room_s_obj[test_nr].ZustandAk);

				ani_nr = spieler.room_s_obj[test_nr].AniFlag;

				if (ani_nr >= 150) {
					start_spz_wait(ani_nr - 150, 1, ANI_VOR, P_CHEWY);
					ani_nr = -1;
				} else if (ani_nr >= 100) {
					ani_nr -= 100;
					obj->calc_static_detail(test_nr);
				}
				if (ani_nr != -1) {
					play_scene_ani(ani_nr, tmp);
				}
				spieler.PersonHide[P_CHEWY] = false;
			}

			if (spieler.room_s_obj[test_nr].InvNr != -1)
				invent_2_slot(spieler.room_s_obj[test_nr].InvNr);
			obj->calc_rsi_flip_flop(test_nr);
			menu_item_vorwahl = CUR_WALK;
			show_cur();
			sib_event_no_inv(test_nr);
			obj->calc_all_static_detail();
			if (!spieler.inv_cur) {
				menu_item = menu_item_vorwahl;
				cursor_wahl(menu_item);
			}
		} else if (ret == NO_ACTION) {
			txt_nr = obj->sib_txt_nr(test_nr);
			look_invent_screen(INVENTAR_STATIC, txt_nr);
		}
	}

	switch (ret) {
	case OBJEKT_1:
		if (mode == INVENTAR_NORMAL)
			calc_inv_use_txt(test_nr);
		break;

	case OBJEKT_2:
		if (mode == INVENTAR_NORMAL)
			calc_inv_use_txt(test_nr);
		break;

	case NO_ACTION:
		if (mode == STATIC_USE && flags.StaticUseTxt == true)
			flags.StaticUseTxt = false;
		else {
			if (mode != STATIC_USE) {
				calc_inv_no_use(test_nr, mode);
			}
		}
		break;

	}
}

void swap_if_l(int16 *x1, int16 *x2) {
	int16 x1_s;
	if (abs(*x1) < abs(*x2)) {
		if (*x1 < 0)
			x1_s = 0;
		else
			x1_s = 1;
		*x1 = abs(*x2);
		if (!x1_s)
			*x1 = -*x1;
	}
}

void palcopy(char *dipal, char *sipal, int16 distart,
             int16 sistart, int16 anz) {
	int16 i, j;
	j = sistart;
	for (i = distart; i < distart + anz; i++) {
		dipal[i * 3] = sipal[j * 3];
		dipal[i * 3 + 1] = sipal[j * 3 + 1];
		dipal[i * 3 + 2] = sipal[j * 3 + 2];
		++j;
	}
}

void check_shad(int16 g_idx, int16 mode) {
	static uint8 pal_0[] = {
		0, 0, 0,
		39, 0, 26,
		43, 0, 29,
		51, 42, 29,
		51, 0, 34,
		49, 13, 34,
		55, 0, 37,
		63, 54, 40,
		63, 0, 42,
		63, 30, 42,
		63, 12, 46,
		63, 24, 50
	};
	static uint8 pal_1[] = {
		0, 0, 0,
		34, 0, 21,
		38, 0, 24,
		51, 42, 29,

		46, 0, 29,
		44, 8, 29,
		50, 0, 32,
		63, 54, 40,

		58, 0, 37,
		58, 25, 37,
		58, 7, 41,
		58, 19, 45
	};
	static uint8 pal_2[] = {
		0, 0, 0,
		26, 0, 13,
		30, 0, 16,
		51, 42, 29,

		38, 0, 21,
		36, 0, 21,
		42, 0, 24,
		63, 54, 40,

		50, 0, 29,
		50, 17, 29,
		50, 0, 33,
		50, 11, 37
	};
	static uint8 pal_3[] = {
		0, 0, 0,
		21, 3, 8,
		25, 3, 11,
		51, 42, 29,

		33, 3, 16,
		31, 3, 16,
		37, 3, 19,
		63, 54, 40,

		45, 3, 24,
		45, 12, 24,
		45, 3, 28,
		45, 6, 32
	};

	if (flags.NoShad == false) {
		switch (g_idx) {
		case 4:
			if (mode)
				out->set_teilpalette((char *)pal_0, 1, 11);
			palcopy(pal, (char *)pal_0, 0, 0, 12);
			break;

		case 1:
			if (mode)
				out->set_teilpalette((char *)pal_1, 1, 11);
			palcopy(pal, (char *)pal_1, 0, 0, 12);
			break;

		case 2:
			if (mode)
				out->set_teilpalette((char *)pal_2, 1, 11);
			palcopy(pal, (char *)pal_2, 0, 0, 12);
			break;

		case 3:
			if (mode)
				out->set_teilpalette((char *)pal_3, 1, 11);
			palcopy(pal, (char *)pal_3, 0, 0, 12);
			break;

		}
	}
}

void print_shad(int16 x, int16 y, int16 fcol, int16 bcol, int16 scol, int16 scr_w_, char *txtptr) {
	out->printxy(x + 1, y + 1, scol, bcol, scr_w_, txtptr);
	out->printxy(x, y, fcol, bcol, scr_w_, txtptr);
}

bool auto_move(int16 mov_nr, int16 p_nr) {
	int16 ende;
	bool move_status = false;
	int16 key;
	int16 tmp;
	if (mov_nr < MAX_AUTO_MOV) {
		if (!flags.ChAutoMov) {
			key = false;
			move_status = true;
			flags.ChAutoMov = true;
			auto_p_nr = p_nr;
			tmp = maus_links_click;
			maus_links_click = false;
			gpkt.Dx = Rdi->AutoMov[mov_nr].X -
			          spieler_mi[p_nr].HotMovX + spieler_mi[p_nr].HotX;
			gpkt.Dy = Rdi->AutoMov[mov_nr].Y -
			          spieler_mi[p_nr].HotMovY + spieler_mi[p_nr].HotY;
			gpkt.Sx = spieler_vector[p_nr].Xypos[0] + spieler_mi[p_nr].HotX;
			gpkt.Sy = spieler_vector[p_nr].Xypos[1] + spieler_mi[p_nr].HotY;
			gpkt.Breite = room->GedXAnz[room_blk.AkAblage];
			gpkt.Hoehe = room->GedYAnz[room_blk.AkAblage];
			gpkt.Mem = ged_mem[room_blk.AkAblage];
			gpkt.Ebenen = room->GedInfo[room_blk.AkAblage].Ebenen;
			gpkt.AkMovEbene = ged_mov_ebene;
			mov->goto_xy(&gpkt);
			spieler_mi[p_nr].XyzStart[0] = spieler_vector[p_nr].Xypos[0];
			spieler_mi[p_nr].XyzStart[1] = spieler_vector[p_nr].Xypos[1];
			spieler_mi[p_nr].XyzEnd[0] = gpkt.Dx - spieler_mi[p_nr].HotX;
			spieler_mi[p_nr].XyzEnd[1] = gpkt.Dy - spieler_mi[p_nr].HotY;
			mov->get_mov_vector((int16
			                     *)spieler_mi[p_nr].XyzStart, spieler_mi[p_nr].Vorschub, &spieler_vector[p_nr]);
			get_phase(&spieler_vector[p_nr], &spieler_mi[p_nr]);
			spieler_vector[p_nr].DelayCount = 0;
			if (mov->auto_go_status()) {
				while (mov->auto_go_status()) {
					if (in->get_switch_code() == ESC) {
						if (flags.ExitMov || flags.BreakAMov) {
							key = ESC;
							mov->stop_auto_go();
							move_status = false;
						}
					}
					set_up_screen(DO_SETUP);
				}
			}
			if (flags.ChAutoMov) {
				ende = 0;
				spieler_mi[p_nr].XyzStart[0] = spieler_vector[p_nr].Xypos[0];
				spieler_mi[p_nr].XyzStart[1] = spieler_vector[p_nr].Xypos[1];
				spieler_mi[p_nr].XyzEnd[0] = gpkt.Dx - spieler_mi[p_nr].HotX;
				spieler_mi[p_nr].XyzEnd[1] = gpkt.Dy - spieler_mi[p_nr].HotY;
				mov->get_mov_vector((int16 *)spieler_mi[p_nr].XyzStart, spieler_mi[p_nr].Vorschub, &spieler_vector[p_nr]);
				get_phase(&spieler_vector[p_nr], &spieler_mi[p_nr]);
				while (!ende) {
					if (in->get_switch_code() == ESC || key == ESC) {
						if (flags.ExitMov || flags.BreakAMov) {
							spieler_vector[p_nr].Count = 0;
							move_status = false;
						}
					}
					if (!spieler_vector[p_nr].Count) {
						if (move_status) {
							if (flags.ExitMov == false && flags.ChAutoMov) {

								set_person_pos(spieler_mi[p_nr].XyzEnd[0],
								               spieler_mi[p_nr].XyzEnd[1],
								               p_nr, Rdi->AutoMov[mov_nr].SprNr);
							}
						}
						ende = 1;
					}
					set_up_screen(DO_SETUP);
				}
			}
			auto_p_nr = P_CHEWY;
			maus_links_click = tmp;
			flags.ChAutoMov = false;
		}
	}
	return (move_status);
}

void go_auto_xy(int16 x, int16 y, int16 p_nr, int16 mode) {
	int16 move_status;
	int16 tmp;
	int16 ende;
	if (!flags.ChAutoMovXy) {
		move_status = true;
		flags.ChAutoMovXy = true;
		tmp = maus_links_click;
		maus_links_click = false;
		ende = 0;
		spieler_mi[p_nr].XyzStart[0] = spieler_vector[p_nr].Xypos[0];
		spieler_mi[p_nr].XyzStart[1] = spieler_vector[p_nr].Xypos[1];
		spieler_mi[p_nr].XyzEnd[0] = x;
		spieler_mi[p_nr].XyzEnd[1] = y;
		mov->get_mov_vector((int16
		                     *)spieler_mi[p_nr].XyzStart, spieler_mi[p_nr].Vorschub, &spieler_vector[p_nr]);
		if (spieler_vector[p_nr].Count)
			get_phase(&spieler_vector[p_nr], &spieler_mi[p_nr]);
		if (mode == ANI_WAIT) {
			while (!ende) {
				if (in->get_switch_code() == ESC) {
					if (flags.ExitMov || flags.BreakAMov) {
						spieler_vector[p_nr].Count = 0;
						move_status = false;
					}
				}
				if (!spieler_vector[p_nr].Count) {
					if (move_status) {
						set_person_pos(spieler_mi[p_nr].XyzEnd[0],
						               spieler_mi[p_nr].XyzEnd[1], p_nr, -1);
					}
					ende = 1;
				}
				set_up_screen(DO_SETUP);
			}
		}
		maus_links_click = tmp;
		flags.ChAutoMovXy = false;
	}
}

int16 get_ani_richtung(int16 zustand) {
	int16 ret;
	ret = ANI_VOR;
	switch (zustand) {
	case OBJZU_ZU:
	case OBJZU_VERSCHLOSSEN:
		ret = ANI_RUECK;
		break;

	}
	return (ret);
}

int16 calc_maus_txt(int16 x, int16 y, int16 mode) {
	int16 idx;
	int16 ret;
	int16 anz;
	int16 i;
	int16 txt_nr;
	int16 txt_mode;
	int16 ok;
	bool disp_flag;
	bool action_flag;
	int16 action_ret;
	int16 inv_no_use_mode;
	int16 r_val;
	disp_flag = true;
	action_flag = false;
	ret = -1;
	inv_no_use_mode = -1;
	if (mode == DO_SETUP) {
		if (flags.ShowAtsInvTxt) {
			inv_no_use_mode = AUTO_OBJ;
			txt_nr = calc_mouse_mov_obj(&idx);
			if (txt_nr == -1) {
				idx = det->maus_vector(x + spieler.scrollx, y + spieler.scrolly);
				if (idx != -1) {
					txt_nr = Rdi->mtxt[idx];
					inv_no_use_mode = DETEDIT_REC;
				} else {
					idx = det->mouse_on_detail(minfo.x, minfo.y, spieler.scrollx, spieler.scrolly);
					if (idx != -1) {
						inv_no_use_mode = DETAIL_OBJ;
						txt_nr = 255;
					}
				}
			} else
				inv_no_use_mode = AUTO_OBJ;

			if (txt_nr != -1) {
				ret = txt_nr;
				if (maus_links_click && !flags.MausTxt) {
					ok = true;
					flags.MausTxt = true;
					switch (menu_item) {
					case CUR_LOOK:
						txt_mode = TXT_MARK_LOOK;
						break;

					case CUR_NICHELLE:
					case CUR_HOWARD:
					case CUR_USER:
					case CUR_USE:
						txt_mode = TXT_MARK_USE;
						if (spieler.inv_cur)
							ok = false;
						break;

					case CUR_WALK:
						txt_mode = TXT_MARK_WALK;
						break;

					case CUR_TALK:
						txt_mode = TXT_MARK_TALK;
						break;

					}
					action_ret = 0;
					if (!atds->get_steuer_bit(txt_nr, ATS_AKTIV_BIT, ATS_DATEI)) {
						if (menu_item != CUR_WALK) {
							if (x + spieler.scrollx > spieler_vector[P_CHEWY].Xypos[0])
								set_person_spr(P_RIGHT, P_CHEWY);
							else
								set_person_spr(P_LEFT, P_CHEWY);
						}
					}

					if (atds->get_steuer_bit(txt_nr, ATS_ACTION_BIT, ATS_DATEI)) {
						action_ret = ats_action(txt_nr, txt_mode, ATS_ACTION_VOR);
					}

					if (ok && !atds->get_steuer_bit(txt_nr, ATS_AKTIV_BIT, ATS_DATEI)) {
						if (start_ats_wait(txt_nr, txt_mode, 14, ATS_DATEI))
							disp_flag = false;
					} else {
						ret = -1;
					}

					if (atds->get_steuer_bit(txt_nr, ATS_ACTION_BIT, ATS_DATEI)) {
						action_ret = ats_action(txt_nr, txt_mode, ATS_ACTION_NACH);
						action_flag = true;
					}

					if (!ok && !action_ret) {
						if (inv_no_use_mode != -1 && !atds->get_steuer_bit(txt_nr, ATS_AKTIV_BIT, ATS_DATEI)) {
							action_flag = calc_inv_no_use(idx + (spieler.PersonRoomNr[P_CHEWY] * 100), inv_no_use_mode);
						}
					}

					if (ok && !action_ret && menu_item == CUR_USE && disp_flag) {
						if (!atds->get_steuer_bit(txt_nr, ATS_AKTIV_BIT, ATS_DATEI)) {
							if (menu_item != CUR_WALK) {
								if (x + spieler.scrollx > spieler_vector[P_CHEWY].Xypos[0])
									set_person_spr(P_RIGHT, P_CHEWY);
								else
									set_person_spr(P_LEFT, P_CHEWY);
							}
							r_val = rand() % MAX_RAND_NO_USE;
							action_flag = start_ats_wait(rand_no_use[r_val], TXT_MARK_USE, 14, INV_USE_DEF);
						}
					}
					flags.MausTxt = false;
				} else {
					ret = -1;
				}
				if (disp_flag && !action_flag) {
					char *str_ = atds->ats_get_txt(txt_nr, TXT_MARK_NAME, &anz, ATS_DATEI);
					if (str_ != 0) {
						ret = txt_nr;
						out->set_fontadr(font8x8);
						out->set_vorschub(fvorx8x8, fvory8x8);
						calc_txt_xy(&x, &y, str_, anz);
						for (i = 0; i < anz; i++)
							print_shad(x, y + i * 10, 255, 300, 0, scr_width, txt->str_pos((char *)str_, i));
					}
				}
			}
			else
				ret = -1;
		}
	}
	return (ret);
}

int16 is_mouse_person(int16 x, int16 y) {
	int16 is_person;
	int16 i;
	int16 *xy;
	int16 check;
	is_person = -1;
	if (flags.ShowAtsInvTxt) {

		for (i = 0; i < MAX_PERSON && is_person == -1; i++) {
			if (spieler_mi[i].Id == NO_MOV_OBJ)
				check = false;
			else {
				check = true;;
				if (!spz_ani[i]) {
					switch (i) {
					case P_CHEWY:
						xy = (int16
						      *)chewy->image[chewy_ph[spieler_vector[P_CHEWY].Phase * 8 + spieler_vector[P_CHEWY].PhNr]];
						break;

					case P_HOWARD:
					case P_NICHELLE:
						if (spieler.PersonRoomNr[i] !=
						        spieler.PersonRoomNr[P_CHEWY])
							check = false;
						xy = (int16 *)PersonTaf[i]->image[PersonSpr[i][spieler_vector[i].PhNr]];
						break;

					}
				} else
					xy = (int16 *)spz_tinfo->image[spz_spr_nr[spieler_vector[i].PhNr]];
				if (check) {
					if (x + spieler.scrollx >= spieler_vector[i].Xypos[0] &&
					        x + spieler.scrollx <= spieler_vector[i].Xypos[0] + xy[0] + spieler_vector[i].Xzoom &&
					        y + spieler.scrolly >= spieler_vector[i].Xypos[1] &&
					        y + spieler.scrolly <= spieler_vector[i].Xypos[1] + xy[1] + spieler_vector[i].Yzoom) {
						is_person = i;
					}
				}
			}
		}
	}
	return(is_person);
}

void calc_mouse_person(int16 x, int16 y) {
	int16 txt_nr;
	int16 def_nr;
	int16 dia_nr;
	int16 mode;
	int16 p_nr;
	char ch_txt[MAX_PERSON][9] = {"Chewy", "Howard", "Nichelle"};
	dia_nr = -1;
	if (flags.ShowAtsInvTxt && !flags.InventMenu) {
		p_nr = is_mouse_person(x, y);
		if (p_nr != -1) {
			if (!spieler.PersonHide[p_nr]) {
				out->set_fontadr(font8x8);
				out->set_vorschub(fvorx8x8, fvory8x8);
				char *str_ = ch_txt[p_nr];
				calc_txt_xy(&x, &y, str_, 1);
				print_shad(x, y, 255, 300, 0, scr_width, str_);
				if (maus_links_click == 1) {
					def_nr = -1;
					txt_nr = -1;
					if (!spieler.inv_cur) {
						txt_nr = calc_person_txt(p_nr);
						switch (menu_item) {
						case CUR_LOOK:
							mode = TXT_MARK_LOOK;
							switch (p_nr) {
							case P_CHEWY:
								def_nr = 18;
								break;

							case P_HOWARD:
								def_nr = 21;
								break;

							case P_NICHELLE:
								def_nr = 24;
								break;

							}
							break;

						case CUR_USE:
							mode = TXT_MARK_USE;
							switch (p_nr) {
							case P_CHEWY:
								def_nr = 19;
								break;

							case P_HOWARD:
								def_nr = 22;
								break;

							case P_NICHELLE:
								def_nr = 23;
								break;

							}
							break;

						case CUR_TALK:
							switch (p_nr) {
							case P_HOWARD:
							case P_NICHELLE:
								dia_nr = true;
								calc_person_dia(p_nr);
								break;

							default:
								def_nr = -1;
								txt_nr = -1;
								break;

							}
							break;

						default:
							def_nr = -1;
							txt_nr = -1;
							break;

						}
						if (dia_nr == -1) {
							if (txt_nr != 30000) {
								if (menu_item != CUR_WALK) {
									if (x + spieler.scrollx > spieler_vector[P_CHEWY].Xypos[0])
										set_person_spr(P_RIGHT, P_CHEWY);
									else
										set_person_spr(P_LEFT, P_CHEWY);
								}
								if (!start_ats_wait(txt_nr, mode, 14, ATS_DATEI))
									start_ats_wait(def_nr, TXT_MARK_USE, 14, INV_USE_DEF);
							}
						}
					} else {
						if (menu_item != CUR_WALK) {
							if (x + spieler.scrollx > spieler_vector[P_CHEWY].Xypos[0])
								set_person_spr(P_RIGHT, P_CHEWY);
							else
								set_person_spr(P_LEFT, P_CHEWY);
						}
						calc_inv_no_use(p_nr, SPIELER_OBJ);
					}
				}
			}
		}
	}
}

int16 calc_mouse_mov_obj(int16 *auto_nr) {
	int16 txt_nr;
	int16 i;
	int16 ok;
	txt_nr = -1;
	*auto_nr = -1;
	ok = 0;
	for (i = 0; i < auto_obj && !ok; i++) {
		if (mouse_auto_obj(i, 0, 0)) {
			ok = 1;
			txt_nr = mov_phasen[i].AtsText;
			*auto_nr = i;
		}
	}
	return(txt_nr);
}

void calc_ani_timer() {
	int16 i;
	for (i = room->room_timer.TimerStart;
	        i < room->room_timer.TimerStart + room->room_timer.TimerAnz; i++) {
		if (ani_timer[i].TimeFlag)
			timer_action(i);
	}
}

void get_user_key(int16 mode) {
	flags.StopAutoObj = true;
	maus_action();
	maus_links_click = false;
	if (!inv_disp_ok) {
		switch (in->get_switch_code()) {
		case F5_KEY:
		case SPACE:
		case ESC:
			maus_old_x = minfo.x;
			maus_old_y = minfo.y;

			tmp_menu_item = menu_item;
			menu_item = CUR_USE;
			invent_menu();
			menu_flag = MENU_AUSBLENDEN;
			menu_display = 0;
			cur_display = true;
			if (spieler.AkInvent == -1) {
				menu_item = tmp_menu_item;
				cursor_wahl(menu_item);
				spieler.inv_cur = false;
			} else {
				menu_item = CUR_USE;
				cursor_wahl(CUR_AK_INVENT);

				get_display_xy(&spieler.DispZx, &spieler.DispZy, spieler.AkInvent);
			}

			kbinfo.key_code = 0;
			break;

		}
	}
	flags.StopAutoObj = false;
}

void clear_prog_ani() {
	int16 i;
	for (i = 0; i < MAX_PROG_ANI; i++) {
		spr_info[i].Image = 0;
		spr_info[i].ZEbene = 255;
	}
}

void set_ani_screen() {
	if (flags.AniUserAction)
		get_user_key(NO_SETUP);
	set_up_screen(DO_SETUP);
}

void del_inventar(int16 nr) {
	obj->del_inventar(nr, &room_blk);
	spieler.inv_cur = false;
	menu_item = CUR_WALK;
	spieler.AkInvent = -1;
	cursor_wahl(menu_item);
	del_invent_slot(nr);
	maus_links_click = false;
}

bool is_cur_inventar(int16 nr) {
	int16 ret = false;

	if (spieler.AkInvent == nr && spieler.inv_cur)
		ret = true;

	return(ret);
}

void check_mouse_ausgang(int16 x, int16 y) {
	int16 nr;

	int16 found;
	found = true;
	if (menu_item == CUR_WALK) {
		nr = obj->is_exit(x, y);
		switch (spieler.room_e_obj[nr].Attribut) {
		case AUSGANG_LINKS:
			cur_ausgang_flag = AUSGANG_LINKS;
			cursor_wahl(CUR_AUSGANG_LINKS);
			break;

		case AUSGANG_RECHTS:
			cur_ausgang_flag = AUSGANG_RECHTS;
			cursor_wahl(CUR_AUSGANG_RECHTS);
			break;

		case AUSGANG_OBEN:
			cur_ausgang_flag = AUSGANG_OBEN;
			cursor_wahl(CUR_AUSGANG_OBEN);
			break;

		case AUSGANG_UNTEN:
			cur_ausgang_flag = AUSGANG_UNTEN;
			cursor_wahl(CUR_AUSGANG_UNTEN);
			break;

		default:
			found = false;
			break;

		}
		if (cur_ausgang_flag && !found) {
			cursor_wahl(menu_item);
			cur_ausgang_flag = false;
		}
	}
}

void calc_ausgang(int16 x, int16 y) {
	int16 nr;
	int16 u_idx;
	int16 *ScrXy;
	if (!flags.ExitMov) {
		maus_links_click = false;
		nr = obj->is_exit(x, y);
		if (nr != -1) {
			flags.ExitMov = true;
			if (auto_move(spieler.room_e_obj[nr].AutoMov, P_CHEWY) == true) {
				flags.ShowAtsInvTxt = false;
				menu_item = CUR_DISK;
				cursor_wahl(CUR_DISK);
				set_up_screen(DO_SETUP);
				cur_hide_flag = true;
				exit_room(nr);
				spieler.PersonRoomNr[P_CHEWY] = spieler.room_e_obj[nr].Exit;
				room->load_room(&room_blk, spieler.PersonRoomNr[P_CHEWY], &spieler);
				ERROR
				set_person_pos(Rdi->AutoMov[spieler.room_e_obj[nr].ExitMov].X -
				               spieler_mi[auto_p_nr].HotMovX,
				               Rdi->AutoMov[spieler.room_e_obj[nr].ExitMov].Y - spieler_mi[auto_p_nr].HotMovY
				               , P_CHEWY, -1);
				ScrXy = (int16 *)ablage[room_blk.AkAblage];
				get_scroll_off(spieler_vector[P_CHEWY].Xypos[0] + spieler_mi[P_CHEWY].HotX,
				               spieler_vector[P_CHEWY].Xypos[1] + spieler_mi[P_CHEWY].HotY,
				               ScrXy[0], ScrXy[1],
				               &spieler.scrollx, &spieler.scrolly);

				u_idx = ged->ged_idx(spieler_vector[P_CHEWY].Xypos[0] + spieler_mi[P_CHEWY].HotX,
				                      spieler_vector[P_CHEWY].Xypos[1] + spieler_mi[P_CHEWY].HotY,
				                      room->GedXAnz[room_blk.AkAblage],
				                      ged_mem[room_blk.AkAblage]);
				check_shad(u_idx, 0);
				set_person_spr(Rdi->AutoMov[spieler.room_e_obj[nr].ExitMov].SprNr, P_CHEWY);
				spieler_vector[P_CHEWY].DelayCount = 0;
				fx_blende = BLENDE1;
				auto_obj = 0;
				spieler_vector[P_CHEWY].Xzoom = 0;
				spieler_vector[P_CHEWY].Yzoom = 0;
				flags.ShowAtsInvTxt = true;
				menu_item = CUR_WALK;
				cursor_wahl(menu_item);
				flags.ExitMov = false;
				enter_room(nr);
			}
			flags.ExitMov = false;
			set_up_screen(DO_SETUP);
		}
	}
}

void get_scroll_off(int16 x, int16 y, int16 pic_x, int16 pic_y,
                    int16 *sc_x, int16 *sc_y) {
	if (pic_x == SCREEN_WIDTH)
		*sc_x = 0;
	else {
		if ((pic_x - x) > (SCREEN_WIDTH / 2)) {
			*sc_x = (((pic_x - 1) / SCREEN_WIDTH) - 1) * SCREEN_WIDTH;
			if (!*sc_x && x > (SCREEN_WIDTH / 2)) {

				*sc_x = x - (SCREEN_WIDTH / 2);
			}
		} else {
			*sc_x = pic_x - SCREEN_WIDTH;
		}
	}

	if (pic_y == SCREEN_HEIGHT)
		*sc_y = 0;
	else {
		if ((pic_y - y) > (SCREEN_HEIGHT / 2)) {
			*sc_y = (((pic_y - 1) / SCREEN_HEIGHT) - 1) * SCREEN_HEIGHT;
			if (!*sc_y && y > (SCREEN_HEIGHT / 2))

				*sc_y = y - (SCREEN_HEIGHT / 2);
		} else {
			*sc_y = pic_y - SCREEN_HEIGHT;
		}
	}
}

#define SCROLL_LEFT 120
#define SCROLL_RIGHT SCREEN_WIDTH-SCROLL_LEFT
#define SCROLL_UP 80
#define SCROLL_DOWN SCREEN_HEIGHT-SCROLL_UP

int16 scroll_delay = 0;
void calc_scroll(int16 x, int16 y, int16 pic_x, int16 pic_y,
                 int16 *sc_x, int16 *sc_y) {
	if (!flags.NoScroll) {
		if (!scroll_delay) {

			if ((spieler.ScrollxStep * spieler.DelaySpeed) > CH_X_PIX)
				scroll_delay = CH_X_PIX / spieler.ScrollxStep;

			if (x - *sc_x < SCROLL_LEFT) {
				if ((*sc_x - spieler.ScrollxStep) > 0) {
					*sc_x -= spieler.ScrollxStep;
				}
			} else if (x - *sc_x > SCROLL_RIGHT) {
				if ((*sc_x + spieler.ScrollxStep) < pic_x - SCREEN_WIDTH) {
					*sc_x += spieler.ScrollxStep;
				}
			}

			if (y - *sc_y < SCROLL_UP) {
				if ((*sc_y - spieler.ScrollyStep) > 0) {
					*sc_y -= spieler.ScrollyStep;
				}
			} else if (y - *sc_y > SCROLL_DOWN) {
				if ((*sc_y + spieler.ScrollyStep) < pic_y - SCREEN_HEIGHT) {
					*sc_y += spieler.ScrollyStep;
				}
			}
		} else
			--scroll_delay;
	}
}

void auto_scroll(int16 scrx, int16 scry) {
	int16 ende;
	int16 tmp_maus_click;
	tmp_maus_click = maus_links_click;
	maus_links_click = false;
	spieler.scrollx >>= 1;
	spieler.scrollx <<= 1;
	spieler.scrolly >>= 1;
	spieler.scrolly <<= 1;
	ende = false;
	while (!ende) {
		if (scrx < spieler.scrollx)
			spieler.scrollx -= spieler.ScrollxStep;
		else if (scrx > spieler.scrollx)
			spieler.scrollx += spieler.ScrollxStep;
		if (scry < spieler.scrolly)
			spieler.scrolly -= spieler.ScrollyStep;
		else if (scry > spieler.scrolly)
			spieler.scrolly += spieler.ScrollyStep;
		if (scrx == spieler.scrollx && scry == spieler.scrolly)
			ende = true;
		set_up_screen(DO_SETUP);
	}
	maus_links_click = tmp_maus_click;
}

void disable_timer() {
	uhr->disable_timer();
	FrameSpeedTmp = FrameSpeed;
}

void enable_timer() {
	uhr->enable_timer();
	FrameSpeed = FrameSpeedTmp;
}

void calc_auto_go() {
	int16 x_offset;
	int16 y_offset;
	x_offset =
	    spieler_mi[auto_p_nr].HotMovX + (spieler_mi[auto_p_nr].HotX - spieler_mi[auto_p_nr].HotMovX - 1);
	y_offset =
	    spieler_mi[auto_p_nr].HotMovY + (spieler_mi[auto_p_nr].HotY - spieler_mi[auto_p_nr].HotMovY - 2);
	if (mov->calc_auto_go(spieler_mi[auto_p_nr].XyzStart[0] + x_offset,
	                       spieler_mi[auto_p_nr].XyzStart[1] + y_offset,
	                       &spieler_mi[auto_p_nr].XyzEnd[0],
	                       &spieler_mi[auto_p_nr].XyzEnd[1]) != -1)
	{
		spieler_mi[auto_p_nr].XyzStart[0] = spieler_vector[auto_p_nr].Xypos[0];
		spieler_mi[auto_p_nr].XyzStart[1] = spieler_vector[auto_p_nr].Xypos[1];
		spieler_mi[auto_p_nr].XyzEnd[0] -= x_offset;
		spieler_mi[auto_p_nr].XyzEnd[1] -= y_offset;
		mov->get_mov_vector((int16
		                     *)spieler_mi[auto_p_nr].XyzStart, spieler_mi[auto_p_nr].Vorschub, &spieler_vector[auto_p_nr])
		;
		get_phase(&spieler_vector[auto_p_nr], &spieler_mi[auto_p_nr]);
	}
}

void hide_person() {
	int16 i;
	for (i = 0; i < MAX_PERSON; i++) {

		if (!spieler.PersonHide[i]) {
			spieler.PersonHide[i] = true;
			person_tmp_hide[i] = true;
		} else
			person_tmp_hide[i] = false;
	}
}

void show_person() {
	int16 i;
	for (i = 0; i < MAX_PERSON; i++) {

		if (person_tmp_hide[i])
			spieler.PersonHide[i] = false;
	}
}

void save_person_rnr() {
	int16 i;
	for (i = 0; i < MAX_PERSON; i++)
		person_tmp_room[i] = spieler.PersonRoomNr[i];
	flags.SavePersonRnr = true;
}

void set_person_rnr() {
	int16 i;
	if (flags.SavePersonRnr) {
		for (i = 0; i < MAX_PERSON; i++)
			spieler.PersonRoomNr[i] = person_tmp_room[i];
		flags.SavePersonRnr = false;
	}
}

//FIXME - custom random
#if 0
int16 random(int16 start, int16 end) {
	int16 r_val;
	int16 step;
	int16 anz;
	++end;
	anz = abs(start - end);
	step = 32762 / anz;
	r_val = rand();
	r_val /= step;
	if (start > end)
		r_val += end;
	else
		r_val += start;
	return (r_val);
}
#endif

int16 is_chewy_busy() {
	int16 ret;
	ret = true;
	if (!atds->ats_get_status()) {
		if (atds->aad_get_status() == -1) {
			if (atds->ads_get_status() == -1) {
				if (!mov->auto_go_status()) {
					if (!spieler_vector[P_CHEWY].Count) {
						if (!flags.ExitMov) {
							if (!spz_ani[P_CHEWY]) {
								ret = false;
							}
						}
					}
				}
			}
		}
	}
	return(ret);
}


ChewyFont::ChewyFont(Common::String filename) {
	const uint32 headerFont = MKTAG('T', 'F', 'F', '\0');
	Common::File stream;

	stream.open(filename);

	uint32 header = stream.readUint32BE();

	if (header != headerFont)
		error("Invalid resource - %s", filename.c_str());

	stream.skip(4);	// total memory
	_count = stream.readUint16LE();
	_first = stream.readUint16LE();
	_last = stream.readUint16LE();
	_deltaX = _dataWidth = stream.readUint16LE();
	_dataHeight = stream.readUint16LE();

	_displayWidth = _dataWidth;
	_displayHeight = _dataHeight;

	_fontSurface.create(_dataWidth * _count, _dataHeight, ::Graphics::PixelFormat::createFormatCLUT8());

	byte cur;
	int bitIndex = 7;
	byte *p;

	cur = stream.readByte();

	for (uint n = 0; n < _count; n++) {
		for (uint y = 0; y < _dataHeight; y++) {
			p = (byte *)_fontSurface.getBasePtr(n * _dataWidth, y);

			for (uint x = n * _dataWidth; x < n * _dataWidth + _dataWidth; x++) {
				*p++ = (cur & (1 << bitIndex)) ? 0 : 0xFF;

				bitIndex--;
				if (bitIndex < 0) {
					bitIndex = 7;
					cur = stream.readByte();
				}
			}
		}
	}
}

ChewyFont::~ChewyFont() {
	_fontSurface.free();
}

void ChewyFont::setDisplaySize(uint16 width, uint16 height) {
	_displayWidth = width;
	_displayHeight = height;
}

void ChewyFont::setDeltaX(uint16 deltaX) {
	_deltaX = deltaX;
}

::Graphics::Surface *ChewyFont::getLine(const Common::String &texts) {
	::Graphics::Surface *line = new ::Graphics::Surface();
	line->create(texts.size() * _dataWidth, _dataHeight, ::Graphics::PixelFormat::createFormatCLUT8());

	for (uint i = 0; i < texts.size(); i++) {
		uint x = (texts[i] - _first) * _dataWidth;
		line->copyRectToSurface(_fontSurface, i * _dataWidth, 0, Common::Rect(x, 0, x + _dataWidth, _dataHeight));
	}

	return line;
}

::Graphics::Surface *FontMgr::getLine(const Common::String &texts) {
	return _font->getLine(texts);
}