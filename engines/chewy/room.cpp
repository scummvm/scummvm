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
#include "chewy/resource.h"
#include "chewy/room.h"
#include "chewy/ngshext.h"

namespace Chewy {

const int16 SURIMY_TAF19_PHASEN[4][2] = {
	{ 0, 0 },
	{ 39, 46 },
	{ 0, 0 },
	{ 0, 0 }
};

static const uint8 CHEWY_PHASEN[8][8] = {
	{  0, 1, 2, 3, 4, 5, 6, 7 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 32, 33, 34, 33, 32 },
	{ 35, 36, 37, 38, 39, 40, 41, 42 },
	{ 43, 44, 45, 46, 47, 48, 49, 50 },
	{ 51, 52, 53, 52, 51 }
};

static const uint8 CHEWY_PHASEN_ANZ[] = { 8, 8, 8, 8, 5, 8, 8, 5 };

static const uint8 CHEWY_BO_PHASEN[8][8] = {
	{  0, 1, 2, 3, 4, 5 },
	{  6, 7, 8, 9, 10, 11 },
	{ 12, 13, 14, 15, 16, 17 },
	{ 12, 13, 14, 15, 16, 17 },
	{ 18, 19, 20, 19, 18 },
	{ 21, 22, 23, 24, 25, 26 },
	{ 21, 22, 23, 24, 25, 26 },
	{ 27, 28, 29, 28, 27 }
};

static const uint8 CHEWY_BO_PHASEN_ANZ[] = { 6, 6, 6, 6, 5, 6, 6, 5 };

static const uint8 CHEWY_MI_PHASEN[8][8] = {
	{  0, 1, 2, 3, 4, 5, 6, 7 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 32, 33, 34, 33, 32 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 35, 36, 37, 36, 35 }
};

static const uint8 CHEWY_MI_PHASEN_ANZ[] = { 8, 8, 8, 8, 5, 8, 8, 5 };

static const uint8 CHEWY_RO_PHASEN[8][8] = {
	{  0, 1, 2, 3, 4, 5, 6, 7 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 24, 25, 26, 27, 28, 29, 30, 31 },
	{ 32, 33, 34, 35, 36, 35, 34, 33 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 37, 38, 39, 40, 41, 40, 39, 38 }
};

static const uint8 CHEWY_RO_PHASEN_ANZ[] = { 8, 8, 8, 8, 8, 8, 8, 8 };

static const uint8 CHEWY_JM_PHASEN[8][8] = {
	{  0, 1, 2, 3, 4, 5, 6, 7 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 16, 17, 18, 19, 20, 21, 22, 23 },
	{ 24, 25, 26, 27, 27, 26, 25, 24 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{  8, 9, 10, 11, 12, 13, 14, 15 },
	{ 28, 29, 30, 31, 31, 30, 29, 28 }
};


void JungleRoom::topEntry() {
	_G(cur_hide_flag) = 0;
	hide_cur();
	set_person_pos(236, 110, P_CHEWY, P_RIGHT);
	set_person_pos(263, 85, P_NICHELLE, P_RIGHT);
	set_person_pos(285, 78, P_HOWARD, P_RIGHT);
	go_auto_xy(266, 113, P_HOWARD, ANI_WAIT);
	show_cur();
}

void JungleRoom::leftEntry() {
	set_person_pos(31, 118, P_CHEWY, P_RIGHT);
	set_person_pos(71, 104, P_NICHELLE, P_RIGHT);
	set_person_pos(6, 111, P_HOWARD, P_RIGHT);
}

void JungleRoom::rightEntry() {
	set_person_pos(587, 114, P_CHEWY, P_LEFT);
	set_person_pos(613, 103, P_NICHELLE, P_LEFT);
	set_person_pos(561, 112, P_HOWARD, P_LEFT);
}

void JungleRoom::setup_func() {
	calc_person_look();

	const int posX = spieler_vector[P_CHEWY].Xypos[0];

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

	go_auto_xy(howDestX, 111, P_HOWARD, ANI_GO);
	go_auto_xy(nicDestX, 110, P_NICHELLE, ANI_GO);
}


Room::Room() {
	_roomTimer.TimerAnz = 0;
	_roomTimer.TimerStart = 0;
	init_ablage();
	for (int16 i = 0; i < MAX_ROOM_HANDLE; i++)
		_roomHandle[i] = 0;
}
Room::~Room() {
	for (int16 i = 0; i < MAX_ROOM_HANDLE; i++)
		if (_roomHandle[i])
			chewy_fclose(_roomHandle[i]);
	free_ablage();
}

Stream *Room::open_handle(const char *fname1, const char *fmode, int16 mode) {
	Stream *stream = chewy_fopen(fname1, fmode);
	if (stream) {
		close_handle(mode);
		_roomHandle[mode] = stream;
	} else {
		error("open_handle error");
	}

	return _roomHandle[mode];
}

void Room::close_handle(int16 mode) {
	if (_roomHandle[mode])
		chewy_fclose(_roomHandle[mode]);
}

void Room::load_room(RaumBlk *Rb, int16 room_nr, Spieler *player) {
	_G(modul) = 0;
	_G(fcode) = 0;

	clear_prog_ani();
	det->load_rdi(Rb->DetFile, room_nr);

	if (player->SoundSwitch == false)
		det->disable_room_sound();
	if (!_G(modul)) {
		room_detail_info *Rdi_ = det->get_room_detail_info();
		_roomInfo = &Rdi_->Ri;
		if (_roomInfo->TafLoad != 255) {
			char tmp_str[MAXPATH];
			strcpy(tmp_str, Rb->RoomDir);
			strcat(tmp_str, _roomInfo->TafName);
			det->load_rdi_taf(tmp_str, _roomInfo->TafLoad);
			Rb->Fti = det->get_taf_info();
			Rb->DetImage = Rb->Fti->image;
			Rb->DetKorrekt = Rb->Fti->korrektur;
		}
		if (!_G(modul)) {
			obj->calc_all_static_detail();
			load_tgp(_roomInfo->BildNr, Rb, EPISODE1_TGP, GED_LOAD, "back/episode1.tgp");
			set_pal(_ablagePal[Rb->AkAblage], Rb->LowPalMem);
			calc_invent(Rb, player);

			if (!_G(modul)) {
				del_timer_old_room();
				add_timer_new_room();
			}

			if (!_G(modul)) {
				if (Rb->AtsLoad)
					atds->load_atds(_roomInfo->RoomNr, ATS_DATEI);
			}

			if (!_G(modul)) {
				if (Rb->AadLoad)
					atds->load_atds(_roomInfo->RoomNr, AAD_DATEI);
			}
		}
	}
}

void Room::set_timer_start(int16 timer_start) {
	_roomTimer.TimerStart = timer_start;
}

void Room::add_timer_new_room() {
	_roomTimer.TimerAnz = 0;

	for (int i = 0; i < MAXDETAILS && _roomTimer.TimerAnz < MAX_ROOM_TIMER; i++) {
		ani_detail_info *adi = det->get_ani_detail(i);
		if (adi->timer_start != 0) {
			set_timer(i, adi->timer_start);
		} else if (adi->start_flag || adi->repeat) {
			det->start_detail(i, 0, ANI_VOR);
		}
	}
}

void Room::del_timer_old_room() {
	for (int i = 0; i < _roomTimer.TimerAnz; i++) {
		uhr->set_status(_roomTimer.TimerNr[i], TIMER_STOP);

	}

	_roomTimer.TimerAnz = 0;
}

int16 Room::set_timer(int16 ani_nr, int16 timer_end) {

	int16 timer_nr_ = _roomTimer.TimerStart + _roomTimer.TimerAnz;
	int16 ret = uhr->set_new_timer(timer_nr_, timer_end, SEC_MODE);
	if (ret != -1) {
		_roomTimer.ObjNr[_roomTimer.TimerAnz] = ani_nr;
		_roomTimer.TimerNr[_roomTimer.TimerAnz] = timer_nr_;
		++_roomTimer.TimerAnz;
	}

	return timer_nr_;
}

void Room::set_timer_status(int16 ani_nr, int16 status) {
	for (int i = 0; i < _roomTimer.TimerAnz; i++) {
		if (_roomTimer.ObjNr[i] == ani_nr) {
			uhr->set_status(_roomTimer.TimerNr[i], status);
		}
	}
}

void Room::set_zoom(int16 zoom) {
	_roomInfo->ZoomFak = (uint8)zoom;
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

void Room::calc_invent(RaumBlk *Rb, Spieler *player) {
	byte *tmp_inv_spr[MAX_MOV_OBJ];

	if (!_G(modul)) {
		obj->sort();
		memcpy(tmp_inv_spr, Rb->InvSprAdr, MAX_MOV_OBJ * sizeof(char *));
		memset(Rb->InvSprAdr, 0, MAX_MOV_OBJ * sizeof(char *));

		SpriteResource *spriteRes = new SpriteResource(Rb->InvFile);

		for (int16 i = 1; i < obj->mov_obj_room[0] + 1; i++) {
			if (!tmp_inv_spr[obj->mov_obj_room[i]]) {
				spriteRes->getSpriteData(obj->mov_obj_room[i], &Rb->InvSprAdr[obj->mov_obj_room[i]], true);
			} else {
				Rb->InvSprAdr[obj->mov_obj_room[i]] = tmp_inv_spr[obj->mov_obj_room[i]];
				tmp_inv_spr[obj->mov_obj_room[i]] = nullptr;
			}
		}

		for (int16 i = 1; i < obj->spieler_invnr[0] + 1; i++) {
			if (!tmp_inv_spr[obj->spieler_invnr[i]]) {
				spriteRes->getSpriteData(obj->spieler_invnr[i], &Rb->InvSprAdr[obj->spieler_invnr[i]], true);
			} else {
				Rb->InvSprAdr[obj->spieler_invnr[i]] = tmp_inv_spr[obj->spieler_invnr[i]];
				tmp_inv_spr[obj->spieler_invnr[i]] = 0;
			}
		}

		for (int16 i = 0; i < MAX_MOV_OBJ; i++) {
			if (tmp_inv_spr[i] != 0)
				free(tmp_inv_spr[i]);
		}

		if (player->AkInvent != -1) {
			if (Rb->InvSprAdr[player->AkInvent] == nullptr) {
				spriteRes->getSpriteData(player->AkInvent, &Rb->InvSprAdr[player->AkInvent], true);
			}
		}

		delete spriteRes;
	}
}

int16 Room::load_tgp(int16 nr, RaumBlk *Rb, int16 tgp_idx, int16 mode, const char *fileName) {
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

		if (mode == GED_LOAD) {
			Common::SeekableReadStream *gstream = dynamic_cast<Common::SeekableReadStream *>(
				_roomHandle[R_GEPDATEI]);
			ged->load_ged_pool(gstream, &_gedInfo[Rb->AkAblage],
						        nr, _gedMem[Rb->AkAblage]);
			_gedXAnz[Rb->AkAblage] = img->width / _gedInfo[Rb->AkAblage].X;
			_gedYAnz[Rb->AkAblage] = img->height / _gedInfo[Rb->AkAblage].Y;
		}
	}

	delete img;
	delete res;

	return true;
}

void Room::init_ablage() {
	_lastAblageSave = 0;
	_ablage[0] = (byte *)MALLOC(MAX_ABLAGE * (ABLAGE_BLOCK_SIZE + 4l));
	_ablagePal[0] = (byte *)MALLOC(MAX_ABLAGE * 768l);
	_gedMem[0] = (byte *)MALLOC(MAX_ABLAGE * GED_BLOCK_SIZE);
	if (!_G(modul)) {
		_akAblage = 0;
		for (int16 i = 0; i < MAX_ABLAGE; i++) {
			_ablage[i] = _ablage[0] + (ABLAGE_BLOCK_SIZE + 4l) * i;
			_ablageInfo[i][0] = -1;
			_ablageInfo[i][1] = -1;
			_ablagePal[i] = _ablagePal[0] + 768l * i;
			_gedMem[i] = _gedMem[0] + (GED_BLOCK_SIZE * i);
		}
	} else {
		_akAblage = -1;
		_ablage[0] = 0;
	}
}

void Room::free_ablage() {
	free(_gedMem[0]);
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

byte **Room::get_ged_mem() {
	byte **ret = nullptr;

	if (_akAblage != -1) {
		ret = &_gedMem[0];
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
	int16 ende = 0;
	int16 count = 0;
	_lastAblageSave = ak_pos;
	while (!ende) {
		if (_lastAblageSave >= MAX_ABLAGE)
			_lastAblageSave = 0;
		if (ablage_bedarf == 1) {
			ende = 1;
			ret = _lastAblageSave;
		} else if (ablage_bedarf <= MAX_ABLAGE - _lastAblageSave) {
			ende = 1;
			ret = _lastAblageSave;
		} else
			++_lastAblageSave;
		++count;
		if (count > MAX_ABLAGE) {
			ret = -1;
			ende = 1;
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
	if (AkChewyTaf != taf_nr) {
		if (chewy) {
			free((char *)chewy);
			chewy = nullptr;
		}
		spieler_mi[P_CHEWY].HotY = CH_HOT_Y;

		const char *fname_;
		switch (taf_nr) {
		case CHEWY_NORMAL:
			fname_ = CHEWY_TAF;
			_G(chewy_ph_anz) = CHEWY_PHASEN_ANZ;
			_G(chewy_ph) = (const uint8 *)CHEWY_PHASEN;
			break;

		case CHEWY_BORK:
			fname_ = CHEWY_BO_TAF;
			_G(chewy_ph_anz) = CHEWY_BO_PHASEN_ANZ;
			_G(chewy_ph) = (const uint8 *)CHEWY_BO_PHASEN;
			break;

		case CHEWY_MINI:
			fname_ = CHEWY_MI_TAF;
			_G(chewy_ph_anz) = CHEWY_MI_PHASEN_ANZ;
			_G(chewy_ph) = (const uint8 *)CHEWY_MI_PHASEN;
			break;

		case CHEWY_PUMPKIN:
			fname_ = CHEWY_PUMP_TAF;
			_G(chewy_ph_anz) = CHEWY_MI_PHASEN_ANZ;
			_G(chewy_ph) = (const uint8 *)CHEWY_MI_PHASEN;
			break;

		case CHEWY_ROCKER:
			fname_ = CHEWY_ROCK_TAF;
			_G(chewy_ph_anz) = CHEWY_RO_PHASEN_ANZ;
			_G(chewy_ph) = (const uint8 *)CHEWY_RO_PHASEN;
			break;

		case CHEWY_JMANS:
			fname_ = CHEWY_JMAN_TAF;
			_G(chewy_ph_anz) = CHEWY_RO_PHASEN_ANZ;
			_G(chewy_ph) = (const uint8 *)CHEWY_JM_PHASEN;
			spieler_mi[P_CHEWY].HotY = 68;
			break;

		case CHEWY_ANI7:
			fname_ = CHEWY_HELM_TAF;
			_G(chewy_ph_anz) = CHEWY_MI_PHASEN_ANZ;
			_G(chewy_ph) = (const uint8 *)CHEWY_MI_PHASEN;
			break;

		default:
			fname_ = NULL;
			break;

		}
		if (fname_ != NULL) {
			_G(spieler).ChewyAni = taf_nr;
			AkChewyTaf = taf_nr;
			chewy = _G(mem)->taf_adr(fname_);
			chewy_kor = chewy->korrektur;
		}
	}
}

void switch_room(int16 nr) {
	fx_blend = BLEND1;
	exit_room(-1);
	_G(spieler).PersonRoomNr[P_CHEWY] = nr;
	room->load_room(&room_blk, _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
	enter_room(-1);
	set_up_screen(DO_SETUP);
}

void calc_person_look() {
	for (int16 i = 1; i < MAX_PERSON; i++) {
		if (spieler_mi[i].Id != NO_MOV_OBJ) {

			if (spieler_vector[i].Xypos[0] > spieler_vector[P_CHEWY].Xypos[0])
				person_end_phase[i] = P_LEFT;
			else
				person_end_phase[i] = P_RIGHT;
		}
	}
}

} // namespace Chewy
