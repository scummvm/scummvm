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

#include "common/system.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/ani_dat.h"
#include "chewy/rooms/rooms.h"

namespace Chewy {

#define STERNE_ANI 17
#define TUER_ZU_ANI 3

static int16 flic_val1, flic_val2;

void play_scene_ani(int16 nr, int16 mode) {
#define ROOM_1_1 101
#define ROOM_1_2 102
#define ROOM_1_5 105
#define ROOM_2_3 203
#define ROOM_3_1 301
#define ROOM_8_17 817
#define ROOM_9_4 904
#define ROOM_18_20 1820
	int16 r_nr = _G(spieler).PersonRoomNr[P_CHEWY] * 100 + nr;

	switch (r_nr) {
	case ROOM_2_3:
		Room2::electrifyWalkway1();
		break;

	case ROOM_8_17:
		start_aad(100, 0);
		det->start_detail(21, 4, ANI_VOR);
		break;

	case ROOM_18_20:
		del_inventar(_G(spieler).AkInvent);
		break;

	default:
		break;
	}

	start_detail_wait(nr, 1, mode);

	switch (r_nr) {
	case ROOM_1_1:
		Room1::gottenCard();
		break;

	case ROOM_2_3:
		Room2::electrifyWalkway2();
		break;

	case ROOM_3_1:
		Room3::terminal();
		break;

	case ROOM_9_4:
		Room9::gtuer();
		break;

	default:
		break;
	}

	kbinfo.scan_code = Common::KEYCODE_INVALID;
}

void timer_action(int16 t_nr) {
	int16 ani_nr = t_nr - room->room_timer.TimerStart;
	bool default_flag = false;

	if (ailsnd->isSpeechActive())
		return;

#define TIMER(NUM) case NUM: Room##NUM::timer(t_nr, ani_nr)
	switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
	TIMER(0);
	TIMER(11);
	TIMER(12);
	TIMER(14);
	TIMER(17);
	TIMER(18);
	TIMER(21);
	TIMER(22);
	TIMER(40);
	TIMER(48);
	TIMER(49);
	TIMER(50);
	TIMER(51);
	TIMER(56);
	TIMER(68);

	default:
		default_flag = true;
		break;
	}
#undef TIMER

	if (default_flag && flags.AutoAniPlay == false) {
		det->start_detail(room->room_timer.ObjNr[ani_nr], 1, 0);
		uhr->reset_timer(t_nr, 0);
	}

	kbinfo.scan_code = Common::KEYCODE_INVALID;
}

void check_ged_action(int16 index) {
#define KABELABDECKUNG 1
	int16 flag;
	index -= 50;
	index /= 4;

	if (!flags.GedAction) {
		flags.GedAction = true;
		flag = false;

		switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
		case 1:
			switch (index) {
			case 0:
				if (!_G(spieler).R2ElectrocutedBork) {
					if (_G(spieler).AkInvent == KABEL_INV) {
						flag = 1;
						del_inventar(_G(spieler).AkInvent);
					} else if (obj->check_inventar(KABEL_INV)) {
						flag = 1;
						obj->del_obj_use(KABEL_INV);
						del_invent_slot(KABEL_INV);
					}
					if (flag) {
						start_aad_wait(54, -1);
						atds->set_ats_str(8, TXT_MARK_LOOK, 0, ATS_DATEI);
						_G(spieler).room_s_obj[KABELABDECKUNG].ZustandFlipFlop = 2;
						obj->calc_rsi_flip_flop(KABELABDECKUNG);
						obj->calc_all_static_detail();
					}
				}
				break;

			default:
				break;
			}
			break;

		case 2:
			switch (index) {
			case 0:
				det->stop_detail(5);
				if (!_G(spieler).R2ElectrocutedBork) {
					det->start_detail(6, 2, ANI_VOR);
				} else {
					start_ani_block(2, ablock4);
				}
				Room2::jump_out_r1(9);
				break;

			default:
				break;
			}
			break;

		case 7:
			switch (index) {
			case 0:
				if (_G(spieler).R7BorkFlug && _G(spieler).R7ChewyFlug) {
					_G(spieler).PersonHide[P_CHEWY] = true;
					start_detail_wait(20, 1, ANI_VOR);
					det->show_static_spr(10);
					wait_show_screen(20 * _G(spieler).DelaySpeed);
					det->hide_static_spr(10);
					set_person_pos(180, 124, P_CHEWY, P_LEFT);
					_G(spieler).PersonHide[P_CHEWY] = false;
					_G(spieler).R7ChewyFlug = false;
				}
				break;

			default:
				break;
			}
			break;

		case 9:
			switch (index) {
			case 0:
				if (!_G(spieler).R9Surimy)
					Room9::surimy();
				break;

			}
			break;

		case 11:
			switch (index) {
			case 0:
				Room11::chewy_bo_use();
				break;

			default:
				break;
			}
			break;

		case 13:
			switch (index) {
			case 2:
				if (_G(spieler).R12ChewyBork) {
					stop_person(P_CHEWY);
					Room13::talk_bork();
				}
				break;

			default:
				break;
			}
			break;

		case 17:
			switch (index) {
			case 0:
				Room17::door_kommando(0);
				break;

			case 1:
				Room17::door_kommando(1);
				break;

			default:
				break;
			}
			break;

		case 18:
			if (!index) {
				if (!_G(spieler).R18SurimyWurf) {
					stop_person(P_CHEWY);
					auto_move(1, P_CHEWY);
					start_aad(40, 0);
				}
			}
			break;

		case 28:
			if (!index)
				Room28::get_pump();
			break;

		case 37:
			if (!index) {
				Room37::dog_bell();
			} else if (index == 1) {
				if (_G(spieler).R37Kloppe && !_G(spieler).R37Mes) {
					stop_person(P_CHEWY);
					_G(spieler).R37Mes = true;
					start_spz(CH_TALK6, 255, ANI_VOR, P_CHEWY);
					start_aad_wait(142, -1);
				}
			}
			break;

		case 42:
			if (!index)
				Room42::calc_xit();
			break;

		case 45:
		case 46:
			if (!index && flags.ExitMov)
				HowardMov = 1;
			break;

		case 49:
			if (!index)
				Room49::calc_boy();
			break;

		case 50:
			if (!index)
				Room50::calc_treppe();
			break;

		case 52:
			if (index == 1)
				Room52::kaker_platt();
			break;

		case 55:
			if (!index)
				Room55::talk_line();
			break;

		case 94:
			if (!index && !_G(spieler).flags35_10)
				switch_room(93);
			break;

		case 97:
			switch (index) {
			case 50:
				Room97::proc2();
				break;
			case 51:
				Room97::proc3();
				break;
			case 52:
				Room97::proc13();
				break;
			case 53:
				Room97::proc12();
				break;
			case 54:
				Room97::proc4();
				break;
			case 55:
				if (_G(spieler).flags36_20)
					auto_scroll(268, 0);
				break;
			case 56:
				Room97::proc15();
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}

		flags.GedAction = false;
	}

	kbinfo.scan_code = Common::KEYCODE_INVALID;
}

