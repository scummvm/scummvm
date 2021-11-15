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

#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/global.h"
#include "chewy/episode1.h"
#include "chewy/episode2.h"
#include "chewy/episode3.h"
#include "chewy/episode4.h"

namespace Chewy {

int16 load_ads_dia(int16 dia_nr) {
	int16 ret = false;

	if (flags.AdsDialog == false) {
		bool tmp = atds->ads_start(dia_nr);
		ERROR
		if (tmp == true) {
			atds->load_atds(dia_nr, ADH_DATEI);
			ERROR
			ret = true;
			ads_blk_nr = 0;
			ads_item_ptr = atds->ads_item_ptr(ads_blk_nr, &ads_item_anz);
			flags.AdsDialog = true;
			ads_push = true;
			ads_tmp_dsp = _G(spieler).DispFlag;
			_G(spieler).DispFlag = false;

			set_ssi_xy();
			ads_dia_nr = dia_nr;
			talk_start_ani = -1;
			talk_hide_static = -1;
		}
	}
	return ret;
}

void set_ssi_xy() {
	int16 tmp = 0;

	for (int16 i = 0; i < MAX_PERSON; i++) {
		if (i >= P_NICHELLE)
			tmp = 1;
		ssi[i + tmp].X = spieler_vector[i].Xypos[0] - _G(spieler).scrollx + spieler_mi[i].HotX;
		ssi[i + tmp].Y = spieler_vector[i].Xypos[1] - _G(spieler).scrolly;
		atds->set_split_win(i + tmp, &ssi[i + tmp]);
	}
}

int16 ats_action(int16 txt_nr, int16 txt_mode, int16 mode) {
	int16 action_ret;
	if (!atds->get_steuer_bit(txt_nr, ATS_AKTIV_BIT, ATS_DATEI)) {
		action_ret = true;
		if (flags.AtsAction == false) {
			flags.AtsAction = true;
			if (mode == ATS_ACTION_NACH) {
				switch (txt_mode) {
				case TXT_MARK_LOOK:
					switch (txt_nr) {
					case 62:
						action_ret = r39_use_howard();
						break;

					case 67:
						action_ret = r8_gips_wurf();
						break;

					case 174:
						action_ret = Room0::getPillow();
						break;

					case 175:
						action_ret = Room0::pullSlime();
						break;

					case 229:
						auto_move(2, P_CHEWY);
						r39_look_tv(1);
						break;

					case 268:
						auto_move(3, P_CHEWY);
						break;

					case 294:
						switch_room(46);
						break;

					case 298:
					case 307:
					case 308:
						r58_exit();
						break;

					case 299:
						switch_room(59);
						break;

					case 302:
						r59_look_poster();
						break;

					case 316:
						r49_look_hotel();
						break;

					case 402:
						r69_look_schild();
						break;

					case 416:
						r68_look_kaktus();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				case TXT_MARK_USE:
					switch (txt_nr) {
					case 71:
						if (is_cur_inventar(ZANGE_INV))
							r8_hole_kohle();
						else if (!_G(spieler).inv_cur)
							r8_start_verbrennen();
						break;

					case 67:
						if (is_cur_inventar(GIPS_EIMER_INV))
							r8_gips_wurf();
						action_ret = false;
						break;

					case 73:
						if (!_G(spieler).R9Gitter) {
							_G(spieler).PersonHide[P_CHEWY] = true;
							start_detail_wait(5, 1, ANI_VOR);
							_G(spieler).PersonHide[P_CHEWY] = false;
						} else
							action_ret = false;
						break;

					case 77:
						if (!_G(spieler).R10SurimyOk && !_G(spieler).inv_cur) {

							auto_move(3, P_CHEWY);
							flc->set_custom_user_function(r6_cut_serv1);
							flic_cut(FCUT_005, CFO_MODE);
							flc->remove_custom_user_function();
						} else
							action_ret = false;
						break;

					case 79:
						action_ret = r22_chewy_amboss();
						break;

					case 80:
						auto_move(3, P_CHEWY);
						break;

					case 81:
						r22_get_bork();
						break;

					case 82:
						action_ret = r22_malen();
						break;

					case 84:
						action_ret = r11_scanner();
						break;

						break;

					case 92:
						action_ret = r13_monitor_knopf();
						break;

					case 104:
						if (is_cur_inventar(TRANSLATOR_INV)) {
							auto_move(6, P_CHEWY);
							_G(spieler).R14Translator = true;
							start_aad_wait(25, -1);
						} else
							action_ret = false;
						break;

					case 107:
						action_ret = r14_use_gleiter();
						break;

					case 108:
						action_ret = r14_use_schrott();
						break;

					case 109:
						action_ret = r14_use_schleim();
						break;

					case 110:
						action_ret = r23_start_gleiter();
						break;

					case 114:
						switch_room(_G(spieler).R23GleiterExit);
						break;

					case 117:
						action_ret = r12_use_linke_rohr();
						break;

					case 118:
						action_ret = r12_chewy_trans();
						break;

					case 119:
						action_ret = r12_use_terminal();
						break;

					case 123:
						action_ret = r16_use_gleiter();
						break;

					case 133:
						action_ret = r21_use_fenster();
						break;

					case 134:
						r21_salto();
						break;

					case 137:
						action_ret = r17_use_seil();
						break;

					case 139:
						r17_calc_seil();
						break;

					case 142:
						action_ret = r17_energie_hebel();
						break;

					case 146:
						action_ret = r17_get_oel();
						break;

					case 149:
					case 153:
						action_ret = r18_calc_surimy();
						break;

					case 158:
						action_ret = r18_go_cyberspace();
						break;

					case 159:
						action_ret = r18_calc_schalter();
						break;

					case 161:
					case 162:
					case 163:
						r24_use_hebel(txt_nr);
						break;

					case 165:
						if (_G(spieler).R16F5Exit) {
							invent_2_slot(29);
							det->hide_static_spr(19);
							atds->set_steuer_bit(158, ATS_AKTIV_BIT, ATS_DATEI);
						}
						switch_room(18);
						break;

					case 179:
						action_ret = r18_sonden_moni();
						break;

					case 154:
						action_ret = r18_use_cart_moni();
						break;

					case 187:
						action_ret = r25_use_gleiter();
						break;

					case 206:
						action_ret = r28_use_breifkasten();
						break;

					case 210:
						action_ret = r33_use_schublade();
						break;

					case 215:
						action_ret = false;
						r29_use_schlauch();
						break;

					case 218:
						action_ret = r29_get_schlauch();
						break;

					case 219:
						action_ret = r25_gleiter_loesch();
						break;

					case 212:
						action_ret = r29_zaun_sprung();
						break;

					case 220:
						r34_use_kuehlschrank();
						break;

					case 227:
						r34_xit_kuehlschrank();
						break;

					case 221:
						r33_use_maschine();
						break;

					case 231:
						r32_use_schreibmaschine();
						break;

					case 203:
						action_ret = r32_get_script();
						break;

					case 234:
						action_ret = r35_schublade();
						break;

					case 235:
						action_ret = r35_use_cat();
						break;

					case 244:
						r31_open_luke();
						break;

					case 245:
						r31_close_luke();
						break;

					case 242:
						action_ret = r31_use_topf();
						break;

					case 249:
						action_ret = r37_use_wippe();
						break;

					case 251:
						r37_use_hahn();
						break;

					case 264:
						action_ret = r42_use_psack();
						break;

					case 62:
						action_ret = r39_use_howard();
						break;

					case 278:
						action_ret = r40_use_mr_pumpkin();
						break;

					case 279:
						action_ret = r40_use_schalter(205);
						break;

					case 288:
						action_ret = r46_use_schloss();
						break;

					case 286:
					case 292:
					case 293:
						r47_use_knopf(txt_nr);
						break;

					case 294:
						switch_room(46);
						break;

					case 295:
					case 386:
						action_ret = r45_use_taxi();
						break;

					case 297:
						action_ret = r45_use_boy();
						break;

					case 315:
						action_ret = r49_use_taxi();
						break;

					case 318:
						action_ret = r49_use_boy();
						break;

					case 319:
						action_ret = r53_use_man();
						break;

					case 323:
						action_ret = r50_use_gutschein();
						break;

					case 327:
						action_ret = r50_use_gum();
						break;

					case 329:
					case 330:
					case 331:
					case 332:
					case 333:
					case 334:
						action_ret = r51_use_door(txt_nr);
						break;

					case 340:
						r55_strasse(0);
						break;

					case 341:
						action_ret = r52_use_hot_dog();
						break;

					case 343:
						action_ret = r54_use_zelle();
						break;

					case 344:
						action_ret = r57_use_pfoertner();
						break;

					case 345:
						action_ret = r54_use_azug();
						break;

					case 346:
						action_ret = r54_use_schalter();
						break;

					case 347:
					case 387:
						action_ret = r54_use_taxi();
						break;

					case 351:
					case 352:
						action_ret = r55_use_kammeraus();
						break;

					case 354:
						action_ret = r55_use_stapel1();
						break;

					case 355:
						action_ret = r55_use_stapel2();
						break;

					case 357:
						action_ret = r55_use_telefon();
						break;

					case 360:
					case 388:
						action_ret = r57_use_taxi();
						break;

					case 367:
						action_ret = r56_use_taxi();
						break;

					case 366:
						action_ret = r56_use_man();
						break;

					case 371:
						action_ret = r62_use_laura();
						break;

					case 375:
						action_ret = r64_use_tasche();
						break;

					case 380:
						action_ret = r63_use_girl();
						break;

					case 383:
						action_ret = r63_use_aschenbecher();
						break;

					case 384:
						action_ret = r63_use_fx_man();
						break;

					case 385:
						action_ret = r63_use_schalter();
						break;

					case 394:
						action_ret = r67_talk_papagei();
						break;

					case 399:
						action_ret = r67_use_grammo();
						break;

					case 400:
						action_ret = r67_use_kommode();
						break;

					case 403:
					case 405:
						action_ret = r69_use_bruecke();
						break;

					case 406:
						action_ret = r68_use_papagei();
						break;

					case 407:
						action_ret = r68_use_diva();
						break;

					case 410:
						action_ret = r68_use_keeper();
						break;

					case 414:
						action_ret = r68_use_indigo();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				case TXT_MARK_WALK:
					switch (txt_nr) {
					case 94:
						r13_jmp_band();
						break;

					case 95:
						if (_G(spieler).R13Band == true)
							start_aad_wait(116, -1);
						break;

					case 100:
						r13_jmp_boden();
						break;

					case 114:
						switch_room(_G(spieler).R23GleiterExit);
						break;

					case 227:
						r34_xit_kuehlschrank();
						break;

					case 294:
						switch_room(46);
						break;

					case 340:
						r55_strasse(0);
						break;

					case 362:
						action_ret = r56_use_kneipe();
						break;

					case 403:
					case 405:
						action_ret = r69_use_bruecke();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				case TXT_MARK_TALK:
					switch (txt_nr) {
					case 11:
						auto_move(3, P_CHEWY);
						det->stop_detail(5);
						det->start_detail(6, 2, ANI_VOR);
						r2_jump_out_r1(9);
						break;

					case 121:
						r11_talk_debug();
						break;

					case 122:
						if (!_G(spieler).R13BorkOk) {
							auto_move(10, P_CHEWY);
							r13_talk_bork();
						}
						break;

					case 235:
						r35_talk_cat();
						break;

					case 251:
						r37_talk_hahn();
						break;

					case 62:
						r39_talk_howard();
						break;

					case 274:
						r27_talk_howard();
						break;

					case 283:
						r40_talk_handler();
						break;

					case 275:
						r40_talk_police();
						break;

					case 295:
						r45_use_taxi();
						break;

					case 297:
						r45_talk_boy();
						break;

					case 318:
						r49_talk_boy();
						break;

					case 319:
						r53_talk_man();
						break;

					case 323:
						r50_talk_page();
						break;

					case 344:
						r57_talk_pfoertner();
						break;

					case 348:
						r54_talk_verkauf();
						break;

					case 352:
					case 358:
						r55_talk_line();
						break;

					case 366:
						r56_talk_man();
						break;

					case 376:
						r64_talk_man();
						break;

					case 380:
						r63_talk_girl();
						break;

					case 381:
						r63_talk_hunter();
						break;

					case 382:
						r63_talk_regie();
						break;

					case 384:
						r63_talk_fx_man();
						break;

					case 394:
						action_ret = r67_talk_papagei();
						break;

					case 414:
						r68_talk_indigo();
						break;

					case 410:
						r68_talk_keeper();
						break;

					case 408:
						r68_talk_papagei();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				default:
					break;
				}
			} else if (mode == ATS_ACTION_VOR) {
				switch (txt_mode) {
				case TXT_MARK_LOOK:
					switch (txt_nr) {
					case 210:
						r33_look_schublade();
						break;

					case 391:
						auto_move(2, P_CHEWY);
						break;

					case 389:
						r67_look_brief();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				case TXT_MARK_USE:
					switch (txt_nr) {
					case 90:
						atds->set_ats_str(90, TXT_MARK_USE, _G(spieler).R13Band);
						break;

					case 124:
						if (!_G(spieler).R16F5Exit)
							auto_move(5, P_CHEWY);
						break;

					case 134:
						auto_move(10, P_CHEWY);
						break;

					case 210:
						auto_move(1, P_CHEWY);
						break;

					case 216:
						action_ret = r29_use_pumpe();
						break;

					case 225:
						action_ret = r33_get_munter();
						break;

					case 229:
						r39_use_tv();
						break;

					case 230:
						action_ret = r32_use_howard();
						break;

					case 256:
						action_ret = r37_use_glas();
						break;

					case 266:
						action_ret = r41_use_brief();
						break;

					case 263:
						action_ret = r42_use_beamter();
						break;

					case 267:
						action_ret = r41_use_lola();
						break;

					case 269:
						action_ret = r41_use_kasse();
						break;

					case 283:
						action_ret = r40_use_haendler();
						break;

					case 284:
						action_ret = r40_use_bmeister();
						break;

					case 275:
						r40_use_police();
						break;

					case 276:
						action_ret = r40_use_tele();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				case TXT_MARK_WALK:
					action_ret = false;
					/*switch (txt_nr) {
					default:
						action_ret = false;
						break;

					}*/
					break;

				case TXT_MARK_TALK:
					switch (txt_nr) {
					case 67:
						r8_talk_nimoy();
						break;

					case 104:
						r14_talk_eremit();
						break;

					case 120:
						r12_talk_bork();
						break;

					case 263:
						r42_talk_beamter();
						break;

					case 265:
						r41_talk_hoggy1();
						break;

					case 266:
						r41_talk_hoggy2();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				default:
					break;
				}
			}

			flags.AtsAction = false;
		}
	} else
		action_ret = false;
	return action_ret;
}

void ads_action(int16 dia_nr, int16 blk_nr, int16 str_end_nr) {
	if (flags.AdsAction == false) {
		flags.AdsAction = true;
		switch (dia_nr) {

			break;

		case 2:
			if (blk_nr == 4 && str_end_nr == 0) {

				obj->show_sib(31);
			}
			break;

		case 3:
			if (str_end_nr == 1) {
				if (_G(spieler).R11IdCardNr == RED_CARD_INV)
					_G(spieler).R11IdCardNr = YEL_CARD_INV;
				else
					_G(spieler).R11IdCardNr = RED_CARD_INV;
			}
			break;

		case 10:
			r42_dia_beamter(str_end_nr);
			break;

		case 11:
			if (blk_nr == 3) {
				r41_sub_dia();
			} else if (blk_nr == 0 && str_end_nr == 3) {
				_G(spieler).R41RepairInfo = true;
				stop_ads_dialog();
			}
			break;

		case 12:
			if (str_end_nr == 1) {
				_G(spieler).R41HowardDiaOK = true;

			}
			break;

		case 15:
			if (blk_nr == 1 && str_end_nr == 0) {
				_G(spieler).R55Job = true;
				stop_ads_dialog();
			}
			break;

		case 16:
			if (blk_nr == 0) {
				if (str_end_nr == 2) {
					if (!_G(spieler).R56Kneipe)
						atds->del_steuer_bit(362, ATS_AKTIV_BIT, ATS_DATEI);
				}
			}
			break;

		case 17:
			if (blk_nr == 0) {
				if (str_end_nr == 2) {
					atds->show_item(17, 0, 1);
				}
			}
			break;

		case 18:
		case 19:
			if (blk_nr == 0) {
				if (str_end_nr == 4) {
					_G(spieler).R67LiedOk = true;
				}
			}
			break;

		case 20:
			if (blk_nr == 0) {

				if (str_end_nr == 1) {
					_G(spieler).PersonHide[P_CHEWY] = true;
					start_detail_wait(28, 3, ANI_VOR);

					_G(spieler).PersonHide[P_CHEWY] = false;
				}
			}
			break;
		}
		flags.AdsAction = false;
	}
}

void ads_ende(int16 dia_nr, int16 blk_nr, int16 str_end_nr) {
	switch (dia_nr) {
	case 0:
		flags.AutoAniPlay = false;
		break;

	case 5:
		auto_move(6, P_CHEWY);
		break;

	}
}

#define R14_EREMIT_DIA 10000
#define R8_NIMOYANER1_DIA 10001
#define R8_NIMOYANER2_DIA 10002
#define R12_BORK_DIA 10004
#define R11_BORK_DIA 10005
#define R8_NIMOYANER3_DIA 10006
#define R35_CAT_DIA1 10007
#define R35_CAT_DIA2 10008
#define R37_HAHN_DIA 10009
#define R42_BEAMTER_DIA 10010
#define R41_HOOGY_DIA 10011
#define R39_HOWARD_DIA 10012
#define R42_BEAMTER2_DIA 10013
#define R42_BEAMTER3_DIA 10014
#define R61_VERLAG_DIA 10015
#define R56_SEEMAN_DIA 10016
#define R63_GIRL_DIA 10017
#define R67_PAPA1_DIA 10018
#define R67_PAPA2_DIA 10019
#define R68_KEEPER_DIA 10020
#define R13_BORK_DIA 33
void atds_string_start(int16 dia_nr, int16 str_nr, int16 person_nr, int16 mode) {
	int16 talk_stop_ani = -1;
	int16 talk_show_static = -1;
	int16 tmp;
	int16 anz;
	int16 i;
	int16 altes_format;
	int16 talk_ani;
	int16 stop_ani;
	int16 ani_nr;
	talk_ani = -1;
	stop_ani = -1;
	altes_format = false;
	tmp = -1;
	switch (dia_nr) {

	case 30000:
	case 25:
	case 34:
	case 252:
	case 253:
	case 259:
	case 271:
	case 276:
	case 299:
	case 309:
	case 313:
	case 314:
	case 318:
	case 319:
	case 326:
	case 328:
	case 329:
	case 331:
	case 332:
	case 333:
	case 334:
	case 336:
	case 344:
	case 353:
	case 354:
	case 359:
	case 361:
	case 363:
	case 364:
	case 366:
	case 367:
	case 369:
	case 370:
	case 382:
	case 383:
	case 387:
	case 393:
		if (mode == AAD_STR_START) {
			switch (_G(spieler).ChewyAni) {
			case CHEWY_NORMAL:
				ani_nr = CH_TALK3;
				break;

			case CHEWY_ROCKER:
				ani_nr = CH_ROCK_TALK1;
				break;

			case CHEWY_JMANS:
				ani_nr = CH_JM_TALK;
				break;

			case CHEWY_PUMPKIN:
				ani_nr = CH_PUMP_TALK;
				break;

			default:
				ani_nr = -1;
				break;

			}
			if (ani_nr != -1)
				start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
		} else {
			stop_spz();
		}
		break;

	case R8_NIMOYANER3_DIA:
	case 249:
		if (person_nr == 0) {
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK4, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
		}
		break;

	case R8_NIMOYANER1_DIA:
	case R8_NIMOYANER2_DIA:
		altes_format = true;
		switch (person_nr) {
		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 1:
			if (mode == AAD_STR_START) {
				talk_start_ani = 15;
			} else {
				talk_stop_ani = 15;
				talk_start_ani = -1;
			}
			break;

		}
		break;

	case R11_BORK_DIA:
		altes_format = true;
		switch (person_nr) {
		case 1:
			if (mode == AAD_STR_START) {
				talk_start_ani = 9;
				talk_hide_static = 8;
			} else {
				talk_stop_ani = 9;
				talk_show_static = 8;
				talk_start_ani = -1;
				talk_hide_static = -1;
			}
			break;

		}
		break;

	case R13_BORK_DIA:
		altes_format = true;
		switch (person_nr) {
		case 0:
			talk_stop_ani = 9;
			talk_start_ani = 10;
			talk_show_static = 13;
			talk_hide_static = 12;
			break;

		case 1:
			talk_start_ani = 9;
			talk_stop_ani = 10;
			talk_hide_static = 13;
			talk_show_static = 12;
			break;

		}
		break;

	case R14_EREMIT_DIA:
	case 24:
		altes_format = true;
		switch (person_nr) {
		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 1:
			if (mode == AAD_STR_START) {
				talk_start_ani = 4;
				talk_hide_static = 9;
			} else {
				talk_stop_ani = 4;
				talk_show_static = 9;
				talk_start_ani = -1;
				talk_hide_static = -1;
			}
			break;

		}
		break;

	case 171:
	case 173:
	case 174:
	case 175:
		switch (person_nr) {
		case 1:
			if (mode == AAD_STR_START) {
				room->set_timer_status(0, TIMER_STOP);
				det->del_static_ani(0);
				talk_ani = 1;
			} else {
				room->set_timer_status(0, TIMER_START);
				det->stop_detail(1);
				det->set_static_ani(0, -1);
			}
			break;

		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;
		}
		break;

	case R35_CAT_DIA2:
	case R35_CAT_DIA1:
	case R37_HAHN_DIA:
		switch (person_nr) {
		case 1:
			if (mode == AAD_STR_START) {
				room->set_timer_status(1, TIMER_STOP);
				det->del_static_ani(1);
				talk_ani = 0;
			} else {
				room->set_timer_status(1, TIMER_START);
				det->stop_detail(0);
				det->set_static_ani(1, -1);
			}
			break;

		}
		break;

	case 169:

		switch (person_nr) {
		case 1:
			if (mode == AAD_STR_START) {
				det->start_detail(5, 255, ANI_VOR);
			} else {
				det->stop_detail(5);
			}
			break;

		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;
		}
		break;

	case 134:
	case 133:
	case 132:
	case 131:
	case 130:
	case 129:
	case 128:
	case 127:
	case 126:
	case 161:
	case 162:
	case 163:
	case 186:
	case R41_HOOGY_DIA:
		switch (person_nr) {
		case 1:
		case 2:
		case 4:
			if (person_nr == 4) {
				anz = 2;
				tmp = 3;
			} else {
				anz = 1;
				tmp = person_nr + 2;
			}
			for (i = 0; i < anz; i++) {
				if (mode == AAD_STR_START) {
					det->start_detail(tmp, 255, ANI_VOR);
				} else {
					det->stop_detail(tmp);
				}
				++tmp;
			}
			break;

		case 0:
			if (mode == AAD_STR_START) {
				if (_G(spieler).R28ChewyPump)
					ani_nr = CH_PUMP_TALK;
				else
					ani_nr = CH_TALK3;
				start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;
		}
		break;

	case 176:
	case 177:
	case 178:
	case 179:
	case 181:
	case 182:
	case 183:
	case 184:
	case 185:
	case 187:
	case 193:
	case 194:
	case 195:
	case 196:
	case 197:
	case 198:
	case 199:
	case 202:
	case 205:
	case 208:
	case 209:
	case 210:
	case 211:
	case 213:
	case 214:
	case 215:
	case 216:
	case 217:
	case 218:
	case 219:
	case 220:
	case 221:
	case 222:
	case 223:
	case 224:
	case 225:
	case 226:
	case 227:
	case 241:
	case 240:
	case 242:
	case 243:
	case 251:
	case 257:
	case 258:
	case 254:
	case 260:
	case 256:
	case 261:
	case 262:
	case 263:
	case 264:
	case 265:
	case 266:
	case 267:
	case 268:
	case 269:
	case 270:
	case 272:
	case 273:
	case 274:
	case 275:
	case 277:
	case 279:
	case 280:
	case 281:
	case 282:
	case 283:
	case 284:
	case 285:
	case 288:
	case 289:
	case 290:
	case 291:
	case 295:
	case 296:
	case 297:
	case 298:
	case 304:
	case 305:
	case 306:
	case 308:
	case 310:
	case 311:
	case 312:
	case 315:
	case 316:
	case 317:
	case 320:
	case 321:
	case 322:
	case 323:
	case 324:
	case 325:
	case 335:
	case 337:
	case 338:
	case 339:
	case 340:
	case 341:
	case 342:
	case 343:
	case 350:
	case 351:

	case 358:
	case 365:
	case 237:
	case 375:
	case 376:
	case 377:
	case 378:
	case 379:
	case 380:
	case 384:
	case 385:
	case 386:
	case 388:
	case 389:
	case 390:
	case 391:
	case 392:
	case 394:
	case 395:
	case 396:
	case 397:
	case 398:
	case 400:
	case 402:
	case R42_BEAMTER2_DIA:
	case R42_BEAMTER3_DIA:
	case R56_SEEMAN_DIA:
	case R63_GIRL_DIA:
	case R67_PAPA1_DIA:
	case R67_PAPA2_DIA:

		switch (person_nr) {
		case 2:
			switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
			case 42:
				if (mode == AAD_STR_START) {
					room->set_timer_status(8, TIMER_STOP);
					det->del_static_ani(8);
					talk_ani = 2;
				} else {
					room->set_timer_status(8, TIMER_START);
					det->stop_detail(2);
					det->set_static_ani(8, -1);
				}
				break;

			case 28:
				if (mode == AAD_STR_START) {
					talk_ani = dia_nr - 194;
				} else {
					det->stop_detail(dia_nr - 194);
				}
				break;

			case 40:
				switch (dia_nr) {
				case 202:
				case 209:
				case 211:
					if (mode == AAD_STR_START) {
						talk_ani = 3;
					} else {
						det->stop_detail(3);
					}
					break;

				case 213:
					if (mode == AAD_STR_START) {
						talk_ani = 5;
					} else {
						det->stop_detail(5);
					}
					break;

				case 205:
				case 227:
					if (mode == AAD_STR_START) {
						talk_ani = 14;
					} else {
						det->stop_detail(14);
					}
					break;

				case 215:
				case 237:
				case 375:
					if (mode == AAD_STR_START) {
						talk_ani = 2;
					} else {
						det->stop_detail(2);
					}
					break;

				}
				break;

			case 45:
				switch (dia_nr) {
				case 257:
				case 258:
				case 259:
					if (mode == AAD_STR_START) {
						room->set_timer_status(0, TIMER_STOP);
						det->del_static_ani(0);
						talk_ani = 2;
					} else {
						room->set_timer_status(0, TIMER_START);
						det->stop_detail(2);
						det->set_static_ani(0, -1);
					}
					break;

				case 254:
				case 260:
				case 256:
				case 400:
					if (mode == AAD_STR_START) {

						talk_ani = 14;
					} else {

						det->stop_detail(14);

					}
					break;

				}
				break;

			case 49:
				if (mode == AAD_STR_START) {
					talk_ani = 2;
				} else {
					det->stop_detail(2);
				}
				break;

			case 53:
				if (mode == AAD_STR_START) {
					talk_ani = 3;
				} else {
					det->stop_detail(3);
				}
				break;

			case 50:
				switch (dia_nr) {
				case 272:
				case 273:
				case 275:
					tmp = 5;
					break;

				case 274:
					tmp = 8;
					break;

				case 277:
					tmp = 10;
					break;

				}
				if (mode == AAD_STR_START) {
					talk_ani = tmp;
				} else {
					det->stop_detail(tmp);
				}
				break;

			case 55:
				switch (dia_nr) {
				case 323:
					tmp = 16;
					break;

				case 335:
					tmp = 18;
					break;

				case 320:
					tmp = 21;
					break;

				}
				if (mode == AAD_STR_START) {
					talk_ani = tmp;
				} else {
					det->stop_detail(tmp);
				}
				break;

			case 57:
				if (mode == AAD_STR_START) {
					talk_ani = 1;
				} else {
					det->stop_detail(1);
				}
				break;

			case 56:
				switch (dia_nr) {
				case 304:
					tmp = 5;
					break;

				case 305:
				case 343:
				case R56_SEEMAN_DIA:
					tmp = 1;
					break;

				case 306:
					tmp = 3;
					break;

				}
				if (mode == AAD_STR_START) {
					talk_ani = tmp;
				} else {
					det->stop_detail(tmp);
				}
				break;

			case 63:
				switch (dia_nr) {
				case 358:
					tmp = 5;
					break;

				case 365:
				case R63_GIRL_DIA:
					tmp = 14;
					break;

				}
				if (mode == AAD_STR_START) {
					talk_ani = tmp;
				} else {
					det->stop_detail(tmp);
				}
				break;

			case 64:
				if (mode == AAD_STR_START) {
					talk_ani = 2;
				} else {
					det->stop_detail(2);
				}
				break;

			case 67:
				if (mode == AAD_STR_START) {
					talk_ani = 1;
				} else {
					det->stop_detail(1);
				}
				break;

			case 68:
				switch (dia_nr) {
				case 384:
				case 385:
					tmp = 9;
					break;

				case 394:
				case 397:
				case 398:
					tmp = 12;
					break;

				case 386:
					tmp = 3;
					break;

				}
				if (mode == AAD_STR_START) {
					talk_ani = tmp;
				} else {
					det->stop_detail(tmp);
				}
				break;

			}
			break;

		case 1:
			if (mode == AAD_STR_START) {
				start_spz(HO_TALK_L, 255, ANI_VOR, P_HOWARD);
			} else {
				stop_spz();
			}
			break;

		case 3:
			if (mode == AAD_STR_START) {
				start_spz(NI_TALK_L, 255, ANI_VOR, P_NICHELLE);
			} else {
				stop_spz();
			}
			break;

		case 0:
			if (mode == AAD_STR_START) {
				switch (_G(spieler).ChewyAni) {
				case CHEWY_NORMAL:
					ani_nr = CH_TALK3;
					break;

				case CHEWY_PUMPKIN:
					ani_nr = CH_PUMP_TALK;
					break;

				case CHEWY_ROCKER:
					ani_nr = CH_ROCK_TALK1;
					break;

				case CHEWY_JMANS:
					ani_nr = CH_JM_TALK;
					break;

				default:
					ani_nr = -1;
					break;

				}
				if (ani_nr != -1)
					start_spz(ani_nr, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;
		}
		break;

	case R61_VERLAG_DIA:
		if (mode == AAD_STR_START) {
			det->start_detail(person_nr, 255, ANI_VOR);
		} else {
			det->stop_detail(person_nr);
		}
		break;

		break;

	case R68_KEEPER_DIA:
		switch (person_nr) {
		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_VOR, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 2:
			if (mode == AAD_STR_START) {
				talk_ani = 16;
			} else {
				det->stop_detail(16);
			}
			break;

		case 3:
			if (mode == AAD_STR_START) {
				_G(spieler).PersonHide[P_CHEWY] = true;
				talk_ani = 29;
			} else {
				det->stop_detail(29);
				_G(spieler).PersonHide[P_CHEWY] = false;
			}
			break;

		}
		break;
	}
	if (altes_format) {
		det->start_detail(talk_start_ani, 255, ANI_VOR);
		det->stop_detail(talk_stop_ani);
		det->hide_static_spr(talk_hide_static);
		det->show_static_spr(talk_show_static);
	} else {
		det->start_detail(talk_ani, 255, ANI_VOR);
		det->stop_detail(stop_ani);
	}
}

void calc_inv_use_txt(int16 test_nr) {
	int16 ret;
	switch (test_nr) {
	case GBUCH_OPEN_INV:
	case MONOKEL_INV:
		room->open_handle("BACK/GBOOK.TGP", "rb", R_TGPDATEI);
		ERROR
		room->load_tgp(BUCH_START, &room_blk, GBOOK_TGP, 0);
		ERROR;
		out->setze_zeiger(workptr);
		out->map_spr2screen(ablage[room_blk.AkAblage], _G(spieler).scrollx, _G(spieler).scrolly);
		out->back2screen(workpage);
		while (in->get_switch_code() != ESC);
		while (in->get_switch_code() != 0);
		room->open_handle(&background[0], "rb", R_TGPDATEI);
		ERROR
		room->load_tgp(_G(spieler).PersonRoomNr[P_CHEWY], &room_blk, EPISODE1_TGP, GED_LOAD);
		ERROR;
		break;

	case ANGEL_INV:
	case KNOCHEN_INV:
		del_inventar(_G(spieler).AkInvent);
		menu_item = CUR_USE;
		cursor_wahl(menu_item);
		ret = del_invent_slot(test_nr);
		_G(spieler).InventSlot[ret] = ANGEL2_INV;
		obj->change_inventar(test_nr, ANGEL2_INV, &room_blk);
		break;

	case KUERBIS1_INV:
		ret = del_invent_slot(KUERBIS1_INV);
		_G(spieler).InventSlot[ret] = K_MASKE_INV;
		obj->change_inventar(KUERBIS1_INV, K_MASKE_INV, &room_blk);
		invent_2_slot(K_FLEISCH_INV);
		invent_2_slot(K_KERNE_INV);
		break;

	case MESSER_INV:
		del_inventar(_G(spieler).AkInvent);
		menu_item = CUR_USE;
		cursor_wahl(menu_item);
		invent_2_slot(K_MASKE_INV);
		invent_2_slot(K_FLEISCH_INV);
		invent_2_slot(K_KERNE_INV);
		break;

	case BRIEF_INV:
		del_inventar(_G(spieler).AkInvent);
		menu_item = CUR_USE;
		cursor_wahl(menu_item);
		_G(spieler).R42BriefMarke = true;
		ret = del_invent_slot(BRIEF_INV);
		_G(spieler).InventSlot[ret] = BRIEF2_INV;
		obj->change_inventar(BRIEF_INV, BRIEF2_INV, &room_blk);
		break;

	case FLASCHE_INV:
		del_inventar(_G(spieler).AkInvent);
		menu_item = CUR_USE;
		cursor_wahl(menu_item);
	case WOLLE_INV:
		del_invent_slot(WOLLE_INV);
		atds->set_ats_str(FLASCHE_INV, 1, INV_ATS_DATEI);
		_G(spieler).R56WhiskyMix = true;
		break;

	case B_MARY_INV:
	case PIRANHA_INV:
		del_inventar(_G(spieler).AkInvent);
		menu_item = CUR_USE;
		cursor_wahl(menu_item);
		ret = del_invent_slot(test_nr);
		_G(spieler).InventSlot[ret] = B_MARY2_INV;
		obj->change_inventar(test_nr, B_MARY2_INV, &room_blk);
		break;

	}
}

bool calc_inv_no_use(int16 test_nr, int16 mode) {
	int16 inv_mode;
	int16 txt_nr = 0;
	bool ret = false;
	int16 r_val = 0;
	int16 ok;
	ret = false;
	switch (mode) {
	case INVENTAR_NORMAL:
		inv_mode = IUID_IIB;
		break;

	case INVENTAR_STATIC:
		inv_mode = IUID_SIB;
		break;

	case DETEDIT_REC:
		inv_mode = IUID_REC;
		break;

	case AUTO_OBJ:
		inv_mode = IUID_AUTO;
		break;

	case DETAIL_OBJ:
		inv_mode = IUID_DET;
		break;

	case SPIELER_OBJ:
		inv_mode = IUID_SPIELER;
		break;

	default:
		inv_mode = -1;
		break;

	}
	if (inv_mode != -1) {
		txt_nr = atds->calc_inv_no_use(_G(spieler).AkInvent, test_nr, inv_mode);
		ERROR
		if (txt_nr != -1) {
			if (!flags.InventMenu) {
				if (txt_nr >= 15000) {
					ret = start_ats_wait(txt_nr - 15000, TXT_MARK_USE, 14, INV_USE_DEF);
				} else {
					ret = start_ats_wait(txt_nr, TXT_MARK_USE, 14, INV_USE_DATEI);
				}
				ERROR
			} else {
				look_invent(-1, INV_USE_ATS_MODE, txt_nr);
			}
		} else {
			ok = false;
			if (inv_mode == IUID_SPIELER)
				ok = calc_person_click(test_nr);
			if (!ok) {
				warning("FIXME - use of random");

				ret = start_ats_wait(RAND_NO_USE[r_val], TXT_MARK_USE, 14, INV_USE_DEF);
			}
		}
	}
	return ret;
}

int16 calc_person_txt(int16 p_nr) {
	int16 txt_nr = -1;
	switch (p_nr) {
	case P_CHEWY:
		switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
		case 40:
		case 41:
		case 42:
			txt_nr = 219 + _G(spieler).PersonRoomNr[P_CHEWY];
			break;

		default:
			txt_nr = -1;
			break;

		}
		break;

	case P_HOWARD:
		if (menu_item == CUR_USE) {
			switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
			case 40:
			case 67:
				menu_item = CUR_HOWARD;
				cursor_wahl(menu_item);
				txt_nr = 30000;
				break;

			case 42:
				if (!_G(spieler).R42MarkeOk) {
					menu_item = CUR_HOWARD;
					cursor_wahl(menu_item);
					txt_nr = 30000;
				}
				break;

			}
		}
		break;

	case P_NICHELLE:
		if (menu_item == CUR_USE) {
			switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
			case 67:
				menu_item = CUR_NICHELLE;
				cursor_wahl(menu_item);
				txt_nr = 30000;
				break;

			default:
				break;

			}
		}
		break;

	default:
		break;
	}

	return txt_nr;
}

int16 calc_person_click(int16 p_nr) {
	int16 action_ret = false;
	switch (p_nr) {
	case P_CHEWY:
		switch (_G(spieler).AkInvent) {
		case K_MASKE_INV:
			r28_set_pump();
			action_ret = true;
			break;

		default:
			action_ret = false;
			break;

		}
		break;

	case P_HOWARD:
		switch (_G(spieler).AkInvent) {
		case GALA_INV:
			if (_G(spieler).PersonRoomNr[P_CHEWY] == 67) {
				r67_kostuem_aad(378);
				action_ret = true;
			}
			break;

		default:
			action_ret = false;
			break;

		}
		break;

	case P_NICHELLE:
		switch (_G(spieler).AkInvent) {
		case GALA_INV:
			if (_G(spieler).PersonRoomNr[P_CHEWY] == 67) {
				r67_kostuem_aad(377);
				action_ret = true;
			} else if (_G(spieler).PersonRoomNr[P_CHEWY] == 68) {
				r68_kostuem_aad(387);
				action_ret = true;
			}
			break;

		default:
			action_ret = false;
			break;

		}
		break;

	default:
		action_ret = false;
		break;

	}
	return action_ret;
}

void calc_person_dia(int16 p_nr) {
	int16 room_nr;
	room_nr = false;
	switch (p_nr) {
	case P_HOWARD:
	case P_NICHELLE:
		switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
		case 69:
			_G(spieler).PersonDia[p_nr] = 381;
			break;

		default:
			_G(spieler).PersonDia[p_nr] = _G(spieler).PersonGlobalDia[p_nr];
			room_nr = _G(spieler).PersonDiaRoom[p_nr];
			break;

		}
		atds->set_string_end_func(&r65_atds_string_start);
		if (!room_nr) {
			if (_G(spieler).DiaAMov != -1)
				auto_move(_G(spieler).DiaAMov, P_CHEWY);
			room_nr = _G(spieler).PersonDiaRoom[p_nr];
			_G(spieler).PersonDiaRoom[p_nr] = false;
			start_aad_wait(_G(spieler).PersonDia[p_nr], -1);
			_G(spieler).PersonDiaRoom[p_nr] = room_nr;
		} else {
			room_blk.AadLoad = false;
			room_blk.AtsLoad = false;
			_G(spieler).PersonDiaTmpRoom[p_nr] = _G(spieler).PersonRoomNr[P_CHEWY];
			save_person_rnr();
			if (p_nr == P_HOWARD)
				switch_room(65);
			else if (p_nr == P_NICHELLE) {
				if (_G(spieler).PersonDia[P_NICHELLE] < 10000) {
					_G(cur_hide_flag) = false;
					hide_cur();
					start_aad_wait(_G(spieler).PersonDia[P_NICHELLE], -1);
					show_cur();
				} else {
					start_ads_wait(_G(spieler).PersonDia[P_NICHELLE] - 10000);
				}
			}
		}
		atds->set_string_end_func(&atds_string_start);
		break;

	}
}

} // namespace Chewy
