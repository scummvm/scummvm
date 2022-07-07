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
#include "chewy/memory.h"
#include "chewy/sound.h"
#include "chewy/detail.h"

namespace Chewy {

#define ZOBJ_ANI_DETAIL 1
#define ZOBJ_STATIC_DETAIL 2
#define ZOBJ_INVENTORY 3
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


void spriteEngine() {
	int16 min_zeiger = 0;
	int16 x, y;
	ObjMov detmov;
	TafSeqInfo *ts_info;
	calc_z_ebene();
	calc_person_ani();

	for (int16 i = 0; i < _G(z_count); i++) {
		int16 zmin = 3000;
		for (int16 j = 0; j < _G(z_count); j++) {
			if (_G(z_obj_sort)[j].ObjZ != 3000 && _G(z_obj_sort)[j].ObjZ < zmin) {
				zmin = _G(z_obj_sort)[j].ObjZ;
				min_zeiger = j;
			}
		}
		int16 nr = (int16)_G(z_obj_sort)[min_zeiger].ObjNr;

		switch (_G(z_obj_sort)[min_zeiger].ObjArt) {
		case ZOBJ_ANI_DETAIL:
			if (_G(Adi)[nr].zoom) {

				y = _G(Adi)[nr].y;
				calc_zoom(y, (int16)_G(room)->_roomInfo->_zoomFactor, (int16)_G(room)->_roomInfo->_zoomFactor, &detmov);
			} else {
				detmov.Xzoom = 0;
				detmov.Yzoom = 0;
			}
			_G(det)->plot_ani_details(_G(gameState).scrollx, _G(gameState).scrolly, nr, nr,
			                       detmov.Xzoom, detmov.Yzoom);
			break;

		case ZOBJ_STATIC_DETAIL:
			_G(det)->plot_static_details(_G(gameState).scrollx, _G(gameState).scrolly, nr, nr);
			break;

		case ZOBJ_INVENTORY:
			_G(out)->spriteSet(_G(inv_spr)[nr],
			                 _G(gameState).room_m_obj[nr].X - _G(gameState).scrollx,
			                 _G(gameState).room_m_obj[nr].Y - _G(gameState).scrolly, 0);
			break;

		case ZOBJ_CHEWY:
			if (!_G(gameState)._personHide[P_CHEWY]) {
				if (!_G(spz_ani)[P_CHEWY]) {
					int16 sprNr = _G(chewy_ph)[_G(moveState)[P_CHEWY].Phase * 8 + _G(moveState)[P_CHEWY].PhNr];
					x = _G(spieler_mi)[P_CHEWY].XyzStart[0] + _G(chewy)->correction[sprNr * 2] - _G(gameState).scrollx;
					y = _G(spieler_mi)[P_CHEWY].XyzStart[1] + _G(chewy)->correction[sprNr * 2 + 1] - _G(gameState).scrolly;
					calc_zoom(_G(spieler_mi)[P_CHEWY].XyzStart[1], (int16)_G(room)->_roomInfo->_zoomFactor,
					          (int16)_G(room)->_roomInfo->_zoomFactor, &_G(moveState)[P_CHEWY]);

					_G(out)->scale_set(_G(chewy)->image[sprNr], x, y,
					                _G(moveState)[P_CHEWY].Xzoom,
					                _G(moveState)[P_CHEWY].Yzoom,
					                _G(scr_width));
				} else {
					int16 sprNr = _G(spz_spr_nr)[_G(moveState)[P_CHEWY].PhNr];
					x = _G(spieler_mi)[P_CHEWY].XyzStart[0] + _G(spz_tinfo)->correction[sprNr * 2] -
					    _G(gameState).scrollx;
					y = _G(spieler_mi)[P_CHEWY].XyzStart[1] + _G(spz_tinfo)->correction[sprNr * 2 + 1] -
					    _G(gameState).scrolly;
					calc_zoom(_G(spieler_mi)[P_CHEWY].XyzStart[1],
					          (int16)_G(room)->_roomInfo->_zoomFactor,
					          (int16)_G(room)->_roomInfo->_zoomFactor,
					          &_G(moveState)[P_CHEWY]);

					_G(out)->scale_set(_G(spz_tinfo)->image[sprNr], x, y,
					                _G(moveState)[P_CHEWY].Xzoom,
					                _G(moveState)[P_CHEWY].Yzoom,
					                _G(scr_width));
				}
			}
			break;

		case ZOBJ_HOWARD:
		case ZOBJ_NICHELLE: {
			int16 personNr = _G(z_obj_sort)[min_zeiger].ObjArt - 6;
			if (!_G(gameState)._personHide[personNr]) {
				int16 sprNr;
				if (!_G(spz_ani)[personNr]) {
					ts_info = _G(PersonTaf)[personNr];
					sprNr = _G(PersonSpr)[personNr][_G(moveState)[personNr].PhNr];
				} else {
					ts_info = _G(spz_tinfo);
					sprNr = _G(spz_spr_nr)[_G(moveState)[personNr].PhNr];
				}

				x = _G(spieler_mi)[personNr].XyzStart[0] + ts_info->correction[sprNr * 2] - _G(gameState).scrollx;
				y = _G(spieler_mi)[personNr].XyzStart[1] + ts_info->correction[sprNr * 2 + 1] - _G(gameState).scrolly;
				calc_zoom(_G(spieler_mi)[personNr].XyzStart[1], _G(gameState).ZoomXy[personNr][0],_G(gameState).ZoomXy[personNr][1], &_G(moveState)[personNr]);
				_G(out)->scale_set(ts_info->image[sprNr], x, y, _G(moveState)[personNr].Xzoom, _G(moveState)[personNr].Yzoom, _G(scr_width));
			}
			}
			break;
		case ZOBJ_PROGANI:
			_G(out)->spriteSet(_G(spr_info)[nr]._image, _G(spr_info)[nr]._x - _G(gameState).scrollx, _G(spr_info)[nr]._y - _G(gameState).scrolly, 0);
			break;

		case ZOBJ_AUTO_OBJ: {
			int16 sprNr = _G(mov_phasen)[nr].Phase[_G(auto_mov_vector)[nr].Phase][0] +
			         _G(auto_mov_vector)[nr].PhNr;
			int16 *cxy = _G(room_blk).DetKorrekt + (sprNr << 1);
			calc_zoom(_G(auto_mov_vector)[nr].Xypos[1],
			          _G(mov_phasen)[nr].ZoomFak,
			          _G(mov_phasen)[nr].ZoomFak,
			          &_G(auto_mov_vector)[nr]);
			_G(out)->scale_set(_G(room_blk)._detImage[sprNr],
			                _G(auto_mov_vector)[nr].Xypos[0] + cxy[0] - _G(gameState).scrollx,
			                _G(auto_mov_vector)[nr].Xypos[1] + cxy[1] - _G(gameState).scrolly,
			                _G(auto_mov_vector)[nr].Xzoom,
			                _G(auto_mov_vector)[nr].Yzoom, _G(scr_width));
			}
			break;

		default:
			break;
		}
		_G(z_obj_sort)[min_zeiger].ObjZ = 3000;
	}
}

void calc_z_ebene() {
	_G(z_count) = 0;

	for (int16 i = 0; i < MAX_PERSON; i++) {
		if (_G(gameState)._personRoomNr[P_CHEWY + i] == _G(gameState)._personRoomNr[P_CHEWY] &&
		        _G(spieler_mi)[P_CHEWY + i].Id != NO_MOV_OBJ) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_CHEWY + i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(moveState)[P_CHEWY + i].Xypos[1] +
			                           _G(spieler_mi)[P_CHEWY + i].HotMovY
			                           - abs(_G(moveState)[P_CHEWY + i].Yzoom);
			++_G(z_count);
		}
	}