int16 ged_user_func(int16 idx_nr) {
	switch (idx_nr) {
	case 40:
		switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
		case 8:
			if (_G(spieler).R8GTuer)
				idx_nr = 0;
			break;

		case 9:
			if (!_G(spieler).R9Gitter)
				idx_nr = 0;
			break;

		case 16:
			if (!_G(spieler).R16F5Exit)
				idx_nr = 0;
			break;

		case 17:
			if (_G(spieler).R17Location != 1)
				idx_nr = 0;
			break;

		case 21:
			if (!_G(spieler).R21Laser2Weg)
				idx_nr = 0;
			break;

		case 31:
			if (!_G(spieler).R31KlappeZu)
				idx_nr = 0;
			break;

		case 41:
			if (!_G(spieler).R41LolaOk)
				idx_nr = 0;
			break;

		case 52:
			if (!_G(spieler).R52LichtAn)
				idx_nr = 2;
			else
				idx_nr = 4;
			break;

		case 71:
			if (!_G(spieler).flags28_2 || !_G(spieler).flags29_4)
				idx_nr = 0;
			else
				idx_nr = 4;
			break;

		case 76:
			return idx_nr;

		case 84:
			if (!_G(spieler).flags31_1)
				_G(spieler).flags31_4 = true;
			break;

		case 86:
			if (!_G(spieler).flags32_2)
				idx_nr = 0;
			break;

		case 94:
			if (!_G(spieler).flags35_10)
				idx_nr = 0;
			break;

		case 97:
			if (!_G(spieler).flags35_80)
				idx_nr = 0;
			break;

		default:
			break;
		}
		break;

	case 41:
		switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
		case 17:
			if (_G(spieler).R17Location != 2)
				idx_nr = 0;
			break;

		case 21:
			if (!_G(spieler).R21Laser1Weg) {
				idx_nr = 0;
			} else
				idx_nr = 3;
			break;

		case 37:
			if (!_G(spieler).flags37_1)
				idx_nr = 0;
			break;

		case 52:
			if (!_G(spieler).R52TuerAuf)
				idx_nr = 2;
			else
				idx_nr = 4;
			break;

		case 97:
			if (!_G(spieler).flags36_20)
				idx_nr = 0;
			break;

		default:
			break;
		}
		break;

	case 42:
		if (_G(spieler).PersonRoomNr[P_CHEWY] == 97) {

		}
		break;

	default:
		break;
	}

	return idx_nr;
}

void enter_room(int16 eib_nr) {
	int16 i;

	if ((!modul) && (flags.InitSound))
		load_room_music(_G(spieler).PersonRoomNr[P_CHEWY]);
	ERROR
	load_chewy_taf(_G(spieler).ChewyAni);
	atds->stop_aad();
	atds->stop_ats();
	_G(spieler).DiaAMov = -1;
	_G(zoom_mov_fak) = 1;

	for (i = 0; i < MAX_PERSON; i++) {
		spieler_mi[i].Vorschub = 8;
		_G(spieler).ZoomXy[i][0] = 0;
		_G(spieler).ZoomXy[i][1] = 0;
	}

	flags.ZoomMov = false;
	_G(spieler).ScrollxStep = 1;
	_G(spieler).ZoomXy[P_CHEWY][ 0] = (int16)room->room_info->ZoomFak;
	_G(spieler).ZoomXy[P_CHEWY][1] = (int16)room->room_info->ZoomFak;

	uhr->reset_timer(0, 0);
	flags.AutoAniPlay = false;
	SetUpScreenFunc = nullptr;
	HowardMov = false;
	_G(cur_hide_flag) = false;

#define ENTRY(NUM) case NUM: Room##NUM::entry(); break
#define ENTRY_NR(NUM) case NUM: Room##NUM::entry(eib_nr); break

	switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
	ENTRY(0);
	ENTRY(2);
	ENTRY(3);
	ENTRY(5);
	ENTRY(6);
	ENTRY(7);
	ENTRY(8);
	ENTRY(9);
	ENTRY(10);
	ENTRY(11);
	ENTRY(12);
	ENTRY(13);
	ENTRY(14);
	ENTRY(16);

	case 17:
		Room17::entry();
		if (_G(spieler).SoundSwitch) {
			if (!_G(spieler).R17EnergieOut)
				det->play_sound(15, 0);
		}
		break;

	ENTRY(18);
	ENTRY(19);
	ENTRY(21);
	ENTRY(22);
	ENTRY(23);

	case 24:
		Room24::entry();
		if (_G(spieler).SoundSwitch)
			det->play_sound(17, 0);
		break;

	ENTRY(25);
	ENTRY(26);
	ENTRY(27);
	ENTRY_NR(28);
	ENTRY(29);
	ENTRY(31);
	ENTRY(32);
	ENTRY(33);
	ENTRY(34);
	ENTRY(35);
	ENTRY(37);
	ENTRY(39);
	ENTRY_NR(40);
	ENTRY(41);
	ENTRY(42);
	ENTRY_NR(45);
	ENTRY_NR(46);
	ENTRY(47);
	ENTRY(48);
	ENTRY_NR(49);
	ENTRY_NR(50);
	ENTRY(51);
	ENTRY(52);
	ENTRY(53);
	ENTRY_NR(54);
	ENTRY(55);
	ENTRY(56);
	ENTRY(57);

	case 58:
	case 59:
	case 60:
		Room58::entry();
		break;

	ENTRY(62);
	ENTRY(63);
	ENTRY(64);
	ENTRY(65);
	ENTRY_NR(66);
	ENTRY(67);
	ENTRY(68);
	ENTRY_NR(69);
	ENTRY(70);
	ENTRY(71);
	ENTRY(72);
	ENTRY(73);
	ENTRY(74);
	ENTRY(75);
	ENTRY(76);
	ENTRY(77);
	ENTRY(78);
	ENTRY(79);
	ENTRY(80);
	ENTRY(81);
	ENTRY(82);
	ENTRY(83);
	ENTRY(84);
	ENTRY_NR(85);
	ENTRY_NR(86);
	ENTRY(87);
	ENTRY(88);
	ENTRY(89);
	ENTRY_NR(90);
	ENTRY(91);
	ENTRY(92);
	ENTRY(93);
	ENTRY(94);
	ENTRY_NR(95);
	ENTRY(96);
	ENTRY(97);

	default:
		break;
	}
#undef ENTRY
#undef ENTRY_NR

	flags.LoadGame = false;
}

