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
	byte zone_index;       /*  0 */
	byte zone_room;        /*  1 */
	byte byte_179B8;       /*  2 */
	byte cur_spot_idx;     /*  3 */
	byte the_wall_phase;   /*  4 */
	byte prev_zone_index;  /*  5 */
	byte unused_179BC;     /*  6 */
	byte unused_179BD;     /*  7 */
	byte byte_179BE;       /*  8 */
	byte unused_179BF;     /*  9 */
	byte unused_179C0;     /*  A */
	byte byte_179C1;       /*  B */
	byte zone_area;        /*  C */
	byte dead_flag;        /*  D */
	volatile byte timer_ticks; /*  E */
	byte gauss_phase;      /*  F */
	byte unused_179C6;     /* 10 */
	byte rand_value;       /* 11 */
	byte load_flag;        /* 12 */
	byte spot_m;           /* 13 */
	byte spot_v;           /* 14 */
	byte unused_179CB;     /* 15 */
	byte unused_179CC;     /* 16 */
	byte unused_179CD;     /* 17 */
	byte unused_179CE;     /* 18 */
	byte unused_179CF;     /* 19 */
	byte unused_179D0;     /* 1A */
	byte unused_179D1;     /* 1B */
	byte unused_179D2;     /* 1C */
	byte unused_179D3;     /* 1D */
	byte unused_179D4;     /* 1E */
	byte unused_179D5;     /* 1F */
	byte cur_pers;         /* 20 */
	byte used_commands;    /* 21 */
	byte tries_left;       /* 22 */
	byte inv_item_index;   /* 23 */
	byte unused_179DA;     /* 24 */
	byte byte_179DB;       /* 25 */
	byte byte_179DC;       /* 26 */
	byte byte_179DD;       /* 27 */
	byte byte_179DE;       /* 28 */
	byte byte_179DF;       /* 29 */
	byte byte_179E0;       /* 2A */
	byte byte_179E1;       /* 2B */ /*TODO: hand height*/
	byte check_used_commands; /* 2C */
	byte byte_179E3;       /* 2D */
	byte palette_index;    /* 2E */
	byte byte_179E5;       /* 2F */
	byte byte_179E6;       /* 30 */
	byte room_items;       /* 31 */
	byte byte_179E8;       /* 32 */
	byte byte_179E9;       /* 33 */
	byte byte_179EA;       /* 34 */
	byte byte_179EB;       /* 35 */
	byte byte_179EC;       /* 36 */
	byte byte_179ED;       /* 37 */
	byte zone_area_copy;   /* 38 */
	byte byte_179EF;       /* 39 */
	byte quest_item_ofs;   /* 3A */
	byte byte_179F1;       /* 3B */
	byte byte_179F2;       /* 3C */
	byte byte_179F3;       /* 3D */
	byte trade_done;       /* 3E */
	byte byte_179F5;       /* 3F */
	byte byte_179F6;       /* 40 */
	byte byte_179F7;       /* 41 */
	byte byte_179F8;       /* 42 */
	byte byte_179F9;       /* 43 */
	byte dirty_rect_kind;  /* 44 */
	byte flag_179FB;       /* 45 */
	byte byte_179FC;       /* 46 */
	byte game_paused;      /* 47 */
	byte trade_status;     /* 48 */
	byte cur_spot_flags;   /* 49 */
	byte byte_17A00;       /* 4A */
	byte byte_17A01;       /* 4B */
	byte byte_17A02;       /* 4C */
	byte byte_17A03;       /* 4D */
	byte byte_17A04;       /* 4E */
	byte byte_17A05;       /* 4F */
	byte byte_17A06;       /* 50 */
	byte byte_17A07;       /* 51 */
	byte byte_17A08;       /* 52 */
	byte byte_17A09;       /* 53 */
	byte byte_17A0A;       /* 54 */
	byte byte_17A0B;       /* 55 */
	byte byte_17A0C;       /* 56 */
	byte need_draw_spots;  /* 57 */
	byte byte_17A0E;       /* 58 */
	byte byte_17A0F;       /* 59 */
	byte psy_energy;       /* 5A */
	byte byte_17A11;       /* 5B */
	byte byte_17A12;       /* 5C */
	byte byte_17A13;       /* 5D */
	byte byte_17A14;       /* 5E */
	byte byte_17A15;       /* 5F */
	byte byte_17A16;       /* 60 */
	byte byte_17A17;       /* 61 */
	byte byte_17A18;       /* 62 */
	byte byte_17A19;       /* 63 */
	byte byte_17A1A;       /* 64 */
	byte byte_17A1B;       /* 65 */
	byte byte_17A1C;       /* 66 */
	byte byte_17A1D;       /* 67 */
	byte zapstik_stolen;   /* 68 */
	byte byte_17A1F;       /* 69 */
	byte byte_17A20;       /* 6A */
	byte byte_17A21;       /* 6B */
	byte byte_17A22;       /* 6C */
	byte byte_17A23[4];    /* 6D */
} script_byte_vars_t;

/*2-byte long vars, in BIG-endian order*/
typedef struct script_word_vars_t {
	uint16 psi_cmds[6];			/*  0 */
	uint16 word_17850;          /*  C */
	uint16 word_17852;          /*  E */
	uint16 timer_ticks2;        /* 10 */
	uint16 zone_obj_cmds[15 * 5];   /* 12 */
	uint16 next_command1;       /* A8 */
	uint16 word_178EE;          /* AA */
	uint16 word_178F0;          /* AC */
	uint16 word_178F2;          /* AE */
	uint16 word_178F4;          /* B0 */
	uint16 word_178F6;          /* B2 */
	uint16 word_178F8;          /* B4 */
	uint16 next_command2;       /* B6 */
	uint16 word_178FC;          /* B8 */
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
#include "common/pack-start.h"
typedef struct item_t {
	byte flags;
	byte flags2;
	byte sprite;   /*item sprite index*/
	byte name;     /*item name index (relative)*/
	uint16 command; /*TODO: warning! in native format, check if never accessed from scripts*/
} item_t;
#include "common/pack-end.h"

#define MAX_INV_ITEMS 63
extern item_t inventory_items[MAX_INV_ITEMS];

#define PERS_MAX 41
extern pers_t pers_list[PERS_MAX];

extern byte *script_stack[5 * 2];
extern byte **script_stack_ptr;

extern pers_t *pers_vort_ptr;

#define SPECIAL_COMMANDS_MAX 20
extern uint16 menu_commands_12[SPECIAL_COMMANDS_MAX];
extern uint16 menu_commands_22[SPECIAL_COMMANDS_MAX];
extern uint16 menu_commands_24[SPECIAL_COMMANDS_MAX];
extern uint16 menu_commands_23[SPECIAL_COMMANDS_MAX];

extern uint16 fight_pers_ofs;

extern byte wait_delta;

extern byte rand_seed;
byte Rand(void);
uint16 RandW(void);

extern uint16 the_command;

byte *GetScriptSubroutine(uint16 index);

uint16 RunCommand(void);
uint16 RunCommandKeepSp(void);

uint16 Swap16(uint16 x);

} // End of namespace Chamber

#endif