	for (int16 i = 0; i < MAXDETAILS; i++) {
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

	for (int16 i = 0; i < _G(obj)->mov_obj_room[0]; i++) {

		if (_G(gameState).room_m_obj[_G(obj)->mov_obj_room[i + 1]].ZEbene < 2000) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_INVENTORY;
			_G(z_obj_sort)[_G(z_count)].ObjNr = _G(obj)->mov_obj_room[i + 1];
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(gameState).room_m_obj[_G(obj)->mov_obj_room[i + 1]].ZEbene;
			++_G(z_count);
		}
	}

	for (int16 i = 0; i < MAX_PROG_ANI; i++) {
		if (_G(spr_info)[i]._zLevel < 200) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_PROGANI;
			_G(z_obj_sort)[_G(z_count)].ObjNr = i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(spr_info)[i]._zLevel;
			++_G(z_count);
		}
	}

	for (int16 i = 0; i < _G(auto_obj); i++) {
		if (_G(auto_mov_vector)[i].Xypos[2] < 200) {
			_G(z_obj_sort)[_G(z_count)].ObjArt = ZOBJ_AUTO_OBJ;
			_G(z_obj_sort)[_G(z_count)].ObjNr = i;
			_G(z_obj_sort)[_G(z_count)].ObjZ = _G(auto_mov_vector)[i].Xypos[2];
			++_G(z_count);
		}
	}
}