void exit_room(int16 eib_nr) {
	int16 *xy;
	int16 x, y;
	int16 no_exit;
	no_exit = false;
	det->disable_room_sound();

	switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
	case 6:
		if (eib_nr == 8)
			_G(spieler).R17Location = 2;
		break;

	case 11:
		atds->set_steuer_bit(121, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case 18:
		if (eib_nr == 40)
			_G(spieler).R17Location = 2;
		else if (eib_nr == 41)
			_G(spieler).R17Location = 3;
		_G(spieler).ScrollxStep = 1;
		flags.NoScroll = false;
		_G(spieler).R18Gitter = false;
		_G(spieler).room_e_obj[50].Attribut = 255;
		_G(spieler).room_e_obj[41].Attribut = AUSGANG_UNTEN;
		break;

	case 19:
		flags.NoScroll = false;
		break;

	case 21:
		if (eib_nr == 47)
			_G(spieler).R17Location = 1;
		flags.NoEndPosMovObj = false;
		SetUpScreenFunc = 0;
		load_chewy_taf(CHEWY_NORMAL);

		break;

	case 22:
		break;

	case 23:
		_G(spieler).PersonHide[P_CHEWY] = false;
		switch (_G(spieler).R23GleiterExit) {
		case 16:
			set_person_pos(126, 110, P_CHEWY, P_RIGHT);
			break;

		case 25:
			_G(spieler).R25GleiterExit = true;
			break;

		}
		menu_item = CUR_WALK;
		cursor_wahl(menu_item);
		_G(maus_links_click) = false;
		break;

	case 24: Room24::xit(); break;
	case 27: Room27::xit(eib_nr); break;
	case 28: Room28::xit(eib_nr); break;
	case 29: Room29::xit(); break;

	case 34:
		flags.ChewyDontGo = false;
		break;

	case 41: Room41::xit(); break;
	case 42: Room42::calc_xit(); break;
	case 45: Room45::xit(eib_nr); break;
	case 46: Room46::xit(); break;
	case 47: Room47::xit(); break;
	case 49: Room49::xit(eib_nr); break;
	case 52: Room52::xit(); break;
	case 54: Room54::xit(eib_nr); break;
	case 56: Room56::xit(); break;
	case 57: Room57::xit(eib_nr); break;

	case 64:
		if (_G(spieler).R64Moni1Ani == 5)
			_G(spieler).R64Moni1Ani = 3;
		break;

	case 65: Room65::xit(); break;
	case 76: Room76::xit(); break;
	case 77: Room77::xit(); break;
	case 78: Room78::xit(); break;
	case 79: Room79::xit(); break;
	case 88: Room88::xit(); break;

	default:
		no_exit = true;
		break;
	}

	x = -1;
	y = -1;

	switch (eib_nr) {
	case 0:
	case 3:
	case 18:
	case 30:
	case 35:
	case 48:
	case 51:
	case 70:
	case 83:
	case 93:
	case 103:
	case 105:
	case 109:
	case 111:
	case 114:
	case 115:
	case 118:
	case 120:
	case 139:
		x = -44;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	case 1:
	case 2:
	case 17:
	case 19:
	case 34:
	case 39:
	case 49:
	case 52:
	case 104:
	case 106:
	case 108:
	case 112:
	case 117:
	case 119:
	case 123:
	case 125:
	case 135:
		xy = (int16 *)ablage[room_blk.AkAblage];
		x = xy[0] + 30;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	case 10:
	case 15:
	case 41:
	case 58:
	case 73:
	case 77:
	case 78:
	case 92:
	case 122:
	case 131:
		xy = (int16 *)ablage[room_blk.AkAblage];
		x = spieler_vector[P_CHEWY].Xypos[0];
		y = xy[1] + 3;
		break;

	case 6:
	case 11:
	case 13:
	case 14:
	case 22:
	case 23:
	case 25:
	case 27:
	case 33:
	case 38:
	case 40:
	case 50:
	case 65:
	case 126:
		set_person_pos(spieler_vector[P_CHEWY].Xypos[0],
		               spieler_vector[P_CHEWY].Xypos[1], P_CHEWY, P_RIGHT);
		set_up_screen(DO_SETUP);
		break;

	case 8:
	case 9:
	case 12:
	case 16:
	case 20:
	case 21:
	case 24:
	case 32:
	case 36:
	case 71:
	case 96:
		set_person_pos(spieler_vector[P_CHEWY].Xypos[0],
		               spieler_vector[P_CHEWY].Xypos[1], P_CHEWY, P_LEFT);
		set_up_screen(DO_SETUP);
		break;

	case 62:
		x = 20;
		y = 80;
		break;

	case 72:
		x = spieler_vector[P_CHEWY].Xypos[0];
		y = spieler_vector[P_CHEWY].Xypos[1] - 10;
		break;

	case 75:
		x = 160;
		y = 200;
		det->show_static_spr(4);
		break;

	case 84:
		x = spieler_vector[P_CHEWY].Xypos[0] - 70;
		y = spieler_vector[P_CHEWY].Xypos[1] - 50;
		HowardMov = 1;
		break;

	case 85:
		x = spieler_vector[P_CHEWY].Xypos[0] + 70;
		y = spieler_vector[P_CHEWY].Xypos[1];
		HowardMov = 1;
		break;

	case 86:
		det->show_static_spr(0);
		x = spieler_vector[P_CHEWY].Xypos[0] - 44;
		y = spieler_vector[P_CHEWY].Xypos[1];
		HowardMov = 2;
		break;

	case 90:
		det->show_static_spr(8);
		x = spieler_vector[P_CHEWY].Xypos[0] - 60;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	case 94:
		det->show_static_spr(3);
		x = spieler_vector[P_CHEWY].Xypos[0] - 40;
		y = spieler_vector[P_CHEWY].Xypos[1] - 10;
		break;

	case 127:
		x = 196;
		y = 133;
		det->show_static_spr(0);
		break;

	case 132:
		x = 505;
		y = 62;
		break;

	case 140:
		x = spieler_vector[P_CHEWY].Xypos[0] + 40;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	case 141:
		x = spieler_vector[P_CHEWY].Xypos[0] - 12;
		y = spieler_vector[P_CHEWY].Xypos[1];
		break;

	default:
		break;
	}

	if (x != -1 && y != -1) {
		spieler_mi[P_CHEWY].Mode = true;
		go_auto_xy(x, y, P_CHEWY, ANI_WAIT);
		spieler_mi[P_CHEWY].Mode = false;
	}

	if (no_exit) {
		switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
		case 40:
			Room40::xit(eib_nr);
			break;

		case 42:
			if (_G(spieler).PersonRoomNr[P_HOWARD] == 42)
				_G(spieler).PersonRoomNr[P_HOWARD] = 40;
			break;

		case 50: Room50::xit(eib_nr); break;
		case 51: Room51::xit(eib_nr); break;
		case 55: Room55::xit(eib_nr); break;
		case 66: Room66::xit(eib_nr); break;
		case 67: Room67::xit(); break;
		case 68: Room68::xit(); break;
		case 69: Room69::xit(eib_nr); break;
		case 70: Room70::xit(eib_nr); break;
		case 71: Room71::xit(eib_nr); break;
		case 72: Room72::xit(eib_nr); break;
		case 73: Room73::xit(eib_nr); break;
		case 74: Room74::xit(eib_nr); break;
		case 75: Room75::xit(eib_nr); break;
		case 81: Room81::xit(eib_nr); break;
		case 82: Room82::xit(eib_nr); break;
		case 84: Room84::xit(eib_nr); break;
		case 85: Room85::xit(eib_nr); break;
		case 86: Room86::xit(eib_nr); break;
		case 87: Room87::xit(eib_nr); break;
		case 89: Room89::xit(); break;
		case 90: Room90::xit(eib_nr); break;
		case 91: Room91::xit(eib_nr); break;
		case 93: Room93::xit(); break;
		case 94: Room94::xit(); break;
		case 95: Room95::xit(eib_nr); break;
		case 96: Room96::xit(eib_nr); break;
		case 97: Room97::xit(); break;
		default: break;
		}
	}
}

