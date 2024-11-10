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
#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/dialogs/inventory.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/rooms/rooms.h"

namespace Chewy {

void loadDialogCloseup(int16 diaNr) {
	if (!_G(flags).DialogCloseup) {
		if (_G(atds)->startDialogCloseup(diaNr)) {
			_G(minfo).button = 0;
			g_events->_kbInfo._keyCode = '\0';
			g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;

			_G(ads_blk_nr) = 0;
			_G(dialogCloseupItemPtr) = _G(atds)->dialogCloseupItemPtr(diaNr, _G(ads_blk_nr), &_G(ads_item_nr));
			_G(flags).DialogCloseup = true;
			_G(ads_push) = true;
			_G(ads_tmp_dsp) = _G(gameState).DispFlag;
			_G(gameState).DispFlag = false;

			setSsiPos();
			_G(ads_dia_nr) = diaNr;
			_G(talk_start_ani) = -1;
			_G(talk_hide_static) = -1;
		}
	}
}

void setSsiPos() {
	int16 tmp = 0;

	for (int16 i = 0; i < MAX_PERSON; i++) {
		if (i >= P_NICHELLE)
			tmp = 1;
		int16 x = _G(moveState)[i].Xypos[0] - _G(gameState).scrollx + _G(spieler_mi)[i].HotX;
		int16 y = _G(moveState)[i].Xypos[1] - _G(gameState).scrolly;
		_G(atds)->set_split_win(i + tmp, x, y);
	}
}

int16 atsAction(int16 txtNr, int16 txtMode, int16 mode) {
	int16 retValue;
	if (!_G(atds)->getControlBit(txtNr, ATS_ACTIVE_BIT)) {
		retValue = true;
		if (_G(flags).AtsAction == false) {
			_G(flags).AtsAction = true;
			if (mode == ATS_ACTION_NACH) {
				switch (txtMode) {
				case TXT_MARK_LOOK:
					switch (txtNr) {
					case 229:
						autoMove(2, P_CHEWY);
						Room39::look_tv(true);
						break;

					case 268:
						autoMove(3, P_CHEWY);
						break;

					case 294:
						switchRoom(46);
						break;

					case 299:
						switchRoom(59);
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
						Room68::lookAtCactus();
						break;

					default:
						retValue = false;
						break;

					}
					break;

				case TXT_MARK_USE:
					switch (txtNr) {
					case 62:
						retValue = Room39::use_howard();
						break;

					case 67:
						retValue = Room8::gips_wurf();
						break;

					case 71:
						if (isCurInventory(ZANGE_INV))
							Room8::hole_kohle();
						else if (!_G(cur)->usingInventoryCursor())
							Room8::start_verbrennen();
						break;

					case 73:
						if (!_G(gameState).R9Grid) {
							_G(gameState)._personHide[P_CHEWY] = true;
							startSetAILWait(5, 1, ANI_FRONT);
							_G(gameState)._personHide[P_CHEWY] = false;
						} else {
							retValue = false;
						}
						break;


					case 77:
						if (!_G(gameState).R10SurimyOk && !_G(cur)->usingInventoryCursor()) {
							hideCur();
							autoMove(3, P_CHEWY);
							flic_cut(FCUT_004);
							start_spz(CH_TALK5, -1, false, P_CHEWY);
							startAadWait(108);
							showCur();
						} else {
							retValue = false;
						}
						break;

					case 79:
						retValue = Room22::chewy_amboss();
						break;

					case 80:
						if (_G(cur)->usingInventoryCursor())
							autoMove(3, P_CHEWY);
						break;

					case 81:
						Room22::get_bork();
						break;

					case 82:
						retValue = Room22::malen();
						break;

					case 84:
						retValue = Room11::scanner();
						break;

					case 92:
						retValue = Room13::monitor_button();
						break;

					case 104:
						if (isCurInventory(TRANSLATOR_INV)) {
							autoMove(6, P_CHEWY);
							_G(gameState).R14Translator = true;
							startAadWait(25);
						} else
							retValue = false;
						break;

					case 107:
						retValue = Room14::use_gleiter();
						break;

					case 108:
						retValue = Room14::use_schrott();
						break;

					case 109:
						retValue = Room14::use_schleim();
						break;

					case 110:
						retValue = Room23::start_gleiter();
						break;

					case 114:
						switchRoom(_G(gameState).R23GliderExit);
						break;

					case 117:
						retValue = Room12::chewy_trans();
						break;

					case 118:
						retValue = Room12::useTransformerTube();
						break;

					case 119:
						retValue = Room12::use_terminal();
						break;

					case 123:
						retValue = Room16::use_gleiter();
						break;

					case 133:
						retValue = Room21::use_fenster();
						break;

					case 134:
						Room21::salto();
						break;

					case 137:
						retValue = Room17::use_seil();
						break;

					case 139:
						Room17::calc_seil();
						break;

					case 142:
						retValue = Room17::energy_lever();
						break;

					case 146:
						retValue = Room17::get_oel();
						break;

					case 149:
					case 153:
						retValue = Room18::calcSurimy();
						break;

					case 154:
						retValue = Room18::use_cart_moni();
						break;

					case 158:
						retValue = Room18::go_cyberspace();
						break;

					case 159:
						retValue = Room18::calcMonitorControls();
						break;

					case 161:
					case 162:
					case 163:
						Room24::use_lever(txtNr);
						break;

					case 165:
						if (_G(gameState).R16F5Exit) {
							invent_2_slot(29);
							_G(det)->hideStaticSpr(19);
							_G(atds)->setControlBit(158, ATS_ACTIVE_BIT);
						}
						switchRoom(18);
						break;

					case 174:
						retValue = Room0::getPillow();
						break;

					case 175:
						retValue = Room0::pullSlime();
						break;

					case 179:
						retValue = Room18::sonden_moni();
						break;

					case 187:
						retValue = Room25::useGlider();
						break;

					case 203:
						retValue = Room32::get_script();
						break;

					case 206:
						retValue = Room28::use_breifkasten();
						break;

					case 210:
						retValue = Room33::use_schublade();
						break;

					case 212:
						retValue = Room29::zaun_sprung();
						break;

					case 215:
						retValue = Room29::useWaterHose();
						break;

					case 216:
						retValue = Room29::use_pumpe();
						break;

					case 218:
						retValue = Room29::getWaterHose();
						break;

					case 219:
						retValue = Room25::extinguishGliderFlames();
						break;

					case 220:
						retValue = Room34::use_kuehlschrank();
						break;

					case 221:
						Room33::use_maschine();
						break;

					case 227:
						Room34::xit_kuehlschrank();
						break;

					case 229:
						retValue = Room39::use_tv();
						break;

					case 230:
						retValue = Room32::use_howard();
						break;

					case 231:
						Room32::use_schreibmaschine();
						break;

					case 234:
						retValue = Room35::schublade();
						break;

					case 235:
						retValue = Room35::use_cat();
						break;

					case 242:
						retValue = Room31::use_topf();
						break;

					case 244:
						retValue = Room31::open_luke();
						break;

					case 245:
						retValue = Room31::close_luke_proc1();
						break;

					case 249:
						retValue = Room37::useSeesaw();
						break;

					case 251:
						Room37::useRooster();
						break;

					case 256:
						retValue = Room37::useGlass();
						break;

					case 263:
						retValue = Room42::useStationEmployee();
						break;

					case 264:
						retValue = Room42::useMailBag();
						break;

					case 266:
						retValue = Room41::use_brief();
						break;

					case 267:
						retValue = Room41::use_lola();
						break;

					case 269:
						retValue = Room41::use_kasse();
						break;

					case 275:
						retValue = Room40::use_police();
						break;

					case 276:
						retValue = Room40::use_tele();
						break;

					case 278:
						retValue = Room40::use_mr_pumpkin();
						break;

					case 284:
						retValue = Room40::use_bmeister();
						break;

					case 286:
					case 292:
					case 293:
						Room47::use_button(txtNr);
						break;

					case 288:
						retValue = Room46::use_schloss();
						break;

					case 294:
						switchRoom(46);
						break;

					case 295:
					case 386:
						retValue = Room45::use_taxi();
						break;

					case 297:
						retValue = Room45::use_boy();
						break;

					case 315:
						retValue = Room49::use_taxi();
						break;

					case 318:
						retValue = Room49::use_boy();
						break;

					case 319:
						retValue = Room53::use_man();
						break;

					case 323:
						retValue = Room50::use_gutschein();
						break;

					case 327:
						retValue = Room50::use_gum();
						break;

					case 329:
					case 330:
					case 331:
					case 332:
					case 333:
					case 334:
						retValue = Room51::use_door(txtNr);
						break;

					case 340:
						Room55::strasse(0);
						break;

					case 341:
						retValue = Room52::use_hot_dog();
						break;

					case 343:
						retValue = Room54::use_zelle();
						break;

					case 344:
						retValue = Room57::use_pfoertner();
						break;

					case 345:
						retValue = Room54::use_azug();
						break;

					case 346:
						retValue = Room54::use_schalter();
						break;

					case 347:
					case 387:
						retValue = Room54::use_taxi();
						break;

					case 351:
					case 352:
						retValue = Room55::use_kammeraus();
						break;

					case 354:
						retValue = Room55::use_stapel1();
						break;

					case 355:
						retValue = Room55::use_stapel2();
						break;

					case 357:
						retValue = Room55::use_telefon();
						break;

					case 360:
					case 388:
						retValue = Room57::use_taxi();
						break;

					case 362:
						Room56::use_kneipe();
						break;

					case 366:
						retValue = Room56::use_man();
						break;

					case 367:
						retValue = Room56::use_taxi();
						break;

					case 371:
						retValue = Room62::use_laura();
						break;

					case 375:
						retValue = Room64::useBag();
						break;

					case 380:
						retValue = Room63::use_girl();
						break;

					case 383:
						retValue = Room63::use_aschenbecher();
						break;

					case 384:
						retValue = Room63::use_fx_man();
						break;

					case 385:
						retValue = Room63::use_schalter();
						break;

					case 394:
						retValue = Room67::talk_papagei();
						break;

					case 399:
						retValue = Room67::use_grammo();
						break;

					case 400:
						retValue = Room67::use_kommode();
						break;

					case 403:
					case 405:
						retValue = Room69::use_bruecke();
						break;

					case 406:
						retValue = Room68::useParrot();
						break;

					case 407:
						retValue = Room68::useDiva();
						break;

					case 410:
						retValue = Room68::useBartender();
						break;

					case 414:
						retValue = Room68::useIndigo();
						break;

					case 425:
						retValue = Room66::proc7();
						break;

					case 430:
						retValue = Room73::procMoveBushes();
						break;

					case 433:
						retValue = Room73::procPickupMachete();
						break;

					case 435:
						retValue = Room74::proc1();
						break;

					case 442:
						retValue = Room71::proc1();
						break;

					case 443:
						retValue = Room71::proc6();
						break;

					case 452:
						retValue = Room76::proc7();
						break;

					case 458:
						retValue = Room76::proc6();
						break;

					case 463:
						retValue = Room77::procOpenBoatHoleWithCorkscrew();
						break;

					case 464:
						retValue = Room77::procPlugBoatHoleWithRubber();
						break;

					case 467:
					case 473:
						retValue = Room82::procClimbLadderToGorilla();
						break;

					case 468:
						retValue = Room82::proc6();
						break;

					case 471:
						retValue = Room82::proc3();
						break;


					case 481:
					case 482:
						retValue = Room84::proc4();
						break;

					case 487:
						Room81::proc1();
						break;

					case 490:
						retValue = Room81::proc2();
						break;

					case 492:
						retValue = Room88::proc2();
						break;

					case 493:
						retValue = Room88::proc3();
						break;

					case 494:
						retValue = Room88::proc1();
						break;

					case 495:
						retValue = Room85::proc2();
						break;

					case 497:
						retValue = Room86::proc2();
						break;

					case 501:
					case 503:
						retValue = Room87::proc2(txtNr);
						break;

					case 502:
						retValue = Room87::proc4();
						break;

					case 508:
						retValue = Room89::proc2();
						break;

					case 509:
						retValue = Room89::proc5();
						break;

					case 514:
						retValue = Room89::proc4();
						break;

					case 517:
						retValue = Room90::getHubcaps();
						break;

					case 519:
						retValue = Room90::shootControlUnit();
						break;

					case 520:
						retValue = Room90::useSurimyOnWreck();
						break;

					case 522:
						retValue = Room94::giveGhostBottle();
						break;

					case 525:
						retValue = Room95::proc2();
						break;

					case 530:
						retValue = Room97::proc8();
						break;

					case 531:
						retValue = Room97::proc6();
						break;

					case 535:
						retValue = Room97::proc11();
						break;

					case 538:
						retValue = Room97::proc7();
						break;

					case 539:
						retValue = Room97::proc9();
						break;

					case 541:
						retValue = Room97::throwSlime();
						break;

					case 542:
						retValue = Room97::proc5();
						break;

					case 543:
						retValue = Room97::proc10();
						break;

					default:
						retValue = false;
						break;

					}
					break;

				case TXT_MARK_WALK:
					switch (txtNr) {
					case 94:
						Room13::jmp_band();
						break;

					case 95:
						if (_G(gameState).R13Band == true)
							startAadWait(116);
						break;

					case 100:
						Room13::jmp_floor();
						break;

					case 114:
						switchRoom(_G(gameState).R23GliderExit);
						break;

					case 227:
						Room34::xit_kuehlschrank();
						break;

					case 294:
						switchRoom(46);
						break;

					case 340:
						Room55::strasse(0);
						break;

					case 362:
						retValue = Room56::use_kneipe();
						break;

					case 403:
					case 405:
						retValue = Room69::use_bruecke();
						break;

					case 487:
					case 489:
						Room81::proc1();
						break;
						
					default:
						retValue = false;
						break;

					}
					break;

				case TXT_MARK_TALK:
					switch (txtNr) {
					case 11:
						autoMove(3, P_CHEWY);
						_G(det)->stopDetail(5);
						_G(det)->startDetail(6, 2, ANI_FRONT);
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
						if (!_G(gameState).R13BorkOk) {
							autoMove(10, P_CHEWY);
							Room13::talk_bork();
						}
						break;

					case 235:
						Room35::talk_cat();
						break;

					case 251:
						Room37::talkWithRooster();
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
						Room64::talk_man(350);
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
						retValue = Room67::talk_papagei();
						break;

					case 408:
						Room68::talkWithParrot();
						break;

					case 410:
						Room68::talkToBartender();
						break;

					case 414:
						Room68::talkToIndigo();
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
						Room82::talkWithDirector();
						break;

					case 469:
						Room82::talkWithFilmDiva();
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
						startAadWait(482);
						break;

					case 506:
						startAadWait(483);
						break;

					case 512:
						Room89::talk1();
						break;

					case 522:
						Room94::talk1();
						break;

					default:
						retValue = false;
						break;
					}
					break;

				default:
					break;
				}

			} else if (mode == ATS_ACTION_VOR) {
				switch (txtMode) {
				case TXT_MARK_LOOK:
					switch (txtNr) {
					case 210:
						Room33::look_schublade();
						break;

					case 389:
						Room67::look_brief();
						break;

					case 391:
						autoMove(2, P_CHEWY);
						break;

					case 431:
						autoMove(3, P_CHEWY);
						break;

					default:
						retValue = false;
						break;
					}
					break;

				case TXT_MARK_USE:
					switch (txtNr) {
					case 90:
						_G(atds)->set_all_ats_str(90, TXT_MARK_USE, _G(gameState).R13Band);
						break;

					case 124:
						if (!_G(gameState).R16F5Exit)
							autoMove(5, P_CHEWY);
						break;

					case 134:
						autoMove(10, P_CHEWY);
						break;

					case 210:
						autoMove(1, P_CHEWY);
						break;

					case 225:
						retValue = Room33::get_munter();
						break;

					case 267:
						if (!_G(gameState).R41LolaOk && _G(gameState).R41RepairInfo)
							_G(atds)->set_all_ats_str(267, 1, ATS_DATA);
						break;

					case 283:
						retValue = Room40::use_haendler();
						break;

					case 423:
						retValue = Room66::proc2();
						break;

					default:
						retValue = false;
						break;
					}
					break;

				case TXT_MARK_WALK:
					retValue = false;
					break;

				case TXT_MARK_TALK:
					switch (txtNr) {
					case 104:
						Room14::talk_eremit();
						break;

					case 120:
						Room12::talk_bork();
						break;

					case 263:
						Room42::talkToStationEmployee();
						break;

					default:
						retValue = false;
						break;

					}
					break;

				default:
					break;
				}
			}

			_G(flags).AtsAction = false;
		}
	} else {
		retValue = false;
	}

	return retValue;
}

// Original name: adsAction
void selectDialogOption(int16 diaNr, int16 blkNr, int16 strEndNr) {
	if (_G(flags).AdsAction == false) {
		_G(flags).AdsAction = true;

		switch (diaNr) {
		case 2:
			if (blkNr == 4 && strEndNr == 0) {

				_G(obj)->show_sib(31);
			}
			break;

		case 3:
			if (strEndNr == 1) {
				if (_G(gameState).R11IdCardNr == RED_CARD_INV)
					_G(gameState).R11IdCardNr = YEL_CARD_INV;
				else
					_G(gameState).R11IdCardNr = RED_CARD_INV;
			}
			break;

		case 10:
			Room42::dialogWithStationEmployee(strEndNr);
			break;

		case 11:
			if (blkNr == 3) {
				Room41::sub_dia();
			} else if (blkNr == 0 && strEndNr == 3) {
				_G(gameState).R41RepairInfo = true;
				stopDialogCloseupDialog();
			}
			break;

		case 12:
			if (strEndNr == 1) {
				_G(gameState).R41HowardDiaOK = true;
			}
			break;

		case 15:
			if (blkNr == 1 && strEndNr == 0) {
				_G(gameState).R55Job = true;
				stopDialogCloseupDialog();
			}
			break;

		case 16:
			if (blkNr == 0 && strEndNr == 2 && !_G(gameState).R56Kneipe)
				_G(atds)->delControlBit(362, ATS_ACTIVE_BIT);

			break;

		case 17:
			if (blkNr == 0 && strEndNr == 2)
				_G(atds)->showDialogCloseupItem(17, 0, 1);

			break;

		case 18:
		case 19:
			if (blkNr == 0 && strEndNr == 4)
				_G(gameState).R67SongOk = true;

			break;

		case 20:
			if (blkNr == 0 && strEndNr == 1) {
				_G(gameState)._personHide[P_CHEWY] = true;
				startSetAILWait(28, 3, ANI_FRONT);
				_G(gameState)._personHide[P_CHEWY] = false;
			}
			break;

		default:
			break;
		}

		_G(flags).AdsAction = false;
	}
}

void endDialogCloseup(int16 diaNr, int16 blkNr, int16 strEndNr) {
	switch (diaNr) {
	case 0:
		_G(flags).AutoAniPlay = false;
		break;

	case 5:
		autoMove(6, P_CHEWY);
		break;

	case 22:
		if (strEndNr == 1) {
			_G(det)->del_static_ani(3);
			startSetAILWait(5, 1, ANI_FRONT);
			_G(det)->set_static_ani(3, -1);
			startAadWait(456);
		}
		break;

	default:
		break;
	}
}

#define R14_HERMIT_DIA 10000
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

#define START_STOP_TMP \
	if (mode == AAD_STR_START) \
		talkAni = tmp; \
	else \
		_G(det)->stopDetail(tmp)

#define START_STOP(NUM) \
	if (mode == AAD_STR_START) \
		talkAni = NUM; \
	else \
		_G(det)->stopDetail(NUM) \


void atdsStringStart(int16 diaNr, int16 strNr, int16 personNr, int16 mode) {
	int16 talkStopAni = -1;
	int16 talkShowStatic = -1;
	int16 aniNr;
	int16 talkAni = -1;
	int16 stopAni = -1;
	bool oldFormat = false;
	int16 tmp = -1;

	switch (diaNr) {
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
			switch (_G(gameState).ChewyAni) {
			case CHEWY_NORMAL:
				aniNr = CH_TALK3;
				break;

			case CHEWY_BORK:
				aniNr = 68;
				break;

			case CHEWY_PUMPKIN:
				aniNr = CH_PUMP_TALK;
				break;

			case CHEWY_ROCKER:
				aniNr = CH_ROCK_TALK1;
				break;

			case CHEWY_JMANS:
				aniNr = CH_JM_TALK;
				break;

			case CHEWY_ANI7:
				aniNr = 46;
				break;

			default:
				aniNr = -1;
				break;
			}

			if (aniNr != -1)
				start_spz(aniNr, 255, ANI_FRONT, P_CHEWY);

		} else {
			stop_spz();
		}
		break;

	case R8_NIMOYANER3_DIA:
	case 249:
		if (personNr == 0) {
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK4, 255, ANI_FRONT, P_CHEWY);
			} else {
				stop_spz();
			}
		}
		break;

	case 61:
	case R14_HERMIT_DIA:
	case R8_NIMOYANER1_DIA:
	case R8_NIMOYANER2_DIA:
		if (personNr <= P_CHEWY) {
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			} else {
				stop_spz();
			}
		} else if (personNr == P_HOWARD) {
			START_STOP(15);
		}
		break;

	case R11_BORK_DIA:
		oldFormat = true;
		if (personNr == 1) {
			if (mode == AAD_STR_START) {
				_G(talk_start_ani) = 9;
				_G(talk_hide_static) = 8;
			} else {
				talkStopAni = 9;
				talkShowStatic = 8;
				_G(talk_start_ani) = -1;
				_G(talk_hide_static) = -1;
			}
		}
		break;

	case R13_BORK_DIA:
		oldFormat = true;
		switch (personNr) {
		case 0:
			talkStopAni = 9;
			_G(talk_start_ani) = 10;
			talkShowStatic = 13;
			_G(talk_hide_static) = 12;
			break;

		case 1:
			_G(talk_start_ani) = 9;
			talkStopAni = 10;
			_G(talk_hide_static) = 13;
			talkShowStatic = 12;
			break;

		default:
			break;
		}
		break;

	case 24:
	case 26:
		oldFormat = true;
		switch (personNr) {
		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 1:
			if (mode == AAD_STR_START) {
				_G(talk_start_ani) = 4;
				_G(talk_hide_static) = 9;
			} else {
				talkStopAni = 4;
				talkShowStatic = 9;
				_G(talk_start_ani) = -1;
				_G(talk_hide_static) = -1;
			}
			break;

		default:
			break;
		}
		break;

	case 171:
	case 173:
	case 174:
	case 175:
		switch (personNr) {
		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 1:
			if (mode == AAD_STR_START) {
				_G(room)->set_timer_status(0, TIMER_STOP);
				_G(det)->del_static_ani(0);
				talkAni = 1;
			} else {
				_G(room)->set_timer_status(0, TIMER_START);
				_G(det)->stopDetail(1);
				_G(det)->set_static_ani(0, -1);
			}
			break;

		default:
			break;
		}
		break;

	case R35_CAT_DIA2:
	case R35_CAT_DIA1:
	case R37_HAHN_DIA:
		if (personNr == 1) {
			if (mode == AAD_STR_START) {
				_G(room)->set_timer_status(1, TIMER_STOP);
				_G(det)->del_static_ani(1);
				talkAni = 0;
			} else {
				_G(room)->set_timer_status(1, TIMER_START);
				_G(det)->stopDetail(0);
				_G(det)->set_static_ani(1, -1);
			}
		}
		break;

	case 169:

		switch (personNr) {
		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 1:
			if (mode == AAD_STR_START) {
				_G(det)->startDetail(5, 255, ANI_FRONT);
			} else {
				_G(det)->stopDetail(5);
			}
			break;

		default:
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
		switch (personNr) {
		case P_CHEWY:
			if (mode == AAD_STR_START) {
				if (_G(gameState).R28ChewyPump)
					aniNr = CH_PUMP_TALK;
				else if (_G(gameState).ChewyAni == CHEWY_ROCKER)
					aniNr = CH_ROCK_TALK1;
				else
					aniNr = CH_TALK3;

				start_spz(aniNr, 255, ANI_FRONT, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case P_HOWARD:
		case P_NICHELLE:
		case P_4: {
			int16 nbr;
			if (personNr == 4) {
				nbr = 2;
				tmp = 3;
			} else {
				nbr = 1;
				tmp = personNr + 2;
			}
			for (int16 i = 0; i < nbr; i++) {
				if (mode == AAD_STR_START) {
					_G(det)->startDetail(tmp, 255, ANI_FRONT);
				} else {
					_G(det)->stopDetail(tmp);
				}
				++tmp;
			}
			}
			break;
		default:
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
		switch (personNr) {
		case P_CHEWY:
			if (mode == AAD_STR_START) {
				switch (_G(gameState).ChewyAni) {
				case CHEWY_NORMAL:
					aniNr = CH_TALK3;
					break;

				case CHEWY_BORK:
					aniNr = 68;
					break;

				case CHEWY_PUMPKIN:
					aniNr = CH_PUMP_TALK;
					break;

				case CHEWY_ROCKER:
					aniNr = CH_ROCK_TALK1;
					break;

				case CHEWY_JMANS:
					aniNr = CH_JM_TALK;
					break;

				case CHEWY_ANI7:
					aniNr = 46;
					break;

				default:
					aniNr = -1;
					break;

				}
				if (aniNr != -1)
					start_spz(aniNr, 255, ANI_FRONT, P_CHEWY);

			} else {
				stop_spz();
			}
			break;

		case P_HOWARD:
			if (mode == AAD_STR_START) {
				switch (_G(gameState).mi[1]) {
				case 2:
					start_spz(50, 255, ANI_FRONT, P_HOWARD);
					break;
				case 3:
					start_spz(57, 255, ANI_FRONT, P_HOWARD);
					break;
				default:
					start_spz(HO_TALK_L, 255, ANI_FRONT, P_HOWARD);
					break;
				}
			} else {
				stop_spz();
			}
			break;

		case P_3:
			if (mode == AAD_STR_START) {
				_G(det)->stopDetail(2);
			} else {
				stop_spz();
			}
			break;

		case P_NICHELLE:
			switch (_G(gameState)._personRoomNr[P_CHEWY]) {
			case 28:
				START_STOP(diaNr - 194);
				break;

			case 40:
				switch (diaNr) {
				case 202:
				case 209:
				case 211:
					START_STOP(3);
					break;

				case 213:
					START_STOP(5);
					break;

				case 205:
				case 227:
					START_STOP(14);
					break;

				case 215:
				case 237:
				case 375:
					START_STOP(2);
					break;

				default:
					break;
				}
				break;

			case 42:
				if (mode == AAD_STR_START) {
					_G(room)->set_timer_status(8, TIMER_STOP);
					_G(det)->del_static_ani(8);
					talkAni = 2;
				} else {
					_G(room)->set_timer_status(8, TIMER_START);
					_G(det)->stopDetail(2);
					_G(det)->set_static_ani(8, -1);
				}
				break;

			case 45:
				switch (diaNr) {
				case 257:
				case 258:
				case 259:
					if (mode == AAD_STR_START) {
						_G(room)->set_timer_status(0, TIMER_STOP);
						_G(det)->del_static_ani(0);
						talkAni = 2;
					} else {
						_G(room)->set_timer_status(0, TIMER_START);
						_G(det)->stopDetail(2);
						_G(det)->set_static_ani(0, -1);
					}
					break;

				case 254:
				case 260:
				case 256:
				case 400:
					START_STOP(14);
					break;

				default:
					break;
				}
				break;

			case 49:
			case 64:
				START_STOP(2);
				break;

			case 50:
				switch (diaNr) {
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

				default:
					break;
				}

				START_STOP_TMP;
				break;

			case 53:
				START_STOP(3);
				break;

			case 55:
				switch (diaNr) {
				case 323:
					tmp = 16;
					break;

				case 335:
					tmp = 18;
					break;

				case 320:
					tmp = 21;
					break;

				default:
					break;
				}

				START_STOP_TMP;
				break;

			case 56:
				switch (diaNr) {
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

				default:
					break;
				}

				START_STOP_TMP;
				break;

			case 57:
				START_STOP(1);
				break;

			case 63:
				switch (diaNr) {
				case 358:
					tmp = 5;
					break;

				case 365:
				case R63_GIRL_DIA:
					tmp = 14;
					break;

				default:
					break;
				}

				START_STOP_TMP;
				break;

			case 67:
				if (mode == AAD_STR_START) {
					if (diaNr != 376)
						talkAni = 1;
				} else if (diaNr != 376)
					_G(det)->stopDetail(1);

				break;

			case 68:
				switch (diaNr) {
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

				default:
					break;
				}

				START_STOP_TMP;
				break;

			case 82:
				switch (diaNr) {
				case 446:
					tmp = 8;
					break;
				case 447:
				case 448:
				case 449:
				case 450:
				case 451:
				case 454:
					tmp = 7;
					break;
				case 452:
					tmp = 2;
					break;
				case 453:
					tmp = 4;
					break;
				default:
					break;
				}

				START_STOP_TMP;
				break;

			case 84:
				if (diaNr == 477 || diaNr == 478 || diaNr == 481) {
					if (mode == AAD_STR_START) {
						talkAni = 6;
						stopAni = 7;
					} else {
						talkAni = 7;
						stopAni = 6;
					}
				
					START_STOP_TMP;
				}
				break;

			case 85:
				if (mode == AAD_STR_START) {
					talkAni = 0;
				} else {
					talkAni = 1;
					stopAni = 0;
				}
				break;

			case 87:
				if (mode == AAD_STR_START) {
					talkAni = 1;
					stopAni = 2;
				} else {
					talkAni = 2;
					stopAni = 1;
				}
				break;

			case 88:
				if (mode == AAD_STR_START) {
					talkAni = 3;
					stopAni = 4;
				} else {
					talkAni = 4;
					stopAni = 3;
				}
				break;

			case 89:
				START_STOP(3);
				break;

			case 93:
				switch (diaNr) {
				case 549:
				case 616:
					START_STOP(0);
					break;

				case 550:
					START_STOP(7);
					break;

				case 10027:
					START_STOP(1);
					break;
				
				default:
					break;
				}
				break;

			case 94:
				switch (diaNr) {
				case 539:
				case 551:
					START_STOP(0);
					break;

				case 552:
					START_STOP(2);
					break;

				default:
					break;
				}
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}
		break;

	case R61_VERLAG_DIA:
		if (mode == AAD_STR_START) {
			_G(det)->startDetail(personNr, 255, ANI_FRONT);
		} else {
			_G(det)->stopDetail(personNr);
		}
		break;

	case R68_KEEPER_DIA:
		switch (personNr) {
		case 0:
			if (mode == AAD_STR_START) {
				start_spz(CH_TALK3, 255, ANI_FRONT, P_CHEWY);
			} else {
				stop_spz();
			}
			break;

		case 2:
			START_STOP(16);
			break;

		case 3:
			if (mode == AAD_STR_START) {
				_G(gameState)._personHide[P_CHEWY] = true;
				talkAni = 29;
			} else {
				_G(det)->stopDetail(29);
				_G(gameState)._personHide[P_CHEWY] = false;
			}
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	if (oldFormat) {
		_G(det)->startDetail(_G(talk_start_ani), 255, ANI_FRONT);
		_G(det)->stopDetail(talkStopAni);
		_G(det)->hideStaticSpr(_G(talk_hide_static));
		_G(det)->showStaticSpr(talkShowStatic);
	} else {
		_G(det)->startDetail(talkAni, 255, ANI_FRONT);
		_G(det)->stopDetail(stopAni);
	}
}

#undef START_STOP
#undef START_STOP_TMP

void useItemWithInvItem(int16 itemId) {
	int scrollx, scrolly;
	int16 ret;

	switch (itemId) {
	case NOTEBOOK_OPEN_INV:
	case MONOCLE_INV:
		scrollx = _G(gameState).scrollx;
		scrolly = _G(gameState).scrolly;
		_G(gameState).scrollx = 0;
		_G(gameState).scrolly = 0;
		_G(cur)->hideCursor();

		_G(room)->load_tgp(NOTEBOOK_START, &_G(room_blk), GBOOK_TGP, false, GBOOK);
		_G(out)->setPointer(_G(workptr));
		_G(out)->map_spr2screen(_G(ablage)[_G(room_blk).AkAblage], _G(gameState).scrollx, _G(gameState).scrolly);
		_G(out)->copyToScreen();

		while (g_events->getSwitchCode() != Common::KEYCODE_ESCAPE) {
			g_events->update();
			SHOULD_QUIT_RETURN;
		}

		EVENTS_CLEAR;
		g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
		g_events->_kbInfo._keyCode = '\0';
		_G(minfo).button = 0;

		_G(room)->load_tgp(_G(gameState)._personRoomNr[P_CHEWY], &_G(room_blk), EPISODE1_TGP, true, EPISODE1);

		_G(cur)->showCursor();
		_G(gameState).scrollx = scrollx;
		_G(gameState).scrolly = scrolly;
		break;

	case FISHINGROD_INV:
	case BONE_INV:
		delInventory(_G(cur)->getInventoryCursor());
		_G(menu_item) = CUR_USE;
		cursorChoice(_G(menu_item));
		ret = del_invent_slot(itemId);
		_G(gameState).InventSlot[ret] = FISHING_ROD_INV;
		_G(obj)->changeInventory(itemId, FISHING_ROD_INV, &_G(room_blk));
		break;

	case PUMPKIN_INV:
		ret = del_invent_slot(PUMPKIN_INV);
		_G(gameState).InventSlot[ret] = K_MASKE_INV;
		_G(obj)->changeInventory(PUMPKIN_INV, K_MASKE_INV, &_G(room_blk));
		invent_2_slot(K_FLEISCH_INV);
		invent_2_slot(K_KERNE_INV);
		break;

	case KNIFE_INV:
		if (_G(cur)->getInventoryCursor() == 40) {
			delInventory(_G(cur)->getInventoryCursor());
			_G(menu_item) = CUR_USE;
			cursorChoice(_G(menu_item));
			invent_2_slot(K_MASKE_INV);
			invent_2_slot(K_FLEISCH_INV);
			invent_2_slot(K_KERNE_INV);
		} else if (_G(cur)->getInventoryCursor() == 88) {
			_G(gameState).changedArtifactOrigin = true;
			startAadWait(_G(gameState)._personRoomNr[P_CHEWY] + 350);
		}
		break;

	case LETTER_INV:
		delInventory(_G(cur)->getInventoryCursor());
		_G(menu_item) = CUR_USE;
		cursorChoice(_G(menu_item));
		_G(gameState).R42LetterStamped = true;
		ret = del_invent_slot(LETTER_INV);
		_G(gameState).InventSlot[ret] = STAMPEDLETTER_INV;
		_G(obj)->changeInventory(LETTER_INV, STAMPEDLETTER_INV, &_G(room_blk));
		break;

	case BOTTLE_INV:
		delInventory(_G(cur)->getInventoryCursor());
		_G(menu_item) = CUR_USE;
		cursorChoice(_G(menu_item));
		// fall through

	case WOOL_INV:
		remove_inventory(WOOL_INV);
		_G(atds)->set_all_ats_str(BOTTLE_INV, 1, INV_ATS_DATA);
		_G(gameState).R56WhiskyMix = true;
		break;

	case B_MARY_INV:
	case PIRANHA_INV:
		delInventory(_G(cur)->getInventoryCursor());
		_G(menu_item) = CUR_USE;
		cursorChoice(_G(menu_item));
		ret = del_invent_slot(itemId);
		_G(gameState).InventSlot[ret] = B_MARY2_INV;
		_G(obj)->changeInventory(itemId, B_MARY2_INV, &_G(room_blk));
		break;

	case 13:
		_G(gameState).changedArtifactOrigin = true;
		_G(flags).InventMenu = false;
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		startAadWait(_G(gameState)._personRoomNr[P_CHEWY] + 350);
		_G(flags).InventMenu = true;
		_G(atds)->set_all_ats_str(ARTIFACT_INV, 1, INV_ATS_DATA);
		break;

	case 88:
		_G(gameState).changedArtifactOrigin = true;
		startAadWait(350);
		_G(atds)->set_all_ats_str(ARTIFACT_INV, 1, INV_ATS_DATA);
		break;

	case 102:
	case 104:
		delInventory(_G(cur)->getInventoryCursor());
		_G(menu_item) = CUR_USE;
		cursorChoice(CUR_USE);

		ret = del_invent_slot(itemId);
		_G(gameState).InventSlot[ret] = 110;
		_G(obj)->changeInventory(104, 110, &_G(room_blk));
		break;

	case 105:
		delInventory(_G(cur)->getInventoryCursor());
		_G(atds)->set_ats_str(105, 0, 1, INV_ATS_DATA);
		_G(menu_item) = CUR_USE;
		cursorChoice(CUR_USE);
		break;

	case 106:
		del_invent_slot(106);
		_G(atds)->set_ats_str(105, 0, 1, INV_ATS_DATA);
		break;

	default:
		break;
	}
}

static void calc_inv_get_text(int16 cur_inv, int16 test_nr) {
	_G(calc_inv_text_str1) = _G(atds)->getTextEntry(0, 31, INV_USE_DEF) + " ";
	_G(calc_inv_text_str1) += _G(atds)->getTextEntry(cur_inv, TXT_MARK_NAME, INV_ATS_DATA);
	_G(calc_inv_text_str2) = _G(atds)->getTextEntry(0, 32, INV_USE_DEF) + " ";
	_G(calc_inv_text_str2) += _G(atds)->getTextEntry(test_nr, TXT_MARK_NAME, INV_ATS_DATA);
}

bool calc_inv_no_use(int16 test_nr, int16 mode) {
	int16 inv_mode;
	bool ret = false;

	switch (mode) {
	case INVENTORY_NORMAL:
		inv_mode = IUID_IIB;
		break;

	case INVENTORY_STATIC:
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
		inv_mode = IUID_PLAYER;
		break;

	default:
		inv_mode = -1;
		break;
	}

	if (inv_mode != -1) {
		int16 txt_nr = _G(atds)->calc_inv_no_use(_G(cur)->getInventoryCursor(), test_nr);
		if (txt_nr != -1) {
			if (!_G(flags).InventMenu) {
				if (txt_nr >= 15000) {
					ret = startAtsWait(txt_nr - 15000, TXT_MARK_USE, 14, INV_USE_DEF);
				} else {
					ret = startAtsWait(txt_nr, TXT_MARK_USE, 14, INV_USE_DATA);
				}
			} else {
				Dialogs::Inventory::look(-1, INV_USE_ATS_MODE, txt_nr);
			}
		} else {
			int16 ok = false;
			if (inv_mode == IUID_PLAYER)
				ok = calc_person_click(test_nr);
			if (!ok) {
				int16 r_val = g_engine->getRandomNumber(5);

				if (_G(flags).InventMenu) {
					calc_inv_get_text(_G(cur)->getInventoryCursor(), test_nr);
					Dialogs::Inventory::look(-1, INV_USE_ATS_MODE, RAND_NO_USE[r_val] + 15000);
				} else {
					ret = startAtsWait(RAND_NO_USE[r_val], TXT_MARK_USE, 14, INV_USE_DEF);
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
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		case 40:
		case 41:
		case 42:
			txt_nr = 219 + _G(gameState)._personRoomNr[P_CHEWY];
			break;

		default:
			txt_nr = -1;
			break;

		}
		break;

	case P_HOWARD:
		switch (_G(menu_item)) {
		case CUR_USE:
			switch (_G(gameState)._personRoomNr[P_CHEWY]) {
			case 40:
				if (!_G(gameState).R40HoUse && _G(gameState).ChewyAni != CHEWY_ROCKER) {
					_G(menu_item) = CUR_HOWARD;
					cursorChoice(_G(menu_item));
					txt_nr = 30000;
				}
				break;

			case 67:
			case 71:
				_G(menu_item) = CUR_HOWARD;
				cursorChoice(_G(menu_item));
				txt_nr = 30000;
				break;

			case 42:
				if (!_G(gameState).R42StampOk && !_G(gameState).R42HoToBeamter) {
					_G(menu_item) = CUR_HOWARD;
					cursorChoice(_G(menu_item));
					txt_nr = 30000;
				}
				break;

			default:
				break;
			}
			break;

		case CUR_LOOK:
			return (_G(gameState)._personRoomNr[P_CHEWY] == 89) ? 513 : -1;

		default:
			break;
		}
		break;

	case P_NICHELLE:
		if (_G(menu_item) == CUR_USE) {
			switch (_G(gameState)._personRoomNr[P_CHEWY]) {
			case 67:
			case 71:
				_G(menu_item) = CUR_NICHELLE;
				cursorChoice(_G(menu_item));
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
		switch (_G(cur)->getInventoryCursor()) {
		case K_MASKE_INV:
			Room28::set_pump();
			action_ret = true;
			break;

		case CIGAR_INV:
			if (_G(gameState)._personRoomNr[P_CHEWY] == 49) {
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
		switch (_G(cur)->getInventoryCursor()) {
		case GALA_INV:
			if (_G(gameState)._personRoomNr[P_CHEWY] == 67) {
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
		switch (_G(cur)->getInventoryCursor()) {
		case GALA_INV:
			if (_G(gameState)._personRoomNr[P_CHEWY] == 67) {
				Room67::kostuem_aad(377);
				action_ret = true;
			} else if (_G(gameState)._personRoomNr[P_CHEWY] == 68) {
				Room68::useDressOnNichelle(387);
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

	switch (p_nr) {
	case P_HOWARD:
	case P_NICHELLE:
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		case 28:
			if (p_nr == P_HOWARD) {
				if (_G(gameState).ChewyAni != CHEWY_ROCKER)
					_G(gameState).PersonDia[p_nr] = 565;
				else
					flag = true;
			}
			break;

		case 46:
			if (_G(gameState).flags32_10)
				_G(gameState).PersonDia[p_nr] = 497;
			else
				flag = true;
			break;

		case 58:
			if (_G(gameState).flags32_10)
				_G(gameState).PersonDia[p_nr] = 507;
			else
				flag = true;
			break;

		case 66:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 525;
			else
				flag = true;
			break;

		case 67:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 526;
			else
				flag = true;
			break;

		case 68:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 527;
			else
				flag = true;
			break;

		case 69:
			_G(gameState).PersonDia[p_nr] = 381;
			break;

		case 70:
			if (p_nr == P_NICHELLE) {
				_G(gameState).PersonDia[p_nr] = 528;
			} else {
				flag = true;
			}
			break;

		case 71:
			if (p_nr == P_NICHELLE) {
				if (!_G(gameState).flags28_4) {
					Room71::proc7();
					_G(gameState).PersonDia[p_nr] = -1;
				}
			} else if (!_G(gameState).flags28_4) {
				_G(gameState).PersonDia[p_nr] = 435;
			} else {
				flag = true;
			}
			break;

		case 73:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 529;
			else
				flag = true;
			break;

		case 74:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 530;
			else
				flag = true;
			break;

		case 75:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 531;
			else
				flag = true;
			break;

		case 79:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 532;
			else
				flag = true;
			break;

		case 81:
			if (!_G(gameState).flags30_2)
				_G(gameState).PersonDia[p_nr] = 457;
			else
				flag = true;
			break;

		case 82:
			if (p_nr != P_NICHELLE)
				flag = true;
			else if (_G(gameState).flags30_10)
				_G(gameState).PersonDia[p_nr] = 533;
			else
				Room82::proc8();
			break;

		case 84:
			if (!_G(gameState).flags32_10)
				_G(gameState).PersonDia[P_NICHELLE] = 479;
			else if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[P_NICHELLE] = 534;
			else
				flag = true;
			break;

		case 85:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 535;
			else
				flag = true;
			break;

		case 86:
			_G(gameState).PersonDia[p_nr] = _G(gameState).flags32_2 ? 469 : 467;
			break;

		case 87:
			if (p_nr == P_HOWARD) {
				_G(gameState).PersonDiaRoom[P_HOWARD] = true;
				_G(gameState).PersonDia[P_HOWARD] = 470;
			} else {
				_G(gameState).PersonDia[P_NICHELLE] = 536;
			}
			break;


		case 88:
			if (_G(gameState).flags32_10) {
				_G(gameState).PersonDia[p_nr] = 465;
				if (_G(gameState).flags30_80 && !_G(gameState).R88UsedMonkey)
					_G(gameState).PersonDia[p_nr] = 466;
			}
			break;

		case 89:
			_G(gameState).PersonDia[p_nr] = 492;
			break;

		case 90:
			_G(gameState).PersonDia[p_nr] = 500;
			break;

		case 91:
			_G(gameState).PersonDia[p_nr] = 501;
			break;

		case 94:
			if (p_nr == P_HOWARD)
				_G(gameState).PersonDia[p_nr] = 540;
			break;

		case 95:
			if (p_nr == P_HOWARD)
				_G(gameState).PersonDia[p_nr] = 543;
			break;

		case 96:
			if (p_nr == P_HOWARD)
				_G(gameState).PersonDia[p_nr] = 542;
			break;

		case 97:
			if (p_nr == P_NICHELLE)
				_G(gameState).PersonDia[p_nr] = 547;
			else if (p_nr == P_HOWARD)
				_G(gameState).PersonDia[p_nr] = 545;
			break;

		default:
			flag = true;
			break;
		}

		if (flag) {
			_G(gameState).PersonDia[p_nr] = _G(gameState).PersonGlobalDia[p_nr];
			room_nr = _G(gameState).PersonDiaRoom[p_nr];
		}

		_G(atds)->set_string_end_func(&Room65::atds_string_start);

		if (!room_nr) {
			if (_G(gameState).DiaAMov != -1)
				autoMove(_G(gameState).DiaAMov, P_CHEWY);

			int16 tmp[3];
			for (int i = 0; i < 3; ++i) {
				tmp[i] = _G(gameState).PersonDiaRoom[i];
				_G(gameState).PersonDiaRoom[i] = false;
				_G(stopAutoMove)[i] = true;
			}

			startAadWait(_G(gameState).PersonDia[p_nr]);

			for (int i = 0; i < 3; ++i) {
				_G(gameState).PersonDiaRoom[i] = tmp[i];
				_G(stopAutoMove)[i] = false;
			}

			showCur();
		} else {
			_G(room_blk).AadLoad = false;
			_G(gameState).PersonDiaTmpRoom[p_nr] = _G(gameState)._personRoomNr[P_CHEWY];
			save_person_rnr();

			if (p_nr == P_HOWARD) {
				_G(gameState).PersonDiaRoom[P_CHEWY] = true;
				switchRoom(65);
				_G(gameState).PersonDiaRoom[P_CHEWY] = false;

			} else if (p_nr == P_NICHELLE) {
				if (_G(gameState).PersonDia[P_NICHELLE] < 10000) {
					hideCur();
					startAadWait(_G(gameState).PersonDia[P_NICHELLE]);
					_G(stopAutoMove)[P_NICHELLE] = _G(gameState).PersonDiaRoom[P_NICHELLE];
					showCur();
				} else {
					startDialogCloseupWait(_G(gameState).PersonDia[P_NICHELLE] - 10000);
				}
			}
		}

		_G(atds)->set_string_end_func(&atdsStringStart);
		break;

	default:
		break;
	}
}

} // namespace Chewy