int16 mouse_on_prog_ani() {
	int16 aniNr = -1;
	for (int16 i = 0; i < MAX_PROG_ANI && aniNr == -1; i++) {
		if (g_events->_mousePos.x >= _G(spr_info)[i]._x && g_events->_mousePos.x <= _G(spr_info)[i].X1 &&
		        g_events->_mousePos.y >= _G(spr_info)[i]._y && g_events->_mousePos.y <= _G(spr_info)[i].Y1) {
			aniNr = i;
		}
	}

	return aniNr;
}

void setPersonPos(int16 x, int16 y, int16 personNr, int16 direction) {
	if (direction != -1)
		setPersonSpr(direction, personNr);
	_G(moveState)[personNr].Xypos[0] = x;
	_G(moveState)[personNr].Xypos[1] = y;
	_G(spieler_mi)[personNr].XyzStart[0] = x;
	_G(spieler_mi)[personNr].XyzStart[1] = y;
	_G(moveState)[personNr].Count = 0;
	_G(moveState)[personNr].Delay = _G(gameState).DelaySpeed;
	_G(moveState)[personNr]._delayCount = 0;
	calc_zoom(_G(spieler_mi)[personNr].XyzStart[1],
	          _G(gameState).ZoomXy[personNr][0],
	          _G(gameState).ZoomXy[personNr][1],
	          &_G(moveState)[personNr]);
	int16 tmpNr = personNr;
	if (personNr >= P_NICHELLE) {
		++tmpNr;
	}
	int16 x1 = _G(moveState)[personNr].Xypos[0] - _G(gameState).scrollx + _G(spieler_mi)[personNr].HotX;
	int16 y1 = _G(moveState)[personNr].Xypos[1] - _G(gameState).scrolly;
	_G(atds)->set_split_win(tmpNr, x1, y1);
	if (!_G(flags).ExitMov && personNr == P_CHEWY) {
		const int16 paletteId = _G(barriers)->getBarrierId(x + _G(spieler_mi)[personNr].HotX, y + _G(spieler_mi)[personNr].HotY);
		setShadowPalette(paletteId, true);
	}
}

