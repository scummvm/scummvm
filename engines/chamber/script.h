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
	ScrPool7_Zapstiks,
	ScrPool8_CurrentPers,
	ScrPools_MAX
};

/*Byte-packed, members accessed from script code by hardcoded offsets*/
typedef struct script_byte_vars_t {
	byte zone_index;       /*  0 */
	byte zone_room;        /*  1 */
	byte last_door;        /*  2 */
	byte cur_spot_idx;     /*  3 */
	byte the_wall_phase;   /*  4 */
	byte prev_zone_index;  /*  5 */
	byte bvar_06;          /*  6 */
	byte bvar_07;          /*  7 */
	byte bvar_08;          /*  8 */
	byte bvar_09;          /*  9 */
	byte bvar_0A;          /*  A */
	byte bvar_0B;          /*  B */
	byte zone_area;        /*  C */
	byte dead_flag;        /*  D */
	volatile byte timer_ticks; /*  E */
	byte gauss_phase;      /*  F */

	byte bvar_10;          /* 10 */
	byte rand_value;       /* 11 */
	byte load_flag;        /* 12 */
	byte spot_m;           /* 13 */
	byte spot_v;           /* 14 */
	byte bvar_15;          /* 15 */
	byte bvar_16;          /* 16 */
	byte bvar_17;          /* 17 */
	byte bvar_18;          /* 18 */
	byte bvar_19;          /* 19 */
	byte bvar_1A;          /* 1A */
	byte bvar_1B;          /* 1B */
	byte bvar_1C;          /* 1C */
	byte bvar_1D;          /* 1D */
	byte bvar_1E;          /* 1E */
	byte bvar_1F;          /* 1F */

	byte cur_pers;         /* 20 */
	byte used_commands;    /* 21 */
	byte tries_left;       /* 22 */
	byte inv_item_index;   /* 23 */
	byte bvar_24;          /* 24 */
	byte bvar_25;          /* 25 */
	byte bvar_26;          /* 26 */
	byte bvar_27;          /* 27 */
	byte bvar_28;          /* 28 */
	byte bvar_29;          /* 29 */
	byte bvar_2A;          /* 2A */
	byte hands;            /* 2B */
	byte check_used_commands; /* 2C */
	byte bvar_2D;          /* 2D */
	byte palette_index;    /* 2E */
	byte bvar_2F;          /* 2F */

	byte bvar_30;          /* 30 */
	byte zapstiks_owned;   /* 31 */
	byte bvar_32;          /* 32 */
	byte bvar_33;          /* 33 */
	byte bvar_34;          /* 34 */
	byte skulls_submitted; /* 35 */
	byte bvar_36;          /* 36 */
	byte bvar_37;          /* 37 */
	byte zone_area_copy;   /* 38 */
	byte aspirant_flags;   /* 39 */
	byte aspirant_pers_ofs;/* 3A */
	byte steals_count;     /* 3B */
	byte fight_status;     /* 3C */
	byte extreme_violence; /* 3D */
	byte trade_accepted;   /* 3E */
	byte bvar_3F;          /* 3F */

	byte bvar_40;          /* 40 */
	byte bvar_41;          /* 41 */
	byte bvar_42;          /* 42 */
	byte bvar_43;          /* 43 */
	byte dirty_rect_kind;  /* 44 */
	byte bvar_45;          /* 45 */
	byte bvar_46;          /* 46 */
	byte game_paused;      /* 47 */
	byte skull_trader_status;/* 48 */
	byte cur_spot_flags;   /* 49 */
	byte bvar_4A;          /* 4A */
	byte bvar_4B;          /* 4B */
	byte bvar_4C;          /* 4C */
	byte bvar_4D;          /* 4D */
	byte bvar_4E;          /* 4E */
	byte bvar_4F;          /* 4F */

	byte bvar_50;          /* 50 */
	byte bvar_51;          /* 51 */
	byte bvar_52;          /* 52 */
	byte bvar_53;          /* 53 */
	byte bvar_54;          /* 54 */
	byte bvar_55;          /* 55 */
	byte bvar_56;          /* 56 */
	byte need_draw_spots;  /* 57 */
	byte bvar_58;          /* 58 */
	byte bvar_59;          /* 59 */
	byte psy_energy;       /* 5A */
	byte bvar_5B;          /* 5B */
	byte bvar_5C;          /* 5C */
	byte bvar_5D;          /* 5D */
	byte bvar_5E;          /* 5E */
	byte bvar_5F;          /* 5F */

	byte bvar_60;          /* 60 */
	byte bvar_61;          /* 61 */
	byte bvar_62;          /* 62 */
	byte bvar_63;          /* 63 */
	byte bvar_64;          /* 64 */
	byte bvar_65;          /* 65 */
	byte bvar_66;          /* 66 */
	byte bvar_67;          /* 67 */
	byte zapstik_stolen;   /* 68 */
	byte bvar_69;          /* 69 */
	byte bvar_6A;          /* 6A */
	byte bvar_6B;          /* 6B */
	byte bvar_6C;          /* 6C */
	byte bvar_6D[4];       /* 6D */
} script_byte_vars_t;

/*2-byte long vars, in BIG-endian order*/
typedef struct script_word_vars_t {
	uint16 psi_cmds[6];         /*  0 */
	uint16 wvar_0C;             /*  C */
	uint16 wvar_0E;             /*  E */
	uint16 timer_ticks2;        /* 10 */
	uint16 zone_obj_cmds[15 * 5];   /* 12 */
	uint16 next_aspirant_cmd;   /* A8 */
	uint16 wvar_AA;             /* AA */
	uint16 wvar_AC;             /* AC */
	uint16 wvar_AE;             /* AE */
	uint16 wvar_B0;             /* B0 */
	uint16 wvar_B2;             /* B2 */
	uint16 wvar_B4;             /* B4 */
	uint16 next_protozorqs_cmd; /* B6 */
	uint16 wvar_B8;             /* B8 */
} script_word_vars_t;

extern void *script_vars[ScrPools_MAX];
extern script_word_vars_t script_word_vars;
extern script_byte_vars_t script_byte_vars;

/*Don't trade this item*/
#define ITEMFLG_DONTWANT 1
#define ITEMFLG_04 0x04
#define ITEMFLG_08 0x08
/*Skull Trader's item*/
#define ITEMFLG_TRADER 0x10
/*Aspirant's item*/
#define ITEMFLG_ASPIR 0x20
/*In a room?*/
#define ITEMFLG_ROOM 0x40
/*In pocket?*/
#define ITEMFLG_OWNED 0x80

/*TODO: manipulated from script, do not change*/
#include "common/pack-start.h"
typedef struct item_t {
	byte flags;
	byte area;		/*item location*/
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

extern pers_t *vort_ptr;

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