void print_rows(int16 id) {
	int16 txt_anz, len;
	char *txtStr, *s;

	out->set_fontadr(font8x8);
	out->set_vorschub(fvorx8x8, fvory8x8);
	txtStr = atds->ats_get_txt(id, TXT_MARK_NAME, &txt_anz, ATS_DATEI);
	out->setze_zeiger(nullptr);

	for (int i = 0; i < txt_anz; ++i) {
		s = txt->str_pos(txtStr, i);
		len = (strlen(s) * fvorx8x8) / 2;

		out->printxy(160 - len, 50 + i * 10, 14, 300, 0, "%s", s);
	}
}

int16 flic_user_function(int16 keys) {
	int ret;

	serve_speech();
	if (atds->aad_get_status() != -1) {
		switch (flic_val1) {
		case 579:
		case 584:
		case 588:
		case 591:
			out->raster_col(254, 63, 12, 46);
			break;
		default:
			break;
		}
	}

	atds->print_aad(_G(spieler).scrollx, _G(spieler).scrolly);
	if (flic_val1 == 593 && keys == 35)
		atds->stop_aad();
	if (flic_val1 == 594 && keys == 18)
		atds->stop_aad();

	ret = in->get_switch_code() == 1 ? -1 : 0;
	if (flic_val2 == 140 && keys == 15)
		ret = -2;
	if (flic_val2 == 144 && keys == 7)
		ret = -2;
	if (flic_val2 == 145 || flic_val2 == 142 ||
			flic_val2 == 141 || flic_val2 == 146) {
		if (atds->aad_get_status() == -1)
			ret = -2;
	}

	return ret;
}

static void flic_proc1() {
	const int16 VALS1[] = {
		135,  145,  142,  140,  145,  144,  142,  134,  148,  138,
		143,  142,  146,  154,  142,  139,  146,  156,  157,  147,
		153,  152,  141,  137,  136,  151,  151,  149,  150
	};
	const byte VALS2[] = {
		1,  1,  0,  0,  0,  0,  0,  1,  1,  1,
		1,  0,  0,  1,  1,  1,  1,  1,  0,  1,
		1,  1,  1,  1,  0,  1,  0,  1,  0
	};
	const int16 VALS3[] = {
		579, 580, 581,  -1, 582, -1, 583, 584, -1, -1,
		585, 586, 587, 588, 589, -1, 590, 591, -1, -1,
		 -1,  -1, 592, 593, 594, -1,  -1,  -1, -1
	};
	const byte VALS4[] = {
		0,  1,  1,  0,  1,  0,  1,  0,  0,  0,
		1,  1,  1,  0,  1,  0,  1,  0,  0,  0,
		0,  0,  1,  0,  0,  0,  0,  0,  0
	};
	int16 ret = 0;

	atds->load_atds(98, AAD_DATEI);
	flc->set_custom_user_function(flic_user_function);
	load_room_music(258);

	for (int i = 0; i < 29 && ret != -1; ++i) {
		if (VALS1[i] == 148)
			load_room_music(259);
		else if (VALS1[i] == 143)
			load_room_music(260);
		if (VALS2[i]) {
			out->setze_zeiger(nullptr);
			out->cls();
		}

		flic_val1 = 0;
		if (VALS3[i] != -1) {
			start_aad(VALS3[i], -1);
			flic_val1 = VALS3[i];
		}

		bool flag;
		do {
			flic_val2 = VALS1[i];
			mem->file->select_pool_item(Ci.Handle, flic_val2);
			ret = flc->custom_play(&Ci);

			flag = VALS4[i] && atds->aad_get_status() != -1;
		} while (flag && ret != -1 && ret != -2);

		atds->stop_aad();
	}

	flc->remove_custom_user_function();
	if (ret == -1) {
		out->setze_zeiger(nullptr);
		out->cls();
		out->raster_col(254, 62, 35, 7);
		start_aad(595);
		atds->print_aad(254, 0);

		if (flags.InitSound && _G(spieler).SpeechSwitch) {
			while (ailsnd->isSpeechActive() && !SHOULD_QUIT) {
				ailsnd->serve_db_samples();
			}
		} else {
			delay(6000);
		}
	}

	out->setze_zeiger(workptr);
	out->cls();
}

#define PLAY_LOOP do { \
		mem->file->select_pool_item(Ci.Handle, nr); \
		ret = flc->custom_play(&Ci); \
	} while (atds->aad_get_status() != -1 && ret != -1)


