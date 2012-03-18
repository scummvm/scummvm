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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1988-1989 Lankhor
 */

#ifndef MORTEVIELLE_VAR_H
#define MORTEVIELLE_VAR_H

#include "common/rect.h"
#include "common/str.h"

namespace Mortevielle {

/*---------------------------------------------------------------------------*/
/*-------------------           MEMORY  MAP          ------------------------*/
/*---------------------------------------------------------------------------*/
/* The following is a list of physical addresses in memory currently used
 * by the game.
 *
 * Address
 * -------
 * 5000:0 - Music data
 * 6000:0 - Decompressed current image
 * 7000:0+ - Compressed images
 * 7000:2 - 16 words representing palette map
 * 7000:4138 - width, height, x/y offset of decoded image
 */


/*---------------------------------------------------------------------------*/
/*-------------------------     DEFINES    ----------------------------------*/
/*---------------------------------------------------------------------------*/

#define ord(v) ((int) v)
#define chr(v) ((unsigned char) v)
#define lo(v) ((v) & 0xff)
#define hi(v) (((v) >> 8) & 0xff)
#define swap(v) (((lo(v)) << 8) | ((hi(v)) >> 8)) 
#define odd(v) (((v) % 2) == 1)

/*---------------------------------------------------------------------------*/
/*-------------------------    CONSTANTS   ----------------------------------*/
/*---------------------------------------------------------------------------*/

const int maxti = 7975;
const int maxtd = 600;

const int kDescriptionStringIndex = 0;                // Unused
const int kInventoryStringIndex = 186;
const int kQuestionStringIndex = 247;
const int kDialogStringIndex = 292;
const int kMenuPlaceStringIndex = 435;
const int kMenuActionStringIndex = 476;
const int kMenuSelfStringIndex = 497;
const int kMenuSayStringIndex = 502;
const int kSecretPassageQuestionStringIndex = 510;    // Unusued?


const int OPCODE_NONE = 0;
enum verbs {OPCODE_ATTACH = 0x301, OPCODE_WAIT = 0x302,  OPCODE_FORCE = 0x303,   OPCODE_SLEEP = 0x304, OPCODE_LISTEN = 0x305, 
            OPCODE_ENTER = 0x306,  OPCODE_CLOSE = 0x307, OPCODE_SEARCH = 0x308,  OPCODE_KNOCK = 0x309, OPCODE_SCRATCH = 0x30a,
			OPCODE_READ = 0x30b,   OPCODE_EAT = 0x30c,   OPCODE_PLACE = 0x30d,   OPCODE_OPEN = 0x30e,  OPCODE_TAKE = 0x30f,
			OPCODE_LOOK = 0x310,   OPCODE_SMELL = 0x311, OPCODE_SOUND = 0x312,   OPCODE_LEAVE = 0x313, OPCODE_LIFT = 0x314,
			OPCODE_TURN = 0x315,   OPCODE_SHIDE = 0x401, OPCODE_SSEARCH = 0x402, OPCODE_SREAD = 0x403, OPCODE_SPUT = 0x404,
			OPCODE_SLOOK = 0x405};

const int kMaxPatt = 20;

extern const byte g_tabdbc[18];
extern const byte g_tabdph[16];
extern const byte g_typcon[26];
extern const byte g_intcon[26];
extern const byte g_tnocon[364];

extern const byte g_menuConstants[8][4];

extern const byte g_addv[2];

extern const byte g_rang[16];

/*---------------------------------------------------------------------------*/
/*--------------------------------   TYPES   --------------------------------*/
/*---------------------------------------------------------------------------*/

/*
9   "A glance at the forbidden$",
18  "It's already open$",
26  "A photograph$",
27  "The coat of arms$",
*/

enum places {
	OWN_ROOM = 0,     GREEN_ROOM = 1,   PURPLE_ROOM = 2,     TOILETS = 3,      DARKBLUE_ROOM = 4,
	BLUE_ROOM = 5,    RED_ROOM = 6,     BATHROOM = 7,        GREEN_ROOM2 = 8,  ROOM9 = 9,
	DINING_ROOM = 10, BUREAU = 11,      KITCHEN = 12,        ATTIC = 13,       CELLAR = 14, 
	LANDING = 15,     CRYPT = 16,       SECRET_PASSAGE = 17, ROOM18 = 18,      MOUNTAIN = 19, 
	CHAPEL = 20,      MANOR_FRONT = 21, MANOR_BACK = 22,     INSIDE_WELL = 23, WELL = 24,
	DOOR = 25,        ROOM26 = 26,      ROOM27 = 27
};

struct sav_chaine {
	int _faithScore;
	byte _pourc[11];
	byte _teauto[43];
	byte _sjer[31];
	int _currPlace, _atticBallHoleObjectId, _atticRodHoleObjectId, _cellarObjectId, _secretPassageObjectId, _wellObjectId;
	int _selectedObjectId, _purpleRoomObjectId, _cryptObjectId;
	bool _alreadyEnteredManor;
	byte _fullHour;
};

struct registres {
	int _ax, _bx, _cx, _dx, _bp, _si, _di, _ds, _es, _flags;
};

struct ind {
	int _indis;
	byte _point;
};

struct chariot {
	int _val,
		_code,
		_acc,
		_freq,
		_rep;
};

struct rectangle {
	int _x1, _x2, _y1, _y2;
	bool _enabled;
};

struct pattern {
	byte _tay, _tax;
	byte _des[kMaxPatt + 1][kMaxPatt + 1];
};


struct nhom {
	byte _id;     /* number between 0 and 32 */
	byte _hom[4];
};

struct t_pcga {
	byte _p;
	nhom _a[16];
};

typedef int tablint[256];
//typedef Common::Point tabdb[17];
typedef int tfxx[108];

/*---------------------------------------------------------------------------*/
/*------------------------------     ENUMS     ------------------------------*/
/*---------------------------------------------------------------------------*/

enum GraphicModes { MODE_AMSTRAD1512 = 0, MODE_CGA = 1, MODE_EGA = 2, MODE_HERCULES = 3, MODE_TANDY = 4 };

/*---------------------------------------------------------------------------*/
/*------------------------------   VARIABLES   ------------------------------*/
/*---------------------------------------------------------------------------*/

extern int g_x,
        g_y,
        g_t,
        g_jh,
        g_mh,
        g_cs,
        g_hdb,
        g_hfb,
        g_key,
        g_num,
        g_res,
        g_ment,
        g_haut,
        g_caff,
        g_maff,
        g_crep,
        g_ades,
        g_iouv,
		g_ctrm,
        g_dobj,
        g_msg3,
        g_msg4,
        g_mlec,
        g_mchai,
        g_menup,
        g_mpers,
        g_mnumo,
        g_xprec,
        g_yprec,
        g_perdep,
        g_prebru,
        g_numpal,
        g_ptr_oct,
        g_k_tempo;

extern int g_t_cph[6401]; // Speech
extern byte g_tabdon[4001];

extern byte g_is;

extern int g_nbrep[9];
extern int g_nbrepm[9];
extern int g_msg[5];
extern byte g_touv[8];
extern sav_chaine g_s, g_s1;
extern byte g_bufcha[391];

extern byte g_lettres[7][24];

extern uint16 g_t_mot[maxti + 1];
extern int g_tay_tchar;
extern ind g_t_rec[maxtd + 1];
extern int g_l[108];
extern int g_tbi[256];
extern chariot g_c1, g_c2, g_c3;
extern float g_addfix;
extern Common::Point g_tabpal[91][17];
extern t_pcga g_palcga[91];
extern pattern g_tpt[15];

extern byte g_adcfiec[822 * 128];

/*---------------------------------------------------------------------------*/
/*-------------------   PROCEDURES  AND  FONCTIONS   ------------------------*/
/*---------------------------------------------------------------------------*/

void hirs();
Common::String copy(const Common::String &s, int idx, size_t size);
void Val(const Common::String &s, int &V, int Code);

/*---------------------------------------------------------------------------*/
/*------------------------------     STUBS     ------------------------------*/
/*---------------------------------------------------------------------------*/
// TODO: Replace the following with proper implementations, or refactor out the code using them

extern int g_port[0xfff];
extern byte g_mem[65536 * 16];

#define hires {}

extern void palette(int v1);
extern void intr(int intNum, registres &regs);
extern int getRandomNumber(int minval, int maxval);

extern void s_char(int Gd, int y, int dy);
extern void s_sauv(int Gd, int y, int dy);

} // End of namespace Mortevielle

#endif
