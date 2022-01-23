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

#include "chewy/chewy.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/global.h"
#include "chewy/rooms/rooms.h"

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
					case 229:
						auto_move(2, P_CHEWY);
						Room39::look_tv(1);
						break;

					case 268:
						auto_move(3, P_CHEWY);
						break;

					case 294:
						switch_room(46);
						break;

					case 299:
						switch_room(59);
						break;

					case 302:
						Room59::look_poster();
						break;

					case 298:
					case 307:
					case 308:
						Room58::xit();
						break;

					case 316:
						Room49::look_hotel();
						break;

					case 362:
						Room56::use_kneipe();
						break;

					case 402:
						Room69::look_schild();
						break;

					case 416:
						Room68::look_kaktus();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				case TXT_MARK_USE:
					switch (txt_nr) {
					case 62:
						action_ret = Room39::use_howard();
						break;

					case 67:
						action_ret = Room8::gips_wurf();
						break;

					case 71:
						if (is_cur_inventar(ZANGE_INV))
							Room8::hole_kohle();
						else if (!_G(spieler).inv_cur)
							Room8::start_verbrennen();
						break;

					case 73:
						if (!_G(spieler).R9Gitter) {
							_G(spieler).PersonHide[P_CHEWY] = true;
							start_detail_wait(5, 1, ANI_VOR);
							_G(spieler).PersonHide[P_CHEWY] = false;
						} else {
							action_ret = false;
						}
						break;


					case 77:
						if (!_G(spieler).R10SurimyOk && !_G(spieler).inv_cur) {
							hide_cur();
							auto_move(3, P_CHEWY);
							flc->set_custom_user_function(Room6::cut_serv1);
							flic_cut(FCUT_005, CFO_MODE);
							flc->remove_custom_user_function();
							start_spz(4, -1, false, P_CHEWY);
							start_aad_wait(108, 0);
							show_cur();
						} else {
							action_ret = false;
						}
						break;

					case 79:
						action_ret = Room22::chewy_amboss();
						break;

					case 80:
						if (_G(spieler).inv_cur)
							auto_move(3, P_CHEWY);
						break;

					case 81:
						Room22::get_bork();
						break;

					case 82:
						action_ret = Room22::malen();
						break;

					case 84:
						action_ret = Room11::scanner();
						break;

					case 92:
						action_ret = Room13::monitor_knopf();
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
						action_ret = Room14::use_gleiter();
						break;

					case 108:
						action_ret = Room14::use_schrott();
						break;

					case 109:
						action_ret = Room14::use_schleim();
						break;

					case 110:
						action_ret = Room23::start_gleiter();
						break;

					case 114:
						switch_room(_G(spieler).R23GleiterExit);
						break;

					case 117:
						action_ret = Room12::chewy_trans();
						break;

					case 118:
						action_ret = Room12::useTransformerTube();
						break;

					case 119:
						action_ret = Room12::use_terminal();
						break;

					case 123:
						action_ret = Room16::use_gleiter();
						break;

					case 133:
						action_ret = Room21::use_fenster();
						break;

					case 134:
						Room21::salto();
						break;

					case 137:
						action_ret = Room17::use_seil();
						break;

					case 139:
						Room17::calc_seil();
						break;

					case 142:
						action_ret = Room17::energie_hebel();
						break;

					case 146:
						action_ret = Room17::get_oel();
						break;

					case 149:
					case 153:
						action_ret = Room18::calc_surimy();
						break;

					case 154:
						action_ret = Room18::use_cart_moni();
						break;

					case 158:
						action_ret = Room18::go_cyberspace();
						break;

					case 159:
						action_ret = Room18::calc_schalter();
						break;

					case 161:
					case 162:
					case 163:
						Room24::use_hebel(txt_nr);
						break;

					case 165:
						if (_G(spieler).R16F5Exit) {
							invent_2_slot(29);
							det->hide_static_spr(19);
							atds->set_steuer_bit(158, ATS_AKTIV_BIT, ATS_DATEI);
						}
						switch_room(18);
						break;

					case 174:
						action_ret = Room0::getPillow();
						break;

					case 175:
						action_ret = Room0::pullSlime();
						break;

					case 179:
						action_ret = Room18::sonden_moni();
						break;

					case 187:
						action_ret = Room25::use_gleiter();
						break;

					case 203:
						action_ret = Room32::get_script();
						break;

					case 206:
						action_ret = Room28::use_breifkasten();
						break;

					case 210:
						action_ret = Room33::use_schublade();
						break;

					case 212:
						action_ret = Room29::zaun_sprung();
						break;

					case 215:
						action_ret = Room29::use_schlauch();
						break;

					case 216:
						action_ret = Room29::use_pumpe();
						break;

					case 218:
						action_ret = Room29::get_schlauch();
						break;

					case 219:
						action_ret = Room25::gleiter_loesch();
						break;

					case 220:
						action_ret = Room34::use_kuehlschrank();
						break;

					case 221:
						Room33::use_maschine();
						break;

					case 227:
						Room34::xit_kuehlschrank();
						break;

					case 229:
						action_ret = Room39::use_tv();
						break;

					case 230:
						action_ret = Room32::use_howard();
						break;

					case 231:
						Room32::use_schreibmaschine();
						break;

					case 234:
						action_ret = Room35::schublade();
						break;

					case 235:
						action_ret = Room35::use_cat();
						break;

					case 242:
						action_ret = Room31::use_topf();
						break;

					case 244:
						Room31::open_luke();
						break;

					case 245:
						Room31::proc1();
						break;

					case 249:
						action_ret = Room37::use_wippe();
						break;

					case 251:
						Room37::use_hahn();
						break;

					case 256:
						action_ret = Room37::use_glas();
						break;

					case 263:
						action_ret = Room42::use_beamter();
						break;

					case 264:
						action_ret = Room42::use_psack();
						break;

					case 266:
						action_ret = Room41::use_brief();
						break;

					case 267:
						action_ret = Room41::use_lola();
						break;

					case 269:
						action_ret = Room41::use_kasse();
						break;

					case 275:
						action_ret = Room40::use_police();
						break;

					case 276:
						action_ret = Room40::use_tele();
						break;

					case 278:
						action_ret = Room40::use_mr_pumpkin();
						break;

					case 284:
						action_ret = Room40::use_bmeister();
						break;

					case 286:
					case 289:
					case 290:
					case 291:
						Room47::use_knopf(txt_nr);
						break;

					case 288:
						action_ret = Room46::use_schloss();
						break;

					case 294:
						switch_room(46);
						break;

					case 295:
					case 297:
						action_ret = Room45::use_boy();
						break;

					case 315:
						action_ret = Room49::use_taxi();
						break;

					case 318:
						action_ret = Room49::use_boy();
						break;

					case 319:
						action_ret = Room53::use_man();
						break;

					case 323:
						action_ret = Room50::use_gutschein();
						break;

					case 327:
						action_ret = Room50::use_gum();
						break;

					case 329:
					case 330:
					case 331:
					case 332:
					case 333:
					case 334:
						action_ret = Room51::use_door(txt_nr);
						break;

					case 340:
						Room55::strasse(0);
						break;

					case 341:
						action_ret = Room52::use_hot_dog();
						break;

					case 343:
						action_ret = Room54::use_zelle();
						break;

					case 344:
						action_ret = Room57::use_pfoertner();
						break;

					case 345:
						action_ret = Room54::use_azug();
						break;

					case 346:
						action_ret = Room54::use_schalter();
						break;

					case 347:
					case 387:
						action_ret = Room54::use_taxi();
						break;

					case 351:
					case 352:
						action_ret = Room55::use_kammeraus();
						break;

					case 354:
						action_ret = Room55::use_stapel1();
						break;

					case 355:
						action_ret = Room55::use_stapel2();
						break;

					case 357:
						action_ret = Room55::use_telefon();
						break;

					case 360:
					case 388:
						action_ret = Room57::use_taxi();
						break;

					case 362:
						Room56::use_kneipe();
						break;

					case 366:
						action_ret = Room56::use_man();
						break;

					case 367:
						action_ret = Room56::use_taxi();
						break;

					case 371:
						action_ret = Room62::use_laura();
						break;

					case 375:
						action_ret = Room64::use_tasche();
						break;

					case 380:
						action_ret = Room63::use_girl();
						break;

					case 383:
						action_ret = Room63::use_aschenbecher();
						break;

					case 384:
						action_ret = Room63::use_fx_man();
						break;

					case 385:
						action_ret = Room63::use_schalter();
						break;

					case 394:
						action_ret = Room67::talk_papagei();
						break;

					case 399:
						action_ret = Room67::use_grammo();
						break;

					case 400:
						action_ret = Room67::use_kommode();
						break;

					case 403:
					case 405:
						action_ret = Room69::use_bruecke();
						break;

					case 406:
						action_ret = Room68::use_papagei();
						break;

					case 407:
						action_ret = Room68::use_diva();
						break;

					case 410:
						action_ret = Room68::use_keeper();
						break;

					case 414:
						action_ret = Room68::use_indigo();
						break;

					case 425:
						action_ret = Room66::proc7();
						break;

					case 430:
						action_ret = Room73::proc1();
						break;

					case 433:
						action_ret = Room73::proc2();
						break;

					case 435:
						action_ret = Room74::proc1();
						break;

					case 442:
						action_ret = Room77::proc1();
						break;

					case 443:
						action_ret = Room71::proc6();
						break;

					case 450:
						action_ret = Room76::proc6();
						break;

					case 452:
						action_ret = Room76::proc7();
						break;

					case 463:
						action_ret = Room77::proc1();
						break;

					case 464:
						action_ret = Room77::proc2();
						break;

					case 467:
					case 473:
						action_ret = Room82::proc9();
						break;

					case 468:
						action_ret = Room82::proc6();
						break;

					case 471:
						action_ret = Room82::proc3();
						break;


					case 481:
					case 482:
						action_ret = Room84::proc4();
						break;

					case 487:
						Room81::proc1();
						break;

					case 490:
						action_ret = Room81::proc2();
						break;

					case 492:
						action_ret = Room88::proc2();
						break;

					case 493:
						action_ret = Room88::proc3();
						break;

					case 494:
						action_ret = Room88::proc1();
						break;

					case 495:
						action_ret = Room85::proc2();
						break;

					case 497:
						action_ret = Room86::proc2();
						break;

					case 501:
					case 503:
						action_ret = Room87::proc2(txt_nr);
						break;

					case 502:
						action_ret = Room87::proc4();
						break;

					case 508:
						action_ret = Room89::proc2();
						break;

					case 509:
						action_ret = Room89::proc5();
						break;

					case 514:
						action_ret = Room89::proc4();
						break;

					case 517:
						action_ret = Room90::proc3();
						break;

					case 519:
						action_ret = Room90::proc4();
						break;

					case 520:
						action_ret = Room90::proc6();
						break;

					case 522:
						action_ret = Room94::proc3();
						break;

					case 525:
						action_ret = Room95::proc2();
						break;

					case 530:
						action_ret = Room97::proc8();
						break;

					case 531:
						action_ret = Room97::proc6();
						break;

					case 535:
						action_ret = Room97::proc11();
						break;

					case 538:
						action_ret = Room97::proc7();
						break;

					case 539:
						action_ret = Room97::proc9();
						break;

					case 541:
						action_ret = Room97::proc14();
						break;

					case 542:
						action_ret = Room97::proc5();
						break;

					case 543:
						action_ret = Room97::proc10();
						break;

					default:
						action_ret = false;
						break;

					}
					break;

				case TXT_MARK_WALK:
					switch (txt_nr) {
					case 94:
						Room13::jmp_band();
						break;

					case 95:
						if (_G(spieler).R13Band == true)
							start_aad_wait(116, -1);
						break;

					case 100:
						Room13::jmp_boden();
						break;

					case 114:
						switch_room(_G(spieler).R23GleiterExit);
						break;

					case 227:
						Room34::xit_kuehlschrank();
						break;

					case 294:
						switch_room(46);
						break;

					case 340:
						Room55::strasse(0);
						break;

					case 362:
						action_ret = Room56::use_kneipe();
						break;

					case 403:
					case 405:
						action_ret = Room69::use_bruecke();
						break;

					case 487:
					case 489:
						Room81::proc1();
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
						Room2::jump_out_r1(9);
						break;

					case 62:
						Room39::talk_howard();
						break;

					case 67:
						Room8::talk_nimoy();
						break;

					case 121:
						Room11::talk_debug();
						break;

					case 122:
						if (!_G(spieler).R13BorkOk) {
							auto_move(10, P_CHEWY);
							Room13::talk_bork();
						}
						break;

					case 235:
						Room35::talk_cat();
						break;

					case 251:
						Room37::talk_hahn();
						break;

					case 265:
						Room41::talk_hoggy1();
						break;

					case 266:
						Room41::talk_hoggy2();
						break;

					case 274:
						Room27::talk_howard();
						break;

					case 275:
						Room40::talk_police();
						break;

					case 283:
						Room40::talk_handler();
						break;

					case 295:
						Room45::use_taxi();
						break;

					case 297:
						Room45::talk_boy();
						break;

					case 318:
						Room49::talk_boy();
						break;

					case 319:
						Room53::talk_man();
						break;

					case 323:
						Room50::talk_page();
						break;

					case 344:
						Room57::talk_pfoertner();
						break;

					case 348:
						Room54::talk_verkauf();
						break;

					case 352:
					case 358:
						Room55::talk_line();
						break;

					case 366:
						Room56::talk_man();
						break;

					case 376:
						Room64::talk_man();
						break;

					case 380:
						Room63::talk_girl();
						break;

					case 381:
						Room63::talk_hunter();
						break;

					case 382:
						Room63::talk_regie();
						break;

					case 384:
						Room63::talk_fx_man();
						break;

					case 394:
						action_ret = Room67::talk_papagei();
						break;

					case 408:
						Room68::talk_papagei();
						break;

					case 410:
						Room68::talk_keeper();
						break;

					case 414:
						Room68::talk_indigo();
						break;

					case 419:
						Room66::talk1();
						break;

					case 420:
						Room66::talk2();
						break;

					case 421:
						Room66::talk3();
						break;

					case 425:
						Room66::talk4();
						break;

					case 447:
					case 448:
						Room76::talk1();
						break;

					case 458:
						Room76::talk2();
						break;

					case 468:
						Room82::talk1();
						break;

					case 469:
						Room82::talk2();
						break;

					case 471:
						Room82::talk3();
						break;

					case 478:
						Room84::talk1();
						break;

					case 504:
						Room84::talk2();
						break;

					case 505:
						start_aad_wait(482, -1);
						break;

					case 506:
						start_aad_wait(483, -1);
						break;

					case 512:
						Room89::talk1();
						break;

					case 522:
						Room94::talk1();
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
						Room33::look_schublade();
						break;

					case 389:
						Room67::look_brief();
						break;

					case 391:
						auto_move(2, P_CHEWY);
						break;

					case 431:
						auto_move(3, P_CHEWY);
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

					case 225:
						action_ret = Room33::get_munter();
						break;

					case 267:
						if (!_G(spieler).R41LolaOk && _G(spieler).R41RepairInfo)
							atds->set_ats_str(267, 1, ATS_DATEI);
						break;

					case 283:
						action_ret = Room40::use_haendler();
						break;

					case 423:
						action_ret = Room66::proc2();
						break;

					default:
						action_ret = false;
						break;
					}
					break;

				case TXT_MARK_WALK:
					action_ret = false;
					break;

				case TXT_MARK_TALK:
					switch (txt_nr) {
					case 104:
						Room14::talk_eremit();
						break;

					case 120:
						Room12::talk_bork();
						break;

					case 263:
						Room42::talk_beamter();
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
	} else {
		action_ret = false;
	}

	return action_ret;
}