void setPersonSpr(int16 nr, int16 personNr) {
	if (nr == 5 || nr == 4)
		nr = 0;
	else if (nr == 10 || nr == 7)
		nr = 1;

	switch (personNr) {
	case P_CHEWY:
		switch (nr) {
		case P_LEFT:
			_G(moveState)[P_CHEWY].Phase = CH_L_STEHEN;
			_G(moveState)[P_CHEWY].PhNr = 0;
			_G(moveState)[P_CHEWY].PhAnz = _G(chewy_ph_nr)[CH_L_STEHEN];
			_G(person_end_phase)[P_CHEWY] = P_LEFT;
			break;

		case P_RIGHT:
			_G(moveState)[P_CHEWY].Phase = CH_R_STEHEN;
			_G(moveState)[P_CHEWY].PhNr = 0;
			_G(moveState)[P_CHEWY].PhAnz = _G(chewy_ph_nr)[CH_R_STEHEN];
			_G(person_end_phase)[P_CHEWY] = P_RIGHT;
			break;

		default:
			break;
		}
		break;

	case P_HOWARD:
	case P_NICHELLE:
#define HO_L_STEHEN 0
#define HO_R_STEHEN 0
		_G(moveState)[personNr].PhNr = 0;
		_G(moveState)[personNr].PhAnz = 8;
		_G(person_end_phase)[personNr] = P_LEFT;
		switch (nr) {
		case P_LEFT:
			_G(moveState)[personNr].Phase = HO_L_STEHEN;
			_G(moveState)[personNr].PhNr = 0;
			_G(moveState)[personNr].PhAnz = 8;
			_G(person_end_phase)[personNr] = P_LEFT;
			break;

		case P_RIGHT:
			_G(moveState)[personNr].Phase = HO_R_STEHEN;
			_G(moveState)[personNr].PhNr = 0;
			_G(moveState)[personNr].PhAnz = 8;
			_G(person_end_phase)[personNr] = P_RIGHT;
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}
}

void stopPerson(int16 personNr) {
	_G(mov)->stop_auto_go();
	_G(moveState)[personNr].Count = 0;

}

void startSetAILWait(int16 aniNr, int16 rep, int16 mode) {
	const int16 oldMouseLeftClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	_G(det)->startDetail(aniNr, rep, mode);
	while (_G(det)->get_ani_status(aniNr) && !SHOULD_QUIT) {
		setupScreen(DO_SETUP);
	}
	_G(mouseLeftClick) = oldMouseLeftClick;
}

void startDetailFrame(int16 aniNr, int16 rep, int16 mode, int16 frame) {
	const int16 oldMouseLeftClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	_G(det)->startDetail(aniNr, rep, mode);
	AniDetailInfo *adi = _G(det)->getAniDetail(aniNr);
	if (mode == ANI_FRONT)
		frame = adi->ani_count + frame;
	else
		frame = adi->ani_count - frame;
	while (_G(det)->get_ani_status(aniNr) && adi->ani_count != frame && !SHOULD_QUIT) {
		setupScreen(DO_SETUP);
	}
	_G(mouseLeftClick) = oldMouseLeftClick;
}

void waitDetail(int16 detNr) {
	while (_G(det)->get_ani_status(detNr) && !SHOULD_QUIT)
		setupScreen(DO_SETUP);
}

void waitShowScreen(int16 frames) {
	const int16 oldMouseLeftClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	if (_G(gameState).DelaySpeed > 0)
		frames *= _G(gameState).DelaySpeed;
	while (--frames > 0 && !SHOULD_QUIT) {
		if (_G(flags).AniUserAction)
			get_user_key(NO_SETUP);
		setupScreen(DO_SETUP);
	}
	_G(mouseLeftClick) = oldMouseLeftClick;
}

void startAniBlock(int16 nr, const AniBlock *ab) {
	const int16 oldMouseLeftClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	for (int16 i = 0; i < nr; i++) {
		if (ab[i].Mode == ANI_WAIT)
			startSetAILWait(ab[i].Nr, ab[i].Repeat, ab[i].Dir);
		else
			_G(det)->startDetail(ab[i].Nr, ab[i].Repeat, ab[i].Dir);
	}
	_G(mouseLeftClick) = oldMouseLeftClick;
}

void startAadWait(int16 diaNr) {
	const int16 oldMouseLeftClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	_G(minfo).button = 0;
	_G(talk_start_ani) = -1;
	_G(talk_hide_static) = -1;
	setSsiPos();
	_G(atds)->start_aad(diaNr);

	while (!SHOULD_QUIT && (
		_G(atds)->aadGetStatus() != -1 ||
		g_engine->_sound->isSpeechActive()
		)) {

		if (_G(minfo).button && _G(atds)->aadGetStatus() == -1) {
			g_engine->_sound->stopSpeech();
		}

		setupScreen(DO_SETUP);

		if (_G(atds)->aadGetStatus() != -1 && g_engine->_sound->speechEnabled() && !g_engine->_sound->isSpeechActive())
			_G(atds)->stopAad();
	}

	_G(mouseLeftClick) = oldMouseLeftClick;
	if (_G(minfo).button)
		_G(flags).mainMouseFlag = 1;
	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
	stop_spz();
}

void start_aad(int16 diaNr) {
	g_engine->_sound->waitForSpeechToFinish();
	setSsiPos();
	_G(atds)->start_aad(diaNr);
}

bool startAtsWait(int16 txtNr, int16 txtMode, int16 col, int16 mode) {
	bool shown = false;
	const int16 oldMouseLeftClick = _G(mouseLeftClick);

	assert(mode == ATS_DATA || mode == INV_USE_DATA || mode == INV_USE_DEF);

	_G(mouseLeftClick) = false;
	_G(minfo).button = 0;

	if (!_G(flags).AtsText) {
		_G(flags).AtsText = true;
		if (txtNr != -1) {
			if (_G(menu_item) != CUR_WALK)
				atdsStringStart(30000, 0, 0, AAD_STR_START);

			int16 vocNr;
			shown = _G(atds)->start_ats(txtNr, txtMode, col, mode, &vocNr);

			if (g_engine->_sound->speechEnabled())  {
				if (vocNr >= 0) {
					g_engine->_sound->playSpeech(vocNr, false);
				}

				setupScreen(DO_SETUP);
			}

			if (shown) {
				while (!SHOULD_QUIT && _G(atds)->atsShown()) {
					if (g_engine->_sound->speechEnabled() && !g_engine->_sound->isSpeechActive())
						_G(atds)->stop_ats();

					if (_G(minfo).button)
						g_engine->_sound->stopSpeech();

					setupScreen(DO_SETUP);
				}
			}

			if (_G(menu_item) != CUR_WALK)
				atdsStringStart(30000, 0, 0, AAD_STR_END);
		}

		_G(flags).AtsText = false;
	}

	if (_G(minfo).button)
		_G(flags).mainMouseFlag = 1;

	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
	_G(mouseLeftClick) = oldMouseLeftClick;

	return shown;
}

void aadWait(int16 strNr) {
	const int16 oldMouseLeftClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	if (strNr == -1) {
		while (_G(atds)->aadGetStatus() != -1 && !SHOULD_QUIT) {
			setupScreen(DO_SETUP);
		}
	} else {
		while (_G(atds)->aadGetStatus() < strNr && !SHOULD_QUIT) {
			setupScreen(DO_SETUP);
		}
	}
	_G(mouseLeftClick) = oldMouseLeftClick;
	if (_G(minfo).button)
		_G(flags).mainMouseFlag = 1;
	g_events->_kbInfo._scanCode = Common::KEYCODE_INVALID;
}

void start_aad(int16 diaNr, int16 ssiNr, bool continueWhenSpeechEnds) {
	if (ssiNr == 0) {
		int16 x = _G(moveState)[P_CHEWY].Xypos[0] - _G(gameState).scrollx + _G(spieler_mi)[P_CHEWY].HotX;
		int16 y = _G(moveState)[P_CHEWY].Xypos[1] - _G(gameState).scrolly;
		_G(atds)->set_split_win(0, x, y);
	}

	_G(atds)->start_aad(diaNr, continueWhenSpeechEnds);
}

void startDialogCloseupWait(int16 diaNr) {
	if (!_G(flags).DialogCloseup) {
		_G(menu_item) = CUR_TALK;
		cursorChoice(_G(menu_item));

		loadDialogCloseup(diaNr);

		while (_G(flags).DialogCloseup && !SHOULD_QUIT) {
			setupScreen(DO_SETUP);
		}
	}
}

void wait_auto_obj(int16 nr) {
	const int16 oldMouseLeftClick = _G(mouseLeftClick);
	_G(mouseLeftClick) = false;
	while (_G(mov_phasen)[nr].Repeat != -1 && !SHOULD_QUIT) {
		setupScreen(DO_SETUP);
	}
	_G(mouseLeftClick) = oldMouseLeftClick;
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
	const MovLine *tmp2 = mline;
	int16 *tmp = (int16 *)_G(mov_phasen)[auto_nr].Phase;
	for (int16 i = 0; i < 8; i++)
		tmp[i] = phasen[i];
	_G(mov_phasen)[auto_nr].Start = 1;

	MovLine *tmp1 = _G(mov_line)[auto_nr];
	for (int16 i = 0; i < lines; i++) {
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
	_G(auto_mov_vector)[auto_nr]._delayCount = 0;
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
				_G(auto_mov_obj)[nr].XyzEnd[0] = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].EndXyz[0];
				_G(auto_mov_obj)[nr].XyzEnd[1] = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].EndXyz[1];
				_G(auto_mov_obj)[nr].XyzEnd[2] = _G(mov_line)[nr][_G(auto_mov_vector)[nr].StNr - 1].EndXyz[2];
				_G(mov)->get_mov_vector((int16 *)_G(auto_mov_obj)[nr].XyzStart, (int16 *)_G(auto_mov_obj)[nr].XyzEnd, 
				                     _G(auto_mov_obj)[nr].Vorschub, &_G(auto_mov_vector)[nr]);
			}
		} else {
			_G(auto_mov_vector)[nr].Xypos[2] = 201;
		}
	}
}

