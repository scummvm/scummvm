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

#define FORBIDDEN_SYMBOL_EXCEPTION_fopen
#define FORBIDDEN_SYMBOL_EXCEPTION_fclose
#define FORBIDDEN_SYMBOL_EXCEPTION_fread
#define FORBIDDEN_SYMBOL_EXCEPTION_fseek
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "chewy/defines.h"
#include "chewy/global.h"
#include "chewy/room.h"
#include "chewy/ngshext.h"

namespace Chewy {

Room::Room() {
	int16 i;
	room_timer.TimerAnz = 0;
	room_timer.TimerStart = 0;
	init_ablage();
	for (i = 0; i < MAX_ROOM_HANDLE; i++)
		roomhandle[i] = 0;
}
Room::~Room() {
	int16 i;
	for (i = 0; i < MAX_ROOM_HANDLE; i++)
		if (roomhandle[i])
			fclose((FILE *)roomhandle[i]);
	free_ablage();
}

void *Room::open_handle(const char *fname1, const char *fmode, int16 mode) {
	FILE *handle;
	handle = fopen(fname1, fmode);
	if (handle) {
		close_handle(mode);
		roomhandle[mode] = handle;
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
		err->set_user_msg(fname1);
	}
	return (roomhandle[mode]);
}

void Room::close_handle(int16 mode) {
	if (roomhandle[mode])
		fclose((FILE *)roomhandle[mode]);
}

void Room::load_room(RaumBlk *Rb, int16 room_nr, Spieler *player) {
	char tmp_str[MAXPATH];
	room_detail_info *Rdi_;
	int16 ret;
	modul = 0;
	fcode = 0;
	clear_prog_ani();
	det->load_rdi(Rb->DetFile, room_nr);
	ERROR
	load_sound();
	if (player->SoundSwitch == false)
		det->disable_room_sound();
	if (!modul) {
		Rdi_ = det->get_room_detail_info();
		room_info = &Rdi_->Ri;
		if (room_info->TafLoad != 255) {
			strcpy(tmp_str, Rb->RoomDir);
			strcat(tmp_str, room_info->TafName);
			det->load_rdi_taf(tmp_str, room_info->TafLoad);
			Rb->Fti = det->get_taf_info();
			Rb->DetImage = Rb->Fti->image;
			Rb->DetKorrekt = Rb->Fti->korrektur;
		}
		if (!modul) {
			obj->calc_all_static_detail();
			ERROR
			ret = load_tgp(room_info->BildNr, Rb, EPISODE1_TGP, GED_LOAD);
			ERROR

			set_pal(AblagePal[Rb->AkAblage], Rb->LowPalMem);
			calc_invent(Rb, player);

			if (!modul) {
				del_timer_old_room();
				add_timer_new_room();
			}

			if (!modul) {
				if (Rb->AtsLoad)
					atds->load_atds(room_info->RoomNr, ATS_DATEI);
			}

			if (!modul) {
				if (Rb->AadLoad)
					atds->load_atds(room_info->RoomNr, AAD_DATEI);
			}
		}
	}
}

void Room::load_taf(const char *filename, byte **speicher, byte *palette, int16 sprnr) {
	uint32 imagesize[MAXSPRITE] = {0};
	if (palette != 0)
		mem->file->load_palette(filename, palette, TAFDATEI);
	if (!modul) {
		imagesize[sprnr] = 0l;
		mem->file->imsize(filename, imagesize);
		if (!modul) {
			*speicher = (byte *)calloc((imagesize[sprnr]) + 4l, 1);
			if (!modul) {
				mem->file->load_tafmcga(filename, *speicher, sprnr);
			}
		}
	}
}

void Room::set_timer_start(int16 timer_start) {
	room_timer.TimerStart = timer_start;
}

void Room::add_timer_new_room() {
	ani_detail_info *adi;
	int16 i;
	room_timer.TimerAnz = 0;

	for (i = 0; i < MAXDETAILS && room_timer.TimerAnz < MAX_ROOM_TIMER; i++) {
		adi = det->get_ani_detail(i);
		if (adi->timer_start != 0) {
			set_timer(i, adi->timer_start);
		}
		else if (adi->start_flag) {
			det->start_detail(i, 0, ANI_VOR);
		}
	}
}

void Room::del_timer_old_room() {
	int16 i;
	for (i = 0; i < room_timer.TimerAnz; i++) {
		uhr->set_status(room_timer.TimerNr[i], TIMER_STOP);

	}
	room_timer.TimerAnz = 0;

}

int16 Room::set_timer(int16 ani_nr, int16 timer_end) {
	int16 timer_nr_;
	int16 ret;

	timer_nr_ = room_timer.TimerStart + room_timer.TimerAnz;
	ret = uhr->set_new_timer(timer_nr_, timer_end, SEC_MODE);
	if (ret != -1) {
		room_timer.ObjNr[room_timer.TimerAnz] = ani_nr;
		room_timer.TimerNr[room_timer.TimerAnz] = timer_nr_;
		++room_timer.TimerAnz;
	}
	return (timer_nr_);
}

void Room::set_timer_status(int16 ani_nr, int16 status) {
	int16 i;
	for (i = 0; i < room_timer.TimerAnz; i++) {
		if (room_timer.ObjNr[i] == ani_nr) {
			uhr->set_status(room_timer.TimerNr[i], status);
		}
	}
}

void Room::set_zoom(int16 zoom) {
	room_info->ZoomFak = (uint8)zoom;
}

void Room::set_pal(const byte *src_pal, byte *dest_pal) {
	memcpy(dest_pal, src_pal, 768l);
	dest_pal [767] = 63;
	dest_pal [766] = 63;
	dest_pal [765] = 63;
	dest_pal [0] = 0;
	dest_pal [1] = 0;
	dest_pal [2] = 0;
}

void Room::set_ak_pal(RaumBlk *Rb) {
	set_pal(AblagePal[Rb->AkAblage], Rb->LowPalMem);
}

void Room::calc_invent(RaumBlk *Rb, Spieler *player) {
	byte *tmp_inv_spr[MAX_MOV_OBJ];
	int16 i;

	if (!modul) {
		obj->sort();
		memcpy(tmp_inv_spr, Rb->InvSprAdr, MAX_MOV_OBJ * sizeof(char *));
		memset(Rb->InvSprAdr, 0, MAX_MOV_OBJ * sizeof(char *));

		for (i = 1; i < obj->mov_obj_room[0] + 1; i++) {
			if (!tmp_inv_spr[obj->mov_obj_room[i]]) {

				load_taf(Rb->InvFile, &Rb->InvSprAdr[obj->mov_obj_room[i]],
				         0, (int16)obj->mov_obj_room[i]);
			} else {
				Rb->InvSprAdr[obj->mov_obj_room[i]] = tmp_inv_spr[obj->mov_obj_room[i]];
				tmp_inv_spr[obj->mov_obj_room[i]] = 0;
			}
		}

		for (i = 1; i < obj->spieler_invnr[0] + 1; i++) {
			if (!tmp_inv_spr[obj->spieler_invnr[i]]) {

				load_taf(Rb->InvFile, &Rb->InvSprAdr[obj->spieler_invnr[i]],
				         0, (int16)obj->spieler_invnr[i]);
			} else {
				Rb->InvSprAdr[obj->spieler_invnr[i]] = tmp_inv_spr[obj->spieler_invnr[i]];
				tmp_inv_spr[obj->spieler_invnr[i]] = 0;
			}
		}

		for (i = 0; i < MAX_MOV_OBJ; i++)
			if (tmp_inv_spr[i] != 0)
				free(tmp_inv_spr[i]);
		if (player->AkInvent != -1) {
			if (Rb->InvSprAdr[player->AkInvent] == NULL)
				load_taf(Rb->InvFile, &Rb->InvSprAdr[player->AkInvent],
				         0, (int16)player->AkInvent);
		}
	}
}

int16 Room::load_tgp(int16 nr, RaumBlk *Rb, int16 tgp_idx, int16 mode) {
	bool ret;
	int16 *tmp;

	tbf_dateiheader tb;
	ret = false;

	if (roomhandle[R_TGPDATEI]) {
		mem->file->select_pool_item(roomhandle[R_TGPDATEI], nr);

		if (!fread(&tb, sizeof(tbf_dateiheader), 1, (FILE *)roomhandle[R_TGPDATEI])) {
			modul = DATEI;
			fcode = READFEHLER;
		}
		if (!modul) {
			fseek((FILE *)roomhandle[R_TGPDATEI], -(int)sizeof(tbf_dateiheader), SEEK_CUR);
			Rb->AkAblage = get_ablage(nr + (1000 * tgp_idx), tb.entpsize + 4);
			if (Rb->AkAblage == -1) {
			} else if (Rb->AkAblage >= 1000) {
				Rb->AkAblage -= 1000;
			} else {
				mem->file->
				load_image(roomhandle[R_TGPDATEI], Ablage[Rb->AkAblage], AblagePal[Rb->AkAblage]);
				if (!modul) {
					set_ablage_info(Rb->AkAblage, nr + (1000 * tgp_idx), tb.entpsize);
					ret = true;
					if (mode == GED_LOAD) {

						ged->load_ged_pool(roomhandle[R_GEPDATEI], &GedInfo[Rb->AkAblage],
						                    nr, GedMem[Rb->AkAblage]);
						if (!modul) {
							tmp = (int16 *)Ablage[Rb->AkAblage];
							GedXAnz[Rb->AkAblage] = tmp[0] / GedInfo[Rb->AkAblage].X;
							GedYAnz[Rb->AkAblage] = tmp[1] / GedInfo[Rb->AkAblage].Y;
						} else {
							modul = DATEI;
							fcode = READFEHLER;
						}
					}
				} else {
					modul = DATEI;
					fcode = READFEHLER;
				}
			}
		}

	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
	}
	return (ret);
}

void Room::init_ablage() {
	int16 i;
	LastAblageSave = 0;
	Ablage[0] = (byte *)calloc(MAX_ABLAGE * (ABLAGE_BLOCK_SIZE + 4l), 1);
	AblagePal[0] = (byte *)calloc(MAX_ABLAGE * 768l, 1);
	GedMem[0] = (byte *)calloc(MAX_ABLAGE * GED_BLOCK_SIZE, 1);
	if (!modul) {
		AkAblage = 0;
		for (i = 0; i < MAX_ABLAGE; i++) {
			Ablage[i] = Ablage[0] + (ABLAGE_BLOCK_SIZE + 4l) * i;
			AblageInfo[i][0] = -1;
			AblageInfo[i][1] = -1;
			AblagePal[i] = AblagePal[0] + 768l * i;
			GedMem[i] = GedMem[0] + (GED_BLOCK_SIZE * i);
		}
	} else {
		AkAblage = -1;
		Ablage[0] = 0;
		ERROR
	}
}

void Room::free_ablage() {
	free(GedMem[0]);
	free(AblagePal[0]);
	free(Ablage[0]);
	AkAblage = -1;
}

byte *Room::get_ablage(int16 nr) {
	byte *ret;
	ret = false;
	if (nr < MAX_ABLAGE && AkAblage != -1) {
		ret = Ablage[nr];
	}
	return (ret);
}

byte **Room::get_ablage() {
	byte **ret;
	ret = false;
	if (AkAblage != -1) {
		ret = &Ablage[0];
	}
	return (ret);
}

byte **Room::get_ged_mem() {
	byte **ret;
	ret = false;
	if (AkAblage != -1) {
		ret = &GedMem[0];
	}
	return (ret);
}

int16 Room::get_ablage(int16 pic_nr, uint32 pic_size) {
	int16 ende;
	uint32 ablage_bedarf;
	int16 ret;
	int16 i, j;
	ret = -1;
	ablage_bedarf = pic_size / ABLAGE_BLOCK_SIZE;
	if (pic_size % ABLAGE_BLOCK_SIZE > 4)
		++ablage_bedarf;
	ende = 0;

	for (i = 0; i < MAX_ABLAGE && !ende; i++) {
		if (AblageInfo[i][0] == pic_nr &&
		        AblageInfo[i][1] != 255) {
			ende = 1;
			ret = 1000 + i;
		}
	}

	if (!ende) {
		for (i = 0; i < MAX_ABLAGE && !ende; i++) {
			ret = get_ablage_g1((int16)ablage_bedarf, i);
			ende = 1;
		}
	}
	if (ret != -1) {
		if (ret < 1000) {

			ende = 0;
			while (!ende) {
				if (AblageInfo[LastAblageSave][1] == 255) {
					--LastAblageSave;
					if (LastAblageSave < 0) {
						LastAblageSave = 0;
						AblageInfo[0][1] = MAX_ABLAGE;
					}
				} else
					ende = 1;
			}
			i = AblageInfo[LastAblageSave][1];
			for (j = LastAblageSave; j < LastAblageSave + i; j++) {
				AblageInfo[j][0] = -1;
				AblageInfo[j][1] = -1;
			}
		}
	}
	return (ret);
}

int16 Room::get_ablage_g1(int16 ablage_bedarf, int16 ak_pos) {
	int16 ende;
	short ret = 0;
	int16 count;
	ende = 0;
	count = 0;
	LastAblageSave = ak_pos;
	while (!ende) {
		if (LastAblageSave >= MAX_ABLAGE)
			LastAblageSave = 0;
		if (ablage_bedarf == 1) {
			ende = 1;
			ret = LastAblageSave;
		}
		else if (ablage_bedarf <= MAX_ABLAGE - LastAblageSave) {
			ende = 1;
			ret = LastAblageSave;
		}
		else
			++LastAblageSave;
		++count;
		if (count > MAX_ABLAGE) {
			ret = -1;
			ende = 1;
		}
	}
	return (ret);
}

void Room::set_ablage_info(int16 ablagenr, int16 bildnr, uint32 pic_size) {
	uint32 ablage_bedarf;
	int16 i;
	int16 j;
	ablage_bedarf = (pic_size / ABLAGE_BLOCK_SIZE);
	if (pic_size % ABLAGE_BLOCK_SIZE > 4)
		++ablage_bedarf;
	j = (int16)ablage_bedarf;
	for (i = ablagenr; i < j + ablagenr; i++) {
		AblageInfo[i][0] = bildnr;
		AblageInfo[i][1] = (int16)ablage_bedarf;
		ablage_bedarf = 255;
		++LastAblageSave;
	}
}

void Room::load_sound() {
	if ((!modul) && (flags.InitSound)) {
		det->load_room_sounds(roomhandle[R_VOCDATEI]);
	}
}

void *Room::get_sound_handle() {
	void *ret;
	if ((!modul) && (flags.InitSound))
		ret = roomhandle[R_VOCDATEI];
	else
		ret = 0;
	return (ret);
}

} // namespace Chewy
