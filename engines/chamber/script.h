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

#ifndef CHAMBER_SCRIPT_H
#define CHAMBER_SCRIPT_H

#include "chamber/common.h"
#include "chamber/room.h"

namespace Chamber {

enum ScriptPools {
	ScrPool0_WordVars0,
	ScrPool1_WordVars1,
	ScrPool2_ByteVars,
	ScrPool3_CurrentItem,
	ScrPool4_ZoneSpots,
	ScrPool5_Persons,
	ScrPool6_Inventory,
	ScrPool7_Inventory38,
	ScrPool8_CurrentPers,
	ScrPools_MAX
};

/*Byte-packed, members accessed from script code by hardcoded offsets*/
typedef struct script_byte_vars_t {
	unsigned char zone_index;       /*  0 */
	unsigned char zone_room;        /*  1 */
	unsigned char byte_179B8;       /*  2 */
	unsigned char cur_spot_idx;     /*  3 */
	unsigned char the_wall_phase;   /*  4 */
	unsigned char prev_zone_index;  /*  5 */
	unsigned char unused_179BC;     /*  6 */
	unsigned char unused_179BD;     /*  7 */
	unsigned char byte_179BE;       /*  8 */
	unsigned char unused_179BF;     /*  9 */
	unsigned char unused_179C0;     /*  A */
	unsigned char byte_179C1;       /*  B */
	unsigned char zone_area;        /*  C */
	unsigned char dead_flag;        /*  D */
	volatile unsigned char timer_ticks; /*  E */
	unsigned char gauss_phase;      /*  F */
	unsigned char unused_179C6;     /* 10 */
	unsigned char rand_value;       /* 11 */
	unsigned char load_flag;        /* 12 */
	unsigned char spot_m;           /* 13 */
	unsigned char spot_v;           /* 14 */
	unsigned char unused_179CB;     /* 15 */
	unsigned char unused_179CC;     /* 16 */
	unsigned char unused_179CD;     /* 17 */
	unsigned char unused_179CE;     /* 18 */
	unsigned char unused_179CF;     /* 19 */
	unsigned char unused_179D0;     /* 1A */
	unsigned char unused_179D1;     /* 1B */
	unsigned char unused_179D2;     /* 1C */
	unsigned char unused_179D3;     /* 1D */
	unsigned char unused_179D4;     /* 1E */
	unsigned char unused_179D5;     /* 1F */
	unsigned char cur_pers;         /* 20 */
	unsigned char used_commands;    /* 21 */
	unsigned char tries_left;       /* 22 */
	unsigned char inv_item_index;   /* 23 */
	unsigned char unused_179DA;     /* 24 */
	unsigned char byte_179DB;       /* 25 */
	unsigned char byte_179DC;       /* 26 */
	unsigned char byte_179DD;       /* 27 */
	unsigned char byte_179DE;       /* 28 */
	unsigned char byte_179DF;       /* 29 */
	unsigned char byte_179E0;       /* 2A */
	unsigned char byte_179E1;       /* 2B */ /*TODO: hand height*/
	unsigned char check_used_commands; /* 2C */
	unsigned char byte_179E3;       /* 2D */
	unsigned char palette_index;    /* 2E */
	unsigned char byte_179E5;       /* 2F */
	unsigned char byte_179E6;       /* 30 */
	unsigned char room_items;       /* 31 */
	unsigned char byte_179E8;       /* 32 */
	unsigned char byte_179E9;       /* 33 */
	unsigned char byte_179EA;       /* 34 */
	unsigned char byte_179EB;       /* 35 */
	unsigned char byte_179EC;       /* 36 */
	unsigned char byte_179ED;       /* 37 */
	unsigned char zone_area_copy;   /* 38 */
	unsigned char byte_179EF;       /* 39 */
	unsigned char quest_item_ofs;   /* 3A */
	unsigned char byte_179F1;       /* 3B */
	unsigned char byte_179F2;       /* 3C */
	unsigned char byte_179F3;       /* 3D */
	unsigned char byte_179F4;       /* 3E */
	unsigned char byte_179F5;       /* 3F */
	unsigned char byte_179F6;       /* 40 */
	unsigned char byte_179F7;       /* 41 */
	unsigned char byte_179F8;       /* 42 */
	unsigned char byte_179F9;       /* 43 */
	unsigned char dirty_rect_kind;  /* 44 */
	unsigned char flag_179FB;       /* 45 */
	unsigned char byte_179FC;       /* 46 */
	unsigned char game_paused;      /* 47 */
	unsigned char trade_status;     /* 48 */
	unsigned char cur_spot_flags;   /* 49 */
	unsigned char byte_17A00;       /* 4A */
	unsigned char byte_17A01;       /* 4B */
	unsigned char byte_17A02;       /* 4C */
	unsigned char byte_17A03;       /* 4D */
	unsigned char byte_17A04;       /* 4E */
	unsigned char byte_17A05;       /* 4F */
	unsigned char byte_17A06;       /* 50 */
	unsigned char byte_17A07;       /* 51 */
	unsigned char byte_17A08;       /* 52 */
	unsigned char byte_17A09;       /* 53 */
	unsigned char byte_17A0A;       /* 54 */
	unsigned char byte_17A0B;       /* 55 */
	unsigned char byte_17A0C;       /* 56 */
	unsigned char need_draw_spots;  /* 57 */
	unsigned char byte_17A0E;       /* 58 */
	unsigned char byte_17A0F;       /* 59 */
	unsigned char psy_energy;       /* 5A */
	unsigned char byte_17A11;       /* 5B */
	unsigned char byte_17A12;       /* 5C */
	unsigned char byte_17A13;       /* 5D */
	unsigned char byte_17A14;       /* 5E */
	unsigned char byte_17A15;       /* 5F */
	unsigned char byte_17A16;       /* 60 */
	unsigned char byte_17A17;       /* 61 */
	unsigned char byte_17A18;       /* 62 */
	unsigned char byte_17A19;       /* 63 */
	unsigned char byte_17A1A;       /* 64 */
	unsigned char byte_17A1B;       /* 65 */
	unsigned char byte_17A1C;       /* 66 */
	unsigned char byte_17A1D;       /* 67 */
	unsigned char byte_17A1E;       /* 68 */
	unsigned char byte_17A1F;       /* 69 */
	unsigned char byte_17A20;       /* 6A */
	unsigned char byte_17A21;       /* 6B */
	unsigned char byte_17A22;       /* 6C */
	unsigned char byte_17A23[4];    /* 6D */
} script_byte_vars_t;

/*2-byte long vars, in BIG-endian order*/
typedef struct script_word_vars_t {
	unsigned short word_17844;          /*  0 */
	unsigned short word_17846;          /*  2 */
	unsigned short word_17848;          /*  4 */
	unsigned short word_1784A;          /*  6 */
	unsigned short word_1784C;          /*  8 */
	unsigned short word_1784E;          /*  A */
	unsigned short word_17850;          /*  C */
	unsigned short word_17852;          /*  E */
	unsigned short timer_ticks2;        /* 10 */
	unsigned short zone_obj_cmds[15 * 5];   /* 12 */
	unsigned short next_command1;       /* A8 */
	unsigned short word_178EE;          /* AA */
	unsigned short word_178F0;          /* AC */
	unsigned short word_178F2;          /* AE */
	unsigned short word_178F4;          /* B0 */
	unsigned short word_178F6;          /* B2 */
	unsigned short word_178F8;          /* B4 */
	unsigned short next_command2;       /* B6 */
	unsigned short word_178FC;          /* B8 */
} script_word_vars_t;

extern void *script_vars[ScrPools_MAX];
extern script_word_vars_t script_word_vars;
extern script_byte_vars_t script_byte_vars;

/*Trader's item*/
#define ITEMFLG_10 0x10
/*Aspirant's item*/
#define ITEMFLG_20 0x20
/*In a room?*/
#define ITEMFLG_40 0x40
/*In pocket?*/
#define ITEMFLG_80 0x80

/*TODO: manipulated from script, do not change*/
typedef struct item_t {
	unsigned char flags;
	unsigned char flags2;
	unsigned char sprite;   /*item sprite index*/
	unsigned char name;     /*item name index (relative)*/
	unsigned short command; /*TODO: warning! in native format, check if never accessed from scripts*/
} item_t;

#if sizeof(item_t) != 6
#error "item_t must be 6 bytes long"
#endif

#define MAX_INV_ITEMS 63
extern item_t inventory_items[MAX_INV_ITEMS];

#define PERS_MAX 41
extern pers_t pers_list[PERS_MAX];

extern unsigned char rand_seed;
unsigned char Rand(void);
unsigned int RandW(void);

extern unsigned short the_command;

unsigned char *GetScriptSubroutine(unsigned int index);

unsigned int RunCommand(void);
unsigned int RunCommandKeepSp(void);

unsigned int Swap16(unsigned int x);

} // End of namespace Chamber

#endif
