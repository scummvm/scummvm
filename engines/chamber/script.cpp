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

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include <setjmp.h>

#include "common/system.h"
#include "common/debug.h"

#include "chamber/chamber.h"
#include "chamber/common.h"
#include "chamber/script.h"
#include "chamber/enums.h"
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
#include "chamber/ifgm.h"

#if 0
#define DEBUG_SCRIPT
char DEBUG_SCRIPT_LOG[] = "!script.log";
#endif

#include "chamber/scrvars.h"

namespace Chamber {

jmp_buf script_jmp;

byte rand_seed;
uint16 the_command;
uint16 script_res;
byte *script_ptr, *script_end_ptr;
byte *script_stack[5 * 2];
byte **script_stack_ptr = script_stack;

void *script_vars[kScrPools_MAX] = {
	&script_word_vars,
	&script_word_vars,
	&script_byte_vars,
	inventory_items,
	zones_data,
	pers_list,
	inventory_items,
	inventory_items + kItemZapstik1 - 1,
	pers_list
};

extern void askDisk2(void);

/*
Get next random byte value
*/
byte getRand(void) {
	script_byte_vars.rand_value = aleat_data[++rand_seed];
	return script_byte_vars.rand_value;
}

/*
Get next random word value
*/
uint16 getRandW(void) {
	uint16 r = getRand() << 8;
	return r | getRand();
}

uint16 Swap16(uint16 x) {
	return (x << 8) | (x >> 8);
}

/*
Script handlers exit codes
*/
enum CommandStatus {
	ScriptContinue = 0, /*run next script command normally*/
	ScriptRerun = 1, /*abort current script, execute new command*/
	/*TODO: maybe define ScriptRestartGame to support game restart?*/
};

uint16 CMD_TRAP(void) {
	warning("CMD TRAP");
	promptWait();
	for (;;) ;
	return 0;
}

uint16 SCR_TRAP(void) {
	warning("SCR TRAP 0x%02X @ 0x%lX", *script_ptr, script_ptr - templ_data);
	promptWait();
	for (;;) ;
	return 0;
}

/*
Remove "not interested in that" tag from owned items
*/
void reclaimRefusedItems(void) {
	int16 i;
	for (i = 0; i < MAX_INV_ITEMS; i++) {
		if (inventory_items[i].flags == (ITEMFLG_OWNED | ITEMFLG_DONTWANT))
			inventory_items[i].flags = ITEMFLG_OWNED;
	}
}

/*
Trade with a fellow Aspirant (the one that offers to swap an item)
*/
uint16 SCR_1_AspirantItemTrade(void) {
	byte *old_script, *old_script_end = script_end_ptr;

	item_t *item1, *item2;

	script_ptr++;
	old_script = script_ptr;

	for (;;) {
		inv_bgcolor = 0xFF;
		openInventory(0xFE, ITEMFLG_OWNED);

		if (inv_count == 0) {
			the_command = 0xC1BC;
			runCommand();
			break;
		}

		if (the_command == 0) {
			the_command = 0xC1C0;
			runCommand();
			break;
		}

		the_command = 0x9140;

		if (aspirant_ptr->item == 0)
			break;

		item1 = &inventory_items[aspirant_ptr->item - 1];	/*aspirant's item*/
		item2 = (item_t *)(script_vars[kScrPool3_CurrentItem]);	/*our offer*/

		if (item2->flags == (ITEMFLG_OWNED | ITEMFLG_DONTWANT) || item1->name == item2->name) {
			the_command = 0xC1C0;
			runCommand();
			break;
		}

		if (item2->name == 109	/*SKULL*/
		        || item2->name == 132	/*ZAPSTIK*/
		        || item2->name == 108  /*DAGGER*/
		        || script_byte_vars.rand_value < 154) {
			/*accept*/
			item2->flags = ITEMFLG_ASPIR;
			item1->flags = ITEMFLG_OWNED;
			aspirant_ptr->item = script_byte_vars.inv_item_index;
			switch (item2->name) {
			case 132:	/*ZAPSTIK*/
				script_byte_vars.zapstiks_owned--;
				the_command = 0xC04B;
				break;
			case 104:	/*ROPE*/
				the_command = 0xC1BA;
				break;
			case 107:	/*GOBLET*/
				the_command = 0xC1BB;
				break;
			default:	/*STONE FLY*/
				the_command = 0xC1B9;
			}
			runCommand();
			break;
		} else {
			/*not interested*/
			item2->flags = ITEMFLG_OWNED | ITEMFLG_DONTWANT;
			the_command = 0xC1BD;
			runCommand();
			continue;
		}
	}

	reclaimRefusedItems();

	script_ptr = old_script;
	script_end_ptr = old_script_end;

	return 0;
}

/*
Trade with a rude/passive Aspirant (the one that says nasty words about you)
*/
uint16 SCR_2_RudeAspirantTrade(void) {
	byte *old_script, *old_script_end = script_end_ptr;

	item_t *item1, *item2;

	script_ptr++;
	old_script = script_ptr;

	the_command = 0x9099;   /*WHAT DO YOU WANT TO EXCHANGE?*/
	runCommand();

	for (;;) {
		inv_bgcolor = 0xFF;
		openInventory(0xFE, ITEMFLG_OWNED);

		if (inv_count == 0) {
			the_command = 0xC1C5;
			runCommand();
			break;
		}

		if (the_command == 0)
			break;

		the_command = 0x9140;   /*NOTHING ON HIM*/

		if (aspirant_ptr->item == 0) {
			runCommand();
			break;
		}

		item1 = &inventory_items[aspirant_ptr->item - 1];	/*aspirant's item*/
		item2 = (item_t *)(script_vars[kScrPool3_CurrentItem]);	/*our offer*/

		if (item2->flags == (ITEMFLG_OWNED | ITEMFLG_DONTWANT)) {
			the_command = 0xC1C0;
			runCommand();
			break;
		}

		/*only trade for ROPE, LANTERN, STONE FLY, GOBLET*/
		if (item1->name < 104 || item1->name >= 108) {
			runCommand();
			break;
		}

		if ((item1->name != item2->name)
		        && (item2->name == 109	/*SKULL*/
		            || item2->name == 132	/*ZAPSTIK*/
		            || item2->name == 108	/*DAGGER*/
		            || script_byte_vars.rand_value < 154)) {

			/*show confirmation menu*/
			script_byte_vars.trade_accepted = 0;
			the_command = 0xC1C6;
			runCommand();
			if (script_byte_vars.trade_accepted == 0)
				break;

			item2->flags = ITEMFLG_ASPIR;
			item1->flags = ITEMFLG_OWNED;
			aspirant_ptr->item = script_byte_vars.inv_item_index;
			switch (item2->name) {
			case 132:	/*ZAPSTIK*/
				script_byte_vars.zapstiks_owned--;
				the_command = 0xC04B;
				break;
			case 104:	/*ROPE*/
				the_command = 0xC1BA;
				break;
			case 107:	/*GOBLET*/
				the_command = 0xC1BB;
				break;
			default:	/*STONE FLY*/
				the_command = 0xC1B9;
			}
			runCommand();
			break;
		} else {
			/*not interested*/
			item2->flags = ITEMFLG_OWNED | ITEMFLG_DONTWANT;
			the_command = 0xC1BD;
			runCommand();
			continue;
		}
	}

	reclaimRefusedItems();

	script_ptr = old_script;
	script_end_ptr = old_script_end;

	return 0;
}

/*
Steal a Zapstik form Protozorq
*/
uint16 SCR_4_StealZapstik(void) {
	byte *old_script;

	pers_t *pers = (pers_t *)(script_vars[kScrPool8_CurrentPers]);

	script_ptr++;
	old_script = script_ptr;

	if ((pers->index & ~7) != 0x30) {
		the_command = 0x9148;   /*YOU`VE ALREADY GOT IT*/
		runCommand();
	} else {
		pers->index &= ~0x18;

		script_vars[kScrPool3_CurrentItem] = &inventory_items[kItemZapstik1 - 1 + (script_byte_vars.cur_pers - 1) - kPersProtozorq1];
		script_byte_vars.steals_count++;

		bounceCurrentItem(ITEMFLG_OWNED, 85);  /*bounce to inventory*/

		the_command = 0x9147;   /*YOU GET HIS ZAPSTIK*/
		if (script_byte_vars.zapstik_stolen == 0) {
			runCommand();
			script_byte_vars.zapstik_stolen = 1;
			the_command = 0x9032;   /*THIS SHOULD GIVE YOU THE EDGE IN MOST COMBATS!*/
		}
		runCommand();
	}

	script_ptr = old_script;

	return 0;
}


byte wait_delta = 0;

/*
Wait for a specified number of seconds (real time) or a keypress
*/
void wait(byte seconds) {
	warning("STUB: Wait(%d)", seconds);

#if 0
	struct time t;
	uint16 endtime;

	seconds += wait_delta;
	if (seconds > 127)  /*TODO: is this a check for a negative value?*/
		seconds = 0;

	gettime(&t);
	endtime = t.ti_sec * 100 + t.ti_hund + seconds * 100;

	while (buttons == 0) {
		uint16 current;
		gettime(&t);
		current = t.ti_sec * 100 + t.ti_hund;
		if (endtime >= 6000 && current < 2048)  /*TODO: some kind of overflow check???*/
			current += 6000;
		if (current >= endtime)
			break;
	}
#endif
}

/*
Wait for a 4 seconds or a keypress
*/
uint16 SCR_2C_Wait4(void) {
	script_ptr++;
	wait(4);
	return 0;
}

/*
Wait for a specified number of seconds or a keypress
TODO: Always waits for a 4 seconds due to a bug?
*/
uint16 SCR_2D_Wait(void) {
	byte seconds;
	script_ptr++;
	seconds = *script_ptr++;
	(void)seconds;
	wait(4);    /*TODO: looks like a bug?*/
	return 0;
}

/*
Show blinking prompt indicator and wait for a keypress
*/
uint16 SCR_2E_promptWait(void) {
	script_ptr++;
	promptWait();
	return 0;
}


#define VARTYPE_VAR 0x80
#define VARTYPE_BLOCK 0x40
#define VARTYPE_WORD 0x20
#define VARTYPE_KIND 0x1F

#define VARSIZE_BYTE 0
#define VARSIZE_WORD 1

byte var_size;

/*
Fetch variable's value and address
*/
uint16 loadVar(byte **ptr, byte **varptr) {
	byte vartype;
	byte *varbase;
	uint16 value = 0;
	var_size = VARSIZE_BYTE;
	vartype = *((*ptr)++);
	if (vartype & VARTYPE_VAR) {
		/*variable*/
		byte varoffs;
		varbase = (byte *)script_vars[vartype & VARTYPE_KIND];
		if (vartype & VARTYPE_BLOCK) {
			byte *end;
			byte index = *((*ptr)++);
			varbase = seekToEntryW(varbase, index, &end);
		}
		varoffs = *((*ptr)++);
#if 1
		{
			int16 maxoffs = 0;
			switch (vartype & VARTYPE_KIND) {
			case kScrPool0_WordVars0:
			case kScrPool1_WordVars1:
				maxoffs = sizeof(script_word_vars);
				break;
			case kScrPool2_ByteVars:
				maxoffs = sizeof(script_byte_vars);
				break;
			case kScrPool3_CurrentItem:
				maxoffs = sizeof(item_t);
				break;
			case kScrPool4_ZoneSpots:
				maxoffs = RES_ZONES_MAX;
				break;
			case kScrPool5_Persons:
				maxoffs = sizeof(pers_list);
				break;
			case kScrPool6_Inventory:
				maxoffs = sizeof(inventory_items);
				break;
			case kScrPool7_Zapstiks:
				maxoffs = sizeof(inventory_items) - sizeof(item_t) * (kItemZapstik1 - 1);
				break;
			case kScrPool8_CurrentPers:
				maxoffs = sizeof(pers_t);
				break;
			}
			if (varoffs >= maxoffs) {
				warning("Scr var out of bounds @ %X (pool %d, ofs 0x%X, max 0x%X)", (uint16)(script_ptr - templ_data), vartype & VARTYPE_KIND, varoffs, maxoffs);
				promptWait();
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
			warning("Var 2.%X = %X", varoffs, value);
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

/*
Perform math/logic operation on two operands
*/
uint16 mathOp(byte op, uint16 op1, uint16 op2) {
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
			if ((int16)op1 <= (int16)op2) return ~0;
		if (op & MATHOP_GE)
			if ((int16)op1 >= (int16)op2) return ~0;
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

/*
Evaluate an expression
*/
uint16 mathExpr(byte **ptr) {
	byte op;
	uint16 op1, op2;
	byte *opptr;
	op1 = loadVar(ptr, &opptr);
	while (((op = *((*ptr)++)) & MATHOP_END) == 0) {
		op2 = loadVar(ptr, &opptr);
		op1 = mathOp(op, op1, op2);
	}
	return op1;
}

/*
Evaluate an expression and assign result to a variable
*/
uint16 SCR_3B_MathExpr(void) {
	uint16 op1, op2;
	byte *opptr;

	script_ptr++;

	/*get result variable pointer*/
	op1 = loadVar(&script_ptr, &opptr);

	/*evaluate*/
	op2 = mathExpr(&script_ptr);

	/*store result*/
	/*TODO: original bug? MathExpr may overwrite global var_size, so mixed-size expressions will produce errorneous results*/
	if (var_size == VARSIZE_BYTE)
		*opptr = op2 & 255;
	else {
		opptr[0] = op2 >> 8;    /*store in big-endian*/
		opptr[1] = op2 & 255;
	}

	(void)op1;
	/*return op1;*/ /*previous value, never used?*/
	return 0;
}

/*
Discard current callchain (the real one) and execute command
*/
uint16 SCR_4D_PriorityCommand(void) {
	script_ptr++;
	the_command = *script_ptr++;          /*little-endian*/
	the_command |= (*script_ptr++) << 8;
	the_command |= 0xF000;

	/*TODO: normally this should be called from the runCommand() itself,
	because command Fxxx may be issued from the other places as well (maybe it's not the case)
	But that would require some sort of synchronization to avoid infinite loop
	So jump to top interepter's loop directly from here for now
	*/
	longjmp(script_jmp, 1);

	return ScriptRerun;
}

/*
Jump to routine
*/
uint16 SCR_12_Chain(void) {
	script_ptr++;
	the_command = *script_ptr++;          /*little-endian*/
	the_command |= (*script_ptr++) << 8;
	script_ptr = getScriptSubroutine(the_command - 1);
	return 0;
}

/*
Absolute jump
Jumping past current routine ends the script
*/
uint16 SCR_33_Jump(void) {
	uint16 offs;
	script_ptr++;
	offs = *script_ptr++;          /*little-endian*/
	offs |= (*script_ptr++) << 8;
	script_ptr = templ_data + offs;
	return 0;
}

/*
Conditional jump (IF/ELSE block)
*/
uint16 SCR_3C_CondExpr(void) {
	script_ptr++;

	if (mathExpr(&script_ptr)) {
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
uint16 SCR_34_Call(void) {
	uint16 offs;
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
uint16 SCR_35_Ret(void) {
	script_end_ptr = *(--script_stack_ptr);
	script_ptr = *(--script_stack_ptr);
	return 0;
}

/*
Draw portrait, pushing it from left to right
*/
uint16 SCR_5_DrawPortraitLiftRight(void) {
	byte x, y, width, height;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	cga_AnimLiftToRight(width, cur_image_pixels + width - 1, width, 1, height, CGA_SCREENBUFFER, cga_CalcXY_p(x, y));
	return 0;
}

/*
Draw portrait, pushing it from right to left
*/
uint16 SCR_6_DrawPortraitLiftLeft(void) {
	byte x, y, width, height;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	cga_AnimLiftToLeft(width, cur_image_pixels, width, 1, height, CGA_SCREENBUFFER, cga_CalcXY_p(x + width - 1, y));
	return 0;
}

/*
Draw portrait, pushing it from top to bottom
*/
uint16 SCR_7_DrawPortraitLiftDown(void) {
	byte x, y, width, height;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	cga_AnimLiftToDown(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	return 0;
}

/*
Draw portrait, pushing it from bottom to top
*/
uint16 SCR_8_DrawPortraitLiftUp(void) {
	byte x, y, width, height;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	/*TODO: use local args instead of globals*/
	cga_AnimLiftToUp(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, x, y + height - 1);
	return 0;
}

/*
Draw portrait, no special effects
*/
uint16 SCR_9_DrawPortrait(void) {
	byte x, y, width, height;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	cga_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	return 0;
}

/*
Draw portrait, no special effects
*/
uint16 SCR_A_DrawPortrait(void) {
	return SCR_9_DrawPortrait();
}

/*
Draw screen pixels using 2-phase clockwise twist
*/
void twistDraw(byte x, byte y, byte width, byte height, byte *source, byte *target) {
	int16 i;
	uint16 sx, ex, sy, ey, t;
	sx = x * 4;
	ex = x * 4 + width * 4 - 1;
	sy = y;
	ey = y + height - 1;

	for (i = 0; i < width * 4; i++) {
		cga_TraceLine(sx, ex, sy, ey, source, target);
		waitVBlank();
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
		cga_TraceLine(sx, ex, sy, ey, source, target);
		waitVBlank();
		sy -= 1;
		ey += 1;
	}
}

/*
Draw image with twist-effect
*/
uint16 SCR_B_DrawPortraitTwistEffect(void) {
	byte x, y, width, height;
	uint16 offs;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	offs = cga_CalcXY_p(x, y);

	cga_SwapScreenRect(cur_image_pixels, width, height, backbuffer, offs);
	twistDraw(x, y, width, height, backbuffer, frontbuffer);
	cga_BlitAndWait(scratch_mem2, width, width, height, backbuffer, offs);

	return 0;
}

/*
Draw screen pixels using arc-like sweep
*/
void arcDraw(byte x, byte y, byte width, byte height, byte *source, byte *target) {
	int16 i;
	uint16 sx, ex, sy, ey;
	sx = x * 4;
	ex = x * 4 + width * 2 - 1;
	sy = y + height - 1;
	ey = y + height - 1;

	for (i = 0; i < height; i++) {
		cga_TraceLine(sx, ex, sy, ey, source, target);
		waitVBlank();
		sy -= 1;
	}

	for (i = 0; i < width * 4; i++) {
		cga_TraceLine(sx, ex, sy, ey, source, target);
		waitVBlank();
		sx += 1;
	}

	for (i = 0; i < height + 1; i++) {
		cga_TraceLine(sx, ex, sy, ey, source, target);
		waitVBlank();
		sy += 1;
	}
}

/*
Draw image with arc-effect
*/
uint16 SCR_C_DrawPortraitArcEffect(void) {
	byte x, y, width, height;
	uint16 offs;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	offs = cga_CalcXY_p(x, y);

	cga_SwapScreenRect(cur_image_pixels, width, height, backbuffer, offs);
	arcDraw(x, y, width, height, backbuffer, frontbuffer);
	cga_BlitAndWait(scratch_mem2, width, width, height, backbuffer, offs);

	return 0;
}

/*
Draw image with slow top-to-down reveal effect by repeatedly draw its every 17th pixel
*/
uint16 SCR_D_DrawPortraitDotEffect(void) {
	//int16 i;
	byte x, y, width, height;
	uint16 offs, step = 17;
	byte *target = CGA_SCREENBUFFER;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	cur_image_end = width * height;
	int16 count = 0;

	for (offs = 0; offs != cur_image_end;) {
		target[cga_CalcXY_p(x + offs % cur_image_size_w, y + offs / cur_image_size_w)] = cur_image_pixels[offs];

		if (count % 5 == 0)
			cga_blitToScreen(offs, 4, 1);

		offs += step;
		if (offs > cur_image_end)
			offs -= cur_image_end;

		count++;
	}
	return 0;
}

/*
Draw image with slow zoom-in reveal effect
*/
uint16 SCR_E_DrawPortraitZoomIn(void) {
	byte x, y, width, height;

	script_ptr++;

	if (!drawPortrait(&script_ptr, &x, &y, &width, &height))
		return 0;

	cga_AnimZoomIn(cur_image_pixels, cur_image_size_w, cur_image_size_h, frontbuffer, cur_image_offs);
	return 0;
}


uint16 drawPortraitZoomed(byte **params) {
	byte x, y, width, height;
	byte zwidth, zheight;

	right_button = 0;   /*prevent cancel or zoom parameters won't be consumed*/
	if (!drawPortrait(params, &x, &y, &width, &height))
		return 0;   /*TODO: maybe just remove the if/return instead?*/

	zwidth = *((*params)++);
	zheight = *((*params)++);

	/*adjust the rect for new size*/
	last_dirty_rect->width = zwidth + 2;
	last_dirty_rect->height = zheight;

	cga_ZoomImage(cur_image_pixels, cur_image_size_w, cur_image_size_h, zwidth, zheight, frontbuffer, cur_image_offs);
	return 0;
}

/*
Draw image with specified w/h zoom
*/
uint16 SCR_10_DrawPortraitZoomed(void) {
	script_ptr++;
	drawPortraitZoomed(&script_ptr);

#if 0
	/*TODO: debug wait*/
	promptWait();
#endif

	return 0;
}

/*
Hide portrait, pushing it from right to left
*/
uint16 SCR_19_HidePortraitLiftLeft(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	/*TODO: This originally was done by reusing door sliding routine*/

	/*offs = cga_CalcXY_p(x + 1, y);*/
	offs++;

	while (--width) {
		cga_HideScreenBlockLiftToLeft(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	offs--;

	/*hide leftmost line*/
	/*TODO: move this to CGA?*/
	uint16 ooffs = offs;
	byte oh = height;
	while (height--) {
		memcpy(frontbuffer + offs, backbuffer + offs, 1);

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}
	cga_blitToScreen(ooffs, 1, oh);

	return 0;
}

/*
Hide portrait, pushing it from left to right
*/
uint16 SCR_1A_HidePortraitLiftRight(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	/*TODO: This originally was done by reusing door sliding routine*/

	offs = cga_CalcXY_p(x + width - 2, y);

	while (--width) {
		cga_HideScreenBlockLiftToRight(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	offs++;

	/*hide leftmost line*/
	/*TODO: move this to CGA?*/
	uint16 ooffs = offs;
	byte oh = height;
	while (height--) {
		memcpy(frontbuffer + offs, backbuffer + offs, 1);

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}
	cga_blitToScreen(ooffs, 1, oh);

	return 0;
}

/*
Hide portrait, pushing it from bottom to top
*/
uint16 SCR_1B_HidePortraitLiftUp(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	offs = cga_CalcXY_p(x, y + 1);

	while (--height) {
		cga_HideScreenBlockLiftToUp(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	/*hide topmost line*/
	/*TODO: move this to CGA?*/
	offs ^= CGA_ODD_LINES_OFS;
	if ((offs & CGA_ODD_LINES_OFS) != 0)
		offs -= CGA_BYTES_PER_LINE;
	memcpy(CGA_SCREENBUFFER + offs, backbuffer + offs, width);
	cga_blitToScreen(offs, width, 1);
	return 0;
}


/*
Hide portrait, pushing it from top to bottom
*/
uint16 SCR_1C_HidePortraitLiftDown(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	offs = cga_CalcXY_p(x, y + height - 2);

	while (--height) {
		cga_HideScreenBlockLiftToDown(1, CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	/*hide bottommost line*/
	/*TODO: move this to CGA?*/
	offs ^= CGA_ODD_LINES_OFS;
	if ((offs & CGA_ODD_LINES_OFS) == 0)
		offs += CGA_BYTES_PER_LINE;
	memcpy(CGA_SCREENBUFFER + offs, backbuffer + offs, width);
	cga_blitToScreen(offs, width, 1);
	return 0;
}


/*
Hide portrait with twist effect
*/
uint16 SCR_1E_HidePortraitTwist(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	twistDraw(x, y, width, height, backbuffer, frontbuffer);

	return 0;
}

/*
Hide portrait with arc effect
*/
uint16 SCR_1F_HidePortraitArc(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	arcDraw(x, y, width, height, backbuffer, frontbuffer);

	return 0;
}

/*
Hide portrait with dots effect
*/
uint16 SCR_20_HidePortraitDots(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	dot_effect_step = 17;
	dot_effect_delay = 100;
	copyScreenBlockWithDotEffect(backbuffer, x, y, width, height, frontbuffer);

	return 0;
}

/*
Play room's door open animation
*/
uint16 SCR_39_AnimRoomDoorOpen(void) {
	byte door;

	script_ptr++;
	door = *script_ptr++;
	animRoomDoorOpen(door);
	return 0;
}

/*
Play room's door close animation
*/
uint16 SCR_3A_AnimRoomDoorClose(void) {
	byte door;

	script_ptr++;
	door = *script_ptr++;
	animRoomDoorClose(door);
	return 0;
}

uint16 SCR_25_ChangeZoneOnly(void) {
	byte index;
	byte old = script_byte_vars.zone_room;

	script_ptr++;
	index = *script_ptr++;

	changeZone(index);
	script_byte_vars.zone_room = old;
	return 0;
}

#define JCOUNT 16

typedef struct jpoint_t {
	signed short x;
	signed short y;
} jpoint_t;

static jpoint_t jdeltas[JCOUNT] = {
	{0, -2},
	{1, -2},
	{2, -2},
	{2, -1},
	{2, 0},
	{2, 1},
	{2, 2},
	{1, 2},
	{0, 2},
	{ -1, 2},
	{ -2, 2},
	{ -2, 1},
	{ -2, 0},
	{ -2, -1},
	{ -2, -2},
	{ -1, -2}
};

/*
Play exploding zoom animation
*/
void jaggedZoom(byte *source, byte *target) {
	int16 i;
	jpoint_t points[JCOUNT + 1];
	uint16 outside = 0;
	uint16 cycle = 0;
	uint16 choices = 0;

	for (i = 0; i < JCOUNT; i++) {
		points[i].x = 320;
		points[i].y = 200;
	}
	points[i].x = 0;
	points[i].y = 0;

	for (;;) {
		cycle++;
		if (cycle % 8 == 0)
			choices = getRandW();

		for (i = 0; i < JCOUNT; i++) {
			signed short t;
			if (choices & (1 << i)) {
				t = points[i].x + jdeltas[i].x;
				if (t < 0 || t >= 600) { /*TODO: 640?*/
					outside |= 0x8000;  /*TODO: should this mask be rotated?*/
					t = points[i].x;
				}
				points[i].x = t;

				t = points[i].y + jdeltas[i].y;
				if (t < 0 || t >= 400) {
					outside |= 0x8000;
					t = points[i].y;
				}
				points[i].y = t;
			}
		}

		if (outside)
			break;

		for (i = 0; i < JCOUNT; i++) {
			uint16 sx = points[i].x;
			uint16 sy = points[i].y;
			uint16 ex = points[i + 1].x;
			uint16 ey = points[i + 1].y;
			if (ex == 0 && ey == 0) {
				ex = points[0].x;
				ey = points[0].y;
			}
			cga_TraceLine(sx / 2, ex / 2, sy / 2, ey / 2, source, target);
			/*TODO: waitVBlank(); maybe?*/
		}
	}
}

typedef struct star_t {
	uint16 ofs;
	byte pixel;
	byte mask;
	signed short x;
	signed short y;
	uint16 z;
} star_t;

/*
Generate random star
*/
void randomStar(star_t *star) {
	star->x = getRandW();
	star->y = getRandW();
	star->z = getRandW() & 0xFFF;
}

/*
Generate a bunch of random stars
*/
star_t *initStarfield(void) {
	int16 i;
	star_t *stars = (star_t *)scratch_mem2;
	for (i = 0; i < 300; i++) {
		stars[i].ofs = 0;
		stars[i].pixel = 0;
		stars[i].mask = 0;
		randomStar(&stars[i]);
	}
	return stars;
}

/*
Draw a frame of starfield animation and update stars
*/
void drawStars(star_t *stars, int16 iter, byte *target) {
	int16 i;
	/*TODO: bug? initialized 300 stars, but animated only 256?*/
	for (i = 0; i < 256; i++, stars++) {
		short z, x, y;
		byte pixel, mask;

		target[stars->ofs] &= stars->mask;
		if (stars->z < 328) {
			if (iter >= 30) {
				randomStar(stars);
				stars->z |= 0x1800;
			}
			continue;
		}

		stars->z -= 328;
		z = 0xCFFFFul / (stars->z + 16);
		x = ((long)z * stars->x) >> 16;
		y = ((long)z * stars->y) >> 16;

		x += 320 / 2;
		y += 200 / 2;
		if (x < 0 || x >= 320 || y < 0 || y >= 200) {
			stars->z = 0;
			continue;
		}

		stars->ofs = cga_CalcXY(x, y);

		pixel = (stars->z < 0xE00) ? 0xC0 : 0x40;
		pixel >>= (x % 4) * 2;
		mask = 0xC0;
		mask = ~(mask >> (x % 4) * 2);
		stars->pixel = pixel;
		stars->mask = mask;

		target[stars->ofs] &= mask;
		target[stars->ofs] |= pixel;
	}
}

/*
Play starfield animation
*/
void animStarfield(star_t *stars, byte *target) {
	int16 i;
	for (i = 100; i; i--)
		drawStars(stars, i, target);
}

/*
Play Game Over sequence and restart the game
*/
uint16 SCR_26_GameOver(void) {
	IFGM_PlaySample(160);
	in_de_profundis = 0;
	script_byte_vars.game_paused = 1;
	memset(backbuffer, 0, sizeof(backbuffer) - 2);  /*TODO: original bug?*/
	jaggedZoom(backbuffer, frontbuffer);
	cga_BackBufferToRealFull();
	cga_ColorSelect(0x30);
	animStarfield(initStarfield(), frontbuffer);
	playAnim(44, 156 / 4, 95);
	script_byte_vars.zone_index = 135;

	/*reload background*/
	while (!loadFond())
		askDisk2();

	jaggedZoom(backbuffer, frontbuffer);

	cga_BackBufferToRealFull();
	restartGame();
	return 0;
}

/*
Draw all active room's persons
*/
uint16 SCR_4C_DrawPersons(void) {
	script_ptr++;
	drawPersons();
	return 0;
}

/*
Redraw all room's static objects
*/
uint16 SCR_13_RedrawRoomStatics(void) {
	byte index;
	script_ptr++;
	index = *script_ptr++;
	redrawRoomStatics(index, 0);
	return 0;
}

/*
Go to a new zone
If go through a door, play door's opening animation
*/
uint16 SCR_42_LoadZone(void) {
	byte index;

	script_ptr++;
	index = *script_ptr++;

	skip_zone_transition = 0;
	if (right_button)
		script_byte_vars.last_door = 0;
	else {
		if ((script_byte_vars.cur_spot_flags & (SPOTFLG_20 | SPOTFLG_10 | SPOTFLG_8)) == 0)
			script_byte_vars.last_door = script_byte_vars.cur_spot_flags & 7;
		else if ((script_byte_vars.cur_spot_flags & ((SPOTFLG_20 | SPOTFLG_10 | SPOTFLG_8))) == SPOTFLG_8) {
			skip_zone_transition = 1;
			animRoomDoorOpen(script_byte_vars.cur_spot_idx);
			script_byte_vars.last_door = script_byte_vars.cur_spot_flags & 7;
		} else
			script_byte_vars.last_door = 0;
	}
	beforeChangeZone(index);
	changeZone(index);
	script_byte_vars.zone_area_copy = script_byte_vars.zone_area;
	script_byte_vars.cur_spot_idx = findInitialSpot();
	skip_zone_transition |= script_byte_vars.cur_spot_idx;

	drawRoomStatics();

	if (script_byte_vars.bvar_5F != 0) {
		redrawRoomStatics(script_byte_vars.bvar_5F, 0);
		script_byte_vars.bvar_5F = 0;
	}

	backupSpotsImages();
	prepareVorts();
	prepareTurkey();
	prepareAspirant();
	drawPersons();
	script_byte_vars.cur_spot_flags = 0;
	return 0;
}

/*
Draw current sprites
*/
uint16 SCR_59_blitSpritesToBackBuffer(void) {
	script_ptr++;
	blitSpritesToBackBuffer();
	return 0;
}

/*
Apply current palette
*/
uint16 SCR_5A_SelectPalette(void) {
	script_ptr++;
	selectPalette();
	return 0;
}

/*
Apply specific palette
*/
uint16 SCR_5E_SelectTempPalette(void) {
	byte index;
	script_ptr++;
	index = *script_ptr++;
	selectSpecificPalette(index);
	return 0;
}

/*
Draw new zone
*/
uint16 SCR_43_RefreshZone(void) {
	script_ptr++;
	refreshZone();
	return 0;
}

/*
Go to new zone and draw it
*/
uint16 SCR_36_ChangeZone(void) {
	SCR_42_LoadZone();
	refreshZone();
	return 0;
}

/*
Draw a static sprite in the room
*/
void SCR_DrawRoomObjectBack(byte *x, byte *y, byte *w, byte *h) {
	byte obj[3];

	script_ptr++;
	obj[0] = *script_ptr++; /*spr*/
	obj[1] = *script_ptr++; /*x*/
	obj[2] = *script_ptr++; /*y*/

	drawRoomStaticObject(obj, x, y, w, h);
}

/*
Draw a static sprite in the room (to backbuffer)
*/
uint16 SCR_5F_DrawRoomObjectBack(void) {
	byte x, y, w, h;
	SCR_DrawRoomObjectBack(&x, &y, &w, &h);
	return 0;
}

/*
Display a static sprite in the room (to screen)
*/
uint16 SCR_11_DrawRoomObject(void) {
	byte x, y, w, h;
	SCR_DrawRoomObjectBack(&x, &y, &w, &h);
	cga_CopyScreenBlock(backbuffer, w, h, frontbuffer, cga_CalcXY_p(x, y));
	return 0;
}

/*
Draw box with item sprite and its name
*/
uint16 SCR_3_DrawItemBox(void) {
	byte current;

	item_t *item;
	byte x, y;
	byte *msg;

	script_ptr++;
	current = *script_ptr++;

	if (current)
		item = (item_t *)script_vars[kScrPool3_CurrentItem];
	else
		item = &inventory_items[aspirant_ptr->item - 1];

	x = dirty_rects[0].x;
	y = dirty_rects[0].y + 70;
	msg = seekToString(desci_data, 274 + item->name);

	desciTextBox(x, y, 18, msg);
	drawSpriteN(item->sprite, x, y + 1, frontbuffer);

	return 0;
}

/*
Draw simple bubble with text
*/
uint16 SCR_37_desciTextBox(void) {
	byte x, y, w;
	byte *msg;
	script_ptr++;
	msg = seekToStringScr(desci_data, *script_ptr, &script_ptr);
	script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;
	w = *script_ptr++;
	desciTextBox(x, y, w, msg);
	return 0;
}


/*
Play portrait animation
*/
uint16 SCR_18_AnimPortrait(void) {
	byte layer, index, delay;
	script_ptr++;

	layer = *script_ptr++;
	index = *script_ptr++;
	delay = *script_ptr++;

	animPortrait(layer, index, delay);

	return 0;
}

/*
Play animation
*/
uint16 SCR_38_PlayAnim(void) {
	byte index, x, y;
	script_ptr++;
	index = *script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;
	playAnim(index, x, y);
	return 0;
}

/*
Pop up the actions menu and handle its commands
*/
uint16 SCR_3D_ActionsMenu(void) {
	uint16 cmd;

	byte *old_script = script_ptr;
	byte *old_script_end = script_end_ptr;

	act_menu_x = 0xFF;

	for (;;) {
		script_ptr++;
		actionsMenu(&script_ptr);
		if (the_command == 0xFFFF)
			break;

		cmd = the_command & 0xF000;
		if (cmd == 0xC000 || cmd == 0xA000) {
			return ScriptRerun;
		}

		runCommand();

		script_byte_vars.used_commands++;
		if (script_byte_vars.bvar_43 == 0 && script_byte_vars.used_commands > script_byte_vars.check_used_commands) {
			the_command = Swap16(script_word_vars.next_aspirant_cmd);
			if (the_command)
				return ScriptRerun;
		}

		script_ptr = old_script;
		if (--script_byte_vars.tries_left == 0)
			resetAllPersons();
	}

	script_end_ptr = old_script_end;
	return ScriptContinue;
}

/*
The Wall room puzzle
*/
uint16 SCR_3E_TheWallAdvance(void) {
	script_ptr++;

	IFGM_PlaySample(29);
	script_byte_vars.the_wall_phase = (script_byte_vars.the_wall_phase + 1) % 4;
	switch (script_byte_vars.the_wall_phase) {
	default:
		theWallPhase3_DoorOpen1();
		break;
	case 0:
		theWallPhase0_DoorOpen2();
		break;
	case 1:
		theWallPhase1_DoorClose1();
		break;
	case 2:
		theWallPhase2_DoorClose2();
		break;
	}

	return 0;
}

/*
When playing cups with proto
*/
uint16 SCR_28_MenuLoop(void) {
	byte cursor;
	byte mask, value;

	script_ptr++;
	cursor = *script_ptr++;
	mask = *script_ptr++;
	value = *script_ptr++;

	selectCursor(cursor);

	menuLoop(mask, value);

	return 0;
}


/*
Restore screen data from back buffer as specified by dirty rects of specified index
*/
uint16 SCR_2A_PopDialogRect(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs); /*TODO: implicit target*/
	cga_CopyScreenBlock(backbuffer, 2, 21, CGA_SCREENBUFFER, offs = (x << 8) | y);  /*TODO: implicit target*/

	cur_dlg_index = 0;

	return 0;
}

/*
Restore screen data from back buffer as specified by dirty rect of kind dialog bubble
*/
uint16 SCR_2B_PopAllBubbles(void) {
	script_ptr++;
	popDirtyRects(DirtyRectBubble);
	return 0;
}

/*
Hide a portrait, with shatter effect
*/
uint16 SCR_22_HidePortraitShatter(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	cga_HideShatterFall(CGA_SCREENBUFFER, backbuffer, width, height, CGA_SCREENBUFFER, offs);

	return 0;
}

/*
Hide a portrait, no special effects
*/
uint16 SCR_23_HidePortrait(void) {
	byte index;
	byte kind;
	byte x, y;
	byte width, height;
	uint16 offs;

	script_ptr++;
	index = *script_ptr++;

	getDirtyRectAndFree(index, &kind, &x, &y, &width, &height, &offs);
	if (right_button) {
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
		return 0;
	}

	cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);

	return 0;
}

/*
Hide a portrait, no special effects
*/
uint16 SCR_1D_HidePortrait(void) {
	return SCR_23_HidePortrait();
}

/*
Hide a portrait, no special effects
*/
uint16 SCR_21_HidePortrait(void) {
	return SCR_23_HidePortrait();
}

/*
Hide a portrait, no special effects
*/
uint16 SCR_3F_HidePortrait(void) {
	return SCR_23_HidePortrait();
}

/*
Restore screen data from back buffer for all portraits
*/
uint16 SCR_24_PopAllPortraits(void) {
	script_ptr++;
	popDirtyRects(DirtyRectSprite);
	return 0;
}

/*
Restore screen data from back buffer for all text bubbles
*/
uint16 SCR_40_PopAllTextBoxes() {
	script_ptr++;
	popDirtyRects(DirtyRectText);
	return 0;
}

/*
Draw updated Hands in Who Will Be Saved
*/
uint16 SCR_41_LiftHand(void) {
	script_ptr++;
	redrawRoomStatics(92, script_byte_vars.hands);
	cga_BackBufferToRealFull();
	playSound(31);
	return 0;
}

byte fight_mode = 0;

uint16 SCR_30_Fight(void) {
	static byte player_image[] = {26, 0, 0};
	byte *image = player_image;

	byte x, y, width, height, kind;
	uint16 offs;
	byte *old_script, *old_script_end = script_end_ptr;
	pers_t *pers = (pers_t *)(script_vars[kScrPool8_CurrentPers]);

	byte strenght, win, rnd;

	script_ptr++;
	old_script = script_ptr;

	x = 140 / 4;
	y = 20;

	fight_mode = 1;

	if (pers->name != 44) {	/*VORT*/
		if (next_vorts_cmd == 0xA015) {
			the_command = 0xA015;
			runCommand();
			selectPerson(PersonOffset(pers - pers_list));
		}
		if (Swap16(script_word_vars.next_aspirant_cmd) == 0xC357) {
			the_command = 0xC357;
			runCommand();
		}

		pers = (pers_t *)(script_vars[kScrPool8_CurrentPers]);
		if (pers->name != 56 && pers->name != 51) {	/*MONKEY, TURKEY*/
			x = dirty_rects[0].x + 64 / 4;
			y = dirty_rects[0].y;
			fight_mode = 0;
		}
	}

	/*draw player portrait*/
	player_image[1] = x;
	player_image[2] = y;
	if (drawPortrait(&image, &x, &y, &width, &height))
		cga_AnimLiftToLeft(width, cur_image_pixels, width, 1, height, CGA_SCREENBUFFER, cga_CalcXY_p(x + width - 1, y));

	blinkToWhite();

	if (pers->name != 44 && pers->name != 56 && pers->name != 51) {	/*VORT, MONKEY, TURKEY*/
		getDirtyRectAndFree(1, &kind, &x, &y, &width, &height, &offs);
		cga_CopyScreenBlock(backbuffer, width, height, CGA_SCREENBUFFER, offs);
	}

	/*check fight outcome*/

	strenght = 0;

	script_byte_vars.fight_status = 0;

	if (script_byte_vars.extreme_violence == 0) {
		static byte character_strenght[] = {
			1,	/*THE MASTER OF ORDEALS*/
			3,	/*PROTOZORQ*/
			1,	/*VORT*/
			1,	/*THE POORMOUTH*/
			1,	/*KHELE*/
			1,  /*THE MISTRESS*/
			5,	/*DEILOS*/
			3,	/*ASPIRANT*/
			2,	/*DIVO*/
			1,	/*TURKEY*/
			1,	/*PRIESTESS*/
			1,	/*SCI FI*/
			1,	/*NORMAJEEN*/
			1,	/*ASH*/
			1,	/*MONKEY*/
			1,	/*HARSSK*/
			1	/*ZORQ*/
		};

		strenght = character_strenght[pers->name - 42];

		/*check if can decrease*/
		if (strenght != 1 && (pers->flags & PERSFLG_80))
			strenght--;

		if (script_byte_vars.zapstiks_owned != 0 || script_byte_vars.bvar_66 != 0)
			strenght--;
	}

	/*check if can increase*/
	if (strenght != 5) {
		if ((pers->item >= kItemDagger1 && pers->item <= kItemDagger4)
		        || (pers->item >= kItemZapstik1 && pers->item <= kItemZapstik13)	/*TODO: ignore kItemZapstik14?*/
		        || pers->item == kItemBlade || pers->item == kItemChopper
		        || ((pers->index >> 3) == 6))
			strenght++;
	}

	/*
	win flags:
	   1 - player win
	   2 - player lose
	   4 - "YOU RUN..."
	   8 - "THE ASPIRANT STEALS EVERYTHING YOU HAVE!"
	0x20 - "TU NUH RAY VUN IN FAY VRABLE SIT YOU AISHUN."
	0x40 - "OUT KUM UNSER TUN."
	0x80 - "SIT YOU ASHUN KRITI KAL FOR TOONUH RAY VUN."
	*/

	win = 1;
	rnd = script_byte_vars.rand_value;

#ifdef CHEAT
	strenght = 1;
#endif

	if (strenght >= 2) {
		if (strenght == 2) {
			if (rnd >= 205)
				win = getRand() < 128 ? (0x40 | 0x10 | 1) : (0x40 | 0x10 | 2);
		} else if (strenght == 4 && rnd < 100) {
			win = getRand() < 128 ? (0x40 | 0x10 | 1) : (0x40 | 0x10 | 2);
		} else {
			win = 2;
			if (strenght == 3) {
				if (rnd < 128)  /*TODO: check me, maybe original bug (checks against wrong reg?)*/
					win = getRand() < 51 ? (0x80 | 0x10 | 1) : (0x80 | 0x10 | 2);
				else
					win = getRand() < 205 ? (0x20 | 0x10 | 1) : (0x20 | 0x10 | 2);
			}
		}
	}

	script_byte_vars.fight_status = win;

	script_ptr = old_script;
	script_end_ptr = old_script_end;
	return 0;
}

byte prev_fight_mode = 0;
uint16 fight_pers_ofs = 0;

typedef struct fightentry_t {
	byte   room;
	animdesc_t      anim;
} fightentry_t;

fightentry_t fightlist1[] = {
	{50, {47, {{36, 153}}}},
	{51, {47, {{36, 153}}}},
	{53, {47, {{37, 160}}}},
	{54, {47, {{36, 153}}}},
	{56, {47, {{31, 128}}}},
	{57, {47, {{27, 161}}}},
	{58, {47, {{28, 152}}}},
	{59, {47, {{25, 153}}}},
	{60, {47, {{22, 155}}}},
	{61, {47, {{27, 160}}}}
};

fightentry_t fightlist2[] = {
	{ 1, {24, {{42, 128}}}},
	{ 2, {24, {{44, 126}}}},
	{ 3, {24, {{47, 126}}}},
	{ 4, {24, {{44, 126}}}},
	{ 5, {24, {{47, 126}}}},
	{ 6, {24, {{28, 126}}}},
	{ 7, {24, {{55, 126}}}},
	{ 8, {24, {{49, 126}}}},
	{10, {24, {{41, 147}}}},
	{11, {24, {{41, 147}}}},
	{18, {24, {{41, 147}}}},
	{19, {24, {{41, 147}}}},
	{90, {24, {{44, 121}}}},
	{91, {28, {{24, 123}}}},
	{12, {24, {{41, 147}}}},
	{13, {24, {{41, 147}}}},
	{35, {24, {{39, 147}}}},
	{42, {24, {{39, 147}}}},
	{50, {55, {{46, 130}}}},
	{52, {24, {{42, 121}}}},
	{54, {55, {{46, 130}}}},
	{61, {67, {{37, 125}}}},
	{62, {55, {{32, 133}}}},
	{63, {55, {{32, 133}}}},
	{64, {55, {{32, 133}}}},
	{65, {55, {{32, 133}}}}
};

fightentry_t fightlist3[] = {
	{ 2, {25, {{35, 144}}}},
	{ 3, {25, {{38, 144}}}},
	{ 4, {25, {{35, 144}}}},
	{ 5, {25, {{38, 144}}}},
	{ 6, {25, {{19, 144}}}},
	{ 7, {25, {{46, 144}}}},
	{ 8, {26, {{64, 132}}}},
	{10, {25, {{32, 165}}}},
	{11, {25, {{32, 165}}}},
	{12, {25, {{32, 165}}}},
	{13, {25, {{32, 165}}}},
	{18, {25, {{32, 165}}}},
	{19, {25, {{32, 165}}}},
	{90, {36, {{27, 127}}}},
	{91, {27, {{44, 123}}}},
	{35, {25, {{30, 165}}}},
	{42, {25, {{30, 165}}}},
	{50, {56, {{36, 153}}}},
	{54, {56, {{36, 153}}}},
	{62, {56, {{22, 156}}}},
	{63, {56, {{22, 156}}}},
	{64, {56, {{22, 156}}}},
	{65, {56, {{22, 156}}}}
};

/*Draw defeated enemy*/
uint16 SCR_31_Fight2(void) {
	script_ptr++;

	if (script_byte_vars.bvar_43 != 18) {
		pers_t *pers = (pers_t *)(script_vars[kScrPool8_CurrentPers]);
		fight_pers_ofs = (byte *)pers - (byte *)pers_list; /*TODO check size*/
		pers->flags |= PERSFLG_40;
		pers->area = 0;
		found_spot->flags &= ~SPOTFLG_80;
		if (pers->index == 16) {	/*Vort trio*/
			pers_list[kPersVort2].area = script_byte_vars.zone_area;
			pers_list[kPersVort2].flags = pers->flags;
			if (script_byte_vars.zapstiks_owned == 0) {
				static const animdesc_t anim19 = {ANIMFLG_USESPOT | 19, { { 0, 0 } }};
				animateSpot(&anim19);
			}
			the_command = next_vorts_cmd;
			runCommand();
		} else if (pers->index == 8) {	/*Vort duo*/
			pers_list[kPersVort3].area = script_byte_vars.zone_area;
			pers_list[kPersVort3].flags = pers->flags;
			if (script_byte_vars.zapstiks_owned == 0) {
				static const animdesc_t anim20 = {ANIMFLG_USESPOT | 20, { { 0, 0 } }};
				animateSpot(&anim20);
			}
			the_command = next_vorts_cmd;
			runCommand();
		} else {
			if (prev_fight_mode == 0
			        && script_byte_vars.zapstiks_owned != 0
			        && fight_mode == 0) {
				script_byte_vars.fight_status &= ~1;
			} else {
				uint16 i;
				fightentry_t *fightlist;
				uint16 fightlistsize;
				byte animidx;

				prev_fight_mode = 0;
				switch (pers->name) {
				case 56:	/*MONKEY*/
					animidx = 47;
					fightlist = fightlist1;
					fightlistsize = 10;
					break;
				case 51:	/*TURKEY*/
					next_turkey_cmd = 0;
					animidx = 66;
					fightlist = fightlist1;
					fightlistsize = 10;
					break;
				default:
					animidx = 0;
					fightlist = fightlist2;
					fightlistsize = 26;
				}

				for (i = 0; i < fightlistsize; i++) {
					if (fightlist[i].room == script_byte_vars.zone_room) {
						if (animidx != 0) {
							fightlist[i].anim.index = animidx;
							IFGM_PlaySample(150);
						}
						if (fightlist[i].anim.index == 55)
							playSound(151);
						playAnim(fightlist[i].anim.index, fightlist[i].anim.params.coords.x, fightlist[i].anim.params.coords.y);
						break;
					}
				}
			}
		}
	}

	return 0;
}

void FightWin(void) {
	script_byte_vars.bvar_67 = 0;

	if (script_byte_vars.bvar_43 != 18 && *spot_sprite != 0) {
		cga_RestoreImage(*spot_sprite, frontbuffer);
		cga_RestoreImage(*spot_sprite, backbuffer);

		if (script_byte_vars.extreme_violence == 0
		        && script_byte_vars.bvar_60 == 0
		        && script_byte_vars.zapstiks_owned != 0
		        && fight_mode == 0) {
			script_byte_vars.bvar_67 = 1;
			playSound(149);
			playAnim(40, found_spot->sx, found_spot->sy);
		}
	}

	prev_fight_mode = script_byte_vars.extreme_violence;
	script_byte_vars.extreme_violence = 0;
}

uint16 SCR_32_FightWin(void) {

	script_ptr++;

	FightWin();

	return 0;
}

void DrawDeathAnim(void) {
	int16 i;

	/*remove existing cadaver if any*/
	if (selectPerson(PersonOffset(kPersCadaver))) {
		found_spot->flags &= ~SPOTFLG_80;
		cga_RestoreImage(*spot_sprite, backbuffer);
	}

	for (i = 0; i < 23; i++) {
		if (fightlist3[i].room == script_byte_vars.zone_room) {
			playAnim(fightlist3[i].anim.index, fightlist3[i].anim.params.coords.x, fightlist3[i].anim.params.coords.y);
			break;
		}
	}
}

uint16 SCR_16_DrawDeathAnim(void) {
	script_ptr++;

	DrawDeathAnim();

	return 0;
}

uint16 SCR_60_ReviveCadaver(void) {
	pers_t *pers;
	script_ptr++;

	blitSpritesToBackBuffer();

	selectPerson(PersonOffset(kPersCadaver));

	script_byte_vars.bvar_60 = 1;
	FightWin();
	script_byte_vars.bvar_60 = 0;
	pers_list[kPersCadaver].area = 0;

	selectPerson(fight_pers_ofs);
	zone_spots[5].flags = SPOTFLG_40 | SPOTFLG_10 | SPOTFLG_2 | SPOTFLG_1;
	found_spot->flags |= SPOTFLG_80;

	pers = (pers_t *)script_vars[kScrPool8_CurrentPers];
	pers->flags &= ~PERSFLG_40;
	pers->area = script_byte_vars.zone_area;

	drawPersons();
	cga_BackBufferToRealFull();

	return 0;
}


uint16 SCR_57_ShowCharacterSprite(void) {
	byte index, x, y;

	script_ptr++;
	index = *script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;

	drawCharacterSprite(index, x, y, frontbuffer);

	return 0;
}

uint16 SCR_58_DrawCharacterSprite(void) {
	byte index, x, y;

	script_ptr++;
	index = *script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;

	drawCharacterSprite(index, x, y, backbuffer);

	return 0;
}

extern void exitGame(void);

uint16 SCR_15_SelectSpot(void) {
	byte mask, index;

	script_ptr++;
	mask = *script_ptr++;
	index = *script_ptr++;
	if (mask != 0) {
		index = findSpotByFlags(mask, index);   /*TODO: return 0 if not found?*/
		if (index == 0xFF) {
			TODO("ERROR: SelectSpot: spot not found");
			exitGame(); /*hard abort*/
		}
	}
	found_spot = &zone_spots[index - 1];
	script_byte_vars.cur_spot_idx = index;
	spot_sprite = &sprites_list[index - 1];

	findPerson();

	if (script_byte_vars.cur_pers == 0)
		script_vars[kScrPool8_CurrentPers] = &pers_list[kPersProtozorq12];

	return 0;
}

uint16 SCR_44_BackBufferToScreen(void) {
	script_ptr++;
	cga_BackBufferToRealFull();
	return 0;
}

/*
Animate De Profundis room on entry
*/
uint16 SCR_45_DeProfundisRoomEntry(void) {
	uint16 w, h;
	uint16 sprofs, ofs;

	script_ptr++;

	/*draw Platform*/
	sprofs = getPuzzlSprite(3, 140 / 4, 174, &w, &h, &ofs);
	cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);

	/*draw Granite Monster*/
	sprofs = getPuzzlSprite(119, 128 / 4, 94, &w, &h, &ofs);
	cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);

	promptWait();

	for (; h; h--) {
		waitVBlank();
		waitVBlank();
		cga_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	cga_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

	return 0;
}

/*
Animate De Profundis hook (lower)
*/
uint16 SCR_46_DeProfundisLowerHook(void) {
	byte y;
	uint16 w, h;
	uint16 sprofs, ofs;

	script_ptr++;

	/*draw Hook*/
	sprofs = getPuzzlSprite(96, 140 / 4, 18, &w, &h, &ofs);

	h = 1;
	y = 15;
	sprofs = y * 20 / 4 * 2;    /*TODO: 20 is the sprite width. replace with w?*/

	for (; y; y--) {
		waitVBlank();
		cga_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

		h++;
		sprofs -= 20 / 4 * 2;

		cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	return 0;
}

/*
Animate De Profundis monster (rise)
*/
uint16 SCR_47_DeProfundisRiseMonster(void) {
	byte y;
	uint16 w, h;
	uint16 sprofs, ofs;

	script_ptr++;

	/*draw Granite Monster head*/
	sprofs = getPuzzlSprite(118, 112 / 4, 174, &w, &h, &ofs);

	h = 1;
	y = 68;

	for (; y; y--) {
		waitVBlank();

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) != 0)
			ofs -= CGA_BYTES_PER_LINE;

		h++;

		cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	return 0;
}

/*
Animate De Profundis monster (lower)
*/
uint16 SCR_48_DeProfundisLowerMonster(void) {
	byte y;
	uint16 w, h;
	uint16 sprofs, ofs;

	script_ptr++;

	/*draw Hook*/
	sprofs = getPuzzlSprite(118, 112 / 4, 106, &w, &h, &ofs);

	y = 34;

	for (; y; y--) {
		waitVBlank();
		cga_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		h--;
		cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	return 0;
}

/*
Animate De Profundis hook (rise)
*/
uint16 SCR_49_DeProfundisRiseHook(void) {
	byte y;
	uint16 w, h;
	uint16 sprofs, ofs;

	script_ptr++;

	/*draw Hook*/
	sprofs = getPuzzlSprite(96, 140 / 4, 18, &w, &h, &ofs);

	h = 16;
	y = 15;

	for (; y; y--) {
		waitVBlank();
		cga_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

		h--;
		sprofs += 20 / 4 * 2;

		cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	cga_BlitFromBackBuffer(w, 1, CGA_SCREENBUFFER, ofs);

	return 0;
}

/*
Animate De Profundis platform
*/
uint16 SCR_65_DeProfundisMovePlatform(void) {
	byte state;
	byte x, y;
	uint16 w, h;
	uint16 sprofs, ofs;

	script_ptr++;
	state = *script_ptr++;

	x = 140 / 4;
	y = 174;
	if (state != 0)
		y += 4;

	/*draw Platform*/
	sprofs = getPuzzlSprite(3, x, y, &w, &h, &ofs);

	y = 4;
	if (state) {
		h -= 4;
		y--;
	}

	for (; y; y--) {
		waitVBlank();
		cga_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) == 0)
			ofs += CGA_BYTES_PER_LINE;

		h--;

		cga_BlitScratchBackSprite(sprofs, w, h, CGA_SCREENBUFFER, ofs);
	}

	if (state)
		cga_BlitFromBackBuffer(w, h, CGA_SCREENBUFFER, ofs);

	return 0;
}

/*
Animate De Profundis monster ride to exit door
*/
uint16 SCR_66_DeProfundisRideToExit(void) {
	uint16 w, h;
	uint16 sprofs, ofs;

	script_ptr++;

	/*draw Granite Monster*/
	sprofs = getPuzzlSprite(119, 128 / 4, 139, &w, &h, &ofs);

	cga_BlitScratchBackSprite(sprofs, w, 20, backbuffer, ofs);

	dot_effect_delay = 1;
	dot_effect_step = 17;
	copyScreenBlockWithDotEffect(backbuffer, 112 / 4, 139, 72 / 4, 40, frontbuffer);

	return 0;
}

/*
Draw item bounce to room objects animation
*/
uint16 SCR_4E_BounceCurrentItemToRoom(void) {
	script_ptr++;
	bounceCurrentItem(ITEMFLG_ROOM, 43);
	return 0;
}

/*
Draw item bounce to inventory animation
*/
uint16 SCR_4F_BounceCurrentItemToInventory(void) {
	script_ptr++;
	bounceCurrentItem(ITEMFLG_OWNED, 85);
	return 0;
}

/*
Draw item bounce to inventory animation
*/
uint16 SCR_50_BounceItemToInventory(void) {
	byte itemidx;

	script_ptr++;
	itemidx = *script_ptr++;
	script_vars[kScrPool3_CurrentItem] = &inventory_items[itemidx - 1];

	bounceCurrentItem(ITEMFLG_OWNED, 85);
	return 0;
}

/*
Take away Protozorq's zapstik and bounce it to room
*/
uint16 SCR_4B_ProtoDropZapstik(void) {
	pers_t *pers = (pers_t *)(script_vars[kScrPool8_CurrentPers]);

	script_ptr++;

	if ((pers->index & 0x38) != 0x30)
		return 0;

	pers->index &= ~0x18;

	script_vars[kScrPool3_CurrentItem] = &inventory_items[kItemZapstik1 - 1 + (script_byte_vars.cur_pers - 1) - kPersProtozorq1];

	bounceCurrentItem(ITEMFLG_ROOM, 43);

	return 0;
}

/*
Take away Aspirant's item and bounce it to the inventory
*/
void LootAspirantsItem(void) {
	if (aspirant_ptr->item != 0) {
		item_t *item = &inventory_items[aspirant_ptr->item - 1];
		aspirant_ptr->item = 0;

		script_vars[kScrPool3_CurrentItem] = item;
		script_byte_vars.steals_count++;
		script_byte_vars.bvar_6D[aspirant_ptr->index >> 6] = item->name; /*TODO: check these index bits*/
		bounceCurrentItem(ITEMFLG_OWNED, 85);
		the_command = 0x90AA;   /*OK*/
	} else
		the_command = 0x9140;   /*NOTHING ON HIM*/
}

/*
Take away Aspirant's item and bounce it to the inventory
*/
uint16 SCR_2F_LootAspirantsItem() {
	script_ptr++;
	LootAspirantsItem();
	return ScriptRerun;
}

/*
Trade with Skull Trader
*/
uint16 SCR_51_SkullTraderItemTrade(void) {
	byte *old_script, *old_script_end = script_end_ptr;
	byte status;

	if (script_byte_vars.bvar_26 >= 63)  /*TODO: hang?*/
		return 0;

	script_ptr++;
	old_script = script_ptr;

	inv_bgcolor = 0xFF;
	openInventory(0xFF, ITEMFLG_OWNED);

	status = 1;
	if (inv_count != 0) {
		status = 2;
		if (the_command != 0) {
			status = 3;
			if (script_byte_vars.inv_item_index >= kItemRope1 && script_byte_vars.inv_item_index <= kItemLantern4) {
				the_command = 0xC204;	/*WHICH ONE DO YOU WANT?*/
				runCommand();

				((item_t *)(script_vars[kScrPool3_CurrentItem]))->flags = 0;

				openInventory(0xFF, ITEMFLG_TRADER);

				status = 4;
				if (the_command != 0) {
					/*50% chance to win the item*/
					status = 5;	/*lose*/
#ifdef CHEAT_TRADER
					{	/*always win at the Skull Trader*/
#else
					if (script_byte_vars.rand_value < 128) {
#endif
						status = 6;	/*win*/
						((item_t *)(script_vars[kScrPool3_CurrentItem]))[-1].flags = ITEMFLG_TRADER; /*offer previous item copy for next trade*/
						((item_t *)(script_vars[kScrPool3_CurrentItem]))->flags = 0;	/*consume selected item*/
					}
				}
			}
		}
	}

	script_byte_vars.skull_trader_status = status;

	script_ptr = old_script;
	script_end_ptr = old_script_end;

	return 0;
}

uint16 SCR_52_RefreshSpritesData(void) {
	script_ptr++;
	refreshSpritesData();
	return 0;
}

uint16 SCR_53_FindInvItem(void) {
	byte first, count, flags, i;
	item_t *item;
	script_ptr++;
	first = *script_ptr++;
	count = *script_ptr++;
	flags = *script_ptr++;
	item = &inventory_items[first - 1];
	for (i = 0; i < count; i++) {
		if (item[i].flags == flags) {
			script_vars[kScrPool3_CurrentItem] = &item[i];
			return 0;
		}
	}
	script_vars[kScrPool3_CurrentItem] = &item[count - 1];
	return 0;
}

/*
Restore whole room from backbuffer, with dot effect
*/
uint16 SCR_54_DotFadeRoom(void) {
	script_ptr++;

	dot_effect_delay = 1;
	dot_effect_step = 17;
	copyScreenBlockWithDotEffect(backbuffer, room_bounds_rect.sx, room_bounds_rect.sy, room_bounds_rect.ex - room_bounds_rect.sx, room_bounds_rect.ey - room_bounds_rect.sy, frontbuffer);

	return 0;
}

uint16 SCR_55_DrawRoomItemsIndicator(void) {
	script_ptr++;
	drawRoomItemsIndicator();
	return 0;
}

/*
TODO: check and rename me
*/
uint16 SCR_56_MorphRoom98(void) {
	int16 h;
	uint16 ofs;
	script_ptr++;

	IFGM_PlaySample(242);

	redrawRoomStatics(98, 0);

	ofs = cga_CalcXY(0, 136);
	for (h = 60; h; h--) {
		memcpy(frontbuffer + ofs, backbuffer + ofs, CGA_BYTES_PER_LINE);
		waitVBlank();
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) != 0)
			ofs -= CGA_BYTES_PER_LINE;
	}

	backupSpotImage(&zone_spots[3], &sprites_list[3], sprites_list[3]);

	IFGM_StopSample();

	return 0;
}

/*
Copy backbuffer to screen, with added vertical mirror
*/
void ShowMirrored(uint16 h, uint16 ofs) {
	uint16 x, ofs2 = ofs;

	/*move 1 line up*/
	ofs2 ^= CGA_ODD_LINES_OFS;
	if ((ofs2 & CGA_ODD_LINES_OFS) != 0)
		ofs2 -= CGA_BYTES_PER_LINE;

	while (h--) {

		for (x = 0; x < CGA_BYTES_PER_LINE; x++) {
			frontbuffer[ofs2 + x] = frontbuffer[ofs + x] = backbuffer[ofs + x];
			backbuffer[ofs + x] = 0;
		}

		/*move 1 line down*/
		ofs += CGA_BYTES_PER_LINE;
		ofs ^= CGA_ODD_LINES_OFS;
		if ((ofs & CGA_ODD_LINES_OFS) != 0)
			ofs -= CGA_BYTES_PER_LINE;

		/*move 1 line up*/
		ofs2 ^= CGA_ODD_LINES_OFS;
		if ((ofs2 & CGA_ODD_LINES_OFS) != 0)
			ofs2 -= CGA_BYTES_PER_LINE;
	}
}

void LiftLines(int16 n, byte *source, uint16 sofs, byte *target, uint16 tofs) {
	while (n--) {
		memcpy(target + tofs, source + sofs, CGA_BYTES_PER_LINE);

		sofs += CGA_BYTES_PER_LINE;
		sofs ^= CGA_ODD_LINES_OFS;
		if ((sofs & CGA_ODD_LINES_OFS) != 0)
			sofs -= CGA_BYTES_PER_LINE;

		tofs += CGA_BYTES_PER_LINE;
		tofs ^= CGA_ODD_LINES_OFS;
		if ((tofs & CGA_ODD_LINES_OFS) != 0)
			tofs -= CGA_BYTES_PER_LINE;
	}
}

#define kSaucerAnimFrames 53

static void AnimSaucer(void) {
	static byte image1[] = {167, 0, 146};
	byte *pimage1 = image1;
	byte *sequence = souco_data;
	byte x, y, width, height;
	uint16 xx, yy, ww, hh;
	byte height_new, height_prev;
	uint16 delay;
	byte scroll_done = 0;

	memset(backbuffer, 0, sizeof(backbuffer) - 2);  /*TODO: original bug?*/
	cga_BackBufferToRealFull();
	cga_ColorSelect(0x30);

	right_button = 0;
	if (!drawPortrait(&pimage1, &x, &y, &width, &height))
		return;

	height_prev = 200 - 1;
	delay = 10000;

	xx = x; /*TODO: is it ok? maybe need *4*/
	yy = y;
	ww = 254;
	hh = 107;

	for (; sequence < souco_data + kSaucerAnimFrames * 8; sequence += 8) {
		uint16 i, ofs, ofs2, baseofs;

		if (sequence != souco_data) {
			/*reuse portrait's params for initial state*/
			xx = (sequence[0] << 8) | sequence[1];
			yy = (sequence[2] << 8) | sequence[3];
			ww = (sequence[4] << 8) | sequence[5];
			hh = (sequence[6] << 8) | sequence[7];
		}

		hh >>= 1;

		height_new = yy + hh;
		height_prev -= (yy - 1);

		/*scale the saucer*/
		cga_ZoomInplaceXY(cur_image_pixels, width, height, ww, hh, xx, yy, backbuffer);

		baseofs = cga_CalcXY(0, yy);

		if (!scroll_done) {
			/*scroll the saucer*/
			scroll_done = 1;

			ofs2 = ofs = baseofs;

			/*previous line*/
			ofs ^= CGA_ODD_LINES_OFS;
			if ((ofs & CGA_ODD_LINES_OFS) != 0)
				ofs -= CGA_BYTES_PER_LINE;

			for (i = 0; i < 55; i++) {
				memcpy(backbuffer + ofs, backbuffer + ofs2, CGA_BYTES_PER_LINE);

				/*next line*/
				ofs2 += CGA_BYTES_PER_LINE;
				ofs2 ^= CGA_ODD_LINES_OFS;
				if ((ofs2 & CGA_ODD_LINES_OFS) != 0)
					ofs2 -= CGA_BYTES_PER_LINE;

				/*previous line line*/
				ofs ^= CGA_ODD_LINES_OFS;
				if ((ofs & CGA_ODD_LINES_OFS) != 0)
					ofs -= CGA_BYTES_PER_LINE;
			}

			ofs2 = cga_CalcXY(0, 200 - 1);

			for (i = 0; i < 108; i++) {
				LiftLines(i + 1, backbuffer, ofs, frontbuffer, ofs2);

				ofs2 ^= CGA_ODD_LINES_OFS;
				if ((ofs2 & CGA_ODD_LINES_OFS) != 0)
					ofs2 -= CGA_BYTES_PER_LINE;

				waitVBlank();
				waitVBlank();
			}

			/*wipe 56 lines*/
			memset(backbuffer + ofs2, 0, 56 / 2 * CGA_BYTES_PER_LINE);
			ofs2 ^= CGA_ODD_LINES_OFS;
			if ((ofs2 & CGA_ODD_LINES_OFS) == 0)
				ofs2 += CGA_BYTES_PER_LINE;
			memset(backbuffer + ofs2, 0, 54 / 2 * CGA_BYTES_PER_LINE);

			for (i = 0xFFFF; i--;) ; /*TODO: weak delay*/

			IFGM_PlaySample(240);
		}

		/*draw the full saucer on screen*/
		ShowMirrored(height_prev + 1, baseofs);
		height_prev = height_new;

		waitVBlank();
		for (i = delay; i--;) ; /*TODO: weak delay*/
		delay += 500;
	}
};

extern int16 loadSplash(const char *filename);

/*
TODO: check me
*/
void theEnd(void) {
	static byte image2[] = {168, 28, 85, 22, 15};
	byte *pimage2 = image2;

	AnimSaucer();

	if (g_vm->getLanguage() == Common::EN_USA) {
		drawPortraitZoomed(&pimage2);

		script_byte_vars.zone_index = 135;

		do {
			pollInputButtonsOnly();
		}
		while(buttons == 0);

		while (!loadFond())
			askDisk2();
		jaggedZoom(backbuffer, frontbuffer);
		cga_BackBufferToRealFull();
	} else {
		while (!loadSplash("PRES.BIN"))
			askDisk2();
		cga_BackBufferToRealFull();
	}
}

uint16 SCR_5B_TheEnd(void) {
	script_ptr++;   /*Useless since this handler never returns*/
	script_byte_vars.game_paused = 5;

	theEnd();

	if (g_vm->getLanguage() == Common::EN_USA)
		restartGame();
	else
		for (;;) ;  /*HANG*/

	return 0;
}


/*
Discard all inventory items
*/
uint16 SCR_5C_ClearInventory(void) {
	int16 i;
	script_ptr++;

	for (i = 0; i < MAX_INV_ITEMS; i++) {
		if (inventory_items[i].flags == ITEMFLG_OWNED)
			inventory_items[i].flags = 0;
	}

	script_byte_vars.zapstiks_owned = 0;

	return 0;
}

/*
Drop group of items from inventory to room
*/
void DropItems(int16 first, int16 count) {
	int16 i;

	for (i = 0; i < count; i++) {
		if (inventory_items[first + i].flags == ITEMFLG_OWNED) {
			inventory_items[first + i].flags = ITEMFLG_ROOM;
			inventory_items[first + i].area = script_byte_vars.zone_area;
		}
	}
}

/*
Drop weapon-like items from inventory to room
*/
uint16 SCR_5D_DropWeapons(void) {
	script_ptr++;

	DropItems(kItemDagger1 - 1, 4);   /*DAGGER*/
	DropItems(kItemZapstik1 - 1, 14);  /*ZAPSTIK*/
	DropItems(kItemBlade - 1, 2);   /*SACRIFICIAL BLADE , CHOPPER*/

	script_byte_vars.zapstiks_owned = 0;

	return 0;
}

/*
React to Psi power
*/
uint16 SCR_62_PsiReaction(void) {
	byte power;
	uint16 cmd;

	script_ptr++;
	power = *script_ptr++;

	cmd = script_word_vars.zone_obj_cmds[(script_byte_vars.cur_spot_idx - 1) * 5 + power];
	if (cmd == 0)
		cmd = script_word_vars.psi_cmds[power];	/*TODO: is this consistent with zone_obj_cmds?*/

	the_command = Swap16(cmd);

	return 0;
}

/*TODO: rename me*/
uint16 SCR_63_LiftSpot6(void) {
	script_ptr++;

	blitSpritesToBackBuffer();
	zone_spots[6].sy -= 5;
	zone_spots[6].ey -= 5;
	backupSpotsImages();
	drawPersons();
	cga_BackBufferToRealFull();

	return 0;
}

uint16 SCR_64_DrawBoxAroundSpot(void) {
	script_ptr++;
	drawBoxAroundSpot();
	return 0;
}

/*
Draw text box
*/
uint16 SCR_14_DrawDesc(void) {
	byte *msg;
	script_ptr++;
	msg = seekToStringScr(desci_data, *script_ptr, &script_ptr);
	script_ptr++;

	drawMessage(msg, CGA_SCREENBUFFER);

	return 0;
}


/*
Draw dialog bubble with text for a person, wait for a key, then hide. Auto find bubble location
Use "thought" spike
*/
uint16 SCR_17_DrawPersonThoughtBubbleDialog(void) {
	byte x, y;
	byte *msg;
	script_ptr++;
	msg = seekToStringScr(diali_data, *script_ptr, &script_ptr);
	script_ptr++;

	x = found_spot->sx;
	y = found_spot->sy;

	if (x < 140 / 4)
		drawPersonBubble(found_spot->ex, y - 40, SPIKE_BUBLEFT | 20, msg);
	else
		drawPersonBubble(x - 80 / 4, y - 40, SPIKE_BUBRIGHT | 20, msg);

	promptWait();
	popDirtyRects(DirtyRectBubble);
	return 0;
}

/*
Draw dialog bubble with text for a person, wait for a key, then hide. Auto find bubble location
Use normal spike
*/
uint16 SCR_61_drawPersonBubbleDialog(void) {
	byte x, y;
	byte *msg;
	script_ptr++;
	msg = seekToStringScr(diali_data, *script_ptr, &script_ptr);
	script_ptr++;

	x = found_spot->sx;
	y = found_spot->sy;

	if (x < 140 / 4)
		drawPersonBubble(found_spot->ex, y - 40, SPIKE_DNLEFT | 20, msg);
	else
		drawPersonBubble(x - 80 / 4, y - 40, SPIKE_DNRIGHT | 20, msg);

	promptWait();
	popDirtyRects(DirtyRectBubble);
	return 0;
}

#if 1
byte *DebugString(char *msg, ...) {
	int16 i;
	byte c;
	static byte m[256];
	va_list ap;

	va_start(ap, msg);
	vsnprintf((char *)m, 256, msg, ap);
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
uint16 SCR_27_DrawGaussBubble(void) {
	byte *msg;

	script_ptr++;
	msg = seekToStringScr(diali_data, *script_ptr, &script_ptr);
	script_ptr++;

	drawPersonBubble(32 / 4, 20, 15, msg);
	return 0;
}

/*
Draw dialog bubble with text
*/
uint16 SCR_29_DialiTextBox(void) {
	byte x, y, f;
	byte *msg;
	script_ptr++;
	msg = seekToStringScr(diali_data, *script_ptr, &script_ptr);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	script_ptr++;
	x = *script_ptr++;
	y = *script_ptr++;
	f = *script_ptr++;

	drawPersonBubble(x, y, f, msg);
	return 0;
}

/*
Do nothing in PC/CGA version
*/
uint16 SCR_F_Unused(void) {
	script_ptr++;
	script_ptr++;
	script_ptr++;
	script_ptr++;
	return 0;
}

/*
Do nothing in PC/CGA version
*/
uint16 SCR_4A_Unused(void) {
	script_ptr++;
	return 0;
}

/*
Do nothing in PC/CGA version
*/
uint16 SCR_67_Unused(void) {
	script_ptr++;
	script_ptr++;
	return 0;
}

/*
Play Sfx
NB! Do nothing in EU PC/CGA version
*/
uint16 SCR_68_PlaySfx(void) {
	byte index;
	script_ptr++;
	index = *script_ptr++;
	script_ptr++;
	IFGM_PlaySfx(index);
	return 0;
}

/*
Play sound
*/
uint16 SCR_69_playSound(void) {
	byte index;
	script_ptr++;
	index = *script_ptr++;
	script_ptr++;

	playSound(index);
	return 0;
}

/*
Do nothing in PC/CGA version
*/
uint16 SCR_6A_Unused(void) {
	script_ptr++;
	return 0;
}

/*
Open room's items inventory
*/
uint16 CMD_1_RoomObjects(void) {
	updateUndrawCursor(CGA_SCREENBUFFER);
	inv_bgcolor = 0xAA;
	openInventory((0xFF << 8) | ITEMFLG_ROOM, (script_byte_vars.zone_area << 8) | ITEMFLG_ROOM);
	return ScriptRerun;
}

/*
Open Psi Powers menu
*/
uint16 CMD_2_PsiPowers(void) {
	/*Psi powers bar*/
	backupAndShowSprite(3, 280 / 4, 40);
	processInput();
	do {
		pollInput();
		selectCursor(CURSOR_FINGER);
		checkPsiCommandHover();
		if (command_hint != 100)
			command_hint += 109;
		if (command_hint != last_command_hint)
			drawCommandHint();
		drawHintsAndCursor(CGA_SCREENBUFFER);
	} while (buttons == 0);
	undrawCursor(CGA_SCREENBUFFER);
	cga_RestoreBackupImage(CGA_SCREENBUFFER);
	return ScriptRerun;
}

/*
Open normal inventory box
*/
uint16 CMD_3_Posessions(void) {
	updateUndrawCursor(CGA_SCREENBUFFER);
	inv_bgcolor = 0x55;
	openInventory(ITEMFLG_OWNED, ITEMFLG_OWNED);
	return ScriptRerun;
}

/*
Show energy level
*/
uint16 CMD_4_EnergyLevel(void) {
	static byte energy_image[] = {130, 236 / 4, 71};
	byte x, y, width, height;
	byte *image = energy_image;
	byte anim = 40;

	popDirtyRects(DirtyRectSprite);
	popDirtyRects(DirtyRectBubble);

	cur_dlg_index = 0;
	ifgm_flag2 = ~0;

	if (script_byte_vars.psy_energy != 0)
		anim = 41 + (script_byte_vars.psy_energy / 16);

	if (drawPortrait(&image, &x, &y, &width, &height)) {
		cga_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	}

	do {
		IFGM_PlaySample(28);
		animPortrait(1, anim, 10);
		animPortrait(1, anim + 14, 10);
		pollInputButtonsOnly();
	} while (buttons == 0);

	popDirtyRects(DirtyRectSprite);

	ifgm_flag2 = 0;
	IFGM_StopSample();

	return 0;
}

/*
Advance time
*/
uint16 CMD_5_Wait(void) {

	script_byte_vars.bvar_25++;
	script_word_vars.timer_ticks2 = Swap16(Swap16(script_word_vars.timer_ticks2) + 300);

	the_command = next_vorts_cmd;
	runCommand();

	the_command = next_turkey_cmd;
	runCommand();

	script_byte_vars.used_commands = script_byte_vars.check_used_commands;

	the_command = Swap16(script_word_vars.wvar_0E);

	if (the_command == 0) {
		if (script_word_vars.next_aspirant_cmd == 0) {
			the_command = 0x9005;
			runCommand();
		}
	} else {
		if (script_byte_vars.bvar_26 >= 63 && script_byte_vars.zone_area < 22 && script_byte_vars.zone_area != 1)
			the_command = 0x9005;
		return ScriptRerun;
	}

	return 0;
}

/*
Load game (menu)
*/
uint16 CMD_6_Load(void) {
	the_command = 0xC35C;
	return ScriptRerun;
}

/*
Save game (menu)
*/
uint16 CMD_7_Save(void) {
	the_command = 0xC35D;
	return ScriptRerun;
}

/*
Show timer
*/
uint16 CMD_8_Timer(void) {
	static byte timer_image[] = {163, 244 / 4, 104};
	byte x, y, width, height;
	byte *image = timer_image;

	if (drawPortrait(&image, &x, &y, &width, &height)) {
		cga_BlitAndWait(cur_image_pixels, cur_image_size_w, cur_image_size_w, cur_image_size_h, CGA_SCREENBUFFER, cur_image_offs);
	}

	do {
		uint16 timer = Swap16(script_word_vars.timer_ticks2);
		uint16 minutes = timer % (60 * 60);

		char_draw_coords_x = 260 / 4;
		char_draw_coords_y = 120;

		waitVBlank();
		cga_PrintChar(timer / (60 * 60) + 16, CGA_SCREENBUFFER);
		cga_PrintChar((minutes & 1) ? 26 : 0, CGA_SCREENBUFFER);    /*colon*/
		cga_PrintChar(minutes / (60 * 10) + 16, CGA_SCREENBUFFER);
		cga_PrintChar(minutes / 60 + 16, CGA_SCREENBUFFER);
		pollInputButtonsOnly();
	} while (buttons == 0);

	popDirtyRects(DirtyRectSprite);

	return 0;
}

int16 ConsumePsiEnergy(byte amount) {
	byte current = script_byte_vars.psy_energy;

	if (current < amount) {
		/*no energy left*/
		playAnim(68, 296 / 4, 71);
		return 0;
	}

	script_byte_vars.psy_energy = current - amount;

	/*significantly changed?*/
	if ((current & 0xF0) != (script_byte_vars.psy_energy & 0xF0))
		playAnim(68, 296 / 4, 71);

	return 1;
}

uint16 CMD_A_PsiSolarEyes(void) {
	if (!ConsumePsiEnergy(2))
		return 0;

	if (zone_palette == 14) {
		redrawRoomStatics(script_byte_vars.zone_room, zone_palette);
		zone_palette = 0;
		cga_BackBufferToRealFull();
	}

	the_command = Swap16(script_word_vars.wvar_AA);
	runCommand();
	script_byte_vars.cur_spot_flags = 0xFF;

	return 0;
}


uint16 GetZoneObjCommand(uint16 offs) {
	/*TODO: fix me: change offs/2 to index*/
	the_command = Swap16(script_word_vars.zone_obj_cmds[(script_byte_vars.cur_spot_idx - 1) * 5 + offs / 2]);
	return the_command;
}

void DrawStickyNet(void) {
	byte x, y, w, h;

	uint16 ofs;
	byte *sprite = loadPuzzlToScratch(80);

	x = room_bounds_rect.sx;
	y = room_bounds_rect.sy;
	w = room_bounds_rect.ex - x;
	h = room_bounds_rect.ey - y;

	ofs = cga_CalcXY_p(x, y);

	/*16x30 is the net sprite size*/

	for (; h; h -= 30) {
		int16 i;
		for (i = 0; i < w; i += 16 / 4)
			drawSprite(sprite, frontbuffer, ofs + i);
		ofs += CGA_BYTES_PER_LINE * 30 / 2;
	}
}

uint16 CMD_B_PsiStickyFingers(void) {
	if (!ConsumePsiEnergy(3))
		return 0;

	if (script_byte_vars.bvar_43 != 0) {
		the_command = Swap16(script_word_vars.wvar_AC);
		return ScriptRerun;
	}

	backupScreenOfSpecialRoom();
	DrawStickyNet();
	selectCursor(CURSOR_FLY);
	menuLoop(0, 0);
	playSound(224);
	cga_BackBufferToRealFull();
	restoreScreenOfSpecialRoom();

	if (script_byte_vars.cur_spot_idx == 0 || GetZoneObjCommand(0 * 2) == 0)
		the_command = Swap16(script_word_vars.psi_cmds[0]);

	if (script_byte_vars.bvar_26 >= 63
	        && script_byte_vars.zone_area < 22
	        && script_byte_vars.zone_area != 1)
		the_command = 0x9005;

	return ScriptRerun;
}

uint16 CMD_C_PsiKnowMind(void) {
	if (!ConsumePsiEnergy(1))
		return 0;

	if (script_byte_vars.bvar_43 != 0) {
		the_command = Swap16(script_word_vars.wvar_AE);
		return ScriptRerun;
	}

	processMenu();

	if (script_byte_vars.cur_spot_idx == 0 || GetZoneObjCommand(2 * 2) == 0)
		the_command = Swap16(script_word_vars.psi_cmds[1]);

	return ScriptRerun;
}

uint16 CMD_D_PsiBrainwarp(void) {
	if (!ConsumePsiEnergy(2))
		return 0;

	if (script_byte_vars.bvar_43 == 0) {
		backupScreenOfSpecialRoom();
		processMenu();

		if (script_byte_vars.cur_spot_idx == 0) {
			the_command = Swap16(script_word_vars.wvar_0C);
			script_byte_vars.dead_flag = 0;
			return ScriptRerun;
		}

		if (GetZoneObjCommand(1 * 2) != 0) {
			playAnim(39, found_spot->sx + 8 / 4, found_spot->sy - 10);
			restoreScreenOfSpecialRoom();
			return ScriptRerun;
		}
	}

	if (script_byte_vars.bvar_43 == 18) {
		script_byte_vars.dead_flag = 1;
		script_byte_vars.tries_left = 2;
		return 0;
	}

	((pers_t *)script_vars[kScrPool8_CurrentPers])->flags |= PERSFLG_80;
	script_byte_vars.dead_flag = script_byte_vars.cur_spot_idx;
	script_byte_vars.tries_left = 2;
	the_command = 0;
	if (script_byte_vars.bvar_43 == 0) {
		playAnim(39, found_spot->sx + 8 / 4, found_spot->sy - 10);
		restoreScreenOfSpecialRoom();
		return ScriptRerun;
	}

	the_command = 0x90AA;
	return ScriptRerun;
}


uint16 CMD_E_PsiZoneScan(void) {
	byte x, y, w, h;
	uint16 offs;

	if (!ConsumePsiEnergy(1))
		return 0;

	if (script_byte_vars.bvar_43 != 0) {
		the_command = Swap16(script_word_vars.wvar_B8);
		return ScriptRerun;
	}

	backupScreenOfSpecialRoom();

	IFGM_PlaySample(26);

	offs = cga_CalcXY_p(room_bounds_rect.sx, room_bounds_rect.sy);
	w = room_bounds_rect.ex - room_bounds_rect.sx;
	h = room_bounds_rect.ey - room_bounds_rect.sy;

	for (y = room_bounds_rect.sy; h; y++, h--) {
		spot_t *spot;
		for (x = 0; x < w; x++) frontbuffer[offs + x] = ~frontbuffer[offs + x];
		cga_blitToScreen(offs, w, 1);
		waitVBlank();
		for (x = 0; x < w; x++) frontbuffer[offs + x] = ~frontbuffer[offs + x];
		cga_blitToScreen(offs, w, 1);

		for (spot = zone_spots; spot != zone_spots_end; spot++) {
			if ((spot->flags & ~(SPOTFLG_40 | 7)) == (SPOTFLG_20 | SPOTFLG_8) && spot->sy == y) {
				playSound(27);
				spot->flags |= SPOTFLG_80;
				playAnim(38, spot->sx, spot->sy);
				break;
			}
		}

		offs ^= CGA_ODD_LINES_OFS;
		if ((offs & CGA_ODD_LINES_OFS) == 0)
			offs += CGA_BYTES_PER_LINE;
	}

	restoreScreenOfSpecialRoom();

	IFGM_StopSample();

	the_command = Swap16(script_word_vars.psi_cmds[2]);

	return ScriptRerun;

}

uint16 CMD_F_PsiPsiShift(void) {
	if (!ConsumePsiEnergy(3))
		return 0;

	if (script_byte_vars.bvar_43 != 0) {
		the_command = Swap16(script_word_vars.wvar_B0);
		return ScriptRerun;
	}

	selectCursor(CURSOR_GRAB);
	menuLoop(0, 0);
	backupScreenOfSpecialRoom();
	playSound(25);
	playAnim(39, cursor_x / 4, cursor_y);
	restoreScreenOfSpecialRoom();

	if (script_byte_vars.cur_spot_idx == 0 || GetZoneObjCommand(3 * 2) == 0)
		the_command = Swap16(script_word_vars.psi_cmds[5]);

	return ScriptRerun;
}

uint16 CMD_10_PsiExtremeViolence(void) {
	uint16 command;

	if (!ConsumePsiEnergy(8))
		return 0;

	script_byte_vars.extreme_violence = 1;

	if (script_byte_vars.bvar_43 != 0) {
		the_command = Swap16(script_word_vars.wvar_B2);
		return ScriptRerun;
	}

	processMenu();

	if (script_byte_vars.cur_spot_idx == 0) {
		the_command = Swap16(script_word_vars.psi_cmds[4]);
		script_byte_vars.extreme_violence = 0;
		return ScriptRerun;
	}

	command = GetZoneObjCommand(4 * 2);

	if ((command & 0xF000) == 0x9000)
		script_byte_vars.extreme_violence = 0;
	else if (command == 0) {
		the_command = Swap16(script_word_vars.psi_cmds[4]);
		script_byte_vars.extreme_violence = 0;
	}

	return ScriptRerun;
}

uint16 CMD_11_PsiTuneIn(void) {
	uint16 command;
	byte *msg;

	if (!ConsumePsiEnergy(4))
		return 0;

	if (script_byte_vars.bvar_43 != 0)
		command = Swap16(script_word_vars.wvar_B4);
	else {
		if (script_byte_vars.bvar_26 < 63 || script_byte_vars.zone_area >= 22)
			command = Swap16(script_word_vars.psi_cmds[3]);
		else
			command = 275;
	}

	/*TODO: is this really neccessary? Maybe it's always set when loaded from script vars?*/
	if (command & 0x8000) {
		the_command = command;
		return ScriptRerun;
	}

	msg = seekToString(diali_data, command);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	drawPersonBubble(32 / 4, 20, 15, msg);

	promptWait();
	popDirtyRects(DirtyRectBubble);

	return 0;
}

void ActionForPersonChoice(uint16 *actions) {
	processMenu();
	the_command = 0x9183;   /*THERE`S NOBODY.*/
	if (script_byte_vars.cur_spot_idx != 0 && script_byte_vars.cur_pers != 0) {
		pers_t *pers = (pers_t *)script_vars[kScrPool8_CurrentPers];
		byte index = pers->name;
		if (index == 93)    /*CADAVER*/
			index = 19 + 42;
		else if (index == 133)  /*SCI FI*/
			index = 18 + 42;

		index -= 42;        /*Person names: THE MASTER OF ORDEALS, etc*/

		the_command = actions[index];

		playSound(22);
	}
}

/*TODO: ensure these are never accessed/modified from the scripts*/
uint16 menu_commands_12[] = {
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

uint16 menu_commands_22[] = {
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

uint16 menu_commands_24[] = {
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

uint16 menu_commands_23[] = {
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

uint16 CMD_12_(void) {
	warning("cmd 12");
	ActionForPersonChoice(menu_commands_12);
	return ScriptRerun;
}

uint16 CMD_13_ActivateFountain(void) {
	static byte water1[] = {125, 156 / 4, 58};
	static byte water2[] = {126, 156 / 4, 58};
	static byte headl[] = {88, 152 / 4, 52};
	static byte headr[] = {88, (160 / 4) | 0x80, 52};

	byte x, y, w, h;
	uint16 i, j;

	script_byte_vars.bvar_6A = 1;
	for (i = 0; i < 10; i++) {
		drawRoomStaticObject(water1, &x, &y, &w, &h);
		waitVBlank();
		cga_BackBufferToRealFull();
		for (j = 0; j < 0x1FFF; j++) ; /*TODO: weak delay*/

		drawRoomStaticObject(water2, &x, &y, &w, &h);
		waitVBlank();
		cga_BackBufferToRealFull();
		for (j = 0; j < 0x1FFF; j++) ; /*TODO: weak delay*/
	}

	drawRoomStaticObject(headl, &x, &y, &w, &h);
	drawRoomStaticObject(headr, &x, &y, &w, &h);
	cga_BackBufferToRealFull();

	return 0;
}

/*
Vorts walking into the room
*/
uint16 CMD_14_VortAppear(void) {
	/*TODO: check me*/
	pers_list[kPersVort].area = script_byte_vars.zone_area;
	selectPerson(0);
	animateSpot(&vortanims_ptr->field_1);
	IFGM_StopSample();
	next_vorts_cmd = 0xA015;
	blitSpritesToBackBuffer();
	drawPersons();
	cga_BackBufferToRealFull();
	next_vorts_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
	return 0;
}

pers_t *vort_ptr;

#define ADJACENT_AREAS_MAX 19

struct {
	byte zone; /* current zone */
	byte area; /* area accessible from this zone */
} adjacent_areas[ADJACENT_AREAS_MAX] = {
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

/*
Vorts walking out of the room
*/
uint16 CMD_15_VortLeave(void) {
	/*TODO: check me*/

	uint16 i;
	animdesc_t *anim;
	pers_t *pers;

	if (pers_list[kPersVort].area != 0) {
		pers = &pers_list[kPersVort];
		anim = &vortanims_ptr->field_4;
	} else if (pers_list[kPersVort2].area != 0) {
		pers = &pers_list[kPersVort2];
		anim = &vortanims_ptr->field_7;
	} else {
		script_byte_vars.bvar_36 |= 0x80;

		pers_list[kPersVort3].area = 0;
		pers_list[kPersVort].flags = pers_list[kPersVort3].flags;

		pers = &pers_list[kPersVort];
		anim = &vortanims_ptr->field_A;
	}

	pers->area = 0;
	next_vorts_cmd = 0;
	for (i = 0; i < ADJACENT_AREAS_MAX; i++) {
		if (adjacent_areas[i].zone == script_byte_vars.zone_index) {
			next_vorts_cmd = 0xA016;
			next_vorts_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
			pers->area = adjacent_areas[i].area;
		}
	}
	vort_ptr = pers;

	zone_spots[(pers->flags & 15) - 1].flags &= ~SPOTFLG_80;

	selectPerson(0);
	animateSpot(anim);
	IFGM_StopSample();
	script_byte_vars.bvar_36 &= 0x80;
	return 0;
}

/*
Vorts left the room
*/
uint16 CMD_16_VortGone(void) {
	vort_ptr->area = 0;
	next_vorts_cmd = 0;
	return 0;
}

/*
Take away Aspirant's item and bounce it to the inventory
*/
uint16 CMD_17_LootAspirantsItem() {
	LootAspirantsItem();
	return ScriptRerun;
}

/*
Aspirant walking out of the room
*/
uint16 CMD_18_AspirantLeave(void) {
	/*TODO: check me*/
	static const animdesc_t anim33 = {ANIMFLG_USESPOT | 33, { { 0, 0 } }};

	popDirtyRects(DirtyRectSprite);
	popDirtyRects(DirtyRectText);

	aspirant_ptr->area = 0;
	script_word_vars.next_aspirant_cmd = BE(0);

	if ((aspirant_ptr->flags & PERSFLG_40) == 0) {
		aspirant_spot->flags &= ~SPOTFLG_80;
		selectPerson(script_byte_vars.aspirant_pers_ofs);
		script_byte_vars.aspirant_flags = 0;
		animateSpot(&anim33);
	}

	return 0;
}

/*
Aspirant walking into the room
*/
uint16 CMD_19_AspirantAppear(void) {
	/*TODO: check me*/
	static const animdesc_t anim23 = {ANIMFLG_USESPOT | 23, { { 0, 0 } }};

	popDirtyRects(DirtyRectSprite);
	aspirant_ptr->area = script_byte_vars.zone_area;
	script_word_vars.next_aspirant_cmd = BE(0xA018);	/*leave*/
	script_byte_vars.check_used_commands = 3;
	script_byte_vars.used_commands = 0;
	selectPerson(script_byte_vars.aspirant_pers_ofs);
	animateSpot(&anim23);
	blitSpritesToBackBuffer();
	drawPersons();
	cga_BackBufferToRealFull();
	if (script_byte_vars.aspirant_flags == 5) {
		the_command = 0xC029;
		script_byte_vars.aspirant_flags = 0;
		return ScriptRerun;
	}
	if (script_byte_vars.aspirant_flags == 6) {
		the_command = 0xC165;
		script_byte_vars.aspirant_flags = 0;
		return ScriptRerun;
	}
	return 0;
}

/*
Aspirant is dead
*/
uint16 CMD_1A_AspirantDie(void) {
	/*TODO: check me, unused in game?*/
	script_byte_vars.bvar_45 = 0;
	zone_spots[5].flags = SPOTFLG_40 | SPOTFLG_10 | SPOTFLG_2 | SPOTFLG_1;
	script_word_vars.next_aspirant_cmd = 0;
	DrawDeathAnim();
	return 0;
}

/*
Show Holo screen anim and speech
*/
uint16 CMD_1B_Holo(void) {
	byte x, y;
	uint16 num;
	byte *msg;

	IFGM_PlaySample(225);

	x = found_spot->sx;
	y = found_spot->sy;

	playAnim(42, x + 4 / 4, y + 6);

	num = 321 + ((Swap16(script_word_vars.timer_ticks2) < 60 * 60) ? 0 : 4) + (script_byte_vars.rand_value % 4);
	msg = seekToString(diali_data, num);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	if (x < 140 / 4)
		drawPersonBubble(x + 32 / 4, y - 40, SPIKE_DNLEFT | 20, msg);
	else
		drawPersonBubble(x - 92 / 4, y - 40, SPIKE_DNRIGHT | 20, msg);

	IFGM_PlaySfx(0);

	playAnim(43, x, y);

	promptWait();
	popDirtyRects(DirtyRectBubble);

	IFGM_PlaySample(225);

	playAnim(45, x, y);

	return 0;
}

/*
Turkey walking into the room
*/
uint16 CMD_1E_TurkeyAppear(void) {
	/*TODO: check me*/
	pers_list[kPersTurkey].area = script_byte_vars.zone_area;
	selectPerson(PersonOffset(kPersTurkey));
	animateSpot(&turkeyanims_ptr->field_1);
	next_turkey_cmd = 0xA01F;
	blitSpritesToBackBuffer();
	drawPersons();
	cga_BackBufferToRealFull();
	next_turkey_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
	return 0;
}

/*
Turkey leaving the room
*/
uint16 CMD_1F_TurkeyLeave(void) {
	uint16 i;
	animdesc_t *anim;
	pers_t *pers;

	pers = &pers_list[kPersTurkey];
	anim = &turkeyanims_ptr->field_4;

	pers->area = 0;
	next_turkey_cmd = 0;
	for (i = 0; i < ADJACENT_AREAS_MAX; i++) {
		if (adjacent_areas[i].zone == script_byte_vars.zone_index) {
			next_turkey_cmd = 0xA020;
			next_turkey_ticks = Swap16(script_word_vars.timer_ticks2) + 5;
			pers->area = adjacent_areas[i].area;
		}
	}

	zone_spots[(pers->flags & 15) - 1].flags &= ~SPOTFLG_80;

	selectPerson(PersonOffset(kPersTurkey));
	animateSpot(anim);
	return 0;
}

/*
Turkey left the room
*/
uint16 CMD_20_TurkeyGone(void) {
	pers_list[kPersTurkey].area = 0;
	next_turkey_cmd = 0;
	return 0;
}

/*
Talk to Vorts
*/
uint16 CMD_21_VortTalk(void) {
	byte x, y;
	uint16 num;
	byte *msg;

	if (script_byte_vars.rand_value >= 85)
		num = 6;
	else if (script_byte_vars.rand_value >= 170)
		num = 7;
	else
		num = 35;

	msg = seekToString(diali_data, num);
	cur_dlg_index = cur_str_index;  /*TODO: useless?*/

	x = found_spot->sx;
	y = found_spot->sy;

	if (x < 140 / 4)
		drawPersonBubble(found_spot->ex, y - 40, SPIKE_DNLEFT | 20, msg);
	else
		drawPersonBubble(x - 80 / 4, y - 40, SPIKE_DNRIGHT | 20, msg);

	IFGM_PlaySfx(0);

	promptWait();
	popDirtyRects(DirtyRectBubble);

	return 0;
}

uint16 CMD_22_(void) {
	ActionForPersonChoice(menu_commands_22);
	return ScriptRerun;
}

uint16 CMD_23_(void) {
	ActionForPersonChoice(menu_commands_23);
	return ScriptRerun;
}

uint16 CMD_24_(void) {
	ActionForPersonChoice(menu_commands_24);
	return ScriptRerun;
}

/*
Load save file
*/
uint16 CMD_25_LoadGame(void) {
	IFGM_StopSample();
	if (loadScena())
		the_command = 0x918F;   /*error loading*/
	else
		the_command = 0x90AA;
	return ScriptRerun;
}

/*
Write save file
*/
uint16 CMD_26_SaveGame(void) {
	if (saveScena())
		the_command = 0x9190;   /*error saving*/
	else
		the_command = 0x90AA;
	return ScriptRerun;
}



typedef uint16 (*cmdhandler_t)(void);

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
	CMD_17_LootAspirantsItem,
	CMD_18_AspirantLeave,
	CMD_19_AspirantAppear,
	CMD_1A_AspirantDie,
	CMD_1B_Holo,
	CMD_TRAP,
	CMD_TRAP,
	CMD_1E_TurkeyAppear,
	CMD_1F_TurkeyLeave,
	CMD_20_TurkeyGone,  /*20*/
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
	SCR_2_RudeAspirantTrade,
	SCR_3_DrawItemBox,
	SCR_4_StealZapstik,
	SCR_5_DrawPortraitLiftRight,
	SCR_6_DrawPortraitLiftLeft,
	SCR_7_DrawPortraitLiftDown,
	SCR_8_DrawPortraitLiftUp,
	SCR_9_DrawPortrait,
	SCR_A_DrawPortrait,     /*TODO: same as SCR_9_DrawPortrait , unused*/
	SCR_B_DrawPortraitTwistEffect,
	SCR_C_DrawPortraitArcEffect,
	SCR_D_DrawPortraitDotEffect,
	SCR_E_DrawPortraitZoomIn,
	SCR_F_Unused,
	SCR_10_DrawPortraitZoomed,   /*10*/
	SCR_11_DrawRoomObject,
	SCR_12_Chain,
	SCR_13_RedrawRoomStatics,
	SCR_14_DrawDesc,
	SCR_15_SelectSpot,
	SCR_16_DrawDeathAnim,
	SCR_17_DrawPersonThoughtBubbleDialog,
	SCR_18_AnimPortrait,
	SCR_19_HidePortraitLiftLeft,
	SCR_1A_HidePortraitLiftRight,
	SCR_1B_HidePortraitLiftUp,
	SCR_1C_HidePortraitLiftDown,
	SCR_1D_HidePortrait,        /*TODO: same as SCR_23_HidePortrait , unused*/
	SCR_1E_HidePortraitTwist,
	SCR_1F_HidePortraitArc,
	SCR_20_HidePortraitDots,    /*20*/
	SCR_21_HidePortrait,        /*TODO: same as SCR_23_HidePortrait , unused*/
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
	SCR_2E_promptWait,
	SCR_2F_LootAspirantsItem,
	SCR_30_Fight,   /*30*/
	SCR_31_Fight2,
	SCR_32_FightWin,
	SCR_33_Jump,
	SCR_34_Call,
	SCR_35_Ret,
	SCR_36_ChangeZone,
	SCR_37_desciTextBox,
	SCR_38_PlayAnim,
	SCR_39_AnimRoomDoorOpen,
	SCR_3A_AnimRoomDoorClose,
	SCR_3B_MathExpr,
	SCR_3C_CondExpr,
	SCR_3D_ActionsMenu,
	SCR_3E_TheWallAdvance,
	SCR_3F_HidePortrait,        /*TODO: same as SCR_23_HidePortrait , unused*/
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
	SCR_4A_Unused,
	SCR_4B_ProtoDropZapstik,
	SCR_4C_DrawPersons,
	SCR_4D_PriorityCommand,
	SCR_4E_BounceCurrentItemToRoom,
	SCR_4F_BounceCurrentItemToInventory,
	SCR_50_BounceItemToInventory,   /*50*/
	SCR_51_SkullTraderItemTrade,
	SCR_52_RefreshSpritesData,
	SCR_53_FindInvItem,
	SCR_54_DotFadeRoom,
	SCR_55_DrawRoomItemsIndicator,
	SCR_56_MorphRoom98,
	SCR_57_ShowCharacterSprite,
	SCR_58_DrawCharacterSprite,
	SCR_59_blitSpritesToBackBuffer,
	SCR_5A_SelectPalette,
	SCR_5B_TheEnd,
	SCR_5C_ClearInventory,
	SCR_5D_DropWeapons,
	SCR_5E_SelectTempPalette,
	SCR_5F_DrawRoomObjectBack,
	SCR_60_ReviveCadaver,   /*60*/
	SCR_61_drawPersonBubbleDialog,
	SCR_62_PsiReaction,
	SCR_63_LiftSpot6,
	SCR_64_DrawBoxAroundSpot,
	SCR_65_DeProfundisMovePlatform,
	SCR_66_DeProfundisRideToExit,
	SCR_67_Unused,
	SCR_68_PlaySfx,
	SCR_69_playSound,
	SCR_6A_Unused,
};
#define MAX_SCR_HANDLERS (sizeof(script_handlers) / sizeof(script_handlers[0]))

#ifdef DEBUG_SCRIPT
int16 runscr_reentr = 0;
int16 runcmd_reentr = 0;
#endif

/*
Run script routine
*/
uint16 RunScript(byte *code) {
	uint16 status = ScriptContinue;

#ifdef DEBUG_SCRIPT
	runscr_reentr += 1;
#endif

	script_ptr = code;
	while (script_ptr != script_end_ptr) {
		byte opcode = *script_ptr;

#ifdef DEBUG_SCRIPT
		{
			FILE *f = fopen(DEBUG_SCRIPT_LOG, "at");
			if (f) {
				uint16 offs = (script_ptr - templ_data) & 0xFFFF;
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

		if (status != ScriptContinue || g_vm->_shouldQuit)
			break;
	}

#ifdef DEBUG_SCRIPT
	runscr_reentr -= 1;
#endif

	return status;
}

/*
Get script routine
*/
byte *getScriptSubroutine(uint16 index) {
	return seekToEntry(templ_data, index, &script_end_ptr);
}

/*
Run script command
*/
uint16 runCommand(void) {
	uint16 res;
	uint16 cmd;

again:;
	res = 0;

	if (the_command == 0)
		return 0;

#ifdef DEBUG_SCRIPT
	{
		FILE *f = fopen(DEBUG_SCRIPT_LOG, "at");
		if (f) {
			fprintf(f, "\nrunCommand 0x%04X rc: %d rs: %d\n", the_command, runcmd_reentr, runscr_reentr);
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
		drawMessage(seekToString(desci_data, cmd), CGA_SCREENBUFFER);
		break;
	case 0xA000:
	case 0xB000:
		debug("Command: $%X 0x%X", the_command, cmd);
		res = command_handlers[cmd]();
		break;
	case 0xF000:
		/*restore sp from keep_sp then run script*/
		/*currently only supposed to work correctly from the SCR_4D_PriorityCommand handler*/
		debug("Restore: $%X 0x%X", the_command, cmd);
	/*TODO("SCR_RESTORE\n");*/
	/*fall through*/
	default:
		res = RunScript(getScriptSubroutine(cmd - 1));
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

uint16 runCommandKeepSp(void) {
	/*keep_sp = sp;*/
	setjmp(script_jmp);
	return runCommand();
}

} // End of namespace Chamber
