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
#include "chewy/memory.h"
#include "chewy/resource.h"
#include "chewy/room.h"

#include "cursor.h"
#include "chewy/sound.h"

namespace Chewy {

const int16 SURIMY_TAF19_PHASES[4][2] = {
	{ 0, 0 },
	{ 39, 46 },
	{ 0, 0 },
	{ 0, 0 }
};

static const uint8 CHEWY_PHASES[8][8] = {
	{  0, 1, 2, 3, 4, 5, 6, 7 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 32, 33, 34, 33, 32 },
	{ 35, 36, 37, 38, 39, 40, 41, 42 },
	{ 43, 44, 45, 46, 47, 48, 49, 50 },
	{ 51, 52, 53, 52, 51 }
};

static const uint8 CHEWY_PHASE_NR[] = { 8, 8, 8, 8, 5, 8, 8, 5 };

static const uint8 CHEWY_BO_PHASES[8][8] = {
	{  0, 1, 2, 3, 4, 5 },
	{  6, 7, 8, 9, 10, 11 },
	{ 12, 13, 14, 15, 16, 17 },
	{ 12, 13, 14, 15, 16, 17 },
	{ 18, 19, 20, 19, 18 },
	{ 21, 22, 23, 24, 25, 26 },
	{ 21, 22, 23, 24, 25, 26 },
	{ 27, 28, 29, 28, 27 }
};

static const uint8 CHEWY_BO_PHASE_NR[] = { 6, 6, 6, 6, 5, 6, 6, 5 };

static const uint8 CHEWY_MI_PHASES[8][8] = {
	{  0, 1, 2, 3, 4, 5, 6, 7 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 32, 33, 34, 33, 32 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 35, 36, 37, 36, 35 }
};

static const uint8 CHEWY_MI_PHASE_NR[] = { 8, 8, 8, 8, 5, 8, 8, 5 };

static const uint8 CHEWY_RO_PHASES[8][8] = {
	{  0, 1, 2, 3, 4, 5, 6, 7 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 32, 33, 34, 35, 36, 35, 34, 33 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 37, 38, 39, 40, 41, 40, 39, 38 }
};

static const uint8 CHEWY_RO_PHASE_NR[] = { 8, 8, 8, 8, 8, 8, 8, 8 };

static const uint8 CHEWY_JM_PHASES[8][8] = {
	{  0,  1,  2,  3,  4,  5,  6,  7 },
	{  8,  9, 10, 11, 12, 13, 14, 15 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 24, 25, 26, 27, 27, 26, 25, 24 },
	{  8,  9, 10, 11, 12, 13, 14, 15 },
	{  8,  9, 10, 11, 12, 13, 14, 15 },
	{ 28, 29, 30, 31, 31, 30, 29, 28 }
};

static const uint8 CHEWY_JM_PHASE_NR[] = {8, 8, 8, 8, 8, 8, 8, 8};

void JungleRoom::topEntry() {
	hideCur();
	setPersonPos(236, 110, P_CHEWY, P_RIGHT);
	setPersonPos(263, 85, P_NICHELLE, P_RIGHT);
	setPersonPos(285, 78, P_HOWARD, P_RIGHT);
	goAutoXy(266, 113, P_HOWARD, ANI_WAIT);
	showCur();
}

void JungleRoom::leftEntry() {
	setPersonPos(31, 118, P_CHEWY, P_RIGHT);
	setPersonPos(71, 104, P_NICHELLE, P_RIGHT);
	setPersonPos(6, 111, P_HOWARD, P_RIGHT);
}

void JungleRoom::rightEntry() {
	setPersonPos(587, 114, P_CHEWY, P_LEFT);
	setPersonPos(613, 103, P_NICHELLE, P_LEFT);
	setPersonPos(561, 112, P_HOWARD, P_LEFT);
}

void JungleRoom::setup_func() {
	calc_person_look();

	const int posX = _G(moveState)[P_CHEWY].Xypos[0];

	int howDestX, nicDestX;
	if (posX < 40) {
		howDestX = 52;
		nicDestX = 100;
	} else if (posX < 230) {
		howDestX = 83;
		nicDestX = 163;
	} else if (posX < 445) {
		howDestX = 261;
		nicDestX = 329;
	} else {
		howDestX = 493;
		nicDestX = 543;
	}

	goAutoXy(howDestX, 111, P_HOWARD, ANI_GO);
	goAutoXy(nicDestX, 110, P_NICHELLE, ANI_GO);
}

Room::Room() {
	_roomTimer._timerMaxNr = 0;
	_roomTimer._timerStart = 0;
	init_ablage();
	_roomInfo = nullptr;
	_barriers = new BarrierResource(EPISODE1_GEP);
}

Room::~Room() {
	delete _barriers;
	free_ablage();
}

void Room::loadRoom(RaumBlk *Rb, int16 room_nr, GameState *player) {
	clear_prog_ani();
	_G(det)->load_rdi(Rb->DetFile, room_nr);

	RoomDetailInfo *Rdi_ = _G(det)->getRoomDetailInfo();
	_roomInfo = &Rdi_->Ri;
	if (_roomInfo->_tafLoad != 255) {
		_G(det)->load_rdi_taf(_roomInfo->_tafName, _roomInfo->_tafLoad);
		Rb->Fti = _G(det)->get_taf_info();
		Rb->_detImage = Rb->Fti->image;
		Rb->DetKorrekt = Rb->Fti->correction;
	}
	_G(obj)->calc_all_static_detail();
	load_tgp(_roomInfo->_imageNr, Rb, EPISODE1_TGP, true, EPISODE1);
	set_pal(_ablagePal[Rb->AkAblage], Rb->LowPalMem);
	calc_invent(Rb, player);

	del_timer_old_room();
	add_timer_new_room();

	if (Rb->AtsLoad)
		_G(atds)->load_atds(_roomInfo->_roomNr, ATS_DATA);

	if (Rb->AadLoad)
		_G(atds)->load_atds(_roomInfo->_roomNr, AAD_DATA);
}

void Room::set_timer_start(int16 timer_start) {
	_roomTimer._timerStart = timer_start;
}

void Room::add_timer_new_room() {
	_roomTimer._timerMaxNr = 0;

	for (int i = 0; i < MAXDETAILS && _roomTimer._timerMaxNr < MAX_ROOM_TIMER; i++) {
		AniDetailInfo *adi = _G(det)->getAniDetail(i);
		if (adi->timer_start != 0) {
			set_timer(i, adi->timer_start);
		} else if (adi->start_flag || adi->repeat) {
			_G(det)->startDetail(i, 0, ANI_FRONT);
		}
	}
}

void Room::del_timer_old_room() {
	for (int i = 0; i < _roomTimer._timerMaxNr; i++) {
		_G(uhr)->setStatus(_roomTimer._timerNr[i], TIMER_STOP);

	}

	_roomTimer._timerMaxNr = 0;
}

int16 Room::set_timer(int16 ani_nr, int16 timer_end) {

	int16 timer_nr_ = _roomTimer._timerStart + _roomTimer._timerMaxNr;
	int16 ret = _G(uhr)->setNewTimer(timer_nr_, timer_end, SEC_MODE);
	if (ret != -1) {
		_roomTimer._objNr[_roomTimer._timerMaxNr] = ani_nr;
		_roomTimer._timerNr[_roomTimer._timerMaxNr] = timer_nr_;
		++_roomTimer._timerMaxNr;
	}

	return timer_nr_;
}

void Room::set_timer_status(int16 ani_nr, int16 status) {
	for (int i = 0; i < _roomTimer._timerMaxNr; i++) {
		if (_roomTimer._objNr[i] == ani_nr) {
			_G(uhr)->setStatus(_roomTimer._timerNr[i], status);
		}
	}
}

void Room::set_zoom(int16 zoom) {
	_roomInfo->_zoomFactor = (uint8)zoom;
}

void Room::set_pal(const byte *src_pal, byte *dest_pal) {
	memcpy(dest_pal, src_pal, 768l);
	dest_pal[767] = 63;
	dest_pal[766] = 63;
	dest_pal[765] = 63;
	dest_pal[0] = 0;
	dest_pal[1] = 0;
	dest_pal[2] = 0;
}

void Room::set_ak_pal(RaumBlk *Rb) {
	set_pal(_ablagePal[Rb->AkAblage], Rb->LowPalMem);
}

void Room::calc_invent(RaumBlk *Rb, GameState *player) {
	byte *tmp_inv_spr[MAX_MOV_OBJ];

	_G(obj)->sort();
	memcpy(tmp_inv_spr, Rb->InvSprAdr, MAX_MOV_OBJ * sizeof(char *));
	memset(Rb->InvSprAdr, 0, MAX_MOV_OBJ * sizeof(char *));

	SpriteResource *spriteRes = new SpriteResource(Rb->InvFile);

	for (int16 i = 1; i < _G(obj)->mov_obj_room[0] + 1; i++) {
		if (!tmp_inv_spr[_G(obj)->mov_obj_room[i]]) {
			spriteRes->getSpriteData(_G(obj)->mov_obj_room[i], &Rb->InvSprAdr[_G(obj)->mov_obj_room[i]], true);
		} else {
			Rb->InvSprAdr[_G(obj)->mov_obj_room[i]] = tmp_inv_spr[_G(obj)->mov_obj_room[i]];
			tmp_inv_spr[_G(obj)->mov_obj_room[i]] = nullptr;
		}
	}

	for (int16 i = 1; i < _G(obj)->spieler_invnr[0] + 1; i++) {
		if (!tmp_inv_spr[_G(obj)->spieler_invnr[i]]) {
			spriteRes->getSpriteData(_G(obj)->spieler_invnr[i], &Rb->InvSprAdr[_G(obj)->spieler_invnr[i]], true);
		} else {
			Rb->InvSprAdr[_G(obj)->spieler_invnr[i]] = tmp_inv_spr[_G(obj)->spieler_invnr[i]];
			tmp_inv_spr[_G(obj)->spieler_invnr[i]] = 0;
		}
	}

	for (int16 i = 0; i < MAX_MOV_OBJ; i++) {
		if (tmp_inv_spr[i] != 0)
			free(tmp_inv_spr[i]);
	}

	if (_G(cur)->usingInventoryCursor()) {
		const int cursor = _G(cur)->getInventoryCursor();
		if (Rb->InvSprAdr[cursor] == nullptr) {
			spriteRes->getSpriteData(cursor, &Rb->InvSprAdr[cursor], true);
		}
	}

	delete spriteRes;
}

int16 Room::load_tgp(int16 nr, RaumBlk *Rb, int16 tgp_idx, bool loadBarriers, const char *fileName) {
	BackgroundResource *res = new BackgroundResource(fileName);
	TBFChunk *img = res->getImage(nr, false);

	Rb->AkAblage = get_ablage(nr + (1000 * tgp_idx), img->size + 4);

	if (Rb->AkAblage == -1) {
	} else if (Rb->AkAblage >= 1000) {
		Rb->AkAblage -= 1000;
	} else {
		// Image width and height is piggy-banked inside the image data
		uint16 *memPtr = (uint16 *)_ablage[Rb->AkAblage];
		memPtr[0] = img->width;
		memPtr[1] = img->height;
		memcpy(_ablage[Rb->AkAblage] + 4, img->data, img->size);
		memcpy(_ablagePal[Rb->AkAblage], img->palette, 3 * 256);
		set_ablage_info(Rb->AkAblage, nr + (1000 * tgp_idx), img->size);

		if (loadBarriers)
			_barriers->init(nr, img->width, img->height);
	}

	delete img;
	delete res;

	return true;
}

void Room::init_ablage() {
	_lastAblageSave = 0;
	_ablage[0] = (byte *)MALLOC(MAX_ABLAGE * (ABLAGE_BLOCK_SIZE + 4l));
	_ablagePal[0] = (byte *)MALLOC(MAX_ABLAGE * 768l);
	_akAblage = 0;
	for (int16 i = 0; i < MAX_ABLAGE; i++) {
		_ablage[i] = _ablage[0] + (ABLAGE_BLOCK_SIZE + 4l) * i;
		_ablageInfo[i][0] = -1;
		_ablageInfo[i][1] = -1;
		_ablagePal[i] = _ablagePal[0] + 768l * i;
	}
}

void Room::free_ablage() {
	free(_ablagePal[0]);
	free(_ablage[0]);
	_akAblage = -1;
}

byte *Room::get_ablage(int16 nr) {
	byte *ret = nullptr;

	if (nr < MAX_ABLAGE && _akAblage != -1) {
		ret = _ablage[nr];
	}
	return ret;
}

byte **Room::get_ablage() {
	byte **ret = nullptr;

	if (_akAblage != -1) {
		ret = &_ablage[0];
	}
	return ret;
}

int16 Room::get_ablage(int16 pic_nr, uint32 pic_size) {
	int16 ret = -1;
	uint32 ablage_bedarf = pic_size / ABLAGE_BLOCK_SIZE;
	if (pic_size % ABLAGE_BLOCK_SIZE > 4)
		++ablage_bedarf;
	int16 ende = 0;

	for (int16 i = 0; i < MAX_ABLAGE && !ende; i++) {
		if (_ablageInfo[i][0] == pic_nr &&
		        _ablageInfo[i][1] != 255) {
			ende = 1;
			ret = 1000 + i;
		}
	}

	if (!ende) {
		for (int16 i = 0; i < MAX_ABLAGE && !ende; i++) {
			ret = get_ablage_g1((int16)ablage_bedarf, i);
			ende = 1;
		}
	}
	if (ret != -1) {
		if (ret < 1000) {

			ende = 0;
			while (!ende) {
				if (_ablageInfo[_lastAblageSave][1] == 255) {
					--_lastAblageSave;
					if (_lastAblageSave < 0) {
						_lastAblageSave = 0;
						_ablageInfo[0][1] = MAX_ABLAGE;
					}
				} else
					ende = 1;
			}
			int16 i = _ablageInfo[_lastAblageSave][1];
			for (int16 j = _lastAblageSave; j < _lastAblageSave + i; j++) {
				_ablageInfo[j][0] = -1;
				_ablageInfo[j][1] = -1;
			}
		}
	}
	return ret;
}

int16 Room::get_ablage_g1(int16 ablage_bedarf, int16 ak_pos) {
	short ret = 0;
	bool endLoop = false;
	int16 count = 0;
	_lastAblageSave = ak_pos;
	while (!endLoop) {
		if (_lastAblageSave >= MAX_ABLAGE)
			_lastAblageSave = 0;
		if (ablage_bedarf == 1) {
			endLoop = true;
			ret = _lastAblageSave;
		} else if (ablage_bedarf <= MAX_ABLAGE - _lastAblageSave) {
			endLoop = true;
			ret = _lastAblageSave;
		} else
			++_lastAblageSave;
		++count;
		if (count > MAX_ABLAGE) {
			ret = -1;
			endLoop = true;
		}
	}
	return ret;
}

void Room::set_ablage_info(int16 ablagenr, int16 bildnr, uint32 pic_size) {
	uint32 ablage_bedarf = (pic_size / ABLAGE_BLOCK_SIZE);
	if (pic_size % ABLAGE_BLOCK_SIZE > 4)
		++ablage_bedarf;
	int16 j = (int16)ablage_bedarf;
	for (int16 i = ablagenr; i < j + ablagenr; i++) {
		_ablageInfo[i][0] = bildnr;
		_ablageInfo[i][1] = (int16)ablage_bedarf;
		ablage_bedarf = 255;
		++_lastAblageSave;
	}
}

void load_chewy_taf(int16 taf_nr) {
	if (_G(AkChewyTaf) != taf_nr) {
		if (_G(chewy)) {
			free((char *)_G(chewy));
			_G(chewy) = nullptr;
		}
		_G(spieler_mi)[P_CHEWY].HotY = CH_HOT_Y;

		const char *filename;
		switch (taf_nr) {
		case CHEWY_NORMAL:
			filename = CHEWY_TAF;
			_G(chewy_ph_nr) = CHEWY_PHASE_NR;
			_G(chewy_ph) = (const uint8 *)CHEWY_PHASES;
			break;

		case CHEWY_BORK:
			filename = CHEWY_BO_TAF;
			_G(chewy_ph_nr) = CHEWY_BO_PHASE_NR;
			_G(chewy_ph) = (const uint8 *)CHEWY_BO_PHASES;
			break;

		case CHEWY_MINI:
			filename = CHEWY_MI_TAF;
			_G(chewy_ph_nr) = CHEWY_MI_PHASE_NR;
			_G(chewy_ph) = (const uint8 *)CHEWY_MI_PHASES;
			break;

		case CHEWY_PUMPKIN:
			filename = CHEWY_PUMP_TAF;
			_G(chewy_ph_nr) = CHEWY_MI_PHASE_NR;
			_G(chewy_ph) = (const uint8 *)CHEWY_MI_PHASES;
			break;

		case CHEWY_ROCKER:
			filename = CHEWY_ROCK_TAF;
			_G(chewy_ph_nr) = CHEWY_RO_PHASE_NR;
			_G(chewy_ph) = (const uint8 *)CHEWY_RO_PHASES;
			break;

		case CHEWY_JMANS:
			filename = CHEWY_JMAN_TAF;
			_G(chewy_ph_nr) = CHEWY_JM_PHASE_NR;
			_G(chewy_ph) = (const uint8 *)CHEWY_JM_PHASES;
			_G(spieler_mi)[P_CHEWY].HotY = 68;
			break;

		case CHEWY_ANI7:
			filename = CHEWY_HELM_TAF;
			_G(chewy_ph_nr) = CHEWY_MI_PHASE_NR;
			_G(chewy_ph) = (const uint8 *)CHEWY_MI_PHASES;
			break;

		default:
			filename = nullptr;
			break;

		}
		if (filename != nullptr) {
			_G(gameState).ChewyAni = taf_nr;
			_G(AkChewyTaf) = taf_nr;
			_G(chewy) = _G(mem)->taf_adr(filename);
		}
	}
}

void switchRoom(int16 nr) {
	_G(fx_blend) = BLEND1;
	exit_room(-1);
	_G(gameState)._personRoomNr[P_CHEWY] = nr;
	_G(room)->loadRoom(&_G(room_blk), _G(gameState)._personRoomNr[P_CHEWY], &_G(gameState));
	enter_room(-1);
	setupScreen(DO_SETUP);
}

void calc_person_look() {
	for (int16 i = 1; i < MAX_PERSON; i++) {
		if (_G(spieler_mi)[i].Id != NO_MOV_OBJ) {

			if (_G(moveState)[i].Xypos[0] > _G(moveState)[P_CHEWY].Xypos[0])
				_G(person_end_phase)[i] = P_LEFT;
			else
				_G(person_end_phase)[i] = P_RIGHT;
		}
	}
}

} // namespace Chewy
