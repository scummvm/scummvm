/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "common/util.h"
#include "sky/compact.h"
#include "sky/compacts/0compact.h"
#include "sky/compacts/1compact.h"
#include "sky/compacts/29comp.h"
#include "sky/compacts/2compact.h"
#include "sky/compacts/30comp.h"
#include "sky/compacts/3compact.h"
#include "sky/compacts/4compact.h"
#include "sky/compacts/5compact.h"
#include "sky/compacts/66comp.h"
#include "sky/compacts/90comp.h"
#include "sky/compacts/9compact.h"
#include "sky/compacts/linc_gen.h"
#include "sky/compacts/lincmenu.h"
#include "sky/compacts/z_compac.h"

namespace Sky {

#define COMPACT_SIZE (sizeof(compactOffsets)/sizeof(uint32))
#define EXTCOMPACT_SIZE (sizeof(extCompactOffsets)/sizeof(uint32))
#define MEGASET_SIZE (sizeof(megaSetOffsets)/sizeof(uint32))
#define TURNTABLE_SIZE (sizeof(turnTableOffsets)/sizeof(uint32))

#define OFFS(type,item) (((long)(&((type*)0)->item)))
#define MK32(type,item) OFFS(type, item),0,0,0
#define MK16(type,item) OFFS(type, item),0
#define MK32_A5(type, item) MK32(type, item[0]), MK32(type, item[1]), \
	MK32(type, item[2]), MK32(type, item[3]), MK32(type, item[4])

namespace TalkAnims {
	extern bool animTalkTableIsPointer[];
	extern uint16 animTalkTableVal[];
	extern void *animTalkTablePtr[];
}

namespace SkyCompact {

uint16 *getGrafixPtr(Compact *cpt) {
	uint16 *buf;
	switch (cpt->grafixProg.ptrType) {
	case PTR_NULL:
		return NULL;
	case AUTOROUTE:
		if (!cpt->extCompact)
			error("::getGrafixPtr: request for AR pointer, extCompact is NULL, though.");
		return (cpt->extCompact->animScratch + cpt->grafixProg.pos);
	case COMPACT:
		buf = (uint16*)SkyEngine::fetchCompact(cpt->grafixProg.ptrTarget);
		if (buf == NULL)
			error("::getGrafixPtr: request for cpt %d pointer. It's NULL.", cpt->grafixProg.ptrTarget);
		return (buf + cpt->grafixProg.pos);
	case COMPACTELEM:
		buf = *(uint16 **)SkyCompact::getCompactElem(cpt, cpt->grafixProg.ptrTarget);
		if (buf == NULL)
			error("::getGrafixPtr: request for elem ptr %d. It's NULL.", cpt->grafixProg.ptrTarget);
		return buf + cpt->grafixProg.pos;
	case TALKTABLE:
		buf = (uint16 *)TalkAnims::animTalkTablePtr[cpt->grafixProg.ptrTarget];
		return buf + cpt->grafixProg.pos;
	default:
		error("::getGrafixPtr: unknown grafixProg type for Compact cpt");
	}
	return NULL; // never reached
}

/**
 * Returns the n'th mega set specified by \a megaSet from Compact \a cpt.
 */
MegaSet *getMegaSet(Compact *cpt, uint16 megaSet) {
	switch (megaSet) {
	case 0:
		return cpt->extCompact->megaSet0;
	case NEXT_MEGA_SET:
		return cpt->extCompact->megaSet1;
	case NEXT_MEGA_SET*2:
		return cpt->extCompact->megaSet2;
	case NEXT_MEGA_SET*3:
		return cpt->extCompact->megaSet3;
	default:
		error("Invalid MegaSet (%d)", megaSet);
	}
}

/**
 \brief Returns the turn table for direction \a dir
 	from Compact \a cpt in \a megaSet.
  
 Functionally equivalent to:
 \verbatim
 clear eax
 mov al,20
 mul (cpt[esi]).c_dir
 add ax,(cpt[esi]).c_mega_set
 lea eax,(cpt[esi+eax]).c_turn_table_up
 \endverbatim
*/
uint16 **getTurnTable(Compact *cpt, uint16 megaSet, uint16 dir) {
	MegaSet *m = getMegaSet(cpt, megaSet);
	switch (dir) {
	case 0:
		return m->turnTable->turnTableUp;
	case 1:
		return m->turnTable->turnTableDown;
	case 2:
		return m->turnTable->turnTableLeft;
	case 3:
		return m->turnTable->turnTableRight;
	case 4:
		return m->turnTable->turnTableTalk;
	default:
		error("No TurnTable (%d) in MegaSet (%d)", dir, megaSet);
	}
}

/**
 * \brief Returns the script for \a mode from Compact \a cpt.
 *        Add 2 to \a mode to get the offset.
 *
 \verbatim
 uint16 *scriptNo = SkyCompact::getSub(_compact, mode);
 uint16 *offset   = SkyCompact::getSub(_compact, mode + 2);
 uint32 script = (*offset << 16) | *scriptNo;
 \endverbatim
 * Is functionally equivalent to:
 \verbatim
 mov eax,c_base_sub[ebx+esi]
 \endverbatim
 where \a esi is the compact and ebx the mode.
 */ 
uint16 *getSub(Compact *cpt, uint16 mode) {
	switch (mode) {
	case 0:
		return &(cpt->baseSub);
	case 2:
		return &(cpt->baseSub_off);
	case 4:
		return &(cpt->extCompact->actionSub);
	case 6:
		return &(cpt->extCompact->actionSub_off);
	case 8:
		return &(cpt->extCompact->getToSub);
	case 10:
		return &(cpt->extCompact->getToSub_off);
	case 12:
		return &(cpt->extCompact->extraSub);
	case 14:
		return &(cpt->extCompact->extraSub_off);
	default:
		error("Invalid Mode (%d)", mode);
	}
}

static const uint32 compactOffsets[] = {
	MK16(Compact, logic),
	MK16(Compact, status),
	MK16(Compact, sync),
	MK16(Compact, screen),
	MK16(Compact, place),
	MK32(Compact, getToTable),
	MK16(Compact, xcood),
	MK16(Compact, ycood),
	MK16(Compact, frame),
	MK16(Compact, cursorText),
	MK16(Compact, mouseOn),
	MK16(Compact, mouseOff),
	MK16(Compact, mouseClick),
	MK16(Compact, mouseRelX),
	MK16(Compact, mouseRelY),
	MK16(Compact, mouseSizeX),
	MK16(Compact, mouseSizeY),
	MK16(Compact, actionScript),
	MK16(Compact, upFlag),
	MK16(Compact, downFlag),
	MK16(Compact, getToFlag),
	MK16(Compact, flag),
	MK16(Compact, mood),
	MK32(Compact, grafixProg),
	MK16(Compact, offset),
	MK16(Compact, mode),
	MK16(Compact, baseSub),
	MK16(Compact, baseSub_off),
};

static const uint32 extCompactOffsets[] = {
	MK16(ExtCompact, actionSub),
	MK16(ExtCompact, actionSub_off),
	MK16(ExtCompact, getToSub),
	MK16(ExtCompact, getToSub_off),
	MK16(ExtCompact, extraSub),
	MK16(ExtCompact, extraSub_off),
	MK16(ExtCompact, dir),
	MK16(ExtCompact, stopScript),
	MK16(ExtCompact, miniBump),
	MK16(ExtCompact, leaving),
	MK16(ExtCompact, atWatch),
	MK16(ExtCompact, atWas),
	MK16(ExtCompact, alt),
	MK16(ExtCompact, request),
	MK16(ExtCompact, spWidth_xx),
	MK16(ExtCompact, spColour),
	MK16(ExtCompact, spTextId),
	MK16(ExtCompact, spTime),
	MK16(ExtCompact, arAnimIndex),
	MK32(ExtCompact, turnProg),
	MK16(ExtCompact, waitingFor),
	MK16(ExtCompact, arTargetX),
	MK16(ExtCompact, arTargetY),
	MK32(ExtCompact, animScratch),
	MK16(ExtCompact, megaSet),
};

static const uint32 megaSetOffsets[] = {
	MK16(MegaSet, gridWidth),
	MK16(MegaSet, colOffset),
	MK16(MegaSet, colWidth),
	MK16(MegaSet, lastChr),
	MK32(MegaSet, animUp),
	MK32(MegaSet, animDown),
	MK32(MegaSet, animLeft),
	MK32(MegaSet, animRight),
	MK32(MegaSet, standUp),
	MK32(MegaSet, standDown),
	MK32(MegaSet, standLeft),
	MK32(MegaSet, standRight),
	MK32(MegaSet, standTalk),
};

static const uint32 turnTableOffsets[] = {
	MK32_A5(TurnTable, turnTableUp),
	MK32_A5(TurnTable, turnTableDown),
	MK32_A5(TurnTable, turnTableLeft),
	MK32_A5(TurnTable, turnTableRight),
	MK32_A5(TurnTable, turnTableTalk),
};

/**
 * Returns a void pointer to offset \a off in compact \a cpt
 * as it would be on a 386.
 */
void *getCompactElem(Compact *cpt, uint32 off) {
	if (off < COMPACT_SIZE)
		return((uint8 *)cpt + compactOffsets[off]);

	off -= COMPACT_SIZE;
	if (off < EXTCOMPACT_SIZE)
		return((uint8 *)(cpt->extCompact) + extCompactOffsets[off]);

	off -= EXTCOMPACT_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet0) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet0->turnTable) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet1) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet1->turnTable) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet2) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet2->turnTable) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)(cpt->extCompact->megaSet3) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)(cpt->extCompact->megaSet3->turnTable) + turnTableOffsets[off]);
	off -= TURNTABLE_SIZE;

	error("Offset %X out of bounds of compact", off + COMPACT_SIZE + EXTCOMPACT_SIZE + 4 * MEGASET_SIZE + 4 * TURNTABLE_SIZE);
}