void flic_cut(int16 nr, int16 mode) {
	const int16 FLIC_CUT_133[] = {
		133, 123, 125, 126, 124, 128, 129, 130, 131,
		132, 133, 127, 158
	};
	const int16 FLIC_CUT_1045[] = {
		30, 47, 41, 34, 45, 52, 53, 57, 64, 63, 62
	};
	const int16 FLIC_CUT_1074[] = { 73, 114, 74, 75 };
	const int16 FLIC_CUT_1080[] = { 80, 78, 77, 81, 82, 79, 76, 116 };
	const int16 FLIC_CUT_1093[] = {
		93, 94, 95, 96, 97, 98, 99, 100, 92, 90, 91, 89
	};
	const int16 FLIC_CUT_1106[] = { 106, 105, 104 };
	const int16 FLIC_CUT_1113[] = { 113, 106, 103, 118, 120 };
	int16 i, ret = 0;

	out->setze_zeiger(0);
	det->disable_room_sound();
	ailsnd->end_sound();
	g_events->delay(50);
	Common::File *f = File::open("cut/cut.tap");
	Ci.Handle = f;
	Ci.Fname = 0;

	if (Ci.Handle) {
		switch (nr) {
		case FCUT_001:
		case 1000:
			ailsnd->stop_mod();
			CurrentSong = -1;
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);
			break;

		case FCUT_019:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
			ailsnd->stop_mod();
			CurrentSong = -1;
			nr = FCUT_019;
			mem->file->select_pool_item(Ci.Handle, nr);
			ret = flc->custom_play(&Ci);
			ailsnd->set_loopmode(1);

			if (!modul && ret != -1) {
				f->seek(ChunkHead::SIZE(), SEEK_CUR);
				out->cls();
				ret = flc->custom_play(&Ci);

				if (ret != -1) {
					mem->file->select_pool_item(Ci.Handle, 21);
					out->cls();
					flc->custom_play(&Ci);
					if (!modul) {
						f->seek(ChunkHead::SIZE(), SEEK_CUR);
						out->cls();
						flc->custom_play(&Ci);
					}
				}
			}
			if (!modul) {
				mem->file->select_pool_item(Ci.Handle, 20);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				mem->file->select_pool_item(Ci.Handle, 23);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				f->seek(ChunkHead::SIZE(), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				f->seek(ChunkHead::SIZE(), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				f->seek(ChunkHead::SIZE(), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				f->seek(ChunkHead::SIZE(), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			if (!modul) {
				f->seek(ChunkHead::SIZE(), SEEK_CUR);
				out->cls();
				flc->custom_play(&Ci);
			}
			ERROR
			ailsnd->fade_out(0);
			out->ausblenden(1);
			out->cls();
			while (ailsnd->music_playing());
			ailsnd->set_loopmode(_G(spieler).soundLoopMode);
			break;

		case FCUT_032:
		case FCUT_035:
		case FCUT_036:
		case FCUT_037:
		case FCUT_040:
			PLAY_LOOP;
			break;

		case FCUT_034:
			ailsnd->stop_mod();
			CurrentSong = -1;

			PLAY_LOOP;
			break;

		case FCUT_053:
			for (i = 0; i < 3; ++i) {
				mem->file->select_pool_item(Ci.Handle, nr);
				flc->custom_play(&Ci);
			}
			break;

		case FCUT_054:
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);

		case FCUT_058:
			nr = FCUT_058;
			mem->file->select_pool_item(Ci.Handle, nr);

			flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
			if (!modul) {
				mem->file->select_pool_item(Ci.Handle, FCUT_059);
				flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
			}
			if (!_G(spieler).R43GetPgLady) {
				if (!modul) {
					mem->file->select_pool_item(Ci.Handle, FCUT_060);
					flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
				}
			} else {
				if (!modul) {
					mem->file->select_pool_item(Ci.Handle, FCUT_061);
					flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
				}
				if (!modul) {
					mem->file->select_pool_item(Ci.Handle, FCUT_062);
					flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
				}
			}
			ailsnd->fade_out(0);
			out->ausblenden(1);
			out->cls();
			while (ailsnd->music_playing());
			break;

		case FCUT_065:
			ailsnd->stop_mod();
			CurrentSong = -1;
			load_room_music(256);
			ailsnd->set_loopmode(1);
			Room46::kloppe();
			ailsnd->set_loopmode(_G(spieler).soundLoopMode);
			CurrentSong = -1;
			break;

		case FCUT_071:
			ailsnd->stop_mod();
			CurrentSong = -1;
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);
			break;

		case 83:
		case 1083:
			for (i = 0; i < 2 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, 83);
				ret = flc->custom_play(&Ci);
			}


		case 95:
			while (atds->aad_get_status() != -1 && !SHOULD_QUIT) {
				mem->file->select_pool_item(Ci.Handle, nr);
				flc->custom_play(&Ci);
			}
			break;

		case 112:
			ailsnd->set_music_mastervol(32);
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);
			mem->file->select_pool_item(Ci.Handle, nr);
			flc->custom_play(&Ci);
			ailsnd->set_music_mastervol(5);
			break;

		case 116:
			for (i = 0; i < 6; ++i) {
				mem->file->select_pool_item(Ci.Handle, nr);
				flc->custom_play(&Ci);
			}
			break;

		case 133:
		case 1123:
			for (i = 0; i < 13 && i != -1 && !modul; ++i) {
				mem->file->select_pool_item(Ci.Handle, FLIC_CUT_133[i]);
				ret = flc->custom_play(&Ci);
				if (i == 0 || i == 1) {
					out->setze_zeiger(nullptr);
					out->cls();
				}
			}
			break;

		case 135:
			flic_proc1();
			break;

		case 1003:
			fx->border(workpage, 100, 0, 0);
			print_rows(590);
			mem->file->select_pool_item(Ci.Handle, 1);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				for (i = 0; i < 3 && ret != -1; ++i) {
					fx->border(workpage, 100, 0, 0);
					print_rows(591);
					mem->file->select_pool_item(Ci.Handle, i + 3);
					ret = flc->custom_play(&Ci);
				}
			}
			break;

		case 1006:
			for (i = 0; i < 3 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 6);
				fx->border(workpage, 100, 0, 0);
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1009:
			for (i = 0; i < 2 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 9);
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1012:
			for (i = 0; i < 3 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 12);
				fx->border(workpage, 100, 0, 0);
				ret = flc->custom_play(&Ci);
			}

			if (ret != -1) {
				out->cls();
				mem->file->select_pool_item(Ci.Handle, 17);
				fx->border(workpage, 100, 0, 0);
			}
			break;

		case 1015:
			for (i = 0; i < 2 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 15);
				fx->border(workpage, 100, 0, 0);
				ret = flc->custom_play(&Ci);
			}

		case 1045:
			for (i = 0; i < 11 && ret != -1; ++i) {
				fx->border(workpage, 100, 0, 0);
				print_rows(594);
				mem->file->select_pool_item(Ci.Handle, FLIC_CUT_1045[i]);
				if (FLIC_CUT_1045[i] == 53) {
					ailsnd->stop_mod();
					CurrentSong = -1;
					load_room_music(256);
				}

				ret = flc->custom_play(&Ci);

				if (FLIC_CUT_1045[i] == 53) {
					ailsnd->stop_mod();
				}
			}
			break;

		case 1031:
			mem->file->select_pool_item(Ci.Handle, 31);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				fx->border(workpage, 100, 0, 0);
				mem->file->select_pool_item(Ci.Handle, 43);
				flc->custom_play(&Ci);
			}
			break;

		case 1048:
			mem->file->select_pool_item(Ci.Handle, 50);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				fx->spr_blende(workpage, 100, false, 0);
				mem->file->select_pool_item(Ci.Handle, 48);
				ret = flc->custom_play(&Ci);
			}
			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 49);
				ret = flc->custom_play(&Ci);
			}
			if (ret != -1) {
				fx->spr_blende(workpage, 100, false, 0);
				ret = flc->custom_play(&Ci);
			}
			if (ret != -1) {
				fx->spr_blende(workpage, 100, false, 0);
				mem->file->select_pool_item(Ci.Handle, 54);
			}
			break;

		case 1055:
			for (i = 0; i < 2 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 55);
				fx->border(workpage, 100, 0, 0);
				ret = flc->custom_play(&Ci);
			}

			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 46);
				fx->border(workpage, 100, 0, 0);
			}
			break;

		case 1058:
			ailsnd->stop_mod();
			CurrentSong = -1;
			load_room_music(255);
			mem->file->select_pool_item(Ci.Handle, 58);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 59);
				ret = flc->custom_play(&Ci);
			}
			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 60);
				ret = flc->custom_play(&Ci);
			}
			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 61);
				fx->spr_blende(workpage, 100, false, 0);
				ret = flc->custom_play(&Ci);
			}
			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 62);
				fx->border(workpage, 100, 0, 0);
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1065:
			ailsnd->stop_mod();
			CurrentSong = -1;
			load_room_music(256);

			for (i = 0; i < 2 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 65);
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1068:
			mem->file->select_pool_item(Ci.Handle, 68);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				fx->border(workpage, 100, 0, 0);
				mem->file->select_pool_item(Ci.Handle, 70);
				flc->custom_play(&Ci);
			}
			break;

		case 1069:
			mem->file->select_pool_item(Ci.Handle, 69);
			ret = flc->custom_play(&Ci);

			for (i = 0; i < 2 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 71);
				fx->spr_blende(workpage, 100, false, 0);
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1074:
			for (i = 0; i < 4 && ret != -1; ++i) {
				fx->border(workpage, 100, 0, 0);
				print_rows(605);
				mem->file->select_pool_item(Ci.Handle, i + FLIC_CUT_1074[i]);
				fx->spr_blende(workpage, 100, false, 0);
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1080:
			for (i = 0; i < 8 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, FLIC_CUT_1080[i]);
				fx->border(workpage, 100, 0, 0);
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1087:
			mem->file->select_pool_item(Ci.Handle, 87);
			ret = flc->custom_play(&Ci);

			for (i = 0; i < 2 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, i + 102);
				fx->border(workpage, 100, 0, 0);
				flc->custom_play(&Ci);
			}
			break;

		case 1088:
			mem->file->select_pool_item(Ci.Handle, 88);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 86);
				fx->spr_blende(workpage, 100, false, 0);
				flc->custom_play(&Ci);
				ailsnd->stop_mod();
			}
			break;

		case 1093:
			for (i = 0; i < 12 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, FLIC_CUT_1093[i]);
				switch (FLIC_CUT_1093[i]) {
				case 90:
				case 91:
				case 92:
					out->cls();
					break;
				default:
					break;
				}

				ret = flc->custom_play(&Ci);
			}
			break;

		case 1106:
			for (i = 0; i < 3 && ret == -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, FLIC_CUT_1106[i]);
				out->cls();
				ret = flc->custom_play(&Ci);
			}
			break;

		case 1107:
			mem->file->select_pool_item(Ci.Handle, 107);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 109);
				fx->border(workpage, 100, 0, 0);
			}
			break;

		case 1108:
			mem->file->select_pool_item(Ci.Handle, 108);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 115);
				fx->spr_blende(workpage, 100, false, 0);
				flc->custom_play(&Ci);
			}
			break;

		case 1110:
			ailsnd->stop_mod();
			CurrentSong = -1;
			load_room_music(257);
			ailsnd->set_music_mastervol(20);
			mem->file->select_pool_item(Ci.Handle, 110);
			ret = flc->custom_play(&Ci);

			if (ret != -1) {
				mem->file->select_pool_item(Ci.Handle, 112);
				fx->spr_blende(workpage, 100, false, 0);
				ailsnd->set_music_mastervol(63);
				flc->custom_play(&Ci);
			}

			ailsnd->stop_mod();
			break;

		case 1113:
			for (i = 0; i < 5 && ret != -1; ++i) {
				mem->file->select_pool_item(Ci.Handle, FLIC_CUT_1113[i]);
				out->cls();
				ret = flc->custom_play(&Ci);
			}

		case 1117:
			if (mem->file->select_pool_item(Ci.Handle, 117) != (uint16)-1) {
				mem->file->select_pool_item(Ci.Handle, 119);
				fx->border(workpage, 100, 0, 0);
				flc->custom_play(&Ci);
			}
			break;

		default:
			switch (mem->file->select_pool_item(Ci.Handle,
				(nr < 1000) ? nr : nr - 1000)) {
			case 0:
				flc->custom_play(&Ci);
				break;
			case 1:
				flc->play(Ci.Handle, Ci.VirtScreen, Ci.TempArea);
				break;
			default:
				break;
			}

			ERROR
			break;
		}

		chewy_fclose(Ci.Handle);
	} else {
		fcode = OPENFEHLER;
		modul = DATEI;
	}

	ERROR

	ailsnd->end_sound();
	g_events->delay(50);
	ailsnd->set_sound_mastervol(_G(spieler).SoundVol);
	ailsnd->set_music_mastervol(_G(spieler).MusicVol);

	if (nr < 1000 && nr != 135) {
		load_room_music(_G(spieler).PersonRoomNr[0]);
		ERROR
		room->load_sound();
		ERROR

		if (_G(spieler).SpeechSwitch)
			det->enable_room_sound();

		uhr->reset_timer(0, 0);
	}

	if (!flags.NoPalAfterFlc)
		out->set_palette(pal);

	atds->stop_aad();
	atds->stop_ats();
	out->setze_zeiger(workptr);
	flags.NoPalAfterFlc = false;
}

