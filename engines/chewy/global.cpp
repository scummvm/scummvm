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

const int16 ANI_INVENT_END[3] = { 7, 16, 24 };

const int16 SPZ_ANI_PH[][2] = {
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

int16 person_tmp_hide[MAX_PERSON];
int16 person_tmp_room[MAX_PERSON];
uint32 ram_start;

int16 FrameSpeed;

int16 FrameSpeedTmp;

int16 frame_delay_count;
int16 show_frame;

byte **ablage;
byte *workpage;
byte *workptr;
byte *cur_back;
byte **ged_mem;
byte *pal;
byte *spblende;

byte *screen0;
int16 scr_width;
BlendMode fx_blend;

byte *font6x8;
byte *font8x8;
int16 fvorx6x8;
int16 fvorx8x8;
int16 fvory6x8;
short fvory8x8;

char *str;
char **ads_item_ptr;

int16 ads_dia_nr;
int16 ads_item_anz;
int16 ads_blk_nr;
int16 ads_push;

int16 ads_tmp_dsp;
int8 menu_display;
int16 menu_lauflicht;
int16 menu_item;
int16 menu_item_vorwahl;
int16 maus_menu_x;
int16 maus_old_x;
int16 maus_old_y;
int16 inventar_nr;
int16 invent_cur_mode;
byte *inv_spr[MAX_MOV_OBJ];

bool life_flag;
int16 life_x;
int16 life_y;
int16 life_anz;
char *life_str;
int16 life_handler;

void (*SetUpScreenFunc)();

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
room_detail_info *Rdi;
static_detail_info *Sdi;
ani_detail_info *Adi;
Flags flags = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
				0, 0, 0, 0, 0, 0, 0, 0, 0 };
CustomInfo Ci;
GotoPkt gpkt;

taf_info *curtaf;
taf_info *menutaf;
taf_seq_info *howard_taf;
taf_info *chewy;
uint8 *chewy_ph_anz;
uint8 *chewy_ph;
int16 *chewy_kor;

int16 person_end_phase[MAX_PERSON];
int16 ani_stand_count[MAX_PERSON];

bool ani_stand_flag[MAX_PERSON];

char io_pal1[7] = {11, 60, 14, 2, 1, 5, 4};

bitclass *bit;
gedclass *ged;
text *txt;
Room *room;
Object *obj;
timer *uhr;
detail *det;
effect *fx;
atdsys *atds;
flic *flc;
movclass *mov;

char backged[] = { EPISODE1_GEP };

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

uint8 chewy_phasen[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 8, 9, 10, 11, 12, 13, 14, 15},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{24, 25, 26, 27, 28, 29, 30, 31},
	{32, 33, 34, 33, 32},
	{35, 36, 37, 38, 39, 40, 41, 42},
	{43, 44, 45, 46, 47, 48, 49, 50},
	{51, 52, 53, 52, 51},
};
uint8 chewy_phasen_anz[] = {8, 8, 8, 8, 5, 8, 8, 5};

uint8 chewy_bo_phasen[8][8] = {
	{ 0, 1, 2, 3, 4, 5},
	{ 6, 7, 8, 9, 10, 11},
	{12, 13, 14, 15, 16, 17},
	{12, 13, 14, 15, 16, 17},
	{18, 19, 20, 19, 18},
	{21, 22, 23, 24, 25, 26},
	{21, 22, 23, 24, 25, 26},
	{27, 28, 29, 28, 27},
};
uint8 chewy_bo_phasen_anz[] = {6, 6, 6, 6, 5, 6, 6, 5};

uint8 chewy_mi_phasen[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 8, 9, 10, 11, 12, 13, 14, 15},
	{24, 25, 26, 27, 28, 29, 30, 31},
	{24, 25, 26, 27, 28, 29, 30, 31},
	{32, 33, 34, 33, 32},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{35, 36, 37, 36, 35},
};
uint8 chewy_mi_phasen_anz[] = {8, 8, 8, 8, 5, 8, 8, 5};

uint8 chewy_ro_phasen[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 8, 9, 10, 11, 12, 13, 14, 15},
	{24, 25, 26, 27, 28, 29, 30, 31},
	{24, 25, 26, 27, 28, 29, 30, 31},
	{32, 33, 34, 35, 36, 35, 34, 33},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{37, 38, 39, 40, 41, 40, 39, 38},
};
uint8 chewy_ro_phasen_anz[] = {8, 8, 8, 8, 8, 8, 8, 8};

uint8 chewy_jm_phasen[8][8] = {
	{ 0, 1, 2, 3, 4, 5, 6, 7},
	{ 8, 9, 10, 11, 12, 13, 14, 15},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{16, 17, 18, 19, 20, 21, 22, 23},
	{24, 25, 26, 27, 27, 26, 25, 24},
	{ 8, 9, 10, 11, 12, 13, 14, 15},
	{ 8, 9, 10, 11, 12, 13, 14, 15},
	{28, 29, 30, 31, 31, 30, 29, 28},
};

int16 PersonAni[MAX_PERSON];
taf_seq_info *PersonTaf[MAX_PERSON];
uint8 PersonSpr[MAX_PERSON][8];

int16 talk_start_ani = -1;
int16 talk_hide_static = -1;

SplitStringInit ssi[AAD_MAX_PERSON] = {

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},

	{ 0, 100, 0, 200, 4, SPLIT_MITTE, 8, 8},
};

int16 frequenz;
int16 CurrentSong = -1;
bool savegameFlag;
Stream *music_handle;
int16 EndOfPool;
int timer_action_ctr;

} // namespace Chewy