// getToTables
uint16 s11_fl_tab288[33] = {
	0x3132, 0x001C, 0x3139, 0x3017, 0x3145, 0x301B, 0x3148, 0x301D, 
	0x314C, 0x301F, 0x314D, 0x3021, 0x314E, 0x3022, 0x314F, 0x3023, 
	0x3150, 0x3024, 0x3152, 0x301F, 0x3153, 0x3021, 0x3154, 0x3022, 
	0x3155, 0x3023, 0x3156, 0x3024, 0x3157, 0x3026, 0x0000, 0x3028, 
	0xFFFF
};

uint16 s19_fl_tab288[17] = {
	0x3042, 0x001C, 0x0002, 0x002A, 0x0000, 0x302A, 0x0001, 0x3029, 
	0x3046, 0x302B, 0x3049, 0x302D, 0x3189, 0x302F, 0x3001, 0x3033, 
	0xFFFF
};

uint16 s20_fl_tab288[37] = {
	0x30A4, 0x001C, 0x0002, 0x002A, 0x0000, 0x3035, 0x0001, 0x303F, 
	0x0003, 0x3049, 0x30A8, 0x3037, 0x30AB, 0x303B, 0x30BA, 0x3047, 
	0x30AE, 0x303D, 0x30BD, 0x3043, 0x30C8, 0x304A, 0x30C9, 0x304C, 
	0x30CA, 0x304E, 0x3102, 0x3052, 0x310B, 0x3054, 0x310C, 0x3056, 
	0x0017, 0x3059, 0x0018, 0x305B, 0xFFFF 
};