uint16 exit_flip_flop(int16 ani_nr, int16 eib_nr1, int16 eib_nr2,
                        int16 ats_nr1, int16 ats_nr2, int16 sib_nr,
                        int16 spr_nr1, int16 spr_nr2, int16 flag) {
	if (ani_nr != -1)
		det->start_detail(ani_nr, 1, flag);
	flag ^= 1;
	if (ats_nr1 != -1)
		atds->set_ats_str(ats_nr1, flag, ATS_DATEI);
	if (ats_nr2 != -1)
		atds->set_ats_str(ats_nr2, flag, ATS_DATEI);
	if (flag) {
		if (eib_nr1 != -1)
			_G(spieler).room_e_obj[eib_nr1].Attribut = spr_nr1;
		if (eib_nr2 != -1)
			_G(spieler).room_e_obj[eib_nr2].Attribut = spr_nr2;
	} else {
		if (eib_nr1 != -1)
			_G(spieler).room_e_obj[eib_nr1].Attribut = 255;
		if (eib_nr2 != -1)
			_G(spieler).room_e_obj[eib_nr1].Attribut = 255;
	}
	if (sib_nr != -1)
		obj->calc_rsi_flip_flop(sib_nr);

	return (uint16)flag;
}

int16 sib_event_no_inv(int16 sib_nr) {
	int16 ret;
	ret = true;

	switch (sib_nr) {
	case SIB_KABEL_R1:
		atds->set_ats_str(8, TXT_MARK_LOOK, 1, ATS_DATEI);
		break;

	case SIB_MONOKEL:
		_G(spieler).R0Monokel = true;
		obj->hide_sib(SIB_MONOKEL);
		if (_G(spieler).R0GBuch)
			atds->del_steuer_bit(12, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_GBUCH:
		_G(spieler).R0GBuch = true;
		obj->hide_sib(SIB_GBUCH);
		if (_G(spieler).R0Monokel)
			atds->del_steuer_bit(12, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_TERMINAL_R5:
		if (_G(spieler).R5Terminal) {
			_G(spieler).R5Terminal = 0;
			det->stop_detail(6);
			atds->set_ats_str(27, TXT_MARK_LOOK, 0, ATS_DATEI);
			atds->set_ats_str(30, TXT_MARK_LOOK, 0, ATS_DATEI);
		}
		break;

	case SIB_KNOPF_R5:
		Room5::pushButton();
		break;

	case SIB_SEIL:
		obj->hide_sib(SIB_SEIL);
		break;

	case SIB_BOLA_SCHACHT:
		atds->set_ats_str(41, TXT_MARK_LOOK, 0, ATS_DATEI);
		break;

	case SIB_BOLA_KNOPF_R6:
		Room6::bola_knopf();
		break;

	case SIB_TKNOPF1_R7:
		_G(spieler).R6DoorLeftB = exit_flip_flop(3, 12, 9, 49, 35, SIB_TKNOPF2_R6,
		                                     AUSGANG_OBEN, AUSGANG_LINKS,
		                                     (int16)_G(spieler).R6DoorLeftB);
		break;

	case SIB_HEBEL_R7:
		_G(spieler).R7Hebel ^= 1;

		if (!_G(spieler).R7Hebel)
			atds->set_ats_str(50, 0, ATS_DATEI);
		else if (!_G(spieler).R7BorkFlug)
			atds->set_ats_str(50, 1, ATS_DATEI);
		else
			atds->set_ats_str(50, 2, ATS_DATEI);
		break;

	case SIB_KLINGEL_R7:
		Room7::klingel();
		break;

	case SIB_GIPS_R7:
		_G(spieler).R7RHaken = true;
		obj->show_sib(SIB_RHAKEN_R7);
		obj->hide_sib(SIB_GIPS_R7);
		break;

	case SIB_TKNOPF2_R7:
		_G(spieler).R7DoorRight = exit_flip_flop(4, 13, 14, 53, 68, SIB_TUER_R8,
		                                     AUSGANG_RECHTS, AUSGANG_RECHTS,
		                                     (int16)_G(spieler).R7DoorRight);
		break;

	case SIB_SCHLOTT_R7:
		obj->hide_sib(SIB_SCHLOTT_R7);
		break;

	case SIB_LHAKEN_R7:
		if (_G(spieler).R7SeilLeft) {
			if (_G(spieler).R7RHaken) {
				_G(spieler).R7SeilOk = true;
				auto_move(4, P_CHEWY);
				obj->calc_rsi_flip_flop(SIB_LHAKEN_R7);
				obj->calc_rsi_flip_flop(SIB_RHAKEN_R7);
				atds->set_ats_str(54, TXT_MARK_LOOK, 1, ATS_DATEI);
				atds->set_ats_str(55, TXT_MARK_LOOK, 1, ATS_DATEI);
				atds->del_steuer_bit(56, ATS_AKTIV_BIT, ATS_DATEI);
				start_aad(9);
			} else {
				obj->set_rsi_flip_flop(SIB_LHAKEN_R7, 2);
				obj->calc_rsi_flip_flop(SIB_LHAKEN_R7);
			}
		}
		break;

	case SIB_FOLTER_R8:
		Room8::stop_folter();
		break;

	case SIB_TUER_R8:
		_G(spieler).R7DoorRight = exit_flip_flop(-1, 14, 13, 68, 53, SIB_TKNOPF2_R7,
		                                     AUSGANG_RECHTS, AUSGANG_RECHTS,
		                                     (int16)_G(spieler).R7DoorRight);
		break;

	case SIB_DEE_PAINT_R9:
		obj->hide_sib(SIB_DEE_PAINT_R9);
		break;

	case SIB_SCHLITZ_R11:
		Room11::get_card();
		break;

	case SIB_BANDKNOPF_R13:
		_G(spieler).R13Bandlauf ^= 1;

		if (_G(spieler).R13Bandlauf) {
			for (int i = 0; i < 5; ++i)
				det->start_detail(i, 255, 0);
		} else {
			for (int i = 0; i < 5; ++i)
				det->stop_detail(i);
		}

		atds->set_ats_str(94, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
		atds->set_ats_str(97, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
		atds->set_ats_str(93, TXT_MARK_LOOK, _G(spieler).R13Bandlauf, ATS_DATEI);
		break;

	case SIB_CARTRIDGE_R23:
		Room23::get_cartridge();
		break;

	case SIB_FLUXO_R23:
		_G(spieler).R23FluxoFlex = false;
		atds->set_ats_str(112, 0, ATS_DATEI);
		menu_item_vorwahl = CUR_USE;
		break;

	case SIB_TRANSLATOR_23:
		atds->set_ats_str(113, 1, ATS_DATEI);
		menu_item_vorwahl = CUR_USE;
		break;

	case SIB_TALISMAN_R12:
		_G(spieler).R12Talisman = true;
		obj->hide_sib(SIB_TALISMAN_R12);
		_G(timer_nr)[0] = room->set_timer(255, 20);
		break;

	case SIB_GITTER_R16:
		atds->set_ats_str(125, 1, ATS_DATEI);
		_G(spieler).room_e_obj[33].Attribut = AUSGANG_OBEN;
		break;

	case SIB_SCHALTER1_R21:
		det->start_detail(0, 1, _G(spieler).R21Hebel1);
		_G(spieler).R21Hebel1 ^= 1;
		Room21::calc_laser();
		atds->set_ats_str(126, TXT_MARK_LOOK, _G(spieler).R21Hebel1, ATS_DATEI);
		break;

	case SIB_SCHALTER2_R21:
		det->start_detail(1, 1, _G(spieler).R21Hebel2);
		_G(spieler).R21Hebel2 ^= 1;
		Room21::calc_laser();
		atds->set_ats_str(127, TXT_MARK_LOOK, _G(spieler).R21Hebel2, ATS_DATEI);
		break;

	case SIB_SCHALTER3_R21:
		det->start_detail(2, 1, _G(spieler).R21Hebel3);
		_G(spieler).R21Hebel3 ^= 1;
		Room21::calc_laser();
		atds->set_ats_str(128, TXT_MARK_LOOK, _G(spieler).R21Hebel3, ATS_DATEI);
		break;

	case SIB_SEIL_R21:
		atds->set_steuer_bit(129, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_GITTER1_R21:
		Room21::use_gitter_energie();
		break;

	case SIB_CART1_R18:
		atds->set_steuer_bit(155, ATS_AKTIV_BIT, ATS_DATEI);
		break;

	case SIB_TUERKNOPF_R18:
		if (_G(spieler).R18DoorBruecke) {
			det->disable_sound(19, 0);
			det->enable_sound(19, 1);
		} else {
			det->enable_sound(19, 0);
			det->disable_sound(19, 1);
		}

		if (!_G(spieler).R6DoorLeftF) {
			_G(spieler).R6DoorLeftF = exit_flip_flop(-1, 8, -1, 33, -1, SIB_TKNOPF1_R6,
				AUSGANG_LINKS, -1,
				(int16)_G(spieler).R6DoorLeftF);
		}
		_G(spieler).R18DoorBruecke = exit_flip_flop(19, 40, 35, 148, -1, -1,
			AUSGANG_OBEN, AUSGANG_LINKS,
			(int16)_G(spieler).R18DoorBruecke);
		break;

	case SIB_CART_FACH_R18:
		start_spz_wait(CH_LGET_O, 1, ANI_VOR, P_CHEWY);
		_G(spieler).R18CartFach = 0;
		cur_2_inventory();
		atds->set_ats_str(157, 1, AAD_DATEI);
		break;

	case SIB_SCHLAUCH_R26:
	case SIB_TRICHTER_R26:
	case SIB_ANGEL0_R26:
	case SIB_MILCH_R27:
	case SIB_KAFFEE_R33:
	case SIB_ZAPPER_R39:
	case SIB_KNOCHEN_R35:
	case SIB_RADIO_R35:
	case SIB_KUERBIS_R37:
	case SIB_ZEITUNG_R27:
	case SIB_FLASCHE_R51:
	case SIB_KAPPE_R51:
	case SIB_AUSRUEST_R51:
	case SIB_PIRANHA:
	case SIB_SCHALL:
	case SIB_ARTEFAKT:
		obj->hide_sib(sib_nr);
		break;

	case SIB_PUTE_R34:
		atds->set_ats_str(226, 1, ATS_DATEI);
		break;

	case SIB_TOPF_R31:
		obj->hide_sib(SIB_TOPF_R31);
		atds->set_ats_str(242, 1, ATS_DATEI);
		_G(spieler).R31PflanzeWeg = true;
		break;

	case SIB_HFUTTER1_R37:
	case SIB_HFUTTER2_R37:
		obj->hide_sib(74);
		obj->hide_sib(75);
		break;

	case SIB_SURIMY_R27:
		Room27::get_surimy();
		break;

	case SIB_MUENZE_R40:
		obj->hide_sib(sib_nr);
		det->del_static_ani(6);
		room->set_timer_status(6, TIMER_STOP);
		_G(spieler).R40Geld = true;
		start_spz(CH_PUMP_TALK, 255, ANI_VOR, P_CHEWY);
		start_aad_wait(201, -1);
		break;

	case SIB_VISIT_R53:
		obj->hide_sib(sib_nr);
		_G(spieler).R53Visit = true;
		break;

	case SIB_CIGAR_R50:
		Room50::stop_cigar();
		break;

	case SIB_LAMPE_R52:
		atds->del_steuer_bit(338, ATS_AKTIV_BIT, ATS_DATEI);
		_G(spieler).R52LichtAn ^= 1;
		check_shad(2 * (_G(spieler).R52LichtAn + 1), 1);
		break;

	case SIB_KAUTABAK_R56:
		obj->hide_sib(sib_nr);
		_G(spieler).R56GetTabak = true;
		break;

	case SIB_ASCHE_R64:
		det->stop_detail(0);
		obj->hide_sib(sib_nr);
		Room64::talk_man(351);
		break;

	case 94:
		det->show_static_spr(7);
		_G(cur_hide_flag) = false;
		hide_cur();
		start_aad_wait(406, -1);
		if (_G(spieler).PersonRoomNr[1] == 66)
			start_aad_wait(613, -1);
		break;

	case 100:
		_G(spieler).flags33_1 = true;
		break;

	default:
		ret = false;
		break;

	}

	return ret;
}

void sib_event_inv(int16 sib_nr) {
	int16 ret;

	switch (sib_nr) {
	case SIB_TERMINAL_R5:
		if (!_G(spieler).R5Terminal) {
			_G(spieler).R5Terminal = true;
			cur_2_inventory();
			del_inventar(RED_CARD_INV);
			start_aad(103, -1);
			det->start_detail(6, 255, 0);
			atds->set_ats_str(27, 1, 1);
			atds->set_ats_str(30, 1, 1);
		}
		break;

	case SIB_TKNOPF1_R6:
		Room29::schlitz_sitz();
		break;

	case SIB_TKNOPF2_R6:
		_G(cur_hide_flag) = false;
		hide_cur();
		start_spz_wait(13, 1, 0, 0);
		_G(spieler).R18CartFach = true;
		del_inventar(_G(spieler).AkInvent);
		det->show_static_spr(7);
		atds->set_ats_str(157, 1, 1);

		if (_G(spieler).R18CartTerminal) {
			start_aad_wait(121, -1);
		} else {
			_G(spieler).R18CartTerminal = true;
			atds->set_ats_str(26, 1, 6);
			start_aad_wait(120, -1);
		}

		show_cur();
		break;

	case SIB_TKNOPF3_R6:
		ret = exit_flip_flop(-1, 46, 27, 132, 90, -1, 2, 2,
			_G(spieler).R6BolaSchild ? 1 : 0);
		_G(spieler).R21GitterMuell = ret & 1;
		atds->set_ats_str(90, 2, 2);
		break;

	case SIB_BOLA_KNOPF_R6:
		_G(spieler).R12TalismanOk = true;
		del_inventar(_G(spieler).AkInvent);
		atds->set_ats_str(118, 1, 1);
		start_spz(5, 255, 0, 0);
		start_aad_wait(115, 0);
		if (_G(spieler).R12TransOn)
			Room12::use_linke_rohr();
		break;

	case SIB_LHAKEN_R7:
		Room14::feuer();
		break;

	case SIB_RHAKEN_R7:
		del_inventar(_G(spieler).AkInvent);
		atds->set_ats_str(113, 0, 1);
		break;

	case SIB_GTUER:
		_G(spieler).R23FluxoFlex = true;
		del_inventar(_G(spieler).AkInvent);
		atds->set_ats_str(112, 1, 1);
		break;

	case SIB_SURIMY_R10:
		Room23::use_cartridge();
		break;

	case SIB_PAINT_R22:
		ret = exit_flip_flop(6, 20, 11, 99, 37, 15, 1, 2,
			(int16)_G(spieler).R6DoorRightB);
		_G(spieler).R6DoorRightB = ret & 1;
		break;

	case SIB_SCHLITZ_R11:
		ret = exit_flip_flop(5, 22, -1, 98, -1, -1,
			AUSGANG_OBEN, -1, (int16)_G(spieler).R11DoorRightB);
		_G(spieler).R11DoorRightB = ret & 1;
		break;

	case SIB_TKNOPF1_R11:
		ret = exit_flip_flop(1, 23, -1, 87, -1, -1,
			AUSGANG_OBEN, -1, (int16)_G(spieler).R11DoorRightF);
		_G(spieler).R11DoorRightB = false;
		_G(spieler).R11TerminalOk |= ret & 1;
		break;

	case SIB_TKNOPF2_R11:
		Room11::put_card();
		break;

	case SIB_TKNOPF3_R11:
		disable_timer();
		Room22::malen();
		enable_timer();
		break;

	case SIB_CARTRIDGE_R23:
		Room10::get_surimy();
		break;

	case SIB_FLUXO_R23:
		Room8::open_gdoor();
		break;

	case SIB_FEUER_R14:
	case SIB_TALISMAN_R12:
		Room7::haken(sib_nr);
		break;

	case SIB_ROEHRE_R12:
		del_inventar(_G(spieler).AkInvent);
		_G(spieler).R6BolaSchild = true;
		det->show_static_spr(2);
		obj->calc_rsi_flip_flop(18);
		obj->hide_sib(18);
		obj->show_sib(21);
		break;

	case SIB_SEIL_R21:
		ret = exit_flip_flop(6, 11, 20, 37, 99, 42, 2, 1,
			(int16)_G(spieler).R6DoorRightB);
		_G(spieler).R6DoorRightB = ret & 1;
		break;

	case SIB_CART_FACH_R18:
		ret = exit_flip_flop(5, 9, 12, 35, 49, 22, 1, 3,
			(int16)_G(spieler).R6DoorLeftB);
		_G(spieler).R6DoorLeftB = ret & 1;
		break;

	case SIB_AUTO_SITZ:
		ret = exit_flip_flop(4, 8, -1, 33, -1, -1, 1, -1,
			(int16)_G(spieler).R6DoorLeftF);
		_G(spieler).R6DoorLeftF = ret & 1;
		break;

	case SIB_AUTO_R28:
		Room28::use_surimy();
		break;

	default:
		break;
	}
}

} // namespace Chewy
