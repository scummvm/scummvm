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
#include "chewy/room.h"
#include "chewy/ngshext.h"

namespace Chewy {

const int16 SURIMY_TAF19_PHASEN[4][2] = {
	{ 0, 0 },
	{ 39, 46 },
	{ 0, 0 },
	{ 0, 0 }
};

Room::Room() {
	room_timer.TimerAnz = 0;
	room_timer.TimerStart = 0;
	init_ablage();
	for (int16 i = 0; i < MAX_ROOM_HANDLE; i++)
		roomhandle[i] = 0;
}
Room::~Room() {
	for (int16 i = 0; i < MAX_ROOM_HANDLE; i++)
		if (roomhandle[i])
			chewy_fclose(roomhandle[i]);
	free_ablage();
}

Stream *Room::open_handle(const char *fname1, const char *fmode, int16 mode) {
	Stream *stream = chewy_fopen(fname1, fmode);
	if (stream) {
		close_handle(mode);
		roomhandle[mode] = stream;
	} else {
		modul = DATEI;
		fcode = OPENFEHLER;
		err->set_user_msg(fname1);
	}

	return roomhandle[mode];
}

void Room::close_handle(int16 mode) {
	if (roomhandle[mode])
		chewy_fclose(roomhandle[mode]);
}

void Room::load_room(RaumBlk *Rb, int16 room_nr, Spieler *player) {
	modul = 0;
	fcode = 0;

	clear_prog_ani();
	det->load_rdi(Rb->DetFile, room_nr);
	ERROR
	load_sound();

	if (player->SoundSwitch == false)
		det->disable_room_sound();
	if (!modul) {
		room_detail_info *Rdi_ = det->get_room_detail_info();
		room_info = &Rdi_->Ri;
		if (room_info->TafLoad != 255) {
			char tmp_str[MAXPATH];
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
			load_tgp(room_info->BildNr, Rb, EPISODE1_TGP, GED_LOAD);
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
	if (palette != nullptr)
		mem->file->load_palette(filename, palette, TAFDATEI);
	if (!modul) {
		imagesize[sprnr] = 0l;
		mem->file->imsize(filename, imagesize);
		if (!modul) {
			*speicher = (byte *)MALLOC((imagesize[sprnr]) + 4l);
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
	room_timer.TimerAnz = 0;

	for (int i = 0; i < MAXDETAILS && room_timer.TimerAnz < MAX_ROOM_TIMER; i++) {
		ani_detail_info *adi = det->get_ani_detail(i);
		if (adi->timer_start != 0) {
			set_timer(i, adi->timer_start);
		} else if (adi->start_flag || adi->repeat) {
			det->start_detail(i, 0, ANI_VOR);
		}
	}
}

void Room::del_timer_old_room() {
	for (int i = 0; i < room_timer.TimerAnz; i++) {
		uhr->set_status(room_timer.TimerNr[i], TIMER_STOP);

	}

	room_timer.TimerAnz = 0;
}

int16 Room::set_timer(int16 ani_nr, int16 timer_end) {

	int16 timer_nr_ = room_timer.TimerStart + room_timer.TimerAnz;
	int16 ret = uhr->set_new_timer(timer_nr_, timer_end, SEC_MODE);
	if (ret != -1) {
		room_timer.ObjNr[room_timer.TimerAnz] = ani_nr;
		room_timer.TimerNr[room_timer.TimerAnz] = timer_nr_;
		++room_timer.TimerAnz;
	}

	return timer_nr_;
}

void Room::set_timer_status(int16 ani_nr, int16 status) {
	for (int i = 0; i < room_timer.TimerAnz; i++) {
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
	dest_pal[767] = 63;
	dest_pal[766] = 63;
	dest_pal[765] = 63;
	dest_pal[0] = 0;
	dest_pal[1] = 0;
	dest_pal[2] = 0;
}

void Room::set_ak_pal(RaumBlk *Rb) {
	set_pal(AblagePal[Rb->AkAblage], Rb->LowPalMem);
}

void Room::calc_invent(RaumBlk *Rb, Spieler *player) {
	byte *tmp_inv_spr[MAX_MOV_OBJ];

	if (!modul) {
		obj->sort();
		memcpy(tmp_inv_spr, Rb->InvSprAdr, MAX_MOV_OBJ * sizeof(char *));
		memset(Rb->InvSprAdr, 0, MAX_MOV_OBJ * sizeof(char *));

		for (int16 i = 1; i < obj->mov_obj_room[0] + 1; i++) {
			if (!tmp_inv_spr[obj->mov_obj_room[i]]) {

				load_taf(Rb->InvFile, &Rb->InvSprAdr[obj->mov_obj_room[i]], nullptr, (int16)obj->mov_obj_room[i]);
			} else {
				Rb->InvSprAdr[obj->mov_obj_room[i]] = tmp_inv_spr[obj->mov_obj_room[i]];
				tmp_inv_spr[obj->mov_obj_room[i]] = nullptr;
			}
		}

		for (int16 i = 1; i < obj->spieler_invnr[0] + 1; i++) {
			if (!tmp_inv_spr[obj->spieler_invnr[i]]) {

				load_taf(Rb->InvFile, &Rb->InvSprAdr[obj->spieler_invnr[i]], nullptr, (int16)obj->spieler_invnr[i]);
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
			if (Rb->InvSprAdr[player->AkInvent] == nullptr)
				load_taf(Rb->InvFile, &Rb->InvSprAdr[player->AkInvent], nullptr, (int16)player->AkInvent);
		}
	}
}

int16 Room::load_tgp(int16 nr, RaumBlk *Rb, int16 tgp_idx, int16 mode) {
	Common::SeekableReadStream *rs = dynamic_cast<Common::SeekableReadStream *>(roomhandle[R_TGPDATEI]);
	tbf_dateiheader tb;
	bool ret = false;

	if (rs) {
		mem->file->select_pool_item(rs, nr);

		if (!tb.load(rs)) {
			modul = DATEI;
			fcode = READFEHLER;
		}

		if (!modul) {
			rs->seek(-tbf_dateiheader::SIZE(), SEEK_CUR);
			Rb->AkAblage = get_ablage(nr + (1000 * tgp_idx), tb.entpsize + 4);

			if (Rb->AkAblage == -1) {
			} else if (Rb->AkAblage >= 1000) {
				Rb->AkAblage -= 1000;

			} else {
				mem->file->load_image(rs, Ablage[Rb->AkAblage], AblagePal[Rb->AkAblage]);
				if (!modul) {
					set_ablage_info(Rb->AkAblage, nr + (1000 * tgp_idx), tb.entpsize);
					ret = true;

					if (mode == GED_LOAD) {
						Common::SeekableReadStream *gstream = dynamic_cast<Common::SeekableReadStream *>(
							roomhandle[R_GEPDATEI]);
						ged->load_ged_pool(gstream, &GedInfo[Rb->AkAblage],
						                   nr, GedMem[Rb->AkAblage]);
						if (!modul) {
							int16 *tmp = (int16 *)Ablage[Rb->AkAblage];
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

	return ret;
}

void Room::init_ablage() {
	LastAblageSave = 0;
	Ablage[0] = (byte *)MALLOC(MAX_ABLAGE * (ABLAGE_BLOCK_SIZE + 4l));
	AblagePal[0] = (byte *)MALLOC(MAX_ABLAGE * 768l);
	GedMem[0] = (byte *)MALLOC(MAX_ABLAGE * GED_BLOCK_SIZE);
	if (!modul) {
		AkAblage = 0;
		for (int16 i = 0; i < MAX_ABLAGE; i++) {
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
	byte *ret = nullptr;

	if (nr < MAX_ABLAGE && AkAblage != -1) {
		ret = Ablage[nr];
	}
	return ret;
}

byte **Room::get_ablage() {
	byte **ret = nullptr;

	if (AkAblage != -1) {
		ret = &Ablage[0];
	}
	return ret;
}

byte **Room::get_ged_mem() {
	byte **ret = nullptr;

	if (AkAblage != -1) {
		ret = &GedMem[0];
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
		if (AblageInfo[i][0] == pic_nr &&
		        AblageInfo[i][1] != 255) {
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
				if (AblageInfo[LastAblageSave][1] == 255) {
					--LastAblageSave;
					if (LastAblageSave < 0) {
						LastAblageSave = 0;
						AblageInfo[0][1] = MAX_ABLAGE;
					}
				} else
					ende = 1;
			}
			int16 i = AblageInfo[LastAblageSave][1];
			for (int16 j = LastAblageSave; j < LastAblageSave + i; j++) {
				AblageInfo[j][0] = -1;
				AblageInfo[j][1] = -1;
			}
		}
	}
	return ret;
}

int16 Room::get_ablage_g1(int16 ablage_bedarf, int16 ak_pos) {
	short ret = 0;
	int16 ende = 0;
	int16 count = 0;
	LastAblageSave = ak_pos;
	while (!ende) {
		if (LastAblageSave >= MAX_ABLAGE)
			LastAblageSave = 0;
		if (ablage_bedarf == 1) {
			ende = 1;
			ret = LastAblageSave;
		} else if (ablage_bedarf <= MAX_ABLAGE - LastAblageSave) {
			ende = 1;
			ret = LastAblageSave;
		} else
			++LastAblageSave;
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
		AblageInfo[i][0] = bildnr;
		AblageInfo[i][1] = (int16)ablage_bedarf;
		ablage_bedarf = 255;
		++LastAblageSave;
	}
}

void Room::load_sound() {
	if (!modul && flags.InitSound) {
		det->load_room_sounds(roomhandle[R_VOCDATEI]);
	}
}

Stream *Room::get_sound_handle() {
	return (modul || !flags.InitSound) ? nullptr :
		roomhandle[R_VOCDATEI];
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
			chewy_ph_anz = chewy_phasen_anz;
			chewy_ph = (uint8 *)chewy_phasen;
			break;

		case CHEWY_BORK:
			fname_ = CHEWY_BO_TAF;
			chewy_ph_anz = chewy_bo_phasen_anz;
			chewy_ph = (uint8 *)chewy_bo_phasen;
			break;

		case CHEWY_MINI:
			fname_ = CHEWY_MI_TAF;
			chewy_ph_anz = chewy_mi_phasen_anz;
			chewy_ph = (uint8 *)chewy_mi_phasen;
			break;

		case CHEWY_PUMPKIN:
			fname_ = CHEWY_PUMP_TAF;
			chewy_ph_anz = chewy_mi_phasen_anz;
			chewy_ph = (uint8 *)chewy_mi_phasen;
			break;

		case CHEWY_ROCKER:
			fname_ = CHEWY_ROCK_TAF;
			chewy_ph_anz = chewy_ro_phasen_anz;
			chewy_ph = (uint8 *)chewy_ro_phasen;
			break;

		case CHEWY_JMANS:
			fname_ = CHEWY_JMAN_TAF;
			chewy_ph_anz = chewy_ro_phasen_anz;
			chewy_ph = (uint8 *)chewy_jm_phasen;
			spieler_mi[P_CHEWY].HotY = 68;
			break;

		default:
			fname_ = NULL;
			break;

		}
		if (fname_ != NULL) {
			_G(spieler).ChewyAni = taf_nr;
			AkChewyTaf = taf_nr;
			chewy = mem->taf_adr(fname_);
			ERROR

			taf_dateiheader *tafheader;
			mem->file->get_tafinfo(fname_, &tafheader);
			if (!modul) {
				chewy_kor = chewy->korrektur;
			} else {
				error();
			}
		}
	}
}

void switch_room(int16 nr) {
	fx_blend = BLEND1;
	exit_room(-1);
	_G(spieler).PersonRoomNr[P_CHEWY] = nr;
	room->load_room(&room_blk, _G(spieler).PersonRoomNr[P_CHEWY], &_G(spieler));
	ERROR

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