uint16 s21_fl_tab288[31] = {
	0x30C2, 0x001C, 0x0002, 0x002A, 0x0000, 0x305C, 0x30C6, 0x305D, 
	0x30DE, 0x305F, 0x30DF, 0x3061, 0x30E0, 0x3063, 0x30E2, 0x3066, 
	0x30E4, 0x3069, 0x30E5, 0x306B, 0x30EC, 0x306D, 0x30EE, 0x3072, 
	0x30EF, 0x3074, 0x30ED, 0x306F, 0x30F6, 0x3077, 0xFFFF
};

uint16 s22_fl_tab288[31] = {
	0x30B3, 0x001C, 0x0002, 0x002A, 0x0000, 0x307A, 0x30B7, 0x307B, 
	0x30CC, 0x307D, 0x30CD, 0x307F, 0x30CE, 0x3081, 0x30CF, 0x3081, 
	0x30D0, 0x3085, 0x30D5, 0x3087, 0x30D9, 0x308A, 0x30DA, 0x308C, 
	0x30DB, 0x308E, 0x30DC, 0x3090, 0x30DD, 0x3092, 0xFFFF
} ;

uint16 s23_fl_tab288[27] = {
	0x3015, 0x001C, 0x0002, 0x002A, 0x0000, 0x3095, 0x0001, 0x3096, 
	0x0003, 0x3097, 0x3019, 0x3098, 0x301B, 0x309A, 0x3027, 0x309D, 
	0x305E, 0x309F, 0x305F, 0x30A1, 0x3060, 0x30A3, 0x3061, 0x30A5, 
	0x3184, 0x30A7, 0xFFFF
};

