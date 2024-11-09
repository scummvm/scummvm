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
#include "chewy/cursor.h"
#include "chewy/defines.h"
#include "chewy/events.h"
#include "chewy/globals.h"
#include "chewy/ani_dat.h"
#include "chewy/rooms/rooms.h"
#include "chewy/resource.h"
#include "chewy/sound.h"
#include "chewy/video/video_player.h"

namespace Chewy {

void play_scene_ani(int16 nr, int16 direction) {
#define ROOM_1_1 101
#define ROOM_1_2 102
#define ROOM_1_5 105
#define ROOM_2_3 203
#define ROOM_3_1 301
#define ROOM_8_17 817
#define ROOM_9_4 904
#define ROOM_18_20 1820
	int16 r_nr = _G(gameState)._personRoomNr[P_CHEWY] * 100 + nr;

	switch (r_nr) {
	case ROOM_2_3:
		Room2::electrifyWalkway1();
		break;

	case ROOM_8_17:
		start_aad(100, 0);
		_G(det)->startDetail(21, 4, ANI_FRONT);
		break;

	case ROOM_18_20:
		delInventory(_G(cur)->getInventoryCursor());
		break;

	default:
		break;
	}

	startSetAILWait(nr, 1, direction);

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

	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
}

void timer_action(int16 t_nr) {
	int16 ani_nr = t_nr - _G(room)->_roomTimer._timerStart;
	bool default_flag = false;

	if (g_engine->_sound->isSpeechActive())
		return;

#define TIMER(NUM) case NUM: default_flag = Room##NUM::timer(t_nr, ani_nr); break;
	switch (_G(gameState)._personRoomNr[P_CHEWY]) {
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

	if (default_flag && _G(flags).AutoAniPlay == false) {
		_G(det)->startDetail(_G(room)->_roomTimer._objNr[ani_nr], 1, ANI_FRONT);
		_G(uhr)->resetTimer(t_nr, 0);
	}

	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
}

void check_ged_action(int16 index) {
	int16 orig_index = index;
	index -= 50;
	index /= 4;

	if (!_G(flags).GedAction) {
		_G(flags).GedAction = true;

#define GED_ACTION(NUM) case NUM: Room##NUM::gedAction(index); break;
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		GED_ACTION(1);
		GED_ACTION(2);
		GED_ACTION(7);
		GED_ACTION(9);
		GED_ACTION(11);
		GED_ACTION(13);
		GED_ACTION(17);
		GED_ACTION(18);
		GED_ACTION(28);
		GED_ACTION(37);

		case 45:
		case 46:
			if (!index && _G(flags).ExitMov)
				_G(HowardMov) = 1;
			break;

		GED_ACTION(49);
		GED_ACTION(50);
		GED_ACTION(52);
		GED_ACTION(55);
		GED_ACTION(94);
		case 97: Room97::gedAction(orig_index); break;

		default:
			break;
		}
#undef GED_ACTION

		_G(flags).GedAction = false;
	}

	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
}

void enter_room(int16 eib_nr) {
	g_engine->_sound->playRoomMusic(_G(gameState)._personRoomNr[P_CHEWY]);
	load_chewy_taf(_G(gameState).ChewyAni);
	_G(atds)->stopAad();
	_G(atds)->stop_ats();
	_G(gameState).DiaAMov = -1;
	_G(zoom_mov_fak) = 1;

	for (int16 i = 0; i < MAX_PERSON; i++) {
		_G(spieler_mi)[i].Vorschub = 8;
		_G(gameState).ZoomXy[i][0] = 0;
		_G(gameState).ZoomXy[i][1] = 0;
	}

	_G(flags).ZoomMov = false;
	_G(gameState).ScrollxStep = 1;
	_G(gameState).ZoomXy[P_CHEWY][0] = (int16)_G(room)->_roomInfo->_zoomFactor;
	_G(gameState).ZoomXy[P_CHEWY][1] = (int16)_G(room)->_roomInfo->_zoomFactor;

	_G(uhr)->resetTimer(0, 0);
	_G(flags).AutoAniPlay = false;
	_G(SetUpScreenFunc) = nullptr;
	_G(HowardMov) = 0;

#define ENTRY(NUM) case NUM: Room##NUM::entry(); break
#define ENTRY_NR(NUM) case NUM: Room##NUM::entry(eib_nr); break

	switch (_G(gameState)._personRoomNr[P_CHEWY]) {
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
		if (g_engine->_sound->soundEnabled()) {
			if (!_G(gameState).R17EnergyOut)
				_G(det)->playSound(15, 0);
		}
		break;

	ENTRY(18);
	ENTRY(19);
	ENTRY(21);
	ENTRY(22);
	ENTRY(23);

	case 24:
		Room24::entry();
		if (g_engine->_sound->soundEnabled())
			_G(det)->playSound(17, 0);
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
	ENTRY_NR(70);
	ENTRY_NR(71);
	ENTRY_NR(72);
	ENTRY_NR(73);
	ENTRY_NR(74);
	ENTRY_NR(75);
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

	_G(flags).LoadGame = false;
}

void exit_room(int16 eib_nr) {
	bool no_exit = false;
	g_engine->_sound->stopAllSounds();

	switch (_G(gameState)._personRoomNr[P_CHEWY]) {
	case 6:
		if (eib_nr == 8)
			_G(gameState).R17Location = 2;
		break;

	case 11:
		_G(atds)->setControlBit(121, ATS_ACTIVE_BIT);
		break;

	case 13:
		Room13::xit();
		break;

	case 18:
		if (eib_nr == 40)
			_G(gameState).R17Location = 2;
		else if (eib_nr == 41)
			_G(gameState).R17Location = 3;
		_G(gameState).ScrollxStep = 1;
		_G(flags).NoScroll = false;
		_G(gameState).R18Grid = false;
		_G(gameState).room_e_obj[50].Attribut = 255;
		_G(gameState).room_e_obj[41].Attribut = EXIT_BOTTOM;
		break;

	case 19:
		_G(flags).NoScroll = false;
		break;

	case 21:
		if (eib_nr == 47)
			_G(gameState).R17Location = 1;
		_G(flags).NoEndPosMovObj = false;
		_G(SetUpScreenFunc) = nullptr;
		load_chewy_taf(CHEWY_NORMAL);

		break;

	case 22:
		break;

	case 23:
		_G(gameState)._personHide[P_CHEWY] = false;
		switch (_G(gameState).R23GliderExit) {
		case 16:
			setPersonPos(126, 110, P_CHEWY, P_RIGHT);
			break;

		case 25:
			_G(gameState).R25GliderExit = true;
			break;

		default:
			break;
		}
		_G(menu_item) = CUR_WALK;
		cursorChoice(_G(menu_item));
		_G(mouseLeftClick) = false;
		break;

	case 24: Room24::xit(); break;
	case 27: Room27::xit(eib_nr); break;
	case 28: Room28::xit(eib_nr); break;
	case 29: Room29::xit(); break;

	case 34:
		_G(flags).ChewyDontGo = false;
		break;

	case 41: Room41::xit(); break;
	case 42: Room42::xit(); break;
	case 45: Room45::xit(eib_nr); break;
	case 46: Room46::xit(); break;
	case 47: Room47::xit(); break;
	case 49: Room49::xit(eib_nr); break;
	case 52: Room52::xit(); break;
	case 54: Room54::xit(eib_nr); break;
	case 56: Room56::xit(); break;
	case 57: Room57::xit(eib_nr); break;

	case 64:
		if (_G(gameState).R64Moni1Ani == 5)
			_G(gameState).R64Moni1Ani = 3;
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

	int16 x = -1;
	int16 y = -1;

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
		y = _G(moveState)[P_CHEWY].Xypos[1];
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
	case 135: {
		int16 *xy = (int16 *)_G(ablage)[_G(room_blk).AkAblage];
		x = xy[0] + 30;
		y = _G(moveState)[P_CHEWY].Xypos[1];
		}
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
	case 131: {
		int16 *xy = (int16 *)_G(ablage)[_G(room_blk).AkAblage];
		x = _G(moveState)[P_CHEWY].Xypos[0];
		y = xy[1] + 3;
		}
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
		setPersonPos(_G(moveState)[P_CHEWY].Xypos[0],
		               _G(moveState)[P_CHEWY].Xypos[1], P_CHEWY, P_RIGHT);
		setupScreen(DO_SETUP);
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
		setPersonPos(_G(moveState)[P_CHEWY].Xypos[0],
		               _G(moveState)[P_CHEWY].Xypos[1], P_CHEWY, P_LEFT);
		setupScreen(DO_SETUP);
		break;

	case 62:
		x = 20;
		y = 80;
		break;

	case 72:
		x = _G(moveState)[P_CHEWY].Xypos[0];
		y = _G(moveState)[P_CHEWY].Xypos[1] - 10;
		break;

	case 75:
		x = 160;
		y = 200;
		_G(det)->showStaticSpr(4);
		break;

	case 84:
		x = _G(moveState)[P_CHEWY].Xypos[0] - 70;
		y = _G(moveState)[P_CHEWY].Xypos[1] - 50;
		_G(HowardMov) = 1;
		break;

	case 85:
		x = _G(moveState)[P_CHEWY].Xypos[0] + 70;
		y = _G(moveState)[P_CHEWY].Xypos[1];
		_G(HowardMov) = 1;
		break;

	case 86:
		_G(det)->showStaticSpr(0);
		x = _G(moveState)[P_CHEWY].Xypos[0] - 44;
		y = _G(moveState)[P_CHEWY].Xypos[1];
		_G(HowardMov) = 2;
		break;

	case 90:
		_G(det)->showStaticSpr(8);
		x = _G(moveState)[P_CHEWY].Xypos[0] - 60;
		y = _G(moveState)[P_CHEWY].Xypos[1];
		break;

	case 94:
		_G(det)->showStaticSpr(3);
		x = _G(moveState)[P_CHEWY].Xypos[0] - 40;
		y = _G(moveState)[P_CHEWY].Xypos[1] - 10;
		break;

	case 127:
		x = 196;
		y = 133;
		_G(det)->showStaticSpr(0);
		break;

	case 132:
		x = 505;
		y = 62;
		break;

	case 140:
		x = _G(moveState)[P_CHEWY].Xypos[0] + 40;
		y = _G(moveState)[P_CHEWY].Xypos[1];
		break;

	case 141:
		x = _G(moveState)[P_CHEWY].Xypos[0] - 12;
		y = _G(moveState)[P_CHEWY].Xypos[1];
		break;

	default:
		break;
	}

	if (x != -1 && y != -1) {
		_G(spieler_mi)[P_CHEWY].Mode = true;
		goAutoXy(x, y, P_CHEWY, ANI_WAIT);
		_G(spieler_mi)[P_CHEWY].Mode = false;
	}

	if (no_exit) {
		switch (_G(gameState)._personRoomNr[P_CHEWY]) {
		case 40:
			Room40::xit(eib_nr);
			break;

		case 42:
			if (_G(gameState)._personRoomNr[P_HOWARD] == 42)
				_G(gameState)._personRoomNr[P_HOWARD] = 40;
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

static void playIntroSequence() {
	const int16 introVideo[] = {
		FCUT_135, FCUT_145, FCUT_142, FCUT_140, FCUT_145,
		FCUT_144, FCUT_142, FCUT_134, FCUT_148, FCUT_138,
		FCUT_143, FCUT_142, FCUT_146, FCUT_154, FCUT_142,
		FCUT_139, FCUT_146, FCUT_156, FCUT_157, FCUT_147,
		FCUT_153, FCUT_152, FCUT_141, FCUT_137, FCUT_136,
		FCUT_151, FCUT_151, FCUT_149, FCUT_150
	};
	const int16 introDialog[] = {
		579, 580, 581,  -1, 582,
		 -1, 583, 584,  -1,  -1,
		585, 586, 587, 588, 589,
		 -1, 590, 591,  -1,  -1,
		 -1,  -1, 592, 593, 594,
		 -1,  -1,  -1,  -1
	};
	int16 ret = 0;

	_G(atds)->load_atds(98, AAD_DATA);
	setupScreen(DO_SETUP);
	_G(out)->setPointer(nullptr);
	_G(out)->cls();

	for (int i = 0; i < 29 && ret != -1; ++i) {
		if (introVideo[i] == FCUT_135)
			g_engine->_sound->playRoomMusic(258);
		else if (introVideo[i] == FCUT_148)
			g_engine->_sound->playRoomMusic(259);
		else if (introVideo[i] == FCUT_143)
			g_engine->_sound->playRoomMusic(260);

		if (introDialog[i] != -1)
			start_aad(introDialog[i], -1, true);

		ret = g_engine->_video->playVideo(introVideo[i], false) ? 0 : -1;
		_G(atds)->stopAad();
		SHOULD_QUIT_RETURN;
	}

	// Chewy says he's in big trouble
	if (ret != -1) {
		_G(out)->setPointer(nullptr);
		_G(out)->cls();
		_G(out)->raster_col(254, 62, 35, 7);
		start_aad(595);
		_G(atds)->print_aad(254, 0);

		if (g_engine->_sound->speechEnabled()) {
			g_engine->_sound->waitForSpeechToFinish();
		} else {
			delay(6000);
		}
	}

	_G(out)->setPointer(_G(workptr));
	_G(out)->cls();
}

void flic_cut(int16 nr) {
	static const int16 FLIC_CUT_133[] = {
		FCUT_133, FCUT_123, FCUT_125, FCUT_126, FCUT_124,
		FCUT_128, FCUT_129, FCUT_130, FCUT_131, FCUT_132,
		FCUT_133, FCUT_127, FCUT_158
	};

	int16 i, ret = 0;
	bool keepPlaying = true;

	EVENTS_CLEAR;

	_G(out)->setPointer(nullptr);
	g_engine->_sound->stopAllSounds();
	g_events->delay(50);

	switch (nr) {
	case FCUT_SPACECHASE_18:
		// Play the space chase video, after escaping F5.
		// The music is placed in the first video of the
		// series, so we need to keep a copy of it and
		// dispose it after the series of videos ends.
		for (i = 0; i < 11 && keepPlaying; i++) {
			keepPlaying = g_engine->_video->playVideo(FCUT_SPACECHASE_18 + i, false, false);
		}

		g_engine->_sound->stopMusic();
		_G(out)->fadeOut();
		_G(out)->cls();
		break;

	case FCUT_058:
		g_engine->_sound->playRoomMusic(255);
		g_engine->_video->playVideo(FCUT_058);
		g_engine->_video->playVideo(FCUT_059);

		if (!_G(gameState).R43GetPgLady) {
			g_engine->_video->playVideo(FCUT_060);
		} else {
			start_aad(623, -1, true);
			g_engine->_video->playVideo(FCUT_061);
			g_engine->_video->playVideo(FCUT_062);
		}

		// TODO: Reimplement
		//_G(sndPlayer)->fadeOut(0);
		_G(out)->fadeOut();
		_G(out)->cls();
		//while (_G(sndPlayer)->musicPlaying() && !SHOULD_QUIT) {}
		break;

	case FCUT_065:
		g_engine->_sound->stopMusic();
		g_engine->_sound->playRoomMusic(256);
		Room46::kloppe();
		break;

	case FCUT_112:
		g_engine->_sound->setActiveMusicVolume(32);
		g_engine->_video->playVideo(nr);
		g_engine->_sound->setActiveMusicVolume(5);
		break;

	case FCUT_133:
		_G(out)->setPointer(nullptr);
		_G(out)->cls();

		for (i = 0; i < 13 && ret != -1; ++i) {
			ret = g_engine->_video->playVideo(FLIC_CUT_133[i]) ? 0 : -1;
			SHOULD_QUIT_RETURN;
		}
		break;

	case FCUT_135:
		playIntroSequence();
		break;

	case FCUT_155:
	case FCUT_160:
		g_engine->_video->playVideo(nr, false);
		break;

	default:
		g_engine->_video->playVideo(nr);
		break;
	}

	_G(det)->stopSound(0);
	SHOULD_QUIT_RETURN;

	g_events->delay(50);

	if (nr != FCUT_135) {
		g_engine->_sound->playRoomMusic(_G(gameState)._personRoomNr[0]);

		_G(uhr)->resetTimer(0, 0);
	}

	if (!_G(flags).NoPalAfterFlc)
		_G(out)->setPalette(_G(pal));

	_G(atds)->stopAad();
	_G(atds)->stop_ats();
	_G(out)->setPointer(_G(workptr));
	_G(flags).NoPalAfterFlc = false;
}

uint16 exit_flip_flop(int16 ani_nr, int16 eib_nr1, int16 eib_nr2,
                        int16 ats_nr1, int16 ats_nr2, int16 sib_nr,
                        int16 spr_nr1, int16 spr_nr2, int16 flag) {
	if (ani_nr != -1)
		_G(det)->startDetail(ani_nr, 1, flag);
	flag ^= 1;
	if (ats_nr1 != -1)
		_G(atds)->set_all_ats_str(ats_nr1, flag, ATS_DATA);
	if (ats_nr2 != -1)
		_G(atds)->set_all_ats_str(ats_nr2, flag, ATS_DATA);
	if (flag) {
		if (eib_nr1 != -1)
			_G(gameState).room_e_obj[eib_nr1].Attribut = spr_nr1;
		if (eib_nr2 != -1)
			_G(gameState).room_e_obj[eib_nr2].Attribut = spr_nr2;
	} else {
		if (eib_nr1 != -1)
			_G(gameState).room_e_obj[eib_nr1].Attribut = 255;
		if (eib_nr2 != -1)
			_G(gameState).room_e_obj[eib_nr2].Attribut = 255;
	}
	if (sib_nr != -1)
		_G(obj)->calc_rsi_flip_flop(sib_nr);

	return (uint16)flag;
}

int16 sib_event_no_inv(int16 sib_nr) {
	int16 ret = true;

	switch (sib_nr) {
	case SIB_CABLE_R1:
		_G(atds)->set_ats_str(8, TXT_MARK_LOOK, 1, ATS_DATA);
		break;

	case SIB_MONOCLE:
		_G(gameState).R0Monocle = true;
		_G(obj)->hide_sib(SIB_MONOCLE);
		if (_G(gameState).R0Noteboook)
			_G(atds)->delControlBit(12, ATS_ACTIVE_BIT);
		break;

	case SIB_NOTEBOOK:
		_G(gameState).R0Noteboook = true;
		_G(obj)->hide_sib(SIB_NOTEBOOK);
		if (_G(gameState).R0Monocle)
			_G(atds)->delControlBit(12, ATS_ACTIVE_BIT);
		break;

	case SIB_TERMINAL_R5:
		if (_G(gameState).R5Terminal) {
			_G(gameState).R5Terminal = 0;
			_G(det)->stopDetail(6);
			_G(atds)->set_ats_str(27, TXT_MARK_LOOK, 0, ATS_DATA);
			_G(atds)->set_ats_str(30, TXT_MARK_LOOK, 0, ATS_DATA);
		}
		break;

	case SIB_BUTTON_R5:
		Room5::pushButton();
		break;

	case SIB_SEIL:
		_G(obj)->hide_sib(SIB_SEIL);
		break;

	case SIB_BOLA_SCHACHT:
		_G(atds)->set_ats_str(41, TXT_MARK_LOOK, 0, ATS_DATA);
		break;

	case SIB_BOLA_BUTTON_R6:
		Room6::bola_button();
		break;

	case SIB_TBUTTON1_R7:
		_G(gameState).R6DoorLeftB = exit_flip_flop(3, 12, 9, 49, 35, SIB_TBUTTON2_R6,
		                                     EXIT_TOP, EXIT_LEFT,
		                                     (int16)_G(gameState).R6DoorLeftB);
		break;

	case SIB_HEBEL_R7:
		_G(gameState).R7Lever ^= 1;

		if (!_G(gameState).R7Lever)
			_G(atds)->set_all_ats_str(50, 0, ATS_DATA);
		else if (!_G(gameState).R7BorkFlug)
			_G(atds)->set_all_ats_str(50, 1, ATS_DATA);
		else
			_G(atds)->set_all_ats_str(50, 2, ATS_DATA);
		break;

	case SIB_KLINGEL_R7:
		Room7::bell();
		break;

	case SIB_GIPS_R7:
		_G(gameState).R7RHaken = true;
		_G(obj)->show_sib(SIB_RHAKEN_R7);
		_G(obj)->hide_sib(SIB_GIPS_R7);
		break;

	case SIB_TBUTTON2_R7:
		_G(gameState).R7DoorRight = exit_flip_flop(4, 13, 14, 53, 68, SIB_TUER_R8,
		                                     EXIT_RIGHT, EXIT_RIGHT,
		                                     (int16)_G(gameState).R7DoorRight);
		break;

	case SIB_SCHLOTT_R7:
		_G(obj)->hide_sib(SIB_SCHLOTT_R7);
		break;

	case SIB_LHAKEN_R7:
		if (_G(gameState).R7RopeLeft) {
			if (_G(gameState).R7RHaken) {
				_G(gameState).R7RopeOk = true;
				autoMove(4, P_CHEWY);
				_G(obj)->calc_rsi_flip_flop(SIB_LHAKEN_R7);
				_G(obj)->calc_rsi_flip_flop(SIB_RHAKEN_R7);
				_G(atds)->set_ats_str(54, TXT_MARK_LOOK, 1, ATS_DATA);
				_G(atds)->set_ats_str(55, TXT_MARK_LOOK, 1, ATS_DATA);
				_G(atds)->delControlBit(56, ATS_ACTIVE_BIT);
				start_aad(9);
			} else {
				_G(obj)->set_rsi_flip_flop(SIB_LHAKEN_R7, 2);
				_G(obj)->calc_rsi_flip_flop(SIB_LHAKEN_R7);
			}
		}
		break;

	case SIB_FOLTER_R8:
		Room8::stop_folter();
		break;

	case SIB_TUER_R8:
		_G(gameState).R7DoorRight = exit_flip_flop(-1, 14, 13, 68, 53, SIB_TBUTTON2_R7,
		                                     EXIT_RIGHT, EXIT_RIGHT,
		                                     (int16)_G(gameState).R7DoorRight);
		break;

	case SIB_DEE_PAINT_R9:
		_G(obj)->hide_sib(SIB_DEE_PAINT_R9);
		break;

	case SIB_SCHLITZ_R11:
		Room11::get_card();
		break;

	case SIB_BANDBUTTON_R13:
		_G(gameState).R13Bandlauf ^= 1;

		if (_G(gameState).R13Bandlauf) {
			for (int i = 0; i < 5; ++i)
				_G(det)->startDetail(i, 255, ANI_FRONT);
		} else {
			for (int i = 0; i < 5; ++i)
				_G(det)->stopDetail(i);
		}

		_G(atds)->set_ats_str(94, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);
		_G(atds)->set_ats_str(97, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);
		_G(atds)->set_ats_str(93, TXT_MARK_LOOK, _G(gameState).R13Bandlauf, ATS_DATA);
		break;

	case SIB_CARTRIDGE_R23:
		Room23::get_cartridge();
		break;

	case SIB_FLUXO_R23:
		_G(gameState).R23FluxoFlex = false;
		_G(atds)->set_all_ats_str(112, 0, ATS_DATA);
		_G(menu_item_vorwahl) = CUR_USE;
		break;

	case SIB_TRANSLATOR_23:
		_G(atds)->set_all_ats_str(113, 1, ATS_DATA);
		_G(menu_item_vorwahl) = CUR_USE;
		break;

	case SIB_TALISMAN_R12:
		_G(gameState).R12Talisman = true;
		_G(obj)->hide_sib(SIB_TALISMAN_R12);
		_G(timer_nr)[0] = _G(room)->set_timer(255, 20);
		break;

	case SIB_GITTER_R16:
		_G(atds)->set_all_ats_str(125, 1, ATS_DATA);
		_G(gameState).room_e_obj[33].Attribut = EXIT_TOP;
		break;

	case SIB_SCHALTER1_R21:
		_G(det)->startDetail(0, 1, _G(gameState).R21Lever1);
		_G(gameState).R21Lever1 ^= 1;
		Room21::calc_laser();
		_G(atds)->set_ats_str(126, TXT_MARK_LOOK, _G(gameState).R21Lever1, ATS_DATA);
		break;

	case SIB_SCHALTER2_R21:
		_G(det)->startDetail(1, 1, _G(gameState).R21Lever2);
		_G(gameState).R21Lever2 ^= 1;
		Room21::calc_laser();
		_G(atds)->set_ats_str(127, TXT_MARK_LOOK, _G(gameState).R21Lever2, ATS_DATA);
		break;

	case SIB_SCHALTER3_R21:
		_G(det)->startDetail(2, 1, _G(gameState).R21Lever3);
		_G(gameState).R21Lever3 ^= 1;
		Room21::calc_laser();
		_G(atds)->set_ats_str(128, TXT_MARK_LOOK, _G(gameState).R21Lever3, ATS_DATA);
		break;

	case SIB_SEIL_R21:
		_G(atds)->setControlBit(129, ATS_ACTIVE_BIT);
		break;

	case SIB_GITTER1_R21:
		Room21::use_gitter_energy();
		break;

	case SIB_CART1_R18:
		_G(atds)->setControlBit(155, ATS_ACTIVE_BIT);
		break;

	case SIB_DOORKNOB_R18:
		if (_G(gameState).R18DoorBridge) {
			_G(det)->stopSound(0);
			_G(det)->playSound(19, 1);
		} else {
			_G(det)->playSound(19, 0);
			_G(det)->stopSound(1);
		}

		if (!_G(gameState).R6DoorLeftF) {
			_G(gameState).R6DoorLeftF = exit_flip_flop(-1, 8, -1, 33, -1, SIB_TBUTTON1_R6,
				EXIT_LEFT, -1,
				(int16)_G(gameState).R6DoorLeftF);
		}
		_G(gameState).R18DoorBridge = exit_flip_flop(19, 40, 35, 148, -1, -1,
			EXIT_TOP, EXIT_LEFT,
			(int16)_G(gameState).R18DoorBridge);
		break;

	case SIB_CART_FACH_R18:
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(gameState).R18CartridgeInSlot = false;
		cur_2_inventory();
		_G(atds)->set_ats_str(157, TXT_MARK_LOOK, 0, ATS_DATA);
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
		_G(obj)->hide_sib(sib_nr);
		break;

	case SIB_PUTE_R34:
		_G(atds)->set_all_ats_str(226, 1, ATS_DATA);
		break;

	case SIB_TOPF_R31:
		_G(obj)->hide_sib(SIB_TOPF_R31);
		_G(atds)->set_all_ats_str(242, 1, ATS_DATA);
		_G(gameState).R31PflanzeWeg = true;
		break;

	case SIB_HFUTTER1_R37:
	case SIB_HFUTTER2_R37:
		_G(obj)->hide_sib(74);
		_G(obj)->hide_sib(75);
		break;

	case SIB_SURIMY_R27:
		Room27::get_surimy();
		break;

	case SIB_MUENZE_R40:
		_G(obj)->hide_sib(sib_nr);
		_G(det)->del_static_ani(6);
		_G(room)->set_timer_status(6, TIMER_STOP);
		_G(gameState).R40Geld = true;
		start_spz(CH_PUMP_TALK, 255, ANI_FRONT, P_CHEWY);
		startAadWait(201);
		break;

	case SIB_VISIT_R53:
		_G(obj)->hide_sib(sib_nr);
		_G(gameState).R53Visit = true;
		break;

	case SIB_CIGAR_R50:
		Room50::stop_cigar();
		break;

	case SIB_LAMPE_R52:
		_G(atds)->delControlBit(338, ATS_ACTIVE_BIT);
		_G(gameState).R52LichtAn ^= 1;
		setShadowPalette(2 * (_G(gameState).R52LichtAn + 1), true);
		break;

	case SIB_KAUTABAK_R56:
		_G(obj)->hide_sib(sib_nr);
		_G(gameState).R56GetTabak = true;
		break;

	case SIB_ASHTRAY_R64:
		// Pick up ashtray
		_G(det)->stopDetail(0);
		_G(obj)->hide_sib(sib_nr);
		_G(gameState).R64AshtrayTaken = true;
		Room64::talk_man(351);
		break;

	case 94:
		_G(det)->showStaticSpr(7);
		hideCur();
		startAadWait(406);
		if (_G(gameState)._personRoomNr[P_HOWARD] == 66)
			startAadWait(613);
		showCur();
		break;

	case 100:
		_G(gameState).flags33_1 = true;
		break;

	default:
		ret = false;
		break;

	}

	return ret;
}

void sib_event_inv(int16 sib_nr) {
	switch (sib_nr) {
	case SIB_TERMINAL_R5:
		if (!_G(gameState).R5Terminal) {
			_G(gameState).R5Terminal = true;
			cur_2_inventory();
			delInventory(RED_CARD_INV);
			start_aad(103, -1);
			_G(det)->startDetail(6, 255, ANI_FRONT);
			_G(atds)->set_all_ats_str(27, 1, ATS_DATA);
			_G(atds)->set_all_ats_str(30, 1, ATS_DATA);
		}
		break;

	case SIB_TBUTTON1_R6:
		_G(gameState).R6DoorLeftF = exit_flip_flop(4, 8, -1, 33, -1, -1,
			EXIT_LEFT, -1, _G(gameState).R6DoorLeftF);
		break;

	case SIB_TBUTTON2_R6:
		_G(gameState).R6DoorLeftB = exit_flip_flop(5, 9, 12, 35, 49, SIB_TBUTTON1_R7,
			EXIT_LEFT, EXIT_TOP, _G(gameState).R6DoorLeftB);
		break;

	case SIB_TBUTTON3_R6:
		_G(gameState).R6DoorRightB = exit_flip_flop(6, 11, 20, 37, 99, SIB_TBUTTON3_R11,
			EXIT_RIGHT, EXIT_LEFT, _G(gameState).R6DoorRightB);
		break;

	case SIB_BOLA_BUTTON_R6:
		delInventory(_G(cur)->getInventoryCursor());
		_G(gameState).R6BolaSchild = true;
		_G(det)->showStaticSpr(2);
		_G(obj)->calc_rsi_flip_flop(SIB_BOLA_BUTTON_R6);
		_G(obj)->hide_sib(SIB_BOLA_BUTTON_R6);
		_G(obj)->show_sib(SIB_BOLA_R6);
		break;

	case SIB_LHAKEN_R7:
	case SIB_RHAKEN_R7:
		Room7::hook(sib_nr);
		break;

	case SIB_GTUER:
		Room8::open_gdoor();
		break;

	case SIB_SURIMY_R10:
		Room10::get_surimy();
		break;

	case SIB_PAINT_R22:
		disable_timer();
		Room22::malen();
		enable_timer();
		break;

	case SIB_SCHLITZ_R11:
		Room11::put_card();
		break;

	case SIB_TBUTTON1_R11:
		_G(gameState).R11DoorRightF = exit_flip_flop(1, 23, -1, 87, -1, -1,
			EXIT_TOP, -1, _G(gameState).R11DoorRightF);
		break;

	case SIB_TBUTTON2_R11:
		_G(gameState).R11DoorRightB = exit_flip_flop(5, 22, -1, 98, -1, -1,
			EXIT_TOP, -1, _G(gameState).R11DoorRightB);
		break;

	case SIB_TBUTTON3_R11:
		_G(gameState).R6DoorRightB = exit_flip_flop(6, 20, 11, 99, 37, SIB_TBUTTON3_R6,
			EXIT_LEFT, EXIT_RIGHT, _G(gameState).R6DoorRightB);
		break;

	case SIB_CARTRIDGE_R23:
		Room23::use_cartridge();
		_G(menu_item_vorwahl) = CUR_USE;
		break;

	case SIB_FLUXO_R23:
		_G(gameState).R23FluxoFlex = true;
		delInventory(_G(cur)->getInventoryCursor());
		_G(atds)->set_all_ats_str(112, 1, ATS_DATA);
		_G(menu_item_vorwahl) = CUR_USE;
		break;

	case SIB_TRANSLATOR_23:
		delInventory(_G(cur)->getInventoryCursor());
		_G(atds)->set_all_ats_str(113, 0, ATS_DATA);
		_G(menu_item_vorwahl) = CUR_USE;
		break;

	case SIB_FEUER_R14:
		Room14::feuer();
		break;

	case SIB_ROEHRE_R12:
		_G(gameState).R12TalismanOk = true;
		delInventory(_G(cur)->getInventoryCursor());
		_G(atds)->set_ats_str(118, TXT_MARK_LOOK, 1, ATS_DATA);
		start_spz(CH_TALK6, 255, false, P_CHEWY);
		startAadWait(115);

		if (_G(gameState).R12TransOn)
			Room12::use_linke_rohr();
		break;

	case SIB_GITTER2_R21:
		_G(gameState).R21GarbageGrid = exit_flip_flop(-1, 46, 27, 132, 90, -1,
			EXIT_RIGHT, EXIT_RIGHT, _G(gameState).R21GarbageGrid);
		_G(atds)->set_ats_str(90, TXT_MARK_USE, 2, ATS_DATA);
		break;

	case SIB_CART_FACH_R18:
		start_spz_wait(CH_LGET_O, 1, false, P_CHEWY);
		_G(gameState).R18CartridgeInSlot = true;
		delInventory(_G(cur)->getInventoryCursor());
		_G(det)->showStaticSpr(7);
		_G(atds)->set_ats_str(157, TXT_MARK_LOOK, 1, ATS_DATA);

		if (_G(gameState).R18CartTerminal) {
			_G(gameState).R18CartSave = true;
			_G(atds)->set_all_ats_str(CARTRIDGE_INV, 1, INV_ATS_DATA);
			startAadWait(120);
		} else {
			startAadWait(121);
		}
		break;

	case SIB_AUTO_SITZ:
		Room29::schlitz_sitz();
		break;

	case SIB_AUTO_R28:
		Room28::use_surimy();
		break;

	default:
		break;
	}
}

} // namespace Chewy