void ads_action(int16 dia_nr, int16 blk_nr, int16 str_end_nr) {
	if (flags.AdsAction == false) {
		flags.AdsAction = true;

		switch (dia_nr) {
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
			Room42::dia_beamter(str_end_nr);
			break;

		case 11:
			if (blk_nr == 3) {
				Room41::sub_dia();
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

		default:
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

	case 22:
		if (str_end_nr == 1) {
			det->del_static_ani(3);
			start_detail_wait(5, 1, ANI_VOR);
			det->set_static_ani(3, -1);
			start_aad_wait(456, -1);
		}
		break;

	default:
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
	int scrollx, scrolly;
	int16 ret;

	switch (test_nr) {
	case GBUCH_OPEN_INV:
	case MONOKEL_INV:
		scrollx = _G(spieler).scrollx;
		scrolly = _G(spieler).scrolly;
		_G(spieler).scrollx = 0;
		_G(spieler).scrolly = 0;

		room->open_handle("BACK/GBOOK.TGP", "rb", R_TGPDATEI);
		ERROR
		room->load_tgp(BUCH_START, &room_blk, GBOOK_TGP, 0);
		ERROR;
		out->setze_zeiger(workptr);
		out->map_spr2screen(ablage[room_blk.AkAblage], _G(spieler).scrollx, _G(spieler).scrolly);
		out->back2screen(workpage);

		while (in->get_switch_code() != ESC) {
			g_events->update();
			SHOULD_QUIT_RETURN;
		}
		while (in->get_switch_code() != 0) {
			g_events->update();
			SHOULD_QUIT_RETURN;
		}

		room->open_handle(EPISODE1, "rb", R_TGPDATEI);
		ERROR
		room->load_tgp(_G(spieler).PersonRoomNr[P_CHEWY], &room_blk, EPISODE1_TGP, GED_LOAD);
		ERROR;

		_G(spieler).scrollx = scrollx;
		_G(spieler).scrolly = scrolly;
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
		if (_G(spieler).AkInvent == 40) {
			del_inventar(_G(spieler).AkInvent);
			menu_item = CUR_USE;
			cursor_wahl(menu_item);
			invent_2_slot(K_MASKE_INV);
			invent_2_slot(K_FLEISCH_INV);
			invent_2_slot(K_KERNE_INV);
		} else if (_G(spieler).AkInvent == 88) {
			_G(spieler).flags26_10 = true;
			start_aad_wait(_G(spieler).PersonRoomNr[P_CHEWY] + 350, -1);
		}
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
		// fall through

	case WOLLE_INV:
		remove_inventory(WOLLE_INV);
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

	case 88:
	case 13:
		_G(spieler).flags26_10 = true;
		flags.InventMenu = false;
		start_spz(5, 255, false, P_CHEWY);
		start_aad_wait(_G(spieler).PersonRoomNr[P_CHEWY] + 350, -1);
		flags.InventMenu = true;
		atds->set_ats_str(88, 1, INV_ATS_DATEI);
		break;

	case 102:
	case 104:
		del_inventar(_G(spieler).AkInvent);
		menu_item = CUR_USE;
		cursor_wahl(CUR_USE);

		ret = del_invent_slot(test_nr);
		_G(spieler).InventSlot[ret] = 110;
		obj->change_inventar(104, 110, &room_blk);
		break;

	case 105:
		del_inventar(_G(spieler).AkInvent);
		atds->set_ats_str(105, 0, 1, 6);
		menu_item = CUR_USE;
		cursor_wahl(CUR_USE);
		break;

	case 106:
		del_invent_slot(106);
		atds->set_ats_str(105, 0, 1, 6);
		break;

	default:
		break;
	}
}

static void calc_inv_get_text(int16 cur_inv, int16 test_nr) {
	int16 txt_anz;
	const char *s;

	s = atds->ats_get_txt(31, TXT_MARK_USE, &txt_anz, 16);
	_G(calc_inv_text_str1) = Common::String::format("%s ", s);

	atds->load_atds(cur_inv, INV_ATS_DATEI);
	ERROR

	s = atds->ats_get_txt(cur_inv, TXT_MARK_NAME, &txt_anz, 6);
	_G(calc_inv_text_str1) += s;

	s = atds->ats_get_txt(32, TXT_MARK_USE, &txt_anz, 16);
	_G(calc_inv_text_str2) = Common::String::format("%s ", s);

	atds->load_atds(test_nr, INV_ATS_DATEI);
	ERROR

	s = atds->ats_get_txt(test_nr, TXT_MARK_NAME, &txt_anz, 6);
	_G(calc_inv_text_str2) += s;
	_G(calc_inv_text_set) = true;
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
				r_val = g_engine->getRandomNumber(5);

				if (flags.InventMenu) {
					calc_inv_get_text(_G(spieler).AkInvent, test_nr);
					look_invent(-1, INV_USE_ATS_MODE, RAND_NO_USE[r_val] + 15000);
				} else {
					ret = start_ats_wait(RAND_NO_USE[r_val], TXT_MARK_USE, 14, INV_USE_DEF);
				}
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
		switch (menu_item) {
		case CUR_USE:
			switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
			case 40:
				if (!_G(spieler).R40HoUse && _G(spieler).ChewyAni != 5) {
					menu_item = CUR_HOWARD;
					cursor_wahl(menu_item);
					txt_nr = 30000;
				}
				break;

			case 67:
			case 71:
				menu_item = CUR_HOWARD;
				cursor_wahl(menu_item);
				txt_nr = 30000;
				break;

			case 42:
				if (!_G(spieler).R42MarkeOk && !_G(spieler).R42HoToBeamter) {
					menu_item = CUR_HOWARD;
					cursor_wahl(menu_item);
					txt_nr = 30000;
				}
				break;

			default:
				break;
			}
			break;

		case CUR_LOOK:
			return (_G(spieler).PersonRoomNr[P_CHEWY] == 89) ? 513 : -1;

		default:
			break;
		}
		break;

	case P_NICHELLE:
		if (menu_item == CUR_USE) {
			switch (_G(spieler).PersonRoomNr[P_CHEWY]) {
			case 67:
			case 71:
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
			Room28::set_pump();
			action_ret = true;
			break;

		case CIGAR_INV:
			if (_G(spieler).PersonRoomNr[P_CHEWY] == 49) {
				Room49::use_boy_cigar();
				action_ret = true;
			}
			break;

		case 112:
			Room90::proc2();
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
				Room67::kostuem_aad(378);
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
				Room67::kostuem_aad(377);
				action_ret = true;
			} else if (_G(spieler).PersonRoomNr[P_CHEWY] == 68) {
				Room68::kostuem_aad(387);
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
	bool flag = false;
	int16 room_nr = 0;
	int i;

	switch (p_nr) {
	case P_HOWARD:
	case P_NICHELLE:
		switch (_G(spieler).PersonRoomNr[P_CHEWY] - 28) {
		case 0:
			_G(spieler).PersonDia[p_nr] = (p_nr == P_HOWARD) ? 545 : 547;
			break;

		case 18:
			if (p_nr == P_HOWARD)
				_G(spieler).PersonDia[p_nr] = 542;
			break;

		case 28:
			if (p_nr == P_HOWARD)
				_G(spieler).PersonDia[p_nr] = 543;
			break;

		case 38:
			if (p_nr == P_HOWARD)
				_G(spieler).PersonDia[p_nr] = 540;
			break;

		case 39:
			_G(spieler).PersonDia[p_nr] = 501;
			break;

		case 40:
			_G(spieler).PersonDia[p_nr] = 500;
			break;

		case 41:
			_G(spieler).PersonDia[p_nr] = 492;
			break;

		case 42:
			if (_G(spieler).flags32_10) {
				_G(spieler).PersonDia[p_nr] = 465;
				if (_G(spieler).flags30_80 && !_G(spieler).flags31_1)
					_G(spieler).PersonDia[p_nr] = 466;
			}
			break;

		case 43:
			if (p_nr == P_HOWARD) {
				_G(spieler).PersonDiaRoom[1] = 1;
				_G(spieler).PersonDia[P_HOWARD] = 470;
			} else {
				_G(spieler).PersonDia[P_NICHELLE] = 536;
			}
			break;

		case 45:
			_G(spieler).PersonDia[p_nr] = _G(spieler).flags32_2 ? 469 : 467;
			break;

		case 46:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 535;
			else
				flag = true;
			break;

		case 47:
			if (!_G(spieler).flags32_10)
				_G(spieler).PersonDia[P_NICHELLE] = 536;
			else if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[P_NICHELLE] = 534;
			else
				flag = true;
			break;

		case 51:
			if (p_nr != P_NICHELLE)
				flag = true;
			else if (_G(spieler).flags30_10)
				_G(spieler).PersonDia[p_nr] = 533;
			else
				Room82::proc8();
			break;

		case 53:
			if (!_G(spieler).flags30_2)
				_G(spieler).PersonDia[p_nr] = 457;
			else
				flag = true;
			break;

		case 54:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 532;
			else
				flag = true;
			break;

		case 56:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 531;
			else
				flag = true;
			break;

		case 57:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 530;
			else
				flag = true;
			break;

		case 58:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 529;
			else
				flag = true;
			break;

		case 59:
			if (p_nr == P_NICHELLE) {
				if (!_G(spieler).flags28_4) {
					Room71::proc7();
					_G(spieler).PersonDia[p_nr] = -1;
				}
			} else if (!_G(spieler).flags28_4) {
				_G(spieler).PersonDia[p_nr] = 528;
			} else {
				flag = true;
			}
			break;

		case 60:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 528;
			else
				flag = true;
			break;

		case 61:
			_G(spieler).PersonDia[p_nr] = 381;
			break;

		case 62:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 527;
			else
				flag = true;
			break;

		case 63:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 526;
			else
				flag = true;
			break;

		case 66:
			if (p_nr == P_NICHELLE)
				_G(spieler).PersonDia[p_nr] = 525;
			else
				flag = true;
			break;

		case 67:
			if (_G(spieler).flags32_10)
				_G(spieler).PersonDia[p_nr] = 507;
			else
				flag = true;
			break;

		case 68:
			if (_G(spieler).flags32_10)
				_G(spieler).PersonDia[p_nr] = 497;
			else
				flag = true;
			break;

		case 69:
			if (p_nr == P_HOWARD) {
				if (_G(spieler).ChewyAni != 5)
					_G(spieler).PersonDia[p_nr] = 565;
				else
					flag = true;
			}			
			break;

		default:
			flag = true;
			break;
		}

		if (flag) {
			_G(spieler).PersonDia[p_nr] = _G(spieler).PersonGlobalDia[p_nr];
			room_nr = _G(spieler).PersonDiaRoom[p_nr];
		}

		atds->set_string_end_func(&Room65::atds_string_start);

		if (!room_nr) {
			if (_G(spieler).DiaAMov != -1)
				auto_move(_G(spieler).DiaAMov, P_CHEWY);

			int16 tmp[3];
			for (i = 0; i < 3; ++i) {
				tmp[i] = _G(spieler).PersonDiaRoom[i];
				_G(spieler).PersonDiaRoom[i] = 0;
				_G(stopAutoMove)[i] = true;
			}

			start_aad_wait(_G(spieler).PersonDia[p_nr], -1);

			for (i = 0; i < 3; ++i) {
				_G(spieler).PersonDiaRoom[i] = tmp[i];
				_G(stopAutoMove)[i] = false;
			}

			show_cur();
		} else {
			room_blk.AadLoad = false;
			room_blk.AtsLoad = false;
			_G(spieler).PersonDiaTmpRoom[p_nr] = _G(spieler).PersonRoomNr[P_CHEWY];
			save_person_rnr();

			if (p_nr == P_HOWARD) {
				_G(spieler).PersonDiaRoom[0] = 1;
				switch_room(65);
				_G(spieler).PersonDiaRoom[0] = 0;

			} else if (p_nr == P_NICHELLE) {
				if (_G(spieler).PersonDia[P_NICHELLE] < 10000) {
					_G(cur_hide_flag) = false;
					hide_cur();
					start_aad_wait(_G(spieler).PersonDia[P_NICHELLE], -1);
					_G(stopAutoMove)[P_NICHELLE] = _G(spieler).PersonDiaRoom[P_NICHELLE] != 0;
					show_cur();
				} else {
					start_ads_wait(_G(spieler).PersonDia[P_NICHELLE] - 10000);
				}
			}
		}

		atds->set_string_end_func(&atds_string_start);
		break;

	default:
		break;
	}
}

} // namespace Chewy