uint16 s24_fl_tab288[23] = {
	0x302C, 0x001C, 0x0002, 0x002A, 0x0000, 0x30A9, 0x3030, 0x30AA, 
	0x3062, 0x30AC, 0x3063, 0x30AE, 0x3064, 0x30B0, 0x3075, 0x30B2, 
	0x309A, 0x30B7, 0x3115, 0x30BA, 0x3118, 0x30BB, 0xFFFF
};

uint16 s25_fl_tab288[19] = {
	0x3020, 0x001C, 0x0002, 0x002A, 0x0000, 0x30BE, 0x3024, 0x30BF, 
	0x308E, 0x30C4, 0x0003, 0x30C7, 0x3093, 0x30C9, 0x3092, 0x30CE, 
	0x3101, 0x30D0, 0xFFFF
};

uint16 s26_fl_tab288[29] = {
	0x304E, 0x001C, 0x0002, 0x002A, 0x0000, 0x30D3, 0x0001, 0x30D2, 
	0x3052, 0x30D4, 0x3066, 0x30E3, 0x3057, 0x30D7, 0x3058, 0x30D7, 
	0x3059, 0x30D7, 0x305A, 0x30D7, 0x305B, 0x30DC, 0x305C, 0x30E9, 
	0x305D, 0x30DF, 0x30FD, 0x30E7, 0xFFFF
};

uint16 body_tab288[7] = {
	0x3081, 0x30F5, 0x3073, 0x30F3, 0x0018, 0x30FC, 0xFFFF
};

uint16 chair_tab288[23] = {
	0x3081, 0x001C, 0x306B, 0x30F7, 0x306F, 0x30F7, 0x0017, 0x30F7, 
	0x3071, 0x30F7, 0x3072, 0x30F7, 0x3073, 0x30F7, 0x3074, 0x30F7, 
	0x3089, 0x30F7, 0x308D, 0x30F7, 0x3084, 0x30F7, 0xFFFF
};

uint16 s27_fl_tab288[29] = {
	0x306B, 0x001C, 0x306F, 0x30ED, 0x0002, 0x002A, 0x0017, 0x30FA, 
	0x0018, 0x30FC, 0x0000, 0x30EC, 0x3071, 0x30EF, 0x3072, 0x30F1, 
	0x3073, 0x30F3, 0x3074, 0x30F8, 0x3089, 0x30FE, 0x3081, 0x30F5, 
	0x3084, 0x3109, 0x308D, 0x3108, 0xFFFF
};

uint16 s28_fl_tab288[27] = {
	0x3037, 0x001C, 0x0002, 0x002A, 0x0000, 0x310E, 0x0001, 0x3113, 
	0x0003, 0x3119, 0x303B, 0x310F, 0x303D, 0x3111, 0x309F, 0x3117, 
	0x3054, 0x3114, 0x0017, 0x311B, 0x0018, 0x311D, 0x3196, 0x3125, 
	0x3199, 0x3128, 0xFFFF
};

uint16 s28_sml_tab288[13] = {
	0x318E, 0x001C, 0x0002, 0x002A, 0x3187, 0x311E, 0x3188, 0x3120, 
	0x0004, 0x3122, 0x0005, 0x3123, 0xFFFF
};

uint16 s29_fl_tab288[27] = {
	0x3004, 0x001C, 0x0002, 0x002A, 0x3008, 0x312C, 0x300B, 0x312E, 
	0x002A, 0x3137, 0x3010, 0x3130, 0x3032, 0x3133, 0x0001, 0x3132, 
	0x0003, 0x3135, 0x002A, 0x3137, 0x0017, 0x313C, 0x0018, 0x313E, 
	0x312C, 0x313F, 0xFFFF
};

uint16 s29_sml_tab288[13] = {
	0x318A, 0x001C, 0x0002, 0x002A, 0x3185, 0x3141, 0x3186, 0x3143, 
	0x0004, 0x3145, 0x0005, 0x3146, 0xFFFF
};

