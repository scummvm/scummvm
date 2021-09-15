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

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/script.h"
#include "chamber/resdata.h"
#include "chamber/cga.h"
#include "chamber/cursor.h"
#include "chamber/portrait.h"
#include "chamber/input.h"
#include "chamber/menu.h"
#include "chamber/room.h"
#include "chamber/dialog.h"
#include "chamber/print.h"
#include "chamber/anim.h"
#include "chamber/invent.h"
#include "chamber/sound.h"
#include "chamber/savegame.h"

namespace Chamber {

#if 1
#define DEBUG_SCRIPT
char DEBUG_SCRIPT_LOG[] = "!script.log";
#endif

#include "scrvars.h"

unsigned char rand_seed;
unsigned short the_command;
unsigned int script_res;
unsigned char *script_ptr, *script_end_ptr;
unsigned char *script_stack[5 * 2];
unsigned char **script_stack_ptr = script_stack;

void *script_vars[ScrPools_MAX] = {
	&script_word_vars,
	&script_word_vars,
	&script_byte_vars,
	inventory_items,
	zones_data,
	pers_list,
	inventory_items,
	inventory_items + 38,
	pers_list
};

unsigned char Rand(void) {
	script_byte_vars.rand_value = aleat_data[++rand_seed];
	return script_byte_vars.rand_value;
}

unsigned int RandW(void) {
	unsigned int r = Rand() << 8;
	return r | Rand();
}

unsigned int Swap16(unsigned int x) {
	return (x << 8) | (x >> 8);
}

/*Script handlers exit codes*/
enum CommandStatus {
	ScriptContinue = 0, /*run next script command normally*/
	ScriptRerun = 1, /*abort current script, execute new command*/
	/*TODO: maybe define ScriptRestartGame to support game restart?*/
};

unsigned int CMD_TRAP(void) {
	printf("CMD TRAP\n");
	PromptWait();
	for (;;) ;
	return 0;
}

unsigned int SCR_TRAP(void) {
	printf("SCR TRAP 0x%02X @ 0x%X\n", *script_ptr, script_ptr - templ_data);
	PromptWait();
	for (;;) ;
	return 0;
}


void ClaimTradedItems(void) {
	int i;
	for (i = 0; i < MAX_INV_ITEMS; i++) {
		if (inventory_items[i].flags == (ITEMFLG_80 | 1))
			inventory_items[i].flags = ITEMFLG_80;
	}
}

unsigned int SCR_1_AspirantItemTrade(void) {
	unsigned char *old_script, *old_script_end = script_end_ptr;

	item_t *item1, *item2;

	script_ptr++;
	old_script = script_ptr;

	for (;;) {
		inv_bgcolor = 0xFF;
		OpenInventory(0xFE, ITEMFLG_80);

		if (inv_count == 0) {
			the_command = 0xC1BC;
			RunCommand();
			break;
		}

		if (the_command == 0) {
			the_command = 0xC1C0;
			RunCommand();
			break;
		}

		the_command = 0x9140;

		if (pers_ptr->item == 0)
			break;

		item1 = &inventory_items[pers_ptr->item - 1];
		item2 = (item_t *)(script_vars[ScrPool3_CurrentItem]);

		if (item2->flags == (ITEMFLG_80 | 1) || item1->name == item2->name) {
			the_command = 0xC1C0;
			RunCommand();
			break;
		}

		if (item2->name == 109
		        || item2->name == 132
		        || item2->name == 108
		        || script_byte_vars.rand_value < 154) {
			item2->flags = ITEMFLG_20;
			item1->flags = ITEMFLG_80;
			pers_ptr->item = script_byte_vars.inv_item_index;
			switch (item2->name) {
			case 132:
				script_byte_vars.room_items--;
				the_command = 0xC04B;
				break;
			case 104:
				the_command = 0xC1BA;
				break;
			case 107:
				the_command = 0xC1BB;
				break;
			default:
				the_command = 0xC1BA;
			}
			RunCommand();
			break;
		} else {
			item2->flags = ITEMFLG_80 | 1;
			the_command = 0xC1BD;
			RunCommand();
			continue;
		}
	}

	ClaimTradedItems();

	script_ptr = old_script;
	script_end_ptr = old_script_end;

	return 0;
}


unsigned char wait_delta = 0;

void Wait(unsigned char seconds) {
	struct time t;
	unsigned int endtime;

	seconds += wait_delta;
	if (seconds > 127)  /*TODO: is this a check for an unsigned value?*/
		seconds = 0;

	gettime(&t);
	endtime = t.ti_sec * 100 + t.ti_hund + seconds * 100;

	while (buttons == 0) {
		unsigned int current;
		gettime(&t);
		current = t.ti_sec * 100 + t.ti_hund;
		if (endtime >= 6000 && current < 2048)  /*TODO: some kind of overflow check???*/
			current += 6000;
		if (current >= endtime)
			break;
	}
}

unsigned int SCR_2C_Wait4(void) {
	script_ptr++;
	Wait(4);
	return 0;
}

unsigned int SCR_2D_Wait(void) {
	unsigned char seconds;
	script_ptr++;
	seconds = *script_ptr++;
	Wait(4);    /*TODO: looks like a bug?*/
	return 0;
}

unsigned int SCR_2E_PromptWait(void) {
	script_ptr++;
	PromptWait();
	return 0;
}


#define VARTYPE_VAR 0x80
#define VARTYPE_BLOCK 0x40
#define VARTYPE_WORD 0x20
#define VARTYPE_KIND 0x1F

#define VARSIZE_BYTE 0
#define VARSIZE_WORD 1

unsigned char var_size;

unsigned short LoadVar(unsigned char **ptr, unsigned char **varptr) {
	unsigned char vartype;
	unsigned char *varbase;
	unsigned short value = 0;
	var_size = VARSIZE_BYTE;
	vartype = *((*ptr)++);
	if (vartype & VARTYPE_VAR) {
		/*variable*/
		unsigned char varoffs;
		varbase = (unsigned char *)script_vars[vartype & VARTYPE_KIND];
		if (vartype & VARTYPE_BLOCK) {
			unsigned char *end;
			unsigned char index = *((*ptr)++);
			varbase = SeekToEntryW(varbase, index, &end);
		}
		varoffs = *((*ptr)++);
#if 1
		{
			int maxoffs = 0;
			switch (vartype & VARTYPE_KIND) {
			case ScrPool0_WordVars0:
			case ScrPool1_WordVars1:
				maxoffs = sizeof(script_word_vars);
				break;
			case ScrPool2_ByteVars:
				maxoffs = sizeof(script_byte_vars);
				break;
			case ScrPool3_CurrentItem:
				maxoffs = sizeof(item_t);
				break;
			case ScrPool4_ZoneSpots:
				maxoffs = RES_ZONES_MAX;
				break;
			case ScrPool5_Persons:
				maxoffs = sizeof(pers_list);
				break;
			case ScrPool6_Inventory:
				maxoffs = sizeof(inventory_items);
				break;
			case ScrPool7_Inventory38:
				maxoffs = sizeof(inventory_items) - sizeof(item_t) * 38;
				break;
			case ScrPool8_CurrentPers:
				maxoffs = sizeof(pers_t);
				break;
			}
			if (varoffs >= maxoffs) {
				printf("Scr var out of bounds @ %X (pool %d, ofs 0x%X, max 0x%X)\n", (unsigned int)(script_ptr - templ_data), vartype & VARTYPE_KIND, varoffs, maxoffs);
				PromptWait();
			}
		}
#endif
		value = varbase[varoffs];
		if (vartype & VARTYPE_WORD) {
			value = (value << 8) | varbase[varoffs + 1];
			var_size = VARSIZE_WORD;
		}
		*varptr = &varbase[varoffs];

#if 0
		/*TODO: debug stuff, remove me*/
		if (varoffs == 0x48)
			printf("Var 2.%X = %X\n", varoffs, value);
#endif
	} else {
		/*immediate value*/
		value = *((*ptr)++);
		if (vartype & VARTYPE_WORD) {
			value = (value << 8) | *((*ptr)++);
			var_size = VARSIZE_WORD;
		}
		*varptr = 0;
	}
	return value;
}

#define MATHOP_END 0x80
#define MATHOP_CMP 0x40

#define MATHOP_ADD 0x20
#define MATHOP_SUB 0x10
#define MATHOP_AND 0x08
#define MATHOP_OR  0x04
#define MATHOP_XOR 0x02

#define MATHOP_EQ  0x20
#define MATHOP_B   0x10
#define MATHOP_A   0x08
#define MATHOP_NEQ 0x04
#define MATHOP_LE  0x02
#define MATHOP_GE  0x01

unsigned short MathOp(unsigned char op, unsigned short op1, unsigned short op2) {
	if (op & MATHOP_CMP) {
		if (op & MATHOP_EQ)
			if (op1 == op2) return ~0;
		if (op & MATHOP_B)
			if (op1 < op2) return ~0;
		if (op & MATHOP_A)
			if (op1 > op2) return ~0;
		if (op & MATHOP_NEQ)
			if (op1 != op2) return ~0;
		if (op & MATHOP_LE)
			if ((signed short)op1 <= (signed short)op2) return ~0;
		if (op & MATHOP_GE)
			if ((signed short)op1 >= (signed short)op2) return ~0;
		return 0;
	} else {
		if (op & MATHOP_ADD)
			op1 += op2;
		if (op & MATHOP_SUB)
			op1 -= op2;
		if (op & MATHOP_AND)
			op1 &= op2;
		if (op & MATHOP_OR)
			op1 |= op2;
		if (op & MATHOP_XOR)
			op1 ^= op2;
		return op1;
	}
}

unsigned short MathExpr(unsigned char **ptr) {
	unsigned char op;
	unsigned short op1, op2;
	unsigned char *opptr;
	op1 = LoadVar(ptr, &opptr);
	while (((op = *((*ptr)++)) & MATHOP_END) == 0) {
		op2 = LoadVar(ptr, &opptr);
		op1 = MathOp(op, op1, op2);
	}
	return op1;
}

/*
Math operations (assignment) on a variable
*/
unsigned int SCR_3B_MathExpr(void) {
	unsigned short op1, op2;
	unsigned char *opptr;

	script_ptr++;

	op1 = LoadVar(&script_ptr, &opptr);
	op2 = MathExpr(&script_ptr);
	if (var_size == VARSIZE_BYTE)
		*opptr = op2 & 255;
	else {
		opptr[0] = op2 >> 8;    /*store in big-endian*/
		opptr[1] = op2 & 255;
	}

	/*return op1;*/ /*previous value, never used?*/
	return 0;
}

/*Jump to routine*/
unsigned int SCR_12_Chain(void) {
	script_ptr++;
	the_command = *script_ptr++;          /*little-endian*/
	the_command |= (*script_ptr++) << 8;
	script_ptr = GetScriptSubroutine(the_command - 1);
	return 0;
}

/*
Absolute jump
Jumping past current routine ends the script
*/
unsigned int SCR_33_Jump(void) {
	unsigned short offs;
	script_ptr++;
	offs = *script_ptr++;          /*little-endian*/
	offs |= (*script_ptr++) << 8;
	script_ptr = templ_data + offs;
	return 0;
}

/*
Conditional jump (IF/ELSE block)
*/
unsigned int SCR_3C_CondExpr(void) {
	script_ptr++;

	if (MathExpr(&script_ptr)) {
		/*fall to IF block*/
		script_ptr += 2;
	} else {
		/*branch to ELSE block*/
		script_ptr -= 1;    /*simulate opcode byte for Jump handler*/
		return SCR_33_Jump();
	}
	return 0;
}

/*
Absolute subroutine call
*/
unsigned int SCR_34_Call(void) {
	unsigned short offs;
	script_ptr++;
	offs = *script_ptr++;          /*little-endian*/
	offs |= (*script_ptr++) << 8;
	*script_stack_ptr++ = script_ptr;
	*script_stack_ptr++ = script_end_ptr;
	script_ptr = templ_data + offs;
	return 0;
}

/*
Return from script subroutine
*/
unsigned int SCR_35_Ret(void) {
	script_end_ptr = *(--script_stack_ptr);
	script_ptr = *(--script_stack_ptr);
	return 0;
}


/*Draw portrait, pushing it from left to right*/
unsigned int SCR_5_DrawPortraitLiftRight(void) {
	unsigned char x, y, width, height;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	CGA_AnimLiftToRight(width, cur_image_pixels + width - 1, width, 1, height, CGA_SCREENBUFFER, CGA_CalcXY_p(x, y));
	return 0;
}

/*Draw portrait, pushing it from right to left*/
unsigned int SCR_6_DrawPortraitLiftLeft(void) {
	unsigned char x, y, width, height;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	CGA_AnimLiftToLeft(width, cur_image_pixels, width, 1, height, CGA_SCREENBUFFER, CGA_CalcXY_p(x + width - 1, y));
	return 0;
}

/*Draw portrait, pushing it from top to bottom*/
unsigned int SCR_7_DrawPortraitLiftDown(void) {
	unsigned char x, y, width, height;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	CGA_AnimLiftToDown(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	return 0;
}

/*Draw portrait, pushing it from bottom to top*/
unsigned int SCR_8_DrawPortraitLiftUp(void) {
	unsigned char x, y, width, height;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	CGA_AnimLiftToUp(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, x, y + height - 1);
	return 0;
}

/*Draw portrait, no special effects*/
unsigned int SCR_9_DrawPortrait(void) {
	unsigned char x, y, width, height;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	CGA_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	return 0;
}

/*Draw screen pixels using 2-phase clockwise twist*/
void TwistDraw(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *source, unsigned char *target) {
	int i;
	unsigned int sx, ex, sy, ey, t;
	sx = x * 4;
	ex = x * 4 + width * 4 - 1;
	sy = y;
	ey = y + height - 1;

	for (i = 0; i < width * 4; i++) {
		CGA_TraceLine(sx, ex, sy, ey, source, target);
		WaitVBlank();
		sx += 1;
		ex -= 1;
	}

	t = sx;
	sx = ex + 1;
	ex = t - 1;

	t = sy;
	sy = ey;
	ey = t;

	for (i = 0; i < height; i++) {
		CGA_TraceLine(sx, ex, sy, ey, source, target);
		WaitVBlank();
		sy -= 1;
		ey += 1;
	}
}

/*Draw image with twist-effect*/
unsigned int SCR_B_DrawPortraitTwistEffect(void) {
	unsigned char x, y, width, height;
	unsigned int offs;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	offs = CGA_CalcXY_p(x, y);

	CGA_SwapScreenRect(cur_image_pixels, width, height, backbuffer, offs);
	TwistDraw(x, y, width, height, backbuffer, frontbuffer);
	CGA_BlitAndWait(scratch_mem2, width, width, height, backbuffer, offs);

	return 0;
}

/*Draw screen pixels using arc-like sweep*/
void ArcDraw(unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *source, unsigned char *target) {
	int i;
	unsigned int sx, ex, sy, ey;
	sx = x * 4;
	ex = x * 4 + width * 2 - 1;
	sy = y + height - 1;
	ey = y + height - 1;

	for (i = 0; i < height; i++) {
		CGA_TraceLine(sx, ex, sy, ey, source, target);
		WaitVBlank();
		sy -= 1;
	}

	for (i = 0; i < width * 4; i++) {
		CGA_TraceLine(sx, ex, sy, ey, source, target);
		WaitVBlank();
		sx += 1;
	}

	for (i = 0; i < height + 1; i++) {
		CGA_TraceLine(sx, ex, sy, ey, source, target);
		WaitVBlank();
		sy += 1;
	}
}

/*Draw image with arc-effect*/
unsigned int SCR_C_DrawPortraitArcEffect(void) {
	unsigned char x, y, width, height;
	unsigned int offs;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	offs = CGA_CalcXY_p(x, y);

	CGA_SwapScreenRect(cur_image_pixels, width, height, backbuffer, offs);
	ArcDraw(x, y, width, height, backbuffer, frontbuffer);
	CGA_BlitAndWait(scratch_mem2, width, width, height, backbuffer, offs);

	return 0;
}

/*Draw image with slow top-to-down reveal effect by repeatedly draw its every 17th pixel*/
unsigned int SCR_D_DrawPortraitDotEffect(void) {
	int i;
	unsigned char x, y, width, height;
	unsigned int offs, step = 17;
	unsigned char *target = CGA_SCREENBUFFER;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	cur_image_end = width * height;

	for (offs = 0; offs != cur_image_end;) {
		target[CGA_CalcXY_p(x + offs % cur_image_size_w, y + offs / cur_image_size_w)] = cur_image_pixels[offs];
		for (i = 0; i < 255; i++) ; /*TODO FIXME weak delay*/
		offs += step;
		if (offs > cur_image_end)
			offs -= cur_image_end;
	}
	return 0;
}

/*Draw image with slow zoom-in reveal effect*/
unsigned int SCR_E_DrawPortraitZoomIn(void) {
	unsigned char x, y, width, height;

	script_ptr++;

	if (!DrawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO*/

	CGA_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);

	return 0;
}


/*Hide portrait, pushing it from right to left*/
unsigned int SCR_19_HidePortraitLiftLeft(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	/*TODO: This originally was done by reusing door sliding routine*/

	/*offs = CGA_CalcXY_p(x + 1, y);*/
	offs++;

	while (--width) {
		CGA_HideScreenBlockLiftToLeft(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	offs--;

	/*hide leftmost line*/
	/*TODO: move this to CGA?*/
	while (height--) {
		memcpy(frontbuffer + offs, backbuffer + offs, 1);

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}

	return 0;
}

/*Hide portrait, pushing it from left to right*/
unsigned int SCR_1A_HidePortraitLiftRight(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	/*TODO: This originally was done by reusing door sliding routine*/

	offs = CGA_CalcXY_p(x + width - 2, y);

	while (--width) {
		CGA_HideScreenBlockLiftToRight(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	offs++;

	/*hide leftmost line*/
	/*TODO: move this to CGA?*/
	while (height--) {
		memcpy(frontbuffer + offs, backbuffer + offs, 1);

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}

	return 0;
}

/*Hide portrait, pushing it from bottom to top*/
unsigned int SCR_1B_HidePortraitLiftUp(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	offs = CGA_CalcXY_p(x, y + 1);

	while (--height) {
		CGA_HideScreenBlockLiftToUp(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	/*hide topmost line*/
	/*TODO: move this to CGA?*/
	offs ^= CGA_ODD_LINES_OFS;
	if ((offs & CGA_ODD_LINES_OFS) != 0)
		offs -= CGA_BYTES_PER_LINE;
	memcpy(CGA_SCREENBUFFER + offs, backbuffer + offs, width);
	return 0;
}


/*Hide portrait, pushing it from top to bottom*/
unsigned int SCR_1C_HidePortraitLiftDown(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	offs = CGA_CalcXY_p(x, y + height - 2);

	while (--height) {
		CGA_HideScreenBlockLiftToDown(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	/*hide bottommost line*/
	/*TODO: move this to CGA?*/
	offs ^= CGA_ODD_LINES_OFS;
	if ((offs & CGA_ODD_LINES_OFS) == 0)
		offs += CGA_BYTES_PER_LINE;
	memcpy(CGA_SCREENBUFFER + offs, backbuffer + offs, width);
	return 0;
}


/*Hide portrait with twist effect*/
unsigned int SCR_1E_HidePortraitTwist(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	TwistDraw(x, y, width, height, backbuffer, frontbuffer);

	return 0;
}

/*Hide portrait with arc effect*/
unsigned int SCR_1F_HidePortraitArc(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	ArcDraw(x, y, width, height, backbuffer, frontbuffer);

	return 0;
}

/*Hide portrait with dots effect*/
unsigned int SCR_20_HidePortraitDots(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	dot_effect_step = 17;
	dot_effect_delay = 100;
	CopyScreenBlockWithDotEffect(backbuffer, x, y, width, height, frontbuffer);

	return 0;
}


unsigned int SCR_39_AnimRoomDoorOpen(void) {
	unsigned char door;

	script_ptr++;
	door = *script_ptr++;
	AnimRoomDoorOpen(door);
	return 0;
}

unsigned int SCR_3A_AnimRoomDoorClose(void) {
	unsigned char door;

	script_ptr++;
	door = *script_ptr++;
	AnimRoomDoorClose(door);
	return 0;
}

unsigned int SCR_25_ChangeZoneOnly(void) {
	unsigned char index;
	unsigned char old = script_byte_vars.zone_room;

	script_ptr++;
	index = *script_ptr++;

	ChangeZone(index);
	script_byte_vars.zone_room = old;
	return 0;
}


void JaggedZoom(void) {
	/*TODO*/
}

void InitStarfield(void) {
	/*TODO*/
}

void AnimStarfield(void) {
	/*TODO*/
}

unsigned int SCR_26_GameOver(void) {
	in_de_profundis = 0;
	script_byte_vars.game_paused = 1;
	memset(backbuffer, 0, sizeof(backbuffer) - 2);  /*TODO: original bug?*/
	JaggedZoom();
	CGA_BackBufferToRealFull();
	CGA_ColorSelect(0x30);
	InitStarfield();
	AnimStarfield();
	PlayAnim(44, 156 / 4, 95);
	script_byte_vars.zone_index = 135;
	JaggedZoom();
	CGA_BackBufferToRealFull();
	/*RestartGame();*/
	/*TODO: this should shomehow abort all current scripts/calls and restart the game*/
	TODO("Restart");

	return 0;
}


unsigned int SCR_4C_DrawZoneObjs(void) {
	script_ptr++;
	DrawZoneObjs();
	return 0;
}


unsigned int SCR_13_RedrawRoomStatics(void) {
	unsigned char index;
	script_ptr++;
	index = *script_ptr++;
	RedrawRoomStatics(index, 0);
	return 0;
}

/*
Load and draw zone (to backbuffer)
*/
unsigned int SCR_42_LoadZone(void) {
	unsigned char index;

	script_ptr++;
	index = *script_ptr++;

	zone_drawn = 0;
	if (right_button)
		script_byte_vars.byte_179B8 = 0;
	else {
		if ((script_byte_vars.cur_spot_flags & (SPOTFLG_20 | SPOTFLG_10 | SPOTFLG_8)) == 0)
			script_byte_vars.byte_179B8 = script_byte_vars.cur_spot_flags & 7;
		else if ((script_byte_vars.cur_spot_flags & ((SPOTFLG_20 | SPOTFLG_10 | SPOTFLG_8))) == SPOTFLG_8) {
			zone_drawn = 1;
			AnimRoomDoorOpen(script_byte_vars.cur_spot_idx);
			script_byte_vars.byte_179B8 = script_byte_vars.cur_spot_flags & 7;
		} else
			script_byte_vars.byte_179B8 = 0;
	}
	UpdateZoneSpot(index);
	ChangeZone(index);
	script_byte_vars.zone_area_copy = script_byte_vars.zone_area;
	script_byte_vars.cur_spot_idx = FindInitialSpot();
	zone_drawn |= script_byte_vars.cur_spot_idx;

	DrawRoomStatics();

	if (script_byte_vars.byte_17A15 != 0) {
		RedrawRoomStatics(script_byte_vars.byte_17A15, 0);
		script_byte_vars.byte_17A15 = 0;
	}

	BackupSpotsImages();
	PrepareCommand3();
	PrepareCommand4();
	PrepareCommand1();
	DrawZoneObjs();
	script_byte_vars.cur_spot_flags = 0;
	return 0;
}

unsigned int SCR_59_BlitSpritesToBackBuffer(void) {
	script_ptr++;
	BlitSpritesToBackBuffer();
	return 0;
}

unsigned int SCR_5A_SelectPalette(void) {
	script_ptr++;
	SelectPalette();
	return 0;
}

unsigned int SCR_43_RefreshZone(void) {
	script_ptr++;
	RefreshZone();
	return 0;
}


unsigned int SCR_36_ChangeZone(void) {
	SCR_42_LoadZone();
	RefreshZone();
	return 0;
}

void SCR_DrawRoomObjectBack(unsigned char *x, unsigned char *y, unsigned char *w, unsigned char *h) {
	unsigned char obj[3];

	script_ptr++;
	obj[0] = *script_ptr++; /*spr*/
	obj[1] = *script_ptr++; /*x*/
	obj[2] = *script_ptr++; /*y*/

	DrawRoomStaticObject(obj, x, y, w, h);
}

unsigned int SCR_5F_DrawRoomObjectBack(void) {
	unsigned char x, y, w, h;
	SCR_DrawRoomObjectBack(&x, &y, &w, &h);
	return 0;
}

unsigned int SCR_11_DrawRoomObject(void) {
	unsigned char x, y, w, h;
	SCR_DrawRoomObjectBack(&x, &y, &w, &h);
	CGA_CopyScreenBlock(backbuffer, w, h, CGA_SCREENBUFFER, CGA_CalcXY_p(x, y));
	return 0;
}

/*
Draw box with item sprite and its name
*/
unsigned int SCR_3_DrawItemBox(void) {
	unsigned char current;

	item_t *item;
	unsigned char x, y;
	unsigned char *msg;

	script_ptr++;
	current = *script_ptr++;

	if (current)
		item = (item_t *)script_vars[ScrPool3_CurrentItem];
	else
		item = &inventory_items[pers_ptr->item - 1];

	x = dirty_rects[0].x;
	y = dirty_rects[0].y + 70;
	msg = SeekToString(desci_data, 274 + item->name);

	DesciTextBox(x, y, 18, msg);
	DrawSpriteN(item->sprite, x, y + 1, CGA_SCREENBUFFER);

	return 0;
}

/*Draw simple bubble with text*/
unsigned int SCR_37_DesciTextBox(void) {
	unsigned char x, y, w;
	unsigned char *msg;
	script_ptr++;
	msg = SeekToStringScr(desci_data, *script_ptr, &script_ptr);
	script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;
	w = *script_ptr++;
	DesciTextBox(x, y, w, msg);
	return 0;
}


/*Play portrait animation*/
unsigned int SCR_18_AnimPortrait(void) {
	unsigned char layer, index, delay;
	script_ptr++;

	layer = *script_ptr++;
	index = *script_ptr++;
	delay = *script_ptr++;

	AnimPortrait(layer, index, delay);

	return 0;
}

/*Play animation*/
unsigned int SCR_38_PlayAnim(void) {
	unsigned char index, x, y;
	script_ptr++;
	index = *script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;
	PlayAnim(index, x, y);
	return 0;
}

/*Pop up the actions menu and handle its commands*/
unsigned int SCR_3D_ActionsMenu(void) {
	unsigned short cmd;

	unsigned char *old_script = script_ptr;
	unsigned char *old_script_end = script_end_ptr;

	act_menu_x = 0xFF;

	for (;;) {
		script_ptr++;
		ActionsMenu(&script_ptr);
		if (the_command == 0xFFFF)
			break;

		cmd = the_command & 0xF000;
		if (cmd == 0xC000 || cmd == 0xA000) {
			return ScriptRerun;
		}

		RunCommand();

		script_byte_vars.used_commands++;

		if (script_byte_vars.byte_179F9 == 0 && script_byte_vars.check_used_commands < script_byte_vars.used_commands) {
			the_command = Swap16(script_word_vars.next_command1);
			if (the_command)
				return ScriptRerun;
		}

		script_ptr = old_script;
		if (--script_byte_vars.tries_left == 0)
			ResetAllPersons();
	}

	script_end_ptr = old_script_end;
	return ScriptContinue;
}

/*The Wall room puzzle*/
unsigned int SCR_3E_TheWallAdvance(void) {
	script_ptr++;

	script_byte_vars.the_wall_phase = (script_byte_vars.the_wall_phase + 1) % 4;
	switch (script_byte_vars.the_wall_phase) {
	default:
		TheWallPhase3_DoorOpen1();
		break;
	case 0:
		TheWallPhase0_DoorOpen2();
		break;
	case 1:
		TheWallPhase1_DoorClose1();
		break;
	case 2:
		TheWallPhase2_DoorClose2();
		break;
	}

	return 0;
}

/*
When playing cups with proto
*/
unsigned int SCR_28_MenuLoop(void) {
	unsigned char cursor;
	unsigned char mask, value;

	script_ptr++;
	cursor = *script_ptr++;
	mask = *script_ptr++;
	value = *script_ptr++;

	SelectCursor(cursor);

	MenuLoop(mask, value);

	return 0;
}


/*
Restore screen data from back buffer as specified by dirty rects of specified index
*/
unsigned int SCR_2A_PopDialogRect(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs); /*TODO: implicit target*/
	CGA_CopyScreenBlock(backbuffer, 2, 21, CGA_SCREENBUFFER, offs = (x << 8) | y);  /*TODO: implicit target*/

	cur_dlg_index = 0;

	return 0;
}

/*
Restore screen data from back buffer as specified by dirty rect of kind dialog bubble
*/
unsigned int SCR_2B_PopAllBubbles(void) {
	script_ptr++;
	PopDirtyRects(DirtyRectBubble);
	return 0;
}

/*
Hide a portrait, with shatter effect
*/
unsigned int SCR_22_HidePortraitShatter(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	CGA_HideShatterFall(CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);

	return 0;
}

/*
Hide a portrait, no special effects
*/
unsigned int SCR_23_HidePortrait(void) {
	unsigned char index;
	unsigned char kind;
	unsigned char x, y;
	unsigned char width, height;
	unsigned int offs;

	script_ptr++;
	index = *script_ptr++;

	GetDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);

	return 0;
}

/*
Restore screen data from back buffer for all portraits
*/
unsigned int SCR_24_PopAllPortraits(void) {
	script_ptr++;
	PopDirtyRects(DirtyRectSprite);
	return 0;
}

/*
Restore screen data from back buffer for all text bubbles
*/
unsigned int SCR_40_PopAllTextBoxes() {
	script_ptr++;
	PopDirtyRects(DirtyRectText);
	return 0;
}

/*
Move a Hand in Who Will Be Saved
*/
unsigned int SCR_41_LiftHand(void) {
	script_ptr++;
	RedrawRoomStatics(92, script_byte_vars.byte_179E1);
	CGA_BackBufferToRealFull();
	PlaySound(31);
	return 0;
}

unsigned char fight_mode = 0;

unsigned int SCR_30_Fight(void) {
	static unsigned char player_image[] = {26, 0, 0};
	unsigned char *image = player_image;

	unsigned char x, y, width, height, kind;
	unsigned int offs;
	unsigned char *old_script, *old_script_end = script_end_ptr;
	pers_t *pers = (pers_t *)(script_vars[ScrPool8_CurrentPers]);

	unsigned char strenght, win, rnd;

	script_ptr++;
	old_script = script_ptr;

	x = 140 / 4;
	y = 20;

	fight_mode = 1;

	if (pers->name != 44) {
		if (next_command3 == 0xA015) {
			the_command = 0xA015;
			RunCommand();
			FindAndSelectSpot((pers - pers_list) * 5);  /*TODO: FindAndSelectSpot assumes plain offset, 5-byte records*/
		}
		if (Swap16(script_word_vars.next_command1) == 0xC357) {
			the_command = 0xC357;
			RunCommand();
		}

		pers = (pers_t *)(script_vars[ScrPool8_CurrentPers]);
		if (pers->name != 56 && pers->name != 51) {
			x = dirty_rects[0].x + 64 / 4;
			y = dirty_rects[0].y;
			fight_mode = 0;
		}
	}

	/*draw player portrait*/
	player_image[1] = x;
	player_image[2] = y;
	if (DrawPortrait(&image, &x, &y, &width, &height))
		CGA_AnimLiftToLeft(width, cur_image_pixels, width, 1, height, CGA_SCREENBUFFER, CGA_CalcXY_p(x + width - 1, y));

	BlinkToWhite();

	if (pers->name != 44 && pers->name != 56 && pers->name != 51) {
		GetDirtyRectAndFree(1, &kind, &x, &y, &width, &height, &offs);
		CGA_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	/*check fight outcome*/

	strenght = 0;

	script_byte_vars.byte_179F2 = 0;

	if (script_byte_vars.byte_179F3 == 0) {
		static unsigned char character_strenght[] = {1, 3, 1, 1, 1, 1, 5, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1};

		strenght = character_strenght[pers->name - 42];

		/*check if can decrease*/
		if (strenght != 1 && (pers->flags & PERSFLG_80))
			strenght--;

		if (script_byte_vars.room_items != 0 || script_byte_vars.byte_17A1C != 0)
			strenght--;
	}

	/*check if can increase*/
	if (strenght != 5) {
		if ((pers->item >= 19 && pers->item < 23)
		        || (pers->item >= 39 && pers->item < 52)
		        || pers->item == 56 || pers->item == 57
		        || ((pers->index >> 3) == 6))
			strenght++;
	}

	win = 1;
	rnd = script_byte_vars.rand_value;

#ifdef CHEAT
	strenght = 1;
#endif

	if (strenght >= 2) {
		if (strenght == 2) {
			if (rnd >= 205)
				win = Rand() < 128 ? 81 : 82;
		} else if (strenght == 4 && rnd < 100) {
			win = Rand() < 128 ? 81 : 82;
		} else {
			win = 2;
			if (strenght == 3) {
				if (rnd < 128)  /*TODO: check me, maybe original bug (checks against wrong reg?)*/
					win = Rand() < 51 ? 145 : 146;
				else
					win = Rand() < 205 ? 49 : 50;
			}
		}
	}

	script_byte_vars.byte_179F2 = win;

	script_ptr = old_script;
	script_end_ptr = old_script_end;
	return 0;
}

unsigned char prev_fight_mode = 0;
unsigned short fight_pers_ofs = 0;

typedef struct fightentry_t {
	unsigned char   room;
	animdesc_t      anim;
} fightentry_t;

fightentry_t fightlist1[] = {
	{50, {47, 36, 153}},
	{51, {47, 36, 153}},
	{53, {47, 37, 160}},
	{54, {47, 36, 153}},
	{56, {47, 31, 128}},
	{57, {47, 27, 161}},
	{58, {47, 28, 152}},
	{59, {47, 25, 153}},
	{60, {47, 22, 155}},
	{61, {47, 27, 160}}
};

fightentry_t fightlist2[] = {
	{ 1, {24, 42, 128}},
	{ 2, {24, 44, 126}},
	{ 3, {24, 47, 126}},
	{ 4, {24, 44, 126}},
	{ 5, {24, 47, 126}},
	{ 6, {24, 28, 126}},
	{ 7, {24, 55, 126}},
	{ 8, {24, 49, 126}},
	{10, {24, 41, 147}},
	{11, {24, 41, 147}},
	{18, {24, 41, 147}},
	{19, {24, 41, 147}},
	{90, {24, 44, 121}},
	{91, {28, 24, 123}},
	{12, {24, 41, 147}},
	{13, {24, 41, 147}},
	{35, {24, 39, 147}},
	{42, {24, 39, 147}},
	{50, {55, 46, 130}},
	{52, {24, 42, 121}},
	{54, {55, 46, 130}},
	{61, {67, 37, 125}},
	{62, {55, 32, 133}},
	{63, {55, 32, 133}},
	{64, {55, 32, 133}},
	{65, {55, 32, 133}}
};

fightentry_t fightlist3[] = {
	{ 2, {25, 35, 144}},
	{ 3, {25, 38, 144}},
	{ 4, {25, 35, 144}},
	{ 5, {25, 38, 144}},
	{ 6, {25, 19, 144}},
	{ 7, {25, 46, 144}},
	{ 8, {26, 64, 132}},
	{10, {25, 32, 165}},
	{11, {25, 32, 165}},
	{12, {25, 32, 165}},
	{13, {25, 32, 165}},
	{18, {25, 32, 165}},
	{19, {25, 32, 165}},
	{90, {36, 27, 127}},
	{91, {27, 44, 123}},
	{35, {25, 30, 165}},
	{42, {25, 30, 165}},
	{50, {56, 36, 153}},
	{54, {56, 36, 153}},
	{62, {56, 22, 156}},
	{63, {56, 22, 156}},
	{64, {56, 22, 156}},
	{65, {56, 22, 156}}
};

/*Draw defeated enemy*/
unsigned int SCR_31_Fight2(void) {
	script_ptr++;

	if (script_byte_vars.byte_179F9 != 18) {
		pers_t *pers = (pers_t *)(script_vars[ScrPool8_CurrentPers]);
		fight_pers_ofs = (unsigned char *)pers - (unsigned char *)pers_list; /*TODO check size*/
		pers->flags |= PERSFLG_40;
		pers->area = 0;
		found_spot->flags &= ~SPOTFLG_80;
		if (pers->index == 16) {
			pers_list[34].area = script_byte_vars.zone_area;
			pers_list[34].flags = pers->flags;
			if (script_byte_vars.room_items == 0) {
				static const animdesc_t anim19 = {ANIMFLG_USESPOT | 19};
				AnimateSpot(&anim19);
			}
			the_command = next_command3;
			RunCommand();
		} else if (pers->index == 8) {
			pers_list[35].area = script_byte_vars.zone_area;
			pers_list[35].flags = pers->flags;
			if (script_byte_vars.room_items == 0) {
				static const animdesc_t anim20 = {ANIMFLG_USESPOT | 20};
				AnimateSpot(&anim20);
			}
			the_command = next_command3;
			RunCommand();
		} else {
			if (prev_fight_mode == 0
			        && script_byte_vars.room_items != 0
			        && fight_mode == 0) {
				script_byte_vars.byte_179F2 &= ~1;
			} else {
				unsigned int i;
				fightentry_t *fightlist;
				unsigned int fightlistsize;
				unsigned char animidx;

				prev_fight_mode = 0;
				switch (pers->name) {
				case 56:
					animidx = 47;
					fightlist = fightlist1;
					fightlistsize = 10;
					break;
				case 51:
					next_command4 = 0;
					animidx = 66;
					fightlist = fightlist1;
					fightlistsize = 10;
					break;
				default:
					animidx = 0;
					fightlist = fightlist3;
					fightlistsize = 26;
				}

				for (i = 0; i < fightlistsize; i++) {
					if (fightlist[i].room == script_byte_vars.zone_room) {
						if (animidx != 0)
							fightlist[i].anim.index = animidx;
						if (fightlist[i].anim.index == 55)
							PlaySound(151);
						PlayAnim(fightlist[i].anim.index, fightlist[i].anim.params.coords.x, fightlist[i].anim.params.coords.y);
						break;
					}
				}
			}
		}
	}

	return 0;
}

unsigned int SCR_32_FightWin(void) {

	script_ptr++;

	script_byte_vars.byte_17A1D = 0;

	if (script_byte_vars.byte_179F9 != 18 && *spot_sprite != 0) {
		CGA_RestoreImage(*spot_sprite, frontbuffer);
		CGA_RestoreImage(*spot_sprite, backbuffer);

		if (script_byte_vars.byte_179F3 == 0
		        && script_byte_vars.byte_17A16 == 0
		        && script_byte_vars.room_items != 0
		        && fight_mode == 0) {
			script_byte_vars.byte_17A1D = 1;
			PlaySound(149);
			PlayAnim(40, found_spot->sx, found_spot->sy);
		}
	}

	prev_fight_mode = script_byte_vars.byte_179F3;
	script_byte_vars.byte_179F3 = 0;

	return 0;
}

extern void ExitGame(void);

unsigned int SCR_15_SelectSpot(void) {
	unsigned char mask, index;

	script_ptr++;
	mask = *script_ptr++;
	index = *script_ptr++;
	if (mask != 0) {
		index = FindSpotByFlags(mask, index);   /*TODO: return 0 if not found?*/
		if (index == 0xFF) {
			TODO("ERROR: SelectSpot: spot not found");
			ExitGame(); /*hard abort*/
		}
	}
	found_spot = &zone_spots[index - 1];
	script_byte_vars.cur_spot_idx = index;
	spot_sprite = &sprites_list[index - 1];

	FindPerson();

	if (script_byte_vars.cur_pers == 0)
		script_vars[ScrPool8_CurrentPers] = &pers_list[20];

	return 0;
}

unsigned int SCR_44_BackBufferToScreen(void) {
	script_ptr++;
	CGA_BackBufferToRealFull();
	return 0;
}

/*
Animate De Profundis room on entry
*/
unsigned int SCR_45_DeProfundisRoomEntry(void) {
	unsigned int w, h;
	unsigned int sprofs, ofs;

	script_ptr++;

	/*draw Platform*/
	sprofs = GetPuzzlSprite(3, 140 / 4, 174, &w, &h, &ofs);
	CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);

	/*draw Granite Monster*/
	sprofs = GetPuzzlSprite(119, 128 / 4, 94, &w, &h, &ofs);
	CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);

	PromptWait();

	for (; h; h--) {
		WaitVBlank();
		WaitVBlank();
		CGA_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	CGA_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

	return 0;
}

/*
Animate De Profundis hook (lower)
*/
unsigned int SCR_46_DeProfundisLowerHook(void) {
	unsigned char y;
	unsigned int w, h;
	unsigned int sprofs, ofs;

	script_ptr++;

	/*draw Hook*/
	sprofs = GetPuzzlSprite(96, 140 / 4, 18, &w, &h, &ofs);

	h = 1;
	y = 15;
	sprofs = y * 20 / 4 * 2;    /*TODO: 20 is the sprite width. replace with w?*/

	for (; y; y--) {
		WaitVBlank();
		CGA_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

		h++;
		sprofs -= 20 / 4 * 2;

		CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	return 0;
}

/*
Animate De Profundis monster (rise)
*/
unsigned int SCR_47_DeProfundisRiseMonster(void) {
	unsigned char y;
	unsigned int w, h;
	unsigned int sprofs, ofs;

	script_ptr++;

	/*draw Granite Monster head*/
	sprofs = GetPuzzlSprite(118, 112 / 4, 174, &w, &h, &ofs);

	h = 1;
	y = 68;

	for (; y; y--) {
		WaitVBlank();

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) != 0)
			ofs -= CGA_BYTES_PER_LINE;

		h++;

		CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	return 0;
}

/*
Animate De Profundis monster (lower)
*/
unsigned int SCR_48_DeProfundisLowerMonster(void) {
	unsigned char y;
	unsigned int w, h;
	unsigned int sprofs, ofs;

	script_ptr++;

	/*draw Hook*/
	sprofs = GetPuzzlSprite(118, 112 / 4, 106, &w, &h, &ofs);

	y = 34;

	for (; y; y--) {
		WaitVBlank();
		CGA_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		h--;
		CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	return 0;
}

/*
Animate De Profundis hook (rise)
*/
unsigned int SCR_49_DeProfundisRiseHook(void) {
	unsigned char y;
	unsigned int w, h;
	unsigned int sprofs, ofs;

	script_ptr++;

	/*draw Hook*/
	sprofs = GetPuzzlSprite(96, 140 / 4, 18, &w, &h, &ofs);

	h = 16;
	y = 15;

	for (; y; y--) {
		WaitVBlank();
		CGA_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

		h--;
		sprofs += 20 / 4 * 2;

		CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	CGA_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

	return 0;
}


/*
Animate De Profundis platform
*/
unsigned int SCR_65_DeProfundisMovePlatform(void) {
	unsigned char state;
	unsigned char x, y;
	unsigned int w, h;
	unsigned int sprofs, ofs;

	script_ptr++;
	state = *script_ptr++;

	x = 140 / 4;
	y = 174;
	if (state != 0)
		y += 4;

	/*draw Platform*/
	sprofs = GetPuzzlSprite(3, x, y, &w, &h, &ofs);

	y = 4;
	if (state) {
		h -= 4;
		y--;
	}

	for (; y; y--) {
		WaitVBlank();
		CGA_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		h--;

		CGA_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	if (state)
		CGA_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

	return 0;
}

/*
Animate De Profundis monster ride to exit door
*/
unsigned int SCR_66_DeProfundisRideToExit(void) {
	unsigned int w, h;
	unsigned int sprofs, ofs;

	script_ptr++;

	/*draw Granite Monster*/
	sprofs = GetPuzzlSprite(119, 128 / 4, 139, &w, &h, &ofs);

	CGA_BlitScratchBackSprite(sprofs, w, 20, backbuffer, ofs);

	dot_effect_delay = 1;
	dot_effect_step = 17;
	CopyScreenBlockWithDotEffect(backbuffer, 112 / 4, 139, 72 / 4, 40, frontbuffer);

	return 0;
}

/*
Draw item bounce to room objects animation
*/
unsigned int SCR_4E_BounceCurrentItemToRoom(void) {
	script_ptr++;
	BounceCurrentItem(ITEMFLG_40, 43);
	return 0;
}

/*
Draw item bounce to inventory animation
*/
unsigned int SCR_4F_BounceCurrentItemToInventory(void) {
	script_ptr++;
	BounceCurrentItem(ITEMFLG_80, 85);
	return 0;
}

/*
Draw item bounce to inventory animation
*/
unsigned int SCR_50_BounceItemToInventory(void) {
	unsigned char itemidx;

	script_ptr++;
	itemidx = *script_ptr++;
	script_vars[ScrPool3_CurrentItem] = &inventory_items[itemidx - 1];

	BounceCurrentItem(ITEMFLG_80, 85);
	return 0;
}

unsigned int SCR_51_ItemTrade(void) {
	unsigned char *old_script, *old_script_end = script_end_ptr;
	unsigned char status;

	if (script_byte_vars.byte_179DC >= 63)  /*TODO: hang?*/
		return 0;

	script_ptr++;
	old_script = script_ptr;

	inv_bgcolor = 0xFF;
	OpenInventory(0xFF, ITEMFLG_80);

	status = 1;
	if (inv_count != 0) {
		status = 2;
		if (the_command != 0) {
			status = 3;
			if (script_byte_vars.inv_item_index >= 6 && script_byte_vars.inv_item_index < 27) {
				the_command = 0xC204;
				RunCommand();

				((item_t *)(script_vars[ScrPool3_CurrentItem]))->flags = 0;

				OpenInventory(0xFF, ITEMFLG_10);

				status = 4;
				if (the_command != 0) {
					status = 5;
					if (script_byte_vars.rand_value < 128) {
						status = 6;
						((item_t *)(script_vars[ScrPool3_CurrentItem]))[-1].flags = ITEMFLG_10; /*TODO: what's up with this index?*/
						((item_t *)(script_vars[ScrPool3_CurrentItem]))->flags = 0;
					}
				}
			}
		}
	}

	script_byte_vars.trade_status = status;

	script_ptr = old_script;
	script_end_ptr = old_script_end;

	return 0;
}

unsigned int SCR_52_RefreshSpritesData(void) {
	script_ptr++;
	RefreshSpritesData();
	return 0;
}

unsigned int SCR_53_FindInvItem(void) {
	unsigned char first, count, flags, i;
	item_t *item;
	script_ptr++;
	first = *script_ptr++;
	count = *script_ptr++;
	flags = *script_ptr++;
	item = &inventory_items[first - 1];
	for (i = 0; i < count; i++) {
		if (item[i].flags == flags) {
			script_vars[ScrPool3_CurrentItem] = &item[i];
			return 0;
		}
	}
	script_vars[ScrPool3_CurrentItem] = &item[count - 1];
	return 0;
}


unsigned int SCR_64_DrawBoxAroundSpot(void) {
	script_ptr++;
	DrawBoxAroundSpot();
	return 0;
}

/*
Draw text box
*/
unsigned int SCR_14_DrawDesc(void) {
	unsigned char *msg;
	script_ptr++;
	msg = SeekToStringScr(desci_data, *script_ptr, &script_ptr);
	script_ptr++;

	DrawMessage(msg, CGA_SCREENBUFFER);

	return 0;
}


/*
Draw dialog bubble with text for a person, wait for a key, then hide. Auto find bubble location
Use "thought" spike
*/
unsigned int SCR_17_DrawPersonThoughtBubbleDialog(void) {
	unsigned char x, y;
	unsigned char *msg;
	script_ptr++;
	msg = SeekToStringScr(diali_data, *script_ptr, &script_ptr);
	script_ptr++;

	x = found_spot->sx;
	y = found_spot->sy;

	if (x < 140 / 4)
		DrawPersonBubble(found_spot->ex, y - 40, SPIKE_BUBLEFT | 20, msg);
	else
		DrawPersonBubble(x - 80 / 4, y - 40, SPIKE_BUBRIGHT | 20, msg);

	PromptWait();
	PopDirtyRects(DirtyRectBubble);
	return 0;
}

/*
Draw dialog bubble with text for a person, wait for a key, then hide. Auto find bubble location
Use normal spike
*/
unsigned int SCR_61_DrawPersonBubbleDialog(void) {
	unsigned char x, y;
	unsigned char *msg;
	script_ptr++;
	msg = SeekToStringScr(diali_data, *script_ptr, &script_ptr);
	script_ptr++;

	x = found_spot->sx;
	y = found_spot->sy;

	if (x < 140 / 4)
		DrawPersonBubble(found_spot->ex, y - 40, SPIKE_DNLEFT | 20, msg);
	else
		DrawPersonBubble(x - 80 / 4, y - 40, SPIKE_DNRIGHT | 20, msg);

	PromptWait();
	PopDirtyRects(DirtyRectBubble);
	return 0;
}

#if 1
unsigned char *DebugString(char *msg, ...) {
	int i;
	unsigned char c;
	static unsigned char m[256];
	va_list ap;

	va_start(ap, msg);
	vsprintf((char *)m, msg, ap);
	va_end(ap);

	for (i = 0; m[i]; i++) {
		c = m[i];
		if (c >= 'A' && c <= 'Z')
			c = 0x21 + (c - 'A');
		else if (c >= 'a' && c <= 'z')
			c = 0x21 + (c - 'a');
		else if (c >= '0' && c <= '9')
			c = 0x10 + (c - '0');
		else if (c == ' ')
			c = 0x20;
		else if (c == '!')
			c = 0x01;
		else if (c == ',')
			c = 0x0C;
		else if (c == '.')
			c = 0x0E;
		else if (c == '\n')
			c = 0x00;
		else
			c = 0x1F;
		m[i] = c;
	}

	cur_str_end = m + i;

	return m;
}
#endif

/*
Draw dialog bubble with text for gauss
*/
unsigned int SCR_27_DrawGaussBubble(void) {
	unsigned char *msg;

	script_ptr++;
	msg = SeekToStringScr(diali_data, *script_ptr, &script_ptr);
	script_ptr++;

	DrawPersonBubble(32 / 4, 20, 15, msg);
	return 0;
}

/*
Draw dialog bubble with text
*/
unsigned int SCR_29_DialiTextBox(void) {
	unsigned char x, y, f;
	unsigned char *msg;
	script_ptr++;
	msg = SeekToStringScr(diali_data, *script_ptr, &script_ptr);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;
	f = *script_ptr++;

	DrawPersonBubble(x, y, f, msg);
	return 0;
}

/*
Do nothing in PC/CGA version
*/
unsigned int SCR_67_Unused(void) {
	script_ptr++;
	script_ptr++;
	return 0;
}

/*
Do nothing in PC/CGA version
*/
unsigned int SCR_68_Unused(void) {
	script_ptr++;
	script_ptr++;
	return 0;
}

/*
Play sound
*/
unsigned int SCR_69_PlaySound(void) {
	unsigned char index;
	script_ptr++;
	index = *script_ptr++;
	script_ptr++;

	PlaySound(index);
	return 0;
}

/*
Do nothing in PC/CGA version
*/
unsigned int SCR_6A_Unused(void) {
	script_ptr++;
	return 0;
}

/*
Open room's items inventory
*/
unsigned int CMD_1_RoomObjects(void) {
	UpdateUndrawCursor(CGA_SCREENBUFFER);
	inv_bgcolor = 0xAA;
	OpenInventory((0xFF << 8) | ITEMFLG_40, (script_byte_vars.zone_area << 8) | ITEMFLG_40);
	return ScriptRerun;
}

/*
Open Psi Powers menu
*/
unsigned int CMD_2_PsiPowers(void) {
	/*Psi powers bar*/
	BackupAndShowSprite(3, 280 / 4, 40);
	ProcessInput();
	do {
		PollInput();
		SelectCursor(CURSOR_FINGER);
		CheckPsiCommandHover();
		if (command_hint != 100)
			command_hint += 109;
		if (command_hint != last_command_hint)
			DrawCommandHint();
		DrawHintsAndCursor(CGA_SCREENBUFFER);
	} while (buttons == 0);
	UndrawCursor(CGA_SCREENBUFFER);
	CGA_RestoreBackupImage(CGA_SCREENBUFFER);
	return ScriptRerun;
}

/*
Open normal inventory box
*/
unsigned int CMD_3_Posessions(void) {
	UpdateUndrawCursor(CGA_SCREENBUFFER);
	inv_bgcolor = 0x55;
	OpenInventory(ITEMFLG_80, ITEMFLG_80);
	return ScriptRerun;
}

/*
Show energy level
*/
unsigned int CMD_4_EnergyLevel(void) {
	static unsigned char energy_image[] = {130, 236 / 4, 71};
	unsigned char x, y, width, height;
	unsigned char *image = energy_image;
	unsigned char anim = 40;

	PopDirtyRects(DirtyRectSprite);
	PopDirtyRects(DirtyRectBubble);

	if (script_byte_vars.psy_energy != 0)
		anim = 41 + (script_byte_vars.psy_energy / 16);

	if (DrawPortrait(&image, &x, &y, &width, &height)) {
		CGA_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	}

	do {
		AnimPortrait(1, anim, 10);
		AnimPortrait(1, anim + 14, 10);
		PollInput();
	} while (buttons == 0);

	PopDirtyRects(DirtyRectSprite);

	return 0;
}

/*
Advance time
*/
unsigned int CMD_5_Wait(void) {

	script_byte_vars.byte_179DB++;
	script_word_vars.timer_ticks2 = Swap16(Swap16(script_word_vars.timer_ticks2) + 300);

	the_command = next_command3;
	RunCommand();

	the_command = next_command4;
	RunCommand();

	script_byte_vars.used_commands = script_byte_vars.check_used_commands;

	the_command = Swap16(script_word_vars.word_17852);

	if (the_command == 0) {
		if (script_word_vars.next_command1 == 0) {
			the_command = 0x9005;
			RunCommand();
		}
	} else {
		if (script_byte_vars.byte_179DC >= 63 && script_byte_vars.zone_area < 22 && script_byte_vars.zone_area != 1)
			the_command = 0x9005;
		return ScriptRerun;
	}

	return 0;
}

/*
Load game (menu)
*/
unsigned int CMD_6_Load(void) {
	the_command = 0xC35C;
	return ScriptRerun;
}

/*
Save game (menu)
*/
unsigned int CMD_7_Save(void) {
	the_command = 0xC35D;
	return ScriptRerun;
}

/*
Show timer
*/
unsigned int CMD_8_Timer(void) {
	static unsigned char timer_image[] = {163, 244 / 4, 104};
	unsigned char x, y, width, height;
	unsigned char *image = timer_image;

	if (DrawPortrait(&image, &x, &y, &width, &height)) {
		CGA_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	}

	do {
		unsigned short timer = Swap16(script_word_vars.timer_ticks2);
		unsigned short minutes = timer % (60 * 60);

		char_draw_coords_x = 260 / 4;
		char_draw_coords_y = 120;

		WaitVBlank();
		CGA_PrintChar(timer / (60 * 60) + 16, CGA_SCREENBUFFER);
		CGA_PrintChar((minutes & 1) ? 26 : 0, CGA_SCREENBUFFER);    /*colon*/
		CGA_PrintChar(minutes / (60 * 10) + 16, CGA_SCREENBUFFER);
		CGA_PrintChar(minutes / 60 + 16, CGA_SCREENBUFFER);
		PollInput();
	} while (buttons == 0);

	PopDirtyRects(DirtyRectSprite);

	return 0;
}

int ConsumePsiEnergy(unsigned char amount) {
	unsigned char current = script_byte_vars.psy_energy;

	if (current < amount) {
		/*no energy left*/
		PlayAnim(68, 296 / 4, 71);
		return 0;
	}

	script_byte_vars.psy_energy = current - amount;

	/*significantly changed?*/
	if ((current & 0xF0) != (script_byte_vars.psy_energy & 0xF0))
		PlayAnim(68, 296 / 4, 71);

	return 1;
}

unsigned int CMD_A_PsiSolarEyes(void) {
	if (!ConsumePsiEnergy(2))
		return 0;

	if (zone_palette == 14) {
		RedrawRoomStatics(script_byte_vars.zone_room, zone_palette);
		zone_palette = 0;
		CGA_BackBufferToRealFull();
	}

	the_command = Swap16(script_word_vars.word_178EE);
	RunCommand();
	script_byte_vars.cur_spot_flags = 0xFF;

	return 0;
}


unsigned short GetZoneObjCommand(unsigned int offs) {
	/*TODO: fix me: change offs/2 to index*/
	the_command = Swap16(script_word_vars.zone_obj_cmds[(script_byte_vars.cur_spot_idx - 1) * 5 + offs / 2]);
	return the_command;
}

void DrawStickyNet(void) {
	unsigned char x, y, w, h;

	unsigned int ofs;
	unsigned char *sprite = LoadPuzzlToScratch(80);

	x = room_bounds_rect.sx;
	y = room_bounds_rect.sy;
	w = room_bounds_rect.ex - x;
	h = room_bounds_rect.ey - y;

	ofs = CGA_CalcXY_p(x, y);

	/*16x30 is the net sprite size*/

	for (; h; h -= 30) {
		int i;
		for (i = 0; i < w; i += 16 / 4)
			DrawSprite(sprite, frontbuffer, ofs + i);
		ofs += CGA_BYTES_PER_LINE * 30 / 2;
	}
}

unsigned int CMD_B_PsiStickyFingers(void) {
	if (!ConsumePsiEnergy(3))
		return 0;

	if (script_byte_vars.byte_179F9 != 0) {
		the_command = Swap16(script_word_vars.word_178F0);
		return ScriptRerun;
	}

	BackupScreenOfSpecialRoom();
	DrawStickyNet();
	SelectCursor(CURSOR_FLY);
	MenuLoop(0, 0);
	PlaySound(224);
	CGA_BackBufferToRealFull();
	RestoreScreenOfSpecialRoom();

	if (script_byte_vars.cur_spot_idx == 0 || GetZoneObjCommand(0) == 0)
		the_command = Swap16(script_word_vars.word_17844);

	if (script_byte_vars.byte_179DC >= 63
	        && script_byte_vars.zone_area < 22
	        && script_byte_vars.zone_area != 1)
		the_command = 0x9005;

	return ScriptRerun;
}

unsigned int CMD_C_PsiKnowMind(void) {
	if (!ConsumePsiEnergy(1))
		return 0;

	if (script_byte_vars.byte_179F9 != 0) {
		the_command = Swap16(script_word_vars.word_178F2);
		return ScriptRerun;
	}

	ProcessMenu();

	if (script_byte_vars.cur_spot_idx == 0 || GetZoneObjCommand(4) == 0)
		the_command = Swap16(script_word_vars.word_17846);

	return ScriptRerun;
}

unsigned int CMD_D_PsiBrainwarp(void) {
	if (!ConsumePsiEnergy(2))
		return 0;

	if (script_byte_vars.byte_179F9 == 0) {
		BackupScreenOfSpecialRoom();
		ProcessMenu();

		if (script_byte_vars.cur_spot_idx == 0) {
			the_command = Swap16(script_word_vars.word_17850);
			script_byte_vars.dead_flag = 0;
			return ScriptRerun;
		}

		if (GetZoneObjCommand(2) != 0) {
			PlayAnim(39, found_spot->sx + 8 / 4, found_spot->sy - 10);
			RestoreScreenOfSpecialRoom();
			return ScriptRerun;
		}
	}

	if (script_byte_vars.byte_179F9 == 18) {
		script_byte_vars.dead_flag = 1;
		script_byte_vars.tries_left = 2;
		return 0;
	}

	((pers_t *)script_vars[ScrPool8_CurrentPers])->flags |= PERSFLG_80;
	script_byte_vars.dead_flag = script_byte_vars.cur_spot_idx;
	script_byte_vars.tries_left = 2;
	the_command = 0;
	if (script_byte_vars.byte_179F9 == 0) {
		PlayAnim(39, found_spot->sx + 8 / 4, found_spot->sy - 10);
		RestoreScreenOfSpecialRoom();
		return ScriptRerun;
	}

	the_command = 0x90AA;
	return ScriptRerun;
}


unsigned int CMD_E_PsiZoneScan(void) {
	unsigned char x, y, w, h;
	unsigned int offs;

	if (!ConsumePsiEnergy(1))
		return 0;

	if (script_byte_vars.byte_179F9 != 0) {
		the_command = Swap16(script_word_vars.word_178FC);
		return ScriptRerun;
	}

	BackupScreenOfSpecialRoom();

	offs = CGA_CalcXY_p(room_bounds_rect.sx, room_bounds_rect.sy);
	w = room_bounds_rect.ex - room_bounds_rect.sx;
	h = room_bounds_rect.ey - room_bounds_rect.sy;

	for (y = room_bounds_rect.sy; h; y++, h--) {
		spot_t *spot;
		for (x = 0; x < w; x++) frontbuffer[offs + x] = ~frontbuffer[offs + x];
		WaitVBlank();
		for (x = 0; x < w; x++) frontbuffer[offs + x] = ~frontbuffer[offs + x];

		for (spot = zone_spots; spot != zone_spots_end; spot++) {
			if ((spot->flags & ~(SPOTFLG_40 | 7)) == (SPOTFLG_20 | SPOTFLG_8) && spot->sy == y) {
				PlaySound(27);
				spot->flags |= SPOTFLG_80;
				PlayAnim(38, spot->sx, spot->sy);
				break;
			}
		}

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}

	RestoreScreenOfSpecialRoom();

	the_command = Swap16(script_word_vars.word_17848);

	return ScriptRerun;

}

unsigned int CMD_F_PsiPsiShift(void) {
	if (!ConsumePsiEnergy(3))
		return 0;

	if (script_byte_vars.byte_179F9 != 0) {
		the_command = Swap16(script_word_vars.word_178F4);
		return ScriptRerun;
	}

	SelectCursor(CURSOR_GRAB);
	MenuLoop(0, 0);
	BackupScreenOfSpecialRoom();
	PlaySound(25);
	PlayAnim(39, cursor_x / 4, cursor_y);
	RestoreScreenOfSpecialRoom();

	if (script_byte_vars.cur_spot_idx == 0 || GetZoneObjCommand(6) == 0)
		the_command = Swap16(script_word_vars.word_1784E);

	return ScriptRerun;
}

unsigned int CMD_10_PsiExtremeViolence(void) {
	unsigned short command;

	if (!ConsumePsiEnergy(8))
		return 0;

	script_byte_vars.byte_179F3 = 1;

	if (script_byte_vars.byte_179F9 != 0) {
		the_command = Swap16(script_word_vars.word_178F6);
		return ScriptRerun;
	}

	ProcessMenu();

	if (script_byte_vars.cur_spot_idx == 0) {
		the_command = Swap16(script_word_vars.word_1784C);
		script_byte_vars.byte_179F3 = 0;
		return ScriptRerun;
	}

	command = GetZoneObjCommand(8);

	if ((command & 0xF000) == 0x9000)
		script_byte_vars.byte_179F3 = 0;
	else if (command == 0) {
		the_command = Swap16(script_word_vars.word_1784C);
		script_byte_vars.byte_179F3 = 0;
	}

	return ScriptRerun;
}

unsigned int CMD_11_PsiTuneIn(void) {
	unsigned short command;
	unsigned char *msg;

	if (!ConsumePsiEnergy(4))
		return 0;

	if (script_byte_vars.byte_179F9 != 0)
		command = Swap16(script_word_vars.word_178F8);
	else {
		if (script_byte_vars.byte_179DC < 63 || script_byte_vars.zone_area >= 22)
			command = Swap16(script_word_vars.word_1784A);
		else
			command = 275;
	}

	/*TODO: is this really neccessary? Maybe it's always set when loaded from script vars?*/
	if (command & 0x8000) {
		the_command = command;
		return ScriptRerun;
	}

	msg = SeekToString(diali_data, command);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	DrawPersonBubble(32 / 4, 20, 15, msg);

	PromptWait();
	PopDirtyRects(DirtyRectBubble);

	return 0;
}

void ActionForPersonChoice(unsigned short *actions) {
	ProcessMenu();
	the_command = 0x9183;   /*THERE`S NOBODY.*/
	if (script_byte_vars.cur_spot_idx != 0 && script_byte_vars.cur_pers != 0) {
		pers_t *pers = (pers_t *)script_vars[ScrPool8_CurrentPers];
		unsigned char index = pers->name;
		if (index == 93)    /*CADAVER*/
			index = 19 + 42;
		else if (index == 133)  /*SCI FI*/
			index = 18 + 42;

		index -= 42;        /*Person names: THE MASTER OF ORDEALS, etc*/

		the_command = actions[index];

		PlaySound(22);
	}
}

/*TODO: ensure these are never accessed/modified from the scripts*/
unsigned short menu_commands_12[] = {
	0xC0F0,
	0xC0D7,
	0x9019,
	0xC0DA,
	0x9019,
	0xC0F1,
	0x9007,
	0xC0D8,
	0x9007,
	0x9019,
	0x9007,
	0x9007,
	0xC34E,
	0xC34F,
	0x9019,
	0x9019,
	0x9019,
	0x9019,
	0xC319,
	0x9007
};

unsigned short menu_commands_22[] = {
	0xC325,
	0xC326,
	0xC31B,
	0xC31D,
	0xC31F,
	0xC31E,
	0xC320,
	0xC327,
	0x90EC,
	0xC31C,
	0xC328,
	0,
	0xC077,
	0xC077,
	0xC321,
	0xC322,
	0x9007,
	0xC323,
	0xC2A7,
	0xC324
};

unsigned short menu_commands_24[] = {
	0xC344,
	0xC34A,
	0xC343,
	0xC34D,
	0x9019,
	0xC0F1,
	0x9007,
	0xC354,
	0x90EC,
	0xC343,
	0xC345,
	0,
	0xC343,
	0xC343,
	0xC343,
	0x9019,
	0x9019,
	0x9019,
	0xC343,
	0xC343
};

unsigned short menu_commands_23[] = {
	0xC002,
	0xC32A,
	0x9019,
	0xC325,
	0xC114,
	0xC32B,
	0xC32C,
	0xC355,
	0x90EC,
	0x9019,
	0xC328,
	0,
	0xC077,
	0xC077,
	0x9019,
	0x9113,
	0,
	0xC323,
	0xC2A7,
	0xC32D
};

unsigned int CMD_12_(void) {
	printf("cmd 12\n");
	ActionForPersonChoice(menu_commands_12);
	return ScriptRerun;
}

unsigned int CMD_13_ActivateFountain(void) {
	static unsigned char water1[] = {125, 156 / 4, 58};
	static unsigned char water2[] = {126, 156 / 4, 58};
	static unsigned char headl[] = {88, 152 / 4, 52};
	static unsigned char headr[] = {88, (160 / 4) | 0x80, 52};

	unsigned char x, y, w, h;
	unsigned int i, j;

	script_byte_vars.byte_17A20 = 1;
	for (i = 0; i < 10; i++) {
		DrawRoomStaticObject(water1, &x, &y, &w, &h);
		WaitVBlank();
		CGA_BackBufferToRealFull();
		for (j = 0; j < 0x1FFF; j++) ; /*TODO: weak delay*/

		DrawRoomStaticObject(water2, &x, &y, &w, &h);
		WaitVBlank();
		CGA_BackBufferToRealFull();
		for (j = 0; j < 0x1FFF; j++) ; /*TODO: weak delay*/
	}

	DrawRoomStaticObject(headl, &x, &y, &w, &h);
	DrawRoomStaticObject(headr, &x, &y, &w, &h);
	CGA_BackBufferToRealFull();

	return 0;
}

/*Vorts walking into the room*/
unsigned int CMD_14_VortAppear(void) {
	/*TODO: check me*/
	pers_list[0].area = script_byte_vars.zone_area;
	FindAndSelectSpot(0);
	AnimateSpot(&vortanims_ptr->field_1);
	next_command3 = 0xA015;
	BlitSpritesToBackBuffer();
	DrawZoneObjs();
	CGA_BackBufferToRealFull();
	next_ticks3 = Swap16(script_word_vars.timer_ticks2) + 5;
	return 0;
}

pers_t *pers_vort_ptr;

struct {
	unsigned char room;
	unsigned char field_1;
} vortleave[] = {
	{  2,  5},
	{  3,  8},
	{  4,  8},
	{  5,  2},
	{120,  3},
	{121,  3},
	{  8, 10},
	{ 10,  8},
	{ 60, 62},
	{ 62, 66},
	{ 68, 66},
	{ 69, 66},
	{ 67, 65},
	{ 65, 66},
	{ 70, 71},
	{ 71, 70},
	{ 59, 60},
	{ 60, 62},
	{ 63, 65}
};

/*Vorts walking out of the room*/
unsigned int CMD_15_VortLeave(void) {
	/*TODO: check me*/

	unsigned int i;
	animdesc_t *anim;
	pers_t *pers;

	if (pers_list[0].area != 0) {
		pers = &pers_list[0];
		anim = &vortanims_ptr->field_4;
	} else if (pers_list[34].area != 0) {
		pers = &pers_list[34];
		anim = &vortanims_ptr->field_7;
	} else {
		script_byte_vars.byte_179EC |= 0x80;

		pers_list[35].area = 0;
		pers_list[0].flags = pers_list[35].flags;

		pers = &pers_list[0];
		anim = &vortanims_ptr->field_A;
	}

	pers->area = 0;
	next_command3 = 0;
	for (i = 0; i < 19; i++) {
		if (vortleave[i].room == script_byte_vars.zone_index) {
			next_command3 = 0xA016;
			next_ticks3 = Swap16(script_word_vars.timer_ticks2) + 5;
			pers->area = vortleave[i].field_1;
		}
	}
	pers_vort_ptr = pers;

	zone_spots[(pers->flags & 15) - 1].flags &= ~SPOTFLG_80;

	FindAndSelectSpot(0);
	AnimateSpot(anim);
	script_byte_vars.byte_179EC &= 0x80;
	return 0;
}

/*
Vorts left the room
*/
unsigned int CMD_16_VortGone(void) {
	pers_vort_ptr->area = 0;
	next_command3 = 0;
	return 0;
}

unsigned int CMD_18_AspirantLeave(void) {
	/*TODO: check me*/
	static const animdesc_t anim33 = {ANIMFLG_USESPOT | 33};

	PopDirtyRects(DirtyRectSprite);
	PopDirtyRects(DirtyRectText);

	pers_ptr->area = 0;
	script_word_vars.next_command1 = BE(0);

	if ((pers_ptr->flags & PERSFLG_40) == 0) {
		spot_ptr->flags &= ~SPOTFLG_80;
		FindAndSelectSpot(script_byte_vars.quest_item_ofs);
		script_byte_vars.byte_179EF = 0;
		AnimateSpot(&anim33);
	}

	return 0;
}


/*
Show Holo screen anim and speech
*/
unsigned int CMD_1B_Holo(void) {
	unsigned char x, y;
	unsigned int num;
	unsigned char *msg;

	x = found_spot->sx;
	y = found_spot->sy;

	PlayAnim(42, x + 4 / 4, y + 6);

	num = 321 + ((Swap16(script_word_vars.timer_ticks2) < 60 * 60) ? 0 : 4) + (script_byte_vars.rand_value % 4);
	msg = SeekToString(diali_data, num);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	if (x < 140 / 4)
		DrawPersonBubble(x + 32 / 4, y - 40, SPIKE_DNLEFT | 20, msg);
	else
		DrawPersonBubble(x - 92 / 4, y - 40, SPIKE_DNRIGHT | 20, msg);

	PlayAnim(43, x, y);

	PromptWait();
	PopDirtyRects(DirtyRectBubble);

	PlayAnim(45, x, y);

	return 0;
}

/*
TODO: check me
*/
unsigned int CMD_20_(void) {
	pers_list[5].area = 0;
	next_command4 = 0;
	return 0;
}

/*
Talk to Vorts
*/
unsigned int CMD_21_VortTalk(void) {
	unsigned char x, y;
	unsigned int num;
	unsigned char *msg;

	if (script_byte_vars.rand_value >= 85)
		num = 6;
	else if (script_byte_vars.rand_value >= 170)
		num = 7;
	else
		num = 35;

	msg = SeekToString(diali_data, num);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	x = found_spot->sx;
	y = found_spot->sy;

	if (x < 140 / 4)
		DrawPersonBubble(found_spot->ex, y - 40, SPIKE_DNLEFT | 20, msg);
	else
		DrawPersonBubble(x - 80 / 4, y - 40, SPIKE_DNRIGHT | 20, msg);

	PromptWait();
	PopDirtyRects(DirtyRectBubble);

	return 0;
}

unsigned int CMD_22_(void) {
	ActionForPersonChoice(menu_commands_22);
	return ScriptRerun;
}

unsigned int CMD_23_(void) {
	ActionForPersonChoice(menu_commands_23);
	return ScriptRerun;
}

unsigned int CMD_24_(void) {
	ActionForPersonChoice(menu_commands_24);
	return ScriptRerun;
}

unsigned int CMD_25_LoadGame(void) {
	if (LoadScena())
		the_command = 0x918F;   /*error loading*/
	else
		the_command = 0x90AA;
	return ScriptRerun;
}

unsigned int CMD_26_SaveGame(void) {
	if (SaveScena())
		the_command = 0x9190;   /*error saving*/
	else
		the_command = 0x90AA;
	return ScriptRerun;
}



typedef unsigned int (*cmdhandler_t)(void);

cmdhandler_t command_handlers[] = {
	0,
	CMD_1_RoomObjects,
	CMD_2_PsiPowers,
	CMD_3_Posessions,
	CMD_4_EnergyLevel,
	CMD_5_Wait,
	CMD_6_Load,
	CMD_7_Save,
	CMD_8_Timer,
	CMD_TRAP,
	CMD_A_PsiSolarEyes,
	CMD_B_PsiStickyFingers,
	CMD_C_PsiKnowMind,
	CMD_D_PsiBrainwarp,
	CMD_E_PsiZoneScan,
	CMD_F_PsiPsiShift,
	CMD_10_PsiExtremeViolence,  /*10*/
	CMD_11_PsiTuneIn,
	CMD_12_,
	CMD_13_ActivateFountain,
	CMD_14_VortAppear,
	CMD_15_VortLeave,
	CMD_16_VortGone,
	CMD_TRAP,
	CMD_18_AspirantLeave,
	CMD_TRAP,
	CMD_TRAP,
	CMD_1B_Holo,
	CMD_TRAP,
	CMD_TRAP,
	CMD_TRAP,
	CMD_TRAP,
	CMD_TRAP,   /*20*/
	CMD_21_VortTalk,
	CMD_22_,
	CMD_23_,
	CMD_24_,
	CMD_25_LoadGame,
	CMD_26_SaveGame
};
#define MAX_CMD_HANDLERS (sizeof(command_handlers) / sizeof(command_handlers[0]))

cmdhandler_t script_handlers[] = {
	0,
	SCR_1_AspirantItemTrade,
	SCR_TRAP,
	SCR_3_DrawItemBox,
	SCR_TRAP,
	SCR_5_DrawPortraitLiftRight,
	SCR_6_DrawPortraitLiftLeft,
	SCR_7_DrawPortraitLiftDown,
	SCR_8_DrawPortraitLiftUp,
	SCR_9_DrawPortrait,
	SCR_TRAP,
	SCR_B_DrawPortraitTwistEffect,
	SCR_C_DrawPortraitArcEffect,
	SCR_D_DrawPortraitDotEffect,
	SCR_E_DrawPortraitZoomIn,
	SCR_TRAP,
	SCR_TRAP,   /*10*/
	SCR_11_DrawRoomObject,
	SCR_12_Chain,
	SCR_13_RedrawRoomStatics,
	SCR_14_DrawDesc,
	SCR_15_SelectSpot,
	SCR_TRAP,
	SCR_17_DrawPersonThoughtBubbleDialog,
	SCR_18_AnimPortrait,
	SCR_19_HidePortraitLiftLeft,
	SCR_1A_HidePortraitLiftRight,
	SCR_1B_HidePortraitLiftUp,
	SCR_1C_HidePortraitLiftDown,
	SCR_TRAP,                   /*TODO: same as SCR_23_HidePortrait , unused*/
	SCR_1E_HidePortraitTwist,
	SCR_1F_HidePortraitArc,
	SCR_20_HidePortraitDots,    /*20*/
	SCR_TRAP,                   /*TODO: same as SCR_23_HidePortrait , unused*/
	SCR_22_HidePortraitShatter,
	SCR_23_HidePortrait,
	SCR_24_PopAllPortraits,
	SCR_25_ChangeZoneOnly,
	SCR_26_GameOver,
	SCR_27_DrawGaussBubble,
	SCR_28_MenuLoop,
	SCR_29_DialiTextBox,
	SCR_2A_PopDialogRect,
	SCR_2B_PopAllBubbles,
	SCR_2C_Wait4,
	SCR_2D_Wait,
	SCR_2E_PromptWait,
	SCR_TRAP,
	SCR_30_Fight,   /*30*/
	SCR_31_Fight2,
	SCR_32_FightWin,
	SCR_33_Jump,
	SCR_34_Call,
	SCR_35_Ret,
	SCR_36_ChangeZone,
	SCR_37_DesciTextBox,
	SCR_38_PlayAnim,
	SCR_39_AnimRoomDoorOpen,
	SCR_3A_AnimRoomDoorClose,
	SCR_3B_MathExpr,
	SCR_3C_CondExpr,
	SCR_3D_ActionsMenu,
	SCR_3E_TheWallAdvance,
	SCR_TRAP,
	SCR_40_PopAllTextBoxes, /*40*/
	SCR_41_LiftHand,
	SCR_42_LoadZone,
	SCR_43_RefreshZone,
	SCR_44_BackBufferToScreen,
	SCR_45_DeProfundisRoomEntry,
	SCR_46_DeProfundisLowerHook,
	SCR_47_DeProfundisRiseMonster,
	SCR_48_DeProfundisLowerMonster,
	SCR_49_DeProfundisRiseHook,
	SCR_TRAP,
	SCR_TRAP,
	SCR_4C_DrawZoneObjs,
	SCR_TRAP,
	SCR_4E_BounceCurrentItemToRoom,
	SCR_4F_BounceCurrentItemToInventory,
	SCR_50_BounceItemToInventory,   /*50*/
	SCR_51_ItemTrade,
	SCR_52_RefreshSpritesData,
	SCR_53_FindInvItem,
	SCR_TRAP,
	SCR_TRAP,
	SCR_TRAP,
	SCR_TRAP,
	SCR_TRAP,
	SCR_59_BlitSpritesToBackBuffer,
	SCR_5A_SelectPalette,
	SCR_TRAP,
	SCR_TRAP,
	SCR_TRAP,
	SCR_TRAP,
	SCR_5F_DrawRoomObjectBack,
	SCR_TRAP,   /*60*/
	SCR_61_DrawPersonBubbleDialog,
	SCR_TRAP,
	SCR_TRAP,
	SCR_64_DrawBoxAroundSpot,
	SCR_65_DeProfundisMovePlatform,
	SCR_66_DeProfundisRideToExit,
	SCR_67_Unused,
	SCR_68_Unused,
	SCR_69_PlaySound,
	SCR_6A_Unused,
};
#define MAX_SCR_HANDLERS (sizeof(script_handlers) / sizeof(script_handlers[0]))

#ifdef DEBUG_SCRIPT
int runscr_reentr = 0;
int runcmd_reentr = 0;
#endif

unsigned int RunScript(unsigned char *code) {
	unsigned int status = ScriptContinue;

#ifdef DEBUG_SCRIPT
	runscr_reentr += 1;
#endif

	script_ptr = code;
	while (script_ptr != script_end_ptr) {
		unsigned char opcode = *script_ptr;

#ifdef DEBUG_SCRIPT
		{
			FILE *f = fopen(DEBUG_SCRIPT_LOG, "at");
			if (f) {
				unsigned int offs = (script_ptr - templ_data) & 0xFFFF;
				fprintf(f, "%04X: %02X\n", offs, opcode);
				fclose(f);
			}
		}
#endif

#ifdef DEBUG_QUEST
		if (script_ptr - templ_data == 0x4F) {
			/*manipulate rand_value to get a quest item we need*/
			script_byte_vars.rand_value = DEBUG_QUEST;
		}
#endif


		if (opcode == 0 || opcode >= 107)
			break;

		status = script_handlers[opcode]();

		if (status != ScriptContinue)
			break;
	}

#ifdef DEBUG_SCRIPT
	runscr_reentr -= 1;
#endif

	return status;
}

unsigned char *GetScriptSubroutine(unsigned int index) {
	return SeekToEntry(templ_data, index, &script_end_ptr);
}

unsigned int RunCommand(void) {
	unsigned int res;
	unsigned short cmd;
again:
	;
	res = 0;

	if (the_command == 0)
		return 0;

#ifdef DEBUG_SCRIPT
	{
		FILE *f = fopen(DEBUG_SCRIPT_LOG, "at");
		if (f) {
			fprintf(f, "\nRunCommand 0x%04X rc: %d rs: %d\n", the_command, runcmd_reentr, runscr_reentr);
			fclose(f);
		}
	}

	runcmd_reentr += 1;

#endif

	cmd = the_command & 0x3FF;

	switch (the_command & 0xF000) {
	case 0: /*TODO what kind of call is this?*/
		res = RunScript(templ_data + the_command);
		break;
	case 0x9000:
		DrawMessage(SeekToString(desci_data, cmd), CGA_SCREENBUFFER);
		break;
	case 0xA000:
	case 0xB000:
		printf("Command: $%X 0x%X\n", the_command, cmd);
		res = command_handlers[cmd]();
		break;
	case 0xF000:
		/*restore sp from keep_sp*/
		TODO("SCR_RESTORE\n");
	default:
		res = RunScript(GetScriptSubroutine(cmd - 1));
	}

#ifdef DEBUG_SCRIPT
	runcmd_reentr -= 1;
#endif

#ifdef DEBUG_SCRIPT
	{
		FILE *f = fopen(DEBUG_SCRIPT_LOG, "at");
		if (f) {
			fprintf(f, "\n");
			fclose(f);
		}
	}
#endif

	/*TODO: this is pretty hacky, original code manipulates the stack to discard old script invocation*/
	if (res == ScriptRerun)
		goto again;

	return res;
}

unsigned int RunCommandKeepSp(void) {
	/*keep_sp = sp;*/
	return RunCommand();
}

} // End of namespace Chamber