int16 mouse_auto_obj(int16 nr, int16 xoff, int16 yoff) {
	int16 ret = false;
	if (_G(mov_phasen)[nr].Start == 1) {
		int16 spr_nr = _G(mov_phasen)[nr].Phase[_G(auto_mov_vector)[nr].Phase][0] +
		               _G(auto_mov_vector)[nr].PhNr;
		int16 *xy = (int16 *)_G(room_blk)._detImage[spr_nr];
		int16 *Cxy = _G(room_blk).DetKorrekt + (spr_nr << 1);

		if (!xoff) {
			xoff = xy ? xy[0] : 0;
			xoff += _G(auto_mov_vector)[nr].Xzoom;
		}
		if (!yoff) {
			yoff = xy ? xy[1] : 0;
			yoff += _G(auto_mov_vector)[nr].Yzoom;
		}
		if (g_events->_mousePos.x >= _G(auto_mov_vector)[nr].Xypos[0] + Cxy[0] - _G(gameState).scrollx &&
		        g_events->_mousePos.x <= _G(auto_mov_vector)[nr].Xypos[0] + xoff + Cxy[0] - _G(gameState).scrollx &&
		        g_events->_mousePos.y >= _G(auto_mov_vector)[nr].Xypos[1] + Cxy[1] - _G(gameState).scrolly &&
		        g_events->_mousePos.y <= _G(auto_mov_vector)[nr].Xypos[1] + yoff + Cxy[1] - _G(gameState).scrolly)
			ret = true;
	}
	return ret;
}