void patchFor288(void) {
	memcpy(s11_floor_table, s11_fl_tab288, sizeof(s11_fl_tab288));
	memcpy(s19_floor_table, s19_fl_tab288, sizeof(s19_fl_tab288));
	memcpy(s20_floor_table, s20_fl_tab288, sizeof(s20_fl_tab288));
	memcpy(s21_floor_table, s21_fl_tab288, sizeof(s21_fl_tab288));
	memcpy(s22_floor_table, s22_fl_tab288, sizeof(s22_fl_tab288));
	memcpy(s23_floor_table, s23_fl_tab288, sizeof(s23_fl_tab288));
	memcpy(s24_floor_table, s24_fl_tab288, sizeof(s24_fl_tab288));
	memcpy(s25_floor_table, s25_fl_tab288, sizeof(s25_fl_tab288));
	memcpy(s26_floor_table, s26_fl_tab288, sizeof(s26_fl_tab288));
	memcpy(s27_floor_table, s27_fl_tab288, sizeof(s27_fl_tab288));
	memcpy(s28_floor_table, s28_fl_tab288, sizeof(s28_fl_tab288));
	memcpy(s28_sml_table, s28_sml_tab288, sizeof(s28_sml_tab288));
	memcpy(s29_floor_table, s29_fl_tab288, sizeof(s29_fl_tab288));
	memcpy(s29_sml_table, s29_sml_tab288, sizeof(s29_sml_tab288));
	memcpy(body_table, body_tab288, sizeof(body_tab288));
	memcpy(chair_table, chair_tab288, sizeof(chair_tab288));

	((Compact *)SkyCompact::data_0[121])->baseSub = 0x50; // full_screen
	((Compact *)SkyCompact::data_0[122])->mouseClick = 0x51; // cancel_button
	((Compact *)SkyCompact::data_0[122])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[123])->mouseClick = 0x52; // button_0
	((Compact *)SkyCompact::data_0[123])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[124])->mouseClick = 0x52; // button_1
	((Compact *)SkyCompact::data_0[124])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[125])->mouseClick = 0x52; // button_2
	((Compact *)SkyCompact::data_0[125])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[126])->mouseClick = 0x52; // button_3
	((Compact *)SkyCompact::data_0[126])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[127])->mouseClick = 0x52; // button_4
	((Compact *)SkyCompact::data_0[127])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[128])->mouseClick = 0x52; // button_5
	((Compact *)SkyCompact::data_0[128])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[129])->mouseClick = 0x52; // button_6
	((Compact *)SkyCompact::data_0[129])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[130])->mouseClick = 0x52; // button_7
	((Compact *)SkyCompact::data_0[130])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[131])->mouseClick = 0x52; // button_8
	((Compact *)SkyCompact::data_0[131])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[132])->mouseClick = 0x52; // button_9
	((Compact *)SkyCompact::data_0[132])->baseSub = 0x53;
	((Compact *)SkyCompact::data_0[173])->baseSub = 0x54; // retina_scan
	((Compact *)SkyCompact::data_3[1])->actionScript = 0x3032; // useless_char
	((Compact *)SkyCompact::data_3[1])->baseSub = 0x3034;
	((Compact *)SkyCompact::data_3[8])->actionScript = 0x312B; // lift_29
	((Compact *)SkyCompact::data_3[8])->baseSub = 0x3129;
	((Compact *)SkyCompact::data_3[11])->actionScript = 0x312D; // s29_card_slot
	((uint16 *)SkyCompact::data_3[14])[11] = 0x312F; // rs_joey_7_29
	((Compact *)SkyCompact::data_3[16])->actionScript = 0x3131; // right_exit_29
	((Compact *)SkyCompact::data_3[25])->actionScript = 0x3099; // left_exit_23
	((Compact *)SkyCompact::data_3[27])->actionScript = 0x309B; // anchor_exit_23
	((Compact *)SkyCompact::data_3[36])->actionScript = 0x30C0; // anchor_exit_25
	((Compact *)SkyCompact::data_3[39])->actionScript = 0x309E; // travel_exit_23
	((Compact *)SkyCompact::data_3[48])->actionScript = 0x30AB; // left_exit_24
	((Compact *)SkyCompact::data_3[50])->actionScript = 0x3134; // left_exit_29
	((Compact *)SkyCompact::data_3[59])->actionScript = 0x3110; // right_exit_28
	((Compact *)SkyCompact::data_3[61])->actionScript = 0x3112; // left_exit_28
	((Compact *)SkyCompact::data_3[70])->actionScript = 0x302C; // right_exit_19
	((Compact *)SkyCompact::data_3[73])->actionScript = 0x302E; // left_exit_19
	((Compact *)SkyCompact::data_3[82])->actionScript = 0x30D5; // right_exit_26
	((Compact *)SkyCompact::data_3[84])->actionScript = 0x3115; // dustbin_28
	((Compact *)SkyCompact::data_3[87])->actionScript = 0x30D8; // poster1
	((Compact *)SkyCompact::data_3[88])->actionScript = 0x30D9; // poster2
	((Compact *)SkyCompact::data_3[89])->actionScript = 0x30DA; // poster3
	((Compact *)SkyCompact::data_3[90])->actionScript = 0x30DB; // poster4
	((Compact *)SkyCompact::data_3[91])->actionScript = 0x30DD; // plant_26
	((Compact *)SkyCompact::data_3[92])->actionScript = 0x30EB; // leaflet
	((Compact *)SkyCompact::data_3[92])->baseSub = 0x30EA;
	((Compact *)SkyCompact::data_3[93])->actionScript = 0x30E0; // holo
	((Compact *)SkyCompact::data_3[93])->baseSub = 0x30E1;
	((Compact *)SkyCompact::data_3[94])->actionScript = 0x30A0; // bin_23
	((Compact *)SkyCompact::data_3[95])->actionScript = 0x30A2; // sculpture
	((Compact *)SkyCompact::data_3[96])->actionScript = 0x30A4; // link_23
	((Compact *)SkyCompact::data_3[97])->actionScript = 0x30A6; // wreck_23
	((Compact *)SkyCompact::data_3[98])->actionScript = 0x30AD; // london_poster
	((Compact *)SkyCompact::data_3[99])->actionScript = 0x30AF; // new_york_poster
	((Compact *)SkyCompact::data_3[100])->actionScript = 0x30B1; // mural
	((Compact *)SkyCompact::data_3[102])->actionScript = 0x30E4; // left_exit_26
	((Compact *)SkyCompact::data_3[111])->actionScript = 0x30EE; // right_exit_27
	((Compact *)SkyCompact::data_3[113])->actionScript = 0x30F0; // chart1
	((Compact *)SkyCompact::data_3[114])->actionScript = 0x30F2; // chart2
	((Compact *)SkyCompact::data_3[115])->actionScript = 0x30F4; // gas
	((Compact *)SkyCompact::data_3[116])->actionScript = 0x30F9; // scanner_27
	((Compact *)SkyCompact::data_3[116])->baseSub = 0x3103;
	((Compact *)SkyCompact::data_3[117])->actionScript = 0x30B3; // pidgeons
	((Compact *)SkyCompact::data_3[119])->baseSub = 0x30FD; // burke
	((Compact *)SkyCompact::data_3[129])->actionScript = 0x30F6; // chair_27
	((Compact *)SkyCompact::data_3[132])->actionScript = 0x310A; // helmet_cole
	((Compact *)SkyCompact::data_3[132])->baseSub = 0x3104;
	((Compact *)SkyCompact::data_3[137])->actionScript = 0x30FF; // medi_comp
	((Compact *)SkyCompact::data_3[137])->baseSub = 0x310D;
	((Compact *)SkyCompact::data_3[141])->actionScript = 0x3106; // body
	((Compact *)SkyCompact::data_3[141])->baseSub = 0x3105;
	((Compact *)SkyCompact::data_3[142])->actionScript = 0x30C3; // anchor
	((Compact *)SkyCompact::data_3[142])->baseSub = 0x30C1; // anchor
	((Compact *)SkyCompact::data_3[146])->actionScript = 0x30CF; // hook
	((Compact *)SkyCompact::data_3[146])->baseSub = 0x30C8; // hook
	((Compact *)SkyCompact::data_3[147])->actionScript = 0x30CA; // statue_25
	((Compact *)SkyCompact::data_3[149])->baseSub = 0x30CC; // lazer_25
	((Compact *)SkyCompact::data_3[151])->baseSub = 0x30CD; // spark_25
	((Compact *)SkyCompact::data_3[154])->actionScript = 0x30B6; // trevor
	((Compact *)SkyCompact::data_3[154])->baseSub = 0x30B4; // trevor
	((Compact *)SkyCompact::data_3[159])->actionScript = 0x3118; // up_exit_28
	((Compact *)SkyCompact::data_3[168])->actionScript = 0x3038; // down_exit_20
	((Compact *)SkyCompact::data_3[171])->mouseOn = 0x3039; // reich_door_20
	((Compact *)SkyCompact::data_3[171])->actionScript = 0x303C;
	((Compact *)SkyCompact::data_3[171])->baseSub = 0x3040;
	((Compact *)SkyCompact::data_3[174])->actionScript = 0x303E; // reich_slot
	((Compact *)SkyCompact::data_3[183])->actionScript = 0x307C; // right_exit_22

	((Compact *)SkyCompact::data_3[186])->mouseOn = 0x3045; // lamb_door_20
	((Compact *)SkyCompact::data_3[186])->actionScript = 0x3048; // lamb_door_20
	((Compact *)SkyCompact::data_3[186])->baseSub = 0x3041; // lamb_door_20

	((Compact *)SkyCompact::data_3[189])->actionScript = 0x3044; // lamb_slot
	((Compact *)SkyCompact::data_3[198])->actionScript = 0x305E; // left_exit_21
	((Compact *)SkyCompact::data_3[200])->actionScript = 0x304B; // shrub_1
	((Compact *)SkyCompact::data_3[201])->actionScript = 0x304D; // shrub_2
	((Compact *)SkyCompact::data_3[202])->actionScript = 0x304F; // shrub_3
	((Compact *)SkyCompact::data_3[204])->actionScript = 0x307E; // lamb_bed
	((Compact *)SkyCompact::data_3[205])->actionScript = 0x3080; // lamb_tv
	((Compact *)SkyCompact::data_3[206])->actionScript = 0x3082; // fish_tank
	((Compact *)SkyCompact::data_3[206])->baseSub = 0x3089;
	((Compact *)SkyCompact::data_3[207])->actionScript = 0x3083; // fish_poster
	((Compact *)SkyCompact::data_3[208])->actionScript = 0x3086; // pillow
	((Compact *)SkyCompact::data_3[208])->baseSub = 0x3084;
	((Compact *)SkyCompact::data_3[213])->actionScript = 0x3088; // magazine
	((Compact *)SkyCompact::data_3[217])->actionScript = 0x308B; // reich_chair
	((Compact *)SkyCompact::data_3[218])->actionScript = 0x308D; // cabinet
	((Compact *)SkyCompact::data_3[219])->actionScript = 0x308F; // cert
	((Compact *)SkyCompact::data_3[220])->actionScript = 0x3091; // reich_picture
	((Compact *)SkyCompact::data_3[221])->actionScript = 0x3093; // fish_food
	((Compact *)SkyCompact::data_3[222])->actionScript = 0x3060; // lambs_books
	((Compact *)SkyCompact::data_3[223])->actionScript = 0x3062; // lambs_chair
	((Compact *)SkyCompact::data_3[224])->actionScript = 0x3064; // dispensor
	((Compact *)SkyCompact::data_3[226])->actionScript = 0x3067; // cat_food
	((Compact *)SkyCompact::data_3[226])->baseSub = 0x3065; // cat_food
	((Compact *)SkyCompact::data_3[228])->actionScript = 0x306A; // video
	((Compact *)SkyCompact::data_3[228])->baseSub = 0x3068; // video
	((Compact *)SkyCompact::data_3[229])->actionScript = 0x306C; // cassette
	((Compact *)SkyCompact::data_3[236])->actionScript = 0x306E; // big_pict1
	((Compact *)SkyCompact::data_3[237])->actionScript = 0x3070; // video_screen
	((Compact *)SkyCompact::data_3[237])->baseSub = 0x3071;
	((Compact *)SkyCompact::data_3[238])->actionScript = 0x3073; // big_pict2
	((Compact *)SkyCompact::data_3[239])->actionScript = 0x3075; // big_pict3
	((Compact *)SkyCompact::data_3[246])->actionScript = 0x3078; // cat
	((Compact *)SkyCompact::data_3[246])->baseSub = 0x3076;
	((Compact *)SkyCompact::data_3[253])->actionScript = 0x30E8; // bio_door
	((Compact *)SkyCompact::data_3[253])->baseSub = 0x30E6;
	((Compact *)SkyCompact::data_3[257])->actionScript = 0x30D1; // sales_chart
	((Compact *)SkyCompact::data_3[258])->actionScript = 0x3053; // gallager_bel
	((Compact *)SkyCompact::data_3[258])->baseSub = 0x3051; // gallager_bel
	((Compact *)SkyCompact::data_3[267])->actionScript = 0x3055; // reich_window
	((Compact *)SkyCompact::data_3[268])->actionScript = 0x3057; // lamb_window
	((Compact *)SkyCompact::data_3[270])->baseSub = 0x3079; // inner_lamb_door
	((Compact *)SkyCompact::data_3[277])->actionScript = 0x30B9; // ticket
	((Compact *)SkyCompact::data_3[277])->baseSub = 0x30B8;
	((Compact *)SkyCompact::data_3[280])->actionScript = 0x30BC; // globe
	((Compact *)SkyCompact::data_3[280])->baseSub = 0x30BD; // globe
	((Compact *)SkyCompact::data_3[283])->baseSub = 0x3094; // inner_reich_door
	((Compact *)SkyCompact::data_3[286])->baseSub = 0x310B; // glass_slot
	((uint16 *)SkyCompact::data_3[13])[13] = 0x3138; // rs_lamb_28
	((Compact *)SkyCompact::data_3[300])->actionScript = 0x3140; // cable_29
	((Compact *)SkyCompact::data_3[313])->actionScript = 0x3018; // cable_fall
	((Compact *)SkyCompact::data_3[313])->baseSub = 0x3014;
	((Compact *)SkyCompact::data_3[316])->actionScript = 0x3018; // cable_fall2
	((Compact *)SkyCompact::data_3[316])->baseSub = 0x3013;
	((Compact *)SkyCompact::data_3[317])->baseSub = 0x3015; // smashed_window
	((Compact *)SkyCompact::data_3[319])->baseSub = 0x3016; // bits
	((Compact *)SkyCompact::data_3[321])->baseSub = 0x3016; // bits2
	((Compact *)SkyCompact::data_3[324])->baseSub = 0x3019; // spy_11
	((Compact *)SkyCompact::data_3[325])->actionScript = 0x301C; // locker_11
	((Compact *)SkyCompact::data_3[325])->baseSub = 0x301A; // locker_11
	((Compact *)SkyCompact::data_3[328])->actionScript = 0x301E; // slot_11
	((Compact *)SkyCompact::data_3[332])->actionScript = 0x3020; // soccer_1
	((Compact *)SkyCompact::data_3[333])->actionScript = 0x3020; // soccer_2
	((Compact *)SkyCompact::data_3[334])->actionScript = 0x3020; // soccer_3
	((Compact *)SkyCompact::data_3[335])->actionScript = 0x3020; // soccer_4
	((Compact *)SkyCompact::data_3[336])->actionScript = 0x3020; // soccer_5
	((Compact *)SkyCompact::data_3[338])->actionScript = 0x3025; // slat_1
	((Compact *)SkyCompact::data_3[339])->actionScript = 0x3025; // slat_2
	((Compact *)SkyCompact::data_3[340])->actionScript = 0x3025; // slat_3
	((Compact *)SkyCompact::data_3[341])->actionScript = 0x3025; // slat_4
	((Compact *)SkyCompact::data_3[342])->actionScript = 0x3025; // slat_5
	((Compact *)SkyCompact::data_3[343])->actionScript = 0x3027; // right_exit_11
	((Compact *)SkyCompact::data_3[388])->actionScript = 0x30A8; // small_exit_23
	((Compact *)SkyCompact::data_3[389])->actionScript = 0x3142; // small_r_29
	((Compact *)SkyCompact::data_3[390])->actionScript = 0x3144; // small_l_29
	((Compact *)SkyCompact::data_3[391])->actionScript = 0x311F; // small_r_28
	((Compact *)SkyCompact::data_3[392])->actionScript = 0x3121; // small_l_28
	((Compact *)SkyCompact::data_3[393])->actionScript = 0x3030; // top_right_19
	((Compact *)SkyCompact::data_3[406])->actionScript = 0x3126; // lift_28
	((Compact *)SkyCompact::data_3[406])->baseSub = 0x3124; // lift_28
	((Compact *)SkyCompact::data_3[409])->actionScript = 0x3127; // slot_28
}

}

} // End of namespace Sky
