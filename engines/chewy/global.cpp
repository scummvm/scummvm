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

#include "chewy/global.h"
#include "chewy/chewy.h"
#include "chewy/types.h"
#include "chewy/defines.h"
#include "chewy/file.h"
#include "chewy/ngshext.h"
#include "chewy/text.h"
#include "chewy/room.h"
#include "chewy/object.h"
#include "chewy/timer.h"
#include "chewy/detail.h"
#include "chewy/effect.h"
#include "chewy/atds.h"
#include "chewy/movclass.h"
#include "chewy/gedclass.h"
#include "chewy/bitclass.h"

namespace Chewy {

#define MAX_RAND_NO_USE 6

const uint8 RAND_NO_USE[MAX_RAND_NO_USE] = {0, 1, 3, 4, 5, 6};


Globals *g_globals = nullptr;

Globals::Globals() {
	g_globals = this;

	static const int16 R45_PINFO[R45_MAX_PERSON][4] = {
		{ 0, 4, 1490, 1500 },
		{ 0, 1, 180, 190 },
		{ 0, 1, 40, 60 },
		{ 0, 1, 40, 150 },
		{ 0, 1, 40, 230 },
		{ 0, 1, 40, 340 },
		{ 0, 1, 49, 50 },
		{ 0, 1, 5, 90 },
		{ 0, 1, 7, 190 }
	};
	Common::copy(&R45_PINFO[0][0], &R45_PINFO[0][0] + R45_MAX_PERSON * 4, &_r45_pinfo[0][0]);
}

Globals::~Globals() {
	g_globals = nullptr;
}


SprInfo spr_info[MAX_PROG_ANI];
maus_info minfo;
kb_info kbinfo;
cur_blk curblk;
cur_ani curani;
iog_init ioptr;
RaumBlk room_blk;
ObjMov spieler_vector[MAX_PERSON];

MovInfo spieler_mi[MAX_PERSON];

ObjMov auto_mov_vector[MAX_OBJ_MOV];
MovInfo auto_mov_obj[MAX_OBJ_MOV];
MovPhasen mov_phasen[MAX_OBJ_MOV];
MovLine mov_line[MAX_OBJ_MOV][5];
TimerBlk ani_timer[MAX_TIMER_OBJ];
RoomDetailInfo *Rdi;
StaticDetailInfo *Sdi;
AniDetailInfo *Adi;
Flags flags = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0 };
CustomInfo Ci;
GotoPkt gpkt;

taf_info *curtaf;
taf_info *menutaf;
taf_seq_info *howard_taf;
taf_info *chewy;
int16 *chewy_kor;

int16 person_end_phase[MAX_PERSON];
int16 ani_stand_count[MAX_PERSON];

bool ani_stand_flag[MAX_PERSON];

BitClass *bit;
GedClass *ged;
Text *txt;
Room *room;
Object *obj;
Timer *uhr;
Detail *det;
Effect *fx;
Atdsys *atds;
Flic *flc;
MovClass *mov;

AutoMov auto_mov[MAX_AUTO_MOV];

int16 HowardMov;

taf_seq_info *spz_tinfo;
int16 SpzDelay;
int16 spz_spr_nr[MAX_SPZ_PHASEN];
int16 spz_start;
int16 spz_akt_id;

int16 spz_p_nr;
int16 spz_delay[MAX_PERSON];
int16 spz_count;
int16 spz_ani[MAX_PERSON];

int16 AkChewyTaf;

int16 PersonAni[MAX_PERSON];
taf_seq_info *PersonTaf[MAX_PERSON];
uint8 PersonSpr[MAX_PERSON][8];

} // namespace Chewy