int16 auto_obj_status(int16 nr) {
	int16 status = false;
	if (nr < _G(auto_obj)) {
		if (_G(mov_phasen)[nr].Repeat != -1)
			status = true;
	}

	return status;
}

void calc_zoom(int16 y, int16 zoomFactorX, int16 zoomFactorY, ObjMov *om) {
	float factorX = (float)zoomFactorX / (float)100.0;
	factorX = -factorX;
	float factorY = (float)zoomFactorY / (float)100.0;
	factorY = -factorY;
	if (!_G(zoom_horizont)) {
		om->Xzoom = -zoomFactorX;
		om->Yzoom = -zoomFactorY;
	} else {
		if (y < _G(zoom_horizont)) {
			om->Xzoom = (_G(zoom_horizont) - y) * factorX;
			om->Yzoom = (_G(zoom_horizont) - y) * factorY;
		} else {
			om->Xzoom = 0;
			om->Yzoom = 0;
		}
	}
}

void mov_objekt(ObjMov *om, MovInfo *mi) {
	int16 u_index = 0;

	if (om->_delayCount > 0)
		--om->_delayCount;
	else {
		om->_delayCount = om->Delay;
		if (om->Count > 0) {

			--om->Count;
			if (om->PhNr < om->PhAnz - 1)
				++om->PhNr;
			else
				om->PhNr = 0;
			int16 tmpx = om->Xyvo[0];
			int16 tmpy = om->Xyvo[1];
			int16 tmpz = om->Xyvo[2];
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
				if (!(u_index = _G(barriers)->getBarrierId(om->Xypos[0] + mi->HotX + tmpx,
				                                      om->Xypos[1] + mi->HotY + tmpy))) {

					if (!(u_index = _G(barriers)->getBarrierId(om->Xypos[0] + mi->HotX + tmpx,
					                                      om->Xypos[1] + mi->HotY))) {

						if (!(u_index = _G(barriers)->getBarrierId(om->Xypos[0] + mi->HotX,
						                                      om->Xypos[1] + mi->HotY + tmpy))) {
							om->Count = 0;
						} else {
							if (!tmpy) {
								if (om->Xyna[1][1] < 0)
									tmpy = -1;
								else
									tmpy = 1;
							}
							if (mi->Id == CHEWY_OBJ)
								setShadowPalette(u_index, true);

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
								_G(mov)->get_mov_vector(mi->XyzStart, mi->XyzEnd, mi->Vorschub, om);
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
							setShadowPalette(u_index, true);

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
							_G(mov)->get_mov_vector(mi->XyzStart, mi->XyzEnd, mi->Vorschub, om);
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
						setShadowPalette(u_index, true);
					om->Xypos[0] += tmpx;
					om->Xypos[1] += tmpy;
					om->Xypos[2] += tmpz;
					if (_G(new_vector)) {
						_G(new_vector) = false;
						mi->XyzStart[0] = om->Xypos[0];
						mi->XyzStart[1] = om->Xypos[1];
						_G(mov)->get_mov_vector(mi->XyzStart, mi->XyzEnd, mi->Vorschub, om);
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
					u_index = _G(barriers)->getBarrierId(om->Xypos[0] + mi->HotX,
					                                om->Xypos[1] + mi->HotY);
					setShadowPalette(u_index, true);
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
	if (_G(ani_stand_count)[p_nr] >= 25 * (_G(gameState).DelaySpeed + p_nr * 2)) {
		_G(ani_stand_count)[p_nr] = 0;
		om->PhNr = 0;
		_G(ani_stand_flag)[p_nr] = true;
		setPersonSpr(_G(person_end_phase)[p_nr], p_nr);
	} else if (_G(ani_stand_flag)[p_nr]) {
		om->Delay = _G(gameState).DelaySpeed + 1;
		if (om->PhNr < om->PhAnz - 1)
			++om->PhNr;
		else
			_G(ani_stand_flag)[p_nr] = false;
	} else {
		setPersonSpr(_G(person_end_phase)[p_nr], p_nr);
		om->PhNr = 0;
		++_G(ani_stand_count)[p_nr];
	}
}

void get_phase(ObjMov *om, MovInfo *mi) {
	int16 p_nr = 255 - mi->Id;
	if (p_nr >= 0) {
		if (om->Xyvo[0] > 0) {
			if (om->Xyvo[1] > 0) {

				if (om->Xyvo[1] > (mi->Vorschub - 1)) {
					om->Phase = CH_DOWN;
					if (om->Xypos[0] - _G(gameState).scrollx > SCREEN_WIDTH / 2)
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
					if (om->Xypos[0] - _G(gameState).scrollx > SCREEN_WIDTH / 2)
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
					if (om->Xypos[0] - _G(gameState).scrollx > SCREEN_WIDTH / 2)
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
					if (om->Xypos[0] - _G(gameState).scrollx > SCREEN_WIDTH / 2)
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
	int16 p_nr = 255 - mode;
	if (p_nr >= 0) {
		if (om->Xyvo[0] > 0) {
			om->Phase = CH_RIGHT_NO;
		} else {
			om->Phase = CH_LEFT_NO;
		}
	}
}

void zoom_mov_anpass(ObjMov *om, MovInfo *mi) {
	int16 tmp_vorschub = mi->Vorschub;
	mi->Vorschub = CH_X_PIX;
	if (_G(flags).ZoomMov) {
		if (om->Xzoom < 0 && _G(zoom_mov_fak) > 0) {
			int16 tmp = om->Xzoom / _G(zoom_mov_fak);
			tmp = abs(tmp);
			mi->Vorschub -= tmp;
			if (mi->Vorschub < 2)
				mi->Vorschub = 2;
		}
		if (tmp_vorschub != mi->Vorschub && om->Count) {
			_G(mov)->get_mov_vector((int16 *)mi->XyzStart, (int16 *)mi->XyzEnd, mi->Vorschub, om);
			get_phase(om, mi);
		}
	}
}

void start_spz_wait(int16 ani_id, int16 count, bool reverse, int16 p_nr) {
	if (start_spz(ani_id, count, reverse, p_nr)) {
		while (_G(spz_count) && !SHOULD_QUIT)
			setupScreen(DO_SETUP);
	}
}

bool start_spz(int16 ani_id, int16 count, bool reverse, int16 p_nr) {
	bool ret = false;
	if (!_G(flags).SpzAni) {
		_G(flags).SpzAni = true;
		_G(spz_ani)[p_nr] = true;
		_G(spz_p_nr) = p_nr;
		int16 spr_start = SPZ_ANI_PH[ani_id][0];
		int16 spr_anz = SPZ_ANI_PH[ani_id][1];
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

		for (int16 i = 0; i < spr_anz; i++) {
			if (!reverse)
				_G(spz_spr_nr)[i] = i;
			else
				_G(spz_spr_nr)[i] = spr_anz - i - 1;
		}

		_G(spz_start) = spr_start;
		_G(spz_delay)[p_nr] = _G(SpzDelay);
		_G(moveState)[p_nr].Count = 0;
		_G(moveState)[p_nr].PhNr = 0;
		_G(moveState)[p_nr].PhAnz = spr_anz;
		_G(moveState)[p_nr].Delay = _G(gameState).DelaySpeed + _G(spz_delay)[p_nr];
		_G(moveState)[p_nr]._delayCount = 0;
		_G(spz_count) = count;
		_G(flags).MouseLeft = true;
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
			om->Delay = _G(gameState).DelaySpeed + _G(spz_delay)[_G(spz_p_nr)];
		} else {
			if (_G(spz_count) != 255)
				stop_spz();
		}
	}
}

void stop_spz() {
	if (_G(flags).SpzAni) {
		_G(flags).SpzAni = false;
		_G(flags).MouseLeft = false;
		_G(spz_ani)[_G(spz_p_nr)] = false;
		_G(moveState)[_G(spz_p_nr)].Count = 0;
		_G(moveState)[_G(spz_p_nr)].PhNr = 0;
		setPersonSpr(_G(person_end_phase)[_G(spz_p_nr)], _G(spz_p_nr));
		_G(spz_delay)[_G(spz_p_nr)] = 0;
	}
}

void set_spz_delay(int16 delay) {
	_G(SpzDelay) = delay;
}

void load_person_ani(int16 ani_id, int16 p_nr) {
	if (_G(PersonAni)[p_nr] != ani_id) {
		int16 ani_start = SPZ_ANI_PH[ani_id][0];
		short ani_anz = SPZ_ANI_PH[ani_id][1];
		_G(PersonAni)[p_nr] = ani_id;
		if (_G(PersonTaf)[p_nr])
			free((char *)_G(PersonTaf)[p_nr]);
		_G(PersonTaf)[p_nr] = _G(mem)->taf_seq_adr(ani_start, ani_anz);
		_G(moveState)[p_nr].PhNr = 0;
		_G(moveState)[p_nr].PhAnz = ani_anz;
	}
}

uint8 p_ani[MAX_PERSON - 1][5] = {
	{HO_WALK_L, HO_WALK_R, HO_BACK, HO_FRONT, HO_STAND_L},
	{NI_WALK_L, NI_WALK_R, NI_BACK, NI_FRONT, NI_STAND_L}
};

void calc_person_ani() {
	int16 ani_nr = 0;
	for (int16 p_nr = 0; p_nr < MAX_PERSON; p_nr++) {
		if (_G(spieler_mi)[p_nr].Id != NO_MOV_OBJ) {
			switch (p_nr) {
			case P_CHEWY:
				break;

			case P_HOWARD:
			case P_NICHELLE:
				if (!_G(spz_ani)[p_nr]) {
					for (int16 i = 0; i < 8; i++)
						_G(PersonSpr)[p_nr][i] = i;

					if (!_G(moveState)[p_nr].Count &&
					        _G(auto_p_nr) != p_nr) {
						ani_nr = (int16)p_ani[p_nr - 1][4] + (_G(person_end_phase)[p_nr] * 4);

						_G(moveState)[p_nr].PhAnz = 5;
						_G(PersonSpr)[p_nr][3] = 1;
						_G(PersonSpr)[p_nr][4] = 0;
					} else {
						switch (_G(moveState)[p_nr].Phase) {
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

						default:
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
