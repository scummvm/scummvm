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

/*
 * Copyright (c) 2007 - 2008 Magnus Lind.
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from
 * the use of this software.
 *
 * Permission is granted to anyone to use this software, alter it and re-
 * distribute it freely for any non-commercial, non-profit purpose subject to
 * the following restrictions:
 *
 *   1. The origin of this software must not be misrepresented; you must not
 *   claim that you wrote the original software. If you use this software in a
 *   product, an acknowledgment in the product documentation would be
 *   appreciated but is not required.
 *
 *   2. Altered source versions must be plainly marked as such, and must not
 *   be misrepresented as being the original software.
 *
 *   3. This notice may not be removed or altered from any distribution.
 *
 *   4. The names of this software and/or it's copyright holders may not be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 */

#include "glk/scott/globals.h"
#include "glk/scott/unp64/6502_emu.h"

namespace Glk {
namespace Scott {

#define FLAG_N 128
#define FLAG_V 64
#define FLAG_D 8
#define FLAG_I 4
#define FLAG_Z 2
#define FLAG_C 1

struct ArgEa {
	uint16_t _value;
};
struct ArgRelative {
	int8_t _value;
};
struct ArgImmediate {
	uint8_t _value;
};

union InstArg {
	ArgEa _ea;
	ArgRelative _rel;
	ArgImmediate _imm;
};

typedef void op_f(CpuCtx *r, int mode, InstArg *arg);
typedef int mode_f(CpuCtx *r, InstArg *arg);

struct OpInfo {
	op_f *_f;
	const char *_fmt;
};

struct ModeInfo {
	mode_f *_f;
	const char *_fmt;
};

struct InstInfo {
	OpInfo *_op;
	ModeInfo *_mode;
	uint8_t _cycles;
};

#define MODE_IMMEDIATE 0
#define MODE_ZERO_PAGE 1
#define MODE_ZERO_PAGE_X 2
#define MODE_ZERO_PAGE_Y 3
#define MODE_ABSOLUTE 4
#define MODE_ABSOLUTE_X 5
#define MODE_ABSOLUTE_Y 6
#define MODE_INDIRECT 7
#define MODE_INDIRECT_X 8
#define MODE_INDIRECT_Y 9
#define MODE_RELATIVE 10
#define MODE_ACCUMULATOR 11
#define MODE_IMPLIED 12

static int mode_imm(CpuCtx *r, InstArg *arg) {
	arg->_imm._value = r->_mem[r->_pc + 1];
	r->_pc += 2;
	return MODE_IMMEDIATE;
}

static int mode_zp(CpuCtx *r, InstArg *arg) {
	arg->_ea._value = r->_mem[r->_pc + 1];
	r->_pc += 2;
	return MODE_ZERO_PAGE;
}

static int mode_zpx(CpuCtx *r, InstArg *arg) { /* iAN: ldx #1 lda $ff,x should fetch from $00 and not $100 */
	uint8_t lsbLo = (r->_mem[r->_pc + 1] + r->_x) & 0xff;
	arg->_ea._value = lsbLo;
	r->_pc += 2;
	return MODE_ZERO_PAGE_X;
}

static int mode_zpy(CpuCtx *r, InstArg *arg) { /* iAN: ldy #1 ldx $ff,y should fetch from $00 and not $100 */
	uint8_t lsbLo = (r->_mem[r->_pc + 1] + r->_y) & 0xff;
	arg->_ea._value = lsbLo;
	r->_pc += 2;
	return MODE_ZERO_PAGE_Y;
}

static int mode_abs(CpuCtx *r, InstArg *arg) {
	uint16_t offset = r->_mem[r->_pc + 1];
	uint16_t base = r->_mem[r->_pc + 2] << 8;
	arg->_ea._value = base + offset;
	r->_pc += 3;
	return MODE_ABSOLUTE;
}

static int mode_absx(CpuCtx *r, InstArg *arg) {
	uint16_t offset = r->_mem[r->_pc + 1] + r->_x;
	uint16_t base = r->_mem[r->_pc + 2] << 8;
	arg->_ea._value = base + offset;
	r->_pc += 3;
	r->_cycles += (offset > 255);
	return MODE_ABSOLUTE_X;
}

static int mode_absy(CpuCtx *r, InstArg *arg) {
	uint16_t offset = r->_mem[r->_pc + 1] + r->_y;
	uint16_t base = r->_mem[r->_pc + 2] << 8;
	arg->_ea._value = base + offset;
	r->_pc += 3;
	r->_cycles += (offset > 255);
	return MODE_ABSOLUTE_Y;
}

static int mode_ind(CpuCtx *r, InstArg *arg) {
	int lsbLo = r->_mem[r->_pc + 1];
	int msbLo = r->_mem[r->_pc + 2];
	int offsetlo = lsbLo | msbLo << 8;
	int offsethi = ((lsbLo + 1) & 0xff) | msbLo << 8;
	arg->_ea._value = r->_mem[offsetlo] | r->_mem[offsethi] << 8;

	r->_pc += 3;
	return MODE_INDIRECT;
}

static int mode_indx(CpuCtx *r, InstArg *arg) {
	uint8_t lsbLo = r->_mem[r->_pc + 1] + r->_x;
	uint8_t msbLo = lsbLo + 1;
	uint16_t base = r->_mem[msbLo] << 8;
	uint16_t offset = r->_mem[lsbLo];
	arg->_ea._value = base + offset;
	r->_pc += 2;
	return MODE_INDIRECT_X;
}

static int mode_indy(CpuCtx *r, InstArg *arg) {
	uint8_t lsbLo = r->_mem[r->_pc + 1];
	uint8_t msbLo = lsbLo + 1;
	uint16_t base = r->_mem[msbLo] << 8;
	uint16_t offset = r->_mem[lsbLo] + r->_y;
	arg->_ea._value = base + offset;
	r->_pc += 2;
	r->_cycles += (offset > 255);
	return MODE_INDIRECT_Y;
}

static int mode_rel(CpuCtx *r, InstArg *arg) {
	arg->_rel._value = r->_mem[r->_pc + 1];
	r->_pc += 2;
	return MODE_RELATIVE;
}

static int mode_acc(CpuCtx *r, InstArg *arg) {
	r->_pc++;
	return MODE_ACCUMULATOR;
}

static int mode_imp(CpuCtx *r, InstArg *arg) {
	r->_pc++;
	return MODE_IMPLIED;
}

static ModeInfo mode_imm_o	= { &mode_imm,	"#$%02x" };
static ModeInfo mode_zp_o	= { &mode_zp,	"$%02x" };
static ModeInfo mode_zpx_o	= { &mode_zpx,	"$%02x,x" };
static ModeInfo mode_zpy_o	= { &mode_zpy,	"$%02x,y" };
static ModeInfo mode_abs_o	= { &mode_abs,	"$%04x" };
static ModeInfo mode_absx_o = { &mode_absx,	"$%04x,x" };
static ModeInfo mode_absy_o = { &mode_absy,	"$%04x,y" };
static ModeInfo mode_ind_o	= { &mode_ind,	"($%04x)" };
static ModeInfo mode_indx_o = { &mode_indx,	"($%02x,x)" };
static ModeInfo mode_indy_o = { &mode_indy,	"($%02x),y" };
static ModeInfo mode_rel_o	= { &mode_rel,	"$%02x" };
static ModeInfo mode_acc_o	= { &mode_acc,	"a" };
static ModeInfo mode_imp_o	= { &mode_imp,	nullptr };

static void updateFlagsNz(CpuCtx *r, uint8_t value) {
	r->_flags &= ~(FLAG_Z | FLAG_N);
	r->_flags |= (value == 0 ? FLAG_Z : 0) | (value & FLAG_N);
}

static void updateCarry(CpuCtx *r, int boolean) {
	r->_flags = (r->_flags & ~FLAG_C) | (boolean != 0 ? FLAG_C : 0);
}

static uint16_t subtract(CpuCtx *r, int carry, uint8_t val1, uint8_t value) {
	uint16_t target = val1 - value - (1 - !!carry);
	updateCarry(r, !(target & 256));
	updateFlagsNz(r, target & 255);
	return target;
}

static void update_overflow(CpuCtx *r, int boolean) {
	r->_flags = (r->_flags & ~FLAG_V) | (boolean != 0 ? FLAG_V : 0);
}

static void op_adc(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	uint16_t result;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	result = r->_a + value + (r->_flags & FLAG_C);
	updateCarry(r, result & 256);
	update_overflow(r, !((r->_a & 0x80) ^ (value & 0x80)) && ((r->_a & 0x80) ^ (result & 0x80)));
	r->_a = result & 0xff;
	updateFlagsNz(r, r->_a);
}

static void op_and(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	r->_a &= value;
	updateFlagsNz(r, r->_a);
}

static void op_asl(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t *valuep;
	switch (mode) {
	case MODE_ACCUMULATOR:
		valuep = &r->_a;
		break;
	default:
		valuep = &r->_mem[arg->_ea._value];
		break;
	}
	updateCarry(r, *valuep & 128);
	*valuep <<= 1;
	updateFlagsNz(r, *valuep);
}

static void branch(CpuCtx *r, InstArg *arg) {
	uint16_t target = r->_pc + arg->_rel._value;
	r->_cycles += 1 + ((target & ~255) != (r->_pc & ~255));
	r->_pc = target;
}

static void op_bcc(CpuCtx *r, int mode, InstArg *arg) {
	if (!(r->_flags & FLAG_C)) {
		branch(r, arg);
	}
}

static void op_bcs(CpuCtx *r, int mode, InstArg *arg) {
	if (r->_flags & FLAG_C) {
		branch(r, arg);
	}
}

static void op_beq(CpuCtx *r, int mode, InstArg *arg) {
	if (r->_flags & FLAG_Z) {
		branch(r, arg);
	}
}

static void op_bit(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags &= ~(FLAG_N | FLAG_V | FLAG_Z);
	r->_flags |= (r->_mem[arg->_ea._value] & FLAG_N) != 0 ? FLAG_N : 0;
	r->_flags |= (r->_mem[arg->_ea._value] & FLAG_V) != 0 ? FLAG_V : 0;
	r->_flags |= (r->_mem[arg->_ea._value] & r->_a) == 0 ? FLAG_Z : 0;
}

static void op_bmi(CpuCtx *r, int mode, InstArg *arg) {
	if (r->_flags & FLAG_N) {
		branch(r, arg);
	}
}

static void op_bne(CpuCtx *r, int mode, InstArg *arg) {
	if (!(r->_flags & FLAG_Z)) {
		branch(r, arg);
	}
}

static void op_bpl(CpuCtx *r, int mode, InstArg *arg) {
	if (!(r->_flags & FLAG_N)) {
		branch(r, arg);
	}
}

static void op_brk(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[0x100 + r->_sp--] = (r->_pc + 1) >> 8;
	r->_mem[0x100 + r->_sp--] = r->_pc + 1;
	r->_mem[0x100 + r->_sp--] = r->_flags | 0x10;
}

static void op_bvc(CpuCtx *r, int mode, InstArg *arg) {
	if (!(r->_flags & FLAG_V)) {
		branch(r, arg);
	}
}

static void op_bvs(CpuCtx *r, int mode, InstArg *arg) {
	if ((r->_flags & FLAG_V)) {
		branch(r, arg);
	}
}

static void op_clc(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags &= ~FLAG_C;
}

static void op_cld(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags &= ~FLAG_D;
}

static void op_cli(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags &= ~FLAG_I;
}

static void op_clv(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags &= ~FLAG_V;
}


static void op_cmp(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	subtract(r, 1, r->_a, value);
}

static void op_cpx(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	subtract(r, 1, r->_x, value);
}

static void op_cpy(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	subtract(r, 1, r->_y, value);
}

static void op_dec(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[arg->_ea._value]--;
	updateFlagsNz(r, r->_mem[arg->_ea._value]);
}

static void op_dex(CpuCtx *r, int mode, InstArg *arg) {
	r->_x--;
	updateFlagsNz(r, r->_x);
}

static void op_dey(CpuCtx *r, int mode, InstArg *arg) {
	r->_y--;
	updateFlagsNz(r, r->_y);
}

static void op_eor(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	r->_a ^= value;
	updateFlagsNz(r, r->_a);
}

static void op_inc(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[arg->_ea._value]++;
	updateFlagsNz(r, r->_mem[arg->_ea._value]);
}

static void op_inx(CpuCtx *r, int mode, InstArg *arg) {
	r->_x++;
	updateFlagsNz(r, r->_x);
}

static void op_iny(CpuCtx *r, int mode, InstArg *arg) {
	r->_y++;
	updateFlagsNz(r, r->_y);
}

static void op_jmp(CpuCtx *r, int mode, InstArg *arg) {
	r->_pc = arg->_ea._value;
}

static void op_jsr(CpuCtx *r, int mode, InstArg *arg) {
	r->_pc--;
	r->_mem[0x100 + r->_sp--] = r->_pc >> 8;
	r->_mem[0x100 + r->_sp--] = r->_pc & 0xff;
	r->_pc = arg->_ea._value;
}

static void op_lda(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	r->_a = value;
	updateFlagsNz(r, r->_a);
}

static void op_ldx(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	r->_x = value;
	updateFlagsNz(r, r->_x);
}

static void op_ldy(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	r->_y = value;
	updateFlagsNz(r, r->_y);
}

static void op_lsr(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t *valuep;
	switch (mode) {
	case MODE_ACCUMULATOR:
		valuep = &r->_a;
		break;
	default:
		valuep = &r->_mem[arg->_ea._value];
		break;
	}
	updateCarry(r, *valuep & 1);
	*valuep >>= 1;
	updateFlagsNz(r, *valuep);
}

static void op_nop(CpuCtx *r, int mode, InstArg *arg) {}

static void op_ora(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	r->_a |= value;
	updateFlagsNz(r, r->_a);
}

static void op_pha(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[0x100 + r->_sp--] = r->_a;
}

static void op_php(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[0x100 + r->_sp--] = r->_flags & ~0x10;
}

static void op_pla(CpuCtx *r, int mode, InstArg *arg) {
	r->_a = r->_mem[0x100 + ++r->_sp];
	updateFlagsNz(r, r->_a);
}

static void op_plp(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags = r->_mem[0x100 + ++r->_sp];
}

static void op_rol(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t *valuep;
	uint8_t old_flags;
	switch (mode) {
	case MODE_ACCUMULATOR:
		valuep = &r->_a;
		break;
	default:
		valuep = &r->_mem[arg->_ea._value];
		break;
	}
	old_flags = r->_flags;
	updateCarry(r, *valuep & 128);
	*valuep <<= 1;
	*valuep |= (old_flags & FLAG_C) != 0 ? 1 : 0;
	updateFlagsNz(r, *valuep);
}

static void op_ror(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t *valuep;
	uint8_t old_flags;
	switch (mode) {
	case MODE_ACCUMULATOR:
		valuep = &r->_a;
		break;
	default:
		valuep = &r->_mem[arg->_ea._value];
		break;
	}
	old_flags = r->_flags;
	updateCarry(r, *valuep & 1);
	*valuep >>= 1;
	*valuep |= (old_flags & FLAG_C) != 0 ? 128 : 0;
	updateFlagsNz(r, *valuep);
}

static void op_rti(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags = r->_mem[0x100 + ++r->_sp];
	r->_pc = r->_mem[0x100 + ++r->_sp];
	r->_pc |= r->_mem[0x100 + ++r->_sp] << 8;
}

static void op_rts(CpuCtx *r, int mode, InstArg *arg) {
	r->_pc = r->_mem[0x100 + ++r->_sp];
	r->_pc |= r->_mem[0x100 + ++r->_sp] << 8;
	r->_pc++;
}

static void op_sbc(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	uint16_t result;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	result = subtract(r, r->_flags & FLAG_C, r->_a, value);
	update_overflow(r, !((r->_a & 0x80) ^ (value & 0x80)) && ((r->_a & 0x80) ^ (result & 0x80)));
	r->_a = result & 0xff;
	updateFlagsNz(r, r->_a);
}

static void op_sec(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags |= FLAG_C;
}

static void op_sed(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags |= FLAG_D;
}

static void op_sei(CpuCtx *r, int mode, InstArg *arg) {
	r->_flags |= FLAG_I;
}

static void op_sta(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[arg->_ea._value] = r->_a;
}

static void op_stx(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[arg->_ea._value] = r->_x;
}

static void op_sty(CpuCtx *r, int mode, InstArg *arg) {
	r->_mem[arg->_ea._value] = r->_y;
}

static void op_tax(CpuCtx *r, int mode, InstArg *arg) {
	r->_x = r->_a;
	updateFlagsNz(r, r->_x);
}

static void op_tay(CpuCtx *r, int mode, InstArg *arg) {
	r->_y = r->_a;
	updateFlagsNz(r, r->_y);
}

static void op_tsx(CpuCtx *r, int mode, InstArg *arg) {
	r->_x = r->_sp;
	updateFlagsNz(r, r->_x);
}

static void op_txa(CpuCtx *r, int mode, InstArg *arg) {
	r->_a = r->_x;
	updateFlagsNz(r, r->_a);
}

static void op_txs(CpuCtx *r, int mode, InstArg *arg) {
	r->_sp = r->_x;
}

static void op_tya(CpuCtx *r, int mode, InstArg *arg) {
	r->_a = r->_y;
	updateFlagsNz(r, r->_a);
}

/* iAN */
static void op_anc(CpuCtx *r, int mode, InstArg *arg) {
	uint8_t value;
	switch (mode) {
	case MODE_IMMEDIATE:
		value = arg->_imm._value;
		break;
	default:
		value = r->_mem[arg->_ea._value];
		break;
	}
	r->_a &= value;
	if (r->_a & 0x80)
		r->_flags |= FLAG_C;
	updateFlagsNz(r, r->_a);
}

/* iAN */
static void op_lax(CpuCtx *r, int mode, InstArg *arg) {
	op_lda(r, mode, arg);
	r->_x = r->_a;
	updateFlagsNz(r, r->_x);
}
/* iAN */
static void op_lae(CpuCtx *r, int mode, InstArg *arg) {
	op_lda(r, mode, arg);
	r->_x = r->_a;
	r->_sp = r->_a;
	updateFlagsNz(r, r->_a);
}

/* iAN */
static void op_dcp(CpuCtx *r, int mode, InstArg *arg) {
	op_dec(r, mode, arg);
	op_cmp(r, mode, arg);
}

/* iAN */
static void op_sax(CpuCtx *r, int mode, InstArg *arg) {
	r->_a &= r->_x;
	op_sta(r, mode, arg);
}

/* iAN */
static void op_rla(CpuCtx *r, int mode, InstArg *arg) {
	op_rol(r, mode, arg);
	op_and(r, mode, arg);
}

/* iAN */
static void op_rra(CpuCtx *r, int mode, InstArg *arg) {
	op_rol(r, mode, arg);
	op_adc(r, mode, arg);
}

/* iAN */
static void op_isb(CpuCtx *r, int mode, InstArg *arg) {
	op_inc(r, mode, arg);
	op_sbc(r, mode, arg);
}

/* iAN */
static void op_slo(CpuCtx *r, int mode, InstArg *arg) {
	op_asl(r, mode, arg);
	op_ora(r, mode, arg);
}

/* iAN */
static void op_sbx(CpuCtx *r, int mode, InstArg *arg) {
	/* immediate mode only: sbx #$ff */
	r->_x &= r->_a;
	r->_flags = (r->_flags & 0xfe) | (r->_x >= arg->_imm._value ? 1 : 0); /* fixed: Carry IS set by SBX but
												  not used during subtraction */
	r->_x -= arg->_imm._value;
	updateFlagsNz(r, r->_x);
}

/* iAN */
static void op_sre(CpuCtx *r, int mode, InstArg *arg) {
	op_lsr(r, mode, arg);
	op_eor(r, mode, arg);
}
/* iAN */
static void
op_asr(CpuCtx *r, int mode,
	   InstArg *arg) { /* first A AND #immediate, then LSR A */
	op_and(r, MODE_IMMEDIATE, arg);
	op_lsr(r, MODE_ACCUMULATOR, arg);
}
/* iAN */
static void op_ane(CpuCtx *r, int mode, InstArg *arg) {
	r->_a |= 0xee;
	r->_a &= r->_x;
	op_and(r, MODE_IMMEDIATE, arg);
}
/* iAN */
static void op_lxa(CpuCtx *r, int mode, InstArg *arg) {
	r->_a |= 0xee;
	op_and(r, MODE_IMMEDIATE, arg);
	r->_x = r->_a;
}

static OpInfo op_adc_o = {&op_adc, "adc"};
static OpInfo op_anc_o = {&op_anc, "anc"};
static OpInfo op_and_o = {&op_and, "and"};
static OpInfo op_ane_o = {&op_ane, "ane"};
static OpInfo op_asl_o = {&op_asl, "asl"};
static OpInfo op_asr_o = {&op_asr, "asr"};
static OpInfo op_bcc_o = {&op_bcc, "bcc"};
static OpInfo op_bcs_o = {&op_bcs, "bcs"};
static OpInfo op_beq_o = {&op_beq, "beq"};
static OpInfo op_bit_o = {&op_bit, "bit"};
static OpInfo op_bmi_o = {&op_bmi, "bmi"};
static OpInfo op_bne_o = {&op_bne, "bne"};
static OpInfo op_bpl_o = {&op_bpl, "bpl"};
static OpInfo op_brk_o = {&op_brk, "brk"};
static OpInfo op_bvc_o = {&op_bvc, "bvc"};
static OpInfo op_bvs_o = {&op_bvs, "bvs"};
static OpInfo op_clc_o = {&op_clc, "clc"};

static OpInfo op_cld_o = {&op_cld, "cld"};
static OpInfo op_cli_o = {&op_cli, "cli"};
static OpInfo op_clv_o = {&op_clv, "clv"};
static OpInfo op_cmp_o = {&op_cmp, "cmp"};
static OpInfo op_cpx_o = {&op_cpx, "cpx"};
static OpInfo op_cpy_o = {&op_cpy, "cpy"};
static OpInfo op_dcp_o = {&op_dcp, "dcp"};
static OpInfo op_dec_o = {&op_dec, "dec"};
static OpInfo op_dex_o = {&op_dex, "dex"};
static OpInfo op_dey_o = {&op_dey, "dey"};
static OpInfo op_eor_o = {&op_eor, "eor"};
static OpInfo op_inc_o = {&op_inc, "inc"};
static OpInfo op_isb_o = {&op_isb, "isb"};
static OpInfo op_inx_o = {&op_inx, "inx"};
static OpInfo op_iny_o = {&op_iny, "iny"};
static OpInfo op_jmp_o = {&op_jmp, "jmp"};

static OpInfo op_jsr_o = {&op_jsr, "jsr"};
static OpInfo op_lda_o = {&op_lda, "lda"};
static OpInfo op_ldx_o = {&op_ldx, "ldx"};
static OpInfo op_ldy_o = {&op_ldy, "ldy"};
static OpInfo op_lsr_o = {&op_lsr, "lsr"};
static OpInfo op_nop_o = {&op_nop, "nop"};
static OpInfo op_ora_o = {&op_ora, "ora"};
static OpInfo op_pha_o = {&op_pha, "pha"};
static OpInfo op_php_o = {&op_php, "php"};
static OpInfo op_pla_o = {&op_pla, "pla"};
static OpInfo op_plp_o = {&op_plp, "plp"};
static OpInfo op_rol_o = {&op_rol, "rol"};
static OpInfo op_rla_o = {&op_rla, "rla"};
static OpInfo op_ror_o = {&op_ror, "ror"};
static OpInfo op_rti_o = {&op_rti, "rti"};
static OpInfo op_rra_o = {&op_rra, "rra"};

static OpInfo op_rts_o = {&op_rts, "rts"};
static OpInfo op_sbc_o = {&op_sbc, "sbc"};
static OpInfo op_sbx_o = {&op_sbx, "sbx"};
static OpInfo op_sec_o = {&op_sec, "sec"};
static OpInfo op_sed_o = {&op_sed, "sed"};
static OpInfo op_sei_o = {&op_sei, "sei"};
static OpInfo op_slo_o = {&op_slo, "slo"};
static OpInfo op_sta_o = {&op_sta, "sta"};
static OpInfo op_stx_o = {&op_stx, "stx"};
static OpInfo op_sty_o = {&op_sty, "sty"};
static OpInfo op_lax_o = {&op_lax, "lax"};
static OpInfo op_lae_o = {&op_lae, "lae"};
static OpInfo op_lxa_o = {&op_lxa, "lxa"};
static OpInfo op_tax_o = {&op_tax, "tax"};
static OpInfo op_sax_o = {&op_sax, "sax"};
static OpInfo op_sre_o = {&op_sre, "sre"};
static OpInfo op_tay_o = {&op_tay, "tay"};
static OpInfo op_tsx_o = {&op_tsx, "tsx"};
static OpInfo op_txa_o = {&op_txa, "txa"};
static OpInfo op_txs_o = {&op_txs, "txs"};
static OpInfo op_tya_o = {&op_tya, "tya"};

#define NULL_OP { nullptr, nullptr, 0 }

static InstInfo g_ops[256] = {
	/* 0x00 */
	{ &op_brk_o, &mode_imp_o,	7 },
	{ &op_ora_o, &mode_indx_o,	6 },
	NULL_OP,
	NULL_OP,
	{ &op_nop_o, &mode_zp_o,	3 }, /* $04 nop $ff */
	{ &op_ora_o, &mode_zp_o,	3 },
	{ &op_asl_o, &mode_zp_o,	5 },
	{ &op_slo_o, &mode_zp_o,	5 }, /* $07 slo $ff */
	{ &op_php_o, &mode_imp_o,	3 },
	{ &op_ora_o, &mode_imm_o,	2 },
	{ &op_asl_o, &mode_acc_o,	2 },
	{ &op_anc_o, &mode_imm_o,	2 }, /* $0b anc #$ff */
	{ &op_nop_o, &mode_abs_o,	4 }, /* $0c nop $ffff */
	{ &op_ora_o, &mode_abs_o,	4 },
	{ &op_asl_o, &mode_abs_o,	6 },
	{ &op_slo_o, &mode_abs_o,	6 }, /* $0f slo $ffff */
	/* 0x10 */
	{ &op_bpl_o, &mode_rel_o,	2 },
	{ &op_ora_o, &mode_indy_o,	5 },
	NULL_OP,
	NULL_OP,
	{ &op_nop_o, &mode_zpx_o,	4 }, /* $14 nop $ff,x */
	{ &op_ora_o, &mode_zpx_o,	4 },
	{ &op_asl_o, &mode_zpx_o,	6 },
	NULL_OP,
	{ &op_clc_o, &mode_imp_o,	2 },
	{ &op_ora_o, &mode_absy_o,	4 },
	{ &op_nop_o, &mode_imp_o,	2 },  /* $1a nop */
	{ &op_slo_o, &mode_absy_o,	4 }, /* $1b slo $ffff,y */
	{ &op_nop_o, &mode_absx_o,	4 }, /* $1c nop $ffff,x */
	{ &op_ora_o, &mode_absx_o,	4 },
	{ &op_asl_o, &mode_absx_o,	7 },
	{ &op_slo_o, &mode_absx_o,	7 }, /* $1f slo $ffff,x */
	/* 0x20 */
	{ &op_jsr_o, &mode_abs_o,	6 },
	{ &op_and_o, &mode_indx_o,	6 },
	NULL_OP,
	{ &op_rla_o, &mode_indx_o,	8 }, /* $23 rla ($ff,x) */
	{ &op_bit_o, &mode_zp_o,	3 },
	{ &op_and_o, &mode_zp_o,	3 },
	{ &op_rol_o, &mode_zp_o,	5 },
	{ &op_rla_o, &mode_zp_o,	5 }, /* $27 rla $ff */
	{ &op_plp_o, &mode_imp_o,	4 },
	{ &op_and_o, &mode_imm_o,	2 },
	{ &op_rol_o, &mode_acc_o,	2 },
	{ &op_anc_o, &mode_imm_o,	2 }, /* $2b anc #$ff */
	{ &op_bit_o, &mode_abs_o,	4 },
	{ &op_and_o, &mode_abs_o,	4 },
	{ &op_rol_o, &mode_abs_o,	6 },
	{ &op_rla_o, &mode_abs_o,	6 }, /* $2f rla $ffff */
	/* 0x30 */
	{ &op_bmi_o, &mode_rel_o,	2 },
	{ &op_and_o, &mode_indy_o,	5 },
	NULL_OP,
	{ &op_rla_o, &mode_indy_o,	8 }, /* $33 rla ($ff),y  */
	{ &op_nop_o, &mode_zpx_o,	4 },  /* $34 nop $ff,x */
	{ &op_and_o, &mode_zpx_o,	4 },
	{ &op_rol_o, &mode_zpx_o,	6 },
	{ &op_rla_o, &mode_zpx_o,	6 }, /* $37 rla $ff,x */
	{ &op_sec_o, &mode_imp_o,	2 },
	{ &op_and_o, &mode_absy_o,	4 },
	{ &op_nop_o, &mode_imp_o,	2 },  /* $3a nop */
	{ &op_rla_o, &mode_absy_o,	7 }, /* $3b rla $ffff,y */
	{ &op_nop_o, &mode_absx_o,	7 }, /* $3c nop $ffff,x */
	{ &op_and_o, &mode_absx_o,	4 },
	{ &op_rol_o, &mode_absx_o,	7 },
	{ &op_rla_o, &mode_absx_o,	7 }, /* $3f rla $ffff,x */
	/* 0x40 */
	{ &op_rti_o, &mode_imp_o,	6 },
	{ &op_eor_o, &mode_indx_o,	6 },
	NULL_OP,
	{ &op_sre_o, &mode_indx_o,	6 }, /* $43 sre ($ff,x) */
	{ &op_nop_o, &mode_zp_o,	3 },   /* $44 nop $ff */
	{ &op_eor_o, &mode_zp_o,	3 },
	{ &op_lsr_o, &mode_zp_o,	5 },
	{ &op_sre_o, &mode_zp_o,	5 }, /* $47 sre $ff */
	{ &op_pha_o, &mode_imp_o,	3 },
	{ &op_eor_o, &mode_imm_o,	2 },
	{ &op_lsr_o, &mode_acc_o,	2 },
	{ &op_asr_o, &mode_imm_o,	2 }, /* $4b asr #$ff */
	{ &op_jmp_o, &mode_abs_o,	3 },
	{ &op_eor_o, &mode_abs_o,	4 },
	{ &op_lsr_o, &mode_abs_o,	6 },
	{ &op_sre_o, &mode_abs_o,	6 }, /* $4f sre $ffff */
	/* 0x50 */
	{ &op_bvc_o, &mode_rel_o,	2 },
	{ &op_eor_o, &mode_indy_o,	5 },
	NULL_OP,
	NULL_OP,
	{ &op_nop_o, &mode_zpx_o,	4 }, /* $54 nop $ff,x */
	{ &op_eor_o, &mode_zpx_o,	4 }, /* fix: eor $ff,x takes 4 cycles*/
	{ &op_lsr_o, &mode_zpx_o,	6 },
	{ &op_sre_o, &mode_zpx_o,	6 }, /* $57 sre $ff,x */
	{ &op_cli_o, &mode_imp_o,	2 },
	{ &op_eor_o, &mode_absy_o,	4 },
	{ &op_nop_o, &mode_imp_o,	2 },  /* $5a nop */
	{ &op_sre_o, &mode_absy_o,	7 }, /* $5b sre $ffff,y */
	{ &op_nop_o, &mode_absx_o,	4 }, /* $5c nop $ffff,x */
	{ &op_eor_o, &mode_absx_o,	4 },
	{ &op_lsr_o, &mode_absx_o,	7 },
	{ &op_sre_o, &mode_absx_o,	7 }, /* $5f sre $ffff,x */
	/* 0x60 */
	{ &op_rts_o, &mode_imp_o,	6 },
	{ &op_adc_o, &mode_indx_o,	6 },
	NULL_OP,
	{ &op_rra_o, &mode_indx_o,	8 }, /* $63 rra ($ff,x) */
	{ &op_nop_o, &mode_zp_o,	3 },   /* $64 nop $ff */
	{ &op_adc_o, &mode_zp_o,	3 },
	{ &op_ror_o, &mode_zp_o,	5 },
	{ &op_rra_o, &mode_zp_o,	5 }, /* $67 rra $ff */
	{ &op_pla_o, &mode_imp_o,	4 },
	{ &op_adc_o, &mode_imm_o,	2 },
	{ &op_ror_o, &mode_acc_o,	2 },
	NULL_OP,                     /* fix: $6b ARR (todo?) */
	{ &op_jmp_o, &mode_ind_o,	5 }, /* fix: $6c JMP ($FFFF) */
	{ &op_adc_o, &mode_abs_o,	4 },
	{ &op_ror_o, &mode_abs_o,	6 },
	{ &op_rra_o, &mode_abs_o,	6 }, /* $6f rra $ffff */
	/* 0x70 */
	{ &op_bvs_o, &mode_rel_o,	2 },
	{ &op_adc_o, &mode_indy_o,	5 },
	NULL_OP,
	{ &op_rra_o, &mode_indy_o,	8 }, /* $73 rra ($ff),y */
	{ &op_nop_o, &mode_zpx_o,	4 },  /* $74 nop $ff,x */
	{ &op_adc_o, &mode_zpx_o,	4 },
	{ &op_ror_o, &mode_zpx_o,	6 },
	{ &op_rra_o, &mode_zpx_o,	6 }, /* $77 rra $ff,x */
	{ &op_sei_o, &mode_imp_o,	2 },
	{ &op_adc_o, &mode_absy_o,	4 },
	{ &op_nop_o, &mode_imp_o,	2 },  /* $7a nop */
	{ &op_rra_o, &mode_absy_o,	7 }, /* $7b rra $ffff,y */
	{ &op_nop_o, &mode_absx_o,	4 }, /* $7c nop $ffff,x */
	{ &op_adc_o, &mode_absx_o,	4 },
	{ &op_ror_o, &mode_absx_o,	7 },
	{ &op_rra_o, &mode_absx_o,	7 }, /* $7f rra $ffff,x */
	/* 0x80 */
	{ &op_nop_o, &mode_imm_o,	2 }, /* $80 nop #$ff */
	{ &op_sta_o, &mode_indx_o,	6 },
	{ &op_nop_o, &mode_imm_o,	2 },  /* $82 nop #$ff */
	{ &op_sax_o, &mode_indx_o,	6 }, /* $83 sax ($ff,x) */
	{ &op_sty_o, &mode_zp_o,	3 },
	{ &op_sta_o, &mode_zp_o,	3 },
	{ &op_stx_o, &mode_zp_o,	3 },
	{ &op_sax_o, &mode_zp_o,	3 }, /* $87 sax $ff */
	{ &op_dey_o, &mode_imp_o,	2 },
	{ &op_nop_o, &mode_imm_o,	2 }, /* $89 nop #$ff */
	{ &op_txa_o, &mode_imp_o,	2 },
	{ &op_ane_o, &mode_imm_o,	2 }, /* $8b ane #$ff */
	{ &op_sty_o, &mode_abs_o,	4 },
	{ &op_sta_o, &mode_abs_o,	4 },
	{ &op_stx_o, &mode_abs_o,	4 },
	{ &op_sax_o, &mode_abs_o,	4 }, /* $8f sax $ffff */
	/* 0x90 */
	{ &op_bcc_o, &mode_rel_o,	2 },
	{ &op_sta_o, &mode_indy_o,	6 },
	NULL_OP,
	NULL_OP,
	{ &op_sty_o, &mode_zpx_o,	4 },
	{ &op_sta_o, &mode_zpx_o,	4 },
	{ &op_stx_o, &mode_zpy_o,	4 },
	{ &op_sax_o, &mode_zpy_o,	4 }, /* $97 sax $ff,y */
	{ &op_tya_o, &mode_imp_o,	2 },
	{ &op_sta_o, &mode_absy_o,	5 },
	{ &op_txs_o, &mode_imp_o,	2 },
	NULL_OP,
	NULL_OP,
	{ &op_sta_o, &mode_absx_o,	5 },
	NULL_OP,
	NULL_OP,
	/* 0xa0 */
	{ &op_ldy_o, &mode_imm_o,	2 },
	{ &op_lda_o, &mode_indx_o,	6 },
	{ &op_ldx_o, &mode_imm_o,	2 },
	{ &op_lax_o, &mode_indx_o,	6 }, /* $a3 lax ($ff,x) */
	{ &op_ldy_o, &mode_zp_o,	3 },
	{ &op_lda_o, &mode_zp_o,	3 },
	{ &op_ldx_o, &mode_zp_o,	3 },
	{ &op_lax_o, &mode_zp_o,	3 }, /* $a7 lax $ff */
	{ &op_tay_o, &mode_imp_o,	2 },
	{ &op_lda_o, &mode_imm_o,	2 },
	{ &op_tax_o, &mode_imp_o,	2 },
	{ &op_lxa_o, &mode_imm_o,	2 }, /* $ab lxa #$ff */
	{ &op_ldy_o, &mode_abs_o,	4 },
	{ &op_lda_o, &mode_abs_o,	4 },
	{ &op_ldx_o, &mode_abs_o,	4 },
	{ &op_lax_o, &mode_abs_o,	4 }, /* $af lax $ffff */
	/* 0xb0 */
	{ &op_bcs_o, &mode_rel_o,	2 },
	{ &op_lda_o, &mode_indy_o,	5 },
	NULL_OP,
	{ &op_lax_o, &mode_indy_o,	5 }, /* $b3 lax ($ff),y */
	{ &op_ldy_o, &mode_zpx_o,	4 },
	{ &op_lda_o, &mode_zpx_o,	4 },
	{ &op_ldx_o, &mode_zpy_o,	4 },
	{ &op_lax_o, &mode_zpy_o,	4 }, /* $b7 lax $ff,y */
	{ &op_clv_o, &mode_imp_o,	2 },
	{ &op_lda_o, &mode_absy_o,	4 },
	{ &op_tsx_o, &mode_imp_o,	2 },
	{ &op_lae_o, &mode_absy_o,	4 }, /* $bb lae $ffff,y */
	{ &op_ldy_o, &mode_absx_o,	4 },
	{ &op_lda_o, &mode_absx_o,	4 },
	{ &op_ldx_o, &mode_absy_o,	4 },
	{ &op_lax_o, &mode_absy_o,	4 }, /* $bf lax $ffff,y */
	/* 0xc0 */
	{ &op_cpy_o, &mode_imm_o,	2 },
	{ &op_cmp_o, &mode_indx_o,	6 },
	{ &op_nop_o, &mode_imm_o,	2 },  /* $c2 nop #$ff */
	{ &op_dcp_o, &mode_indx_o,	8 }, /* $c3 dcp ($ff,x) */
	{ &op_cpy_o, &mode_zp_o,	3 },
	{ &op_cmp_o, &mode_zp_o,	3 },
	{ &op_dec_o, &mode_zp_o,	5 },
	{ &op_dcp_o, &mode_zp_o,	5 }, /* $c7 dcp $FF */
	{ &op_iny_o, &mode_imp_o,	2 },
	{ &op_cmp_o, &mode_imm_o,	2 },
	{ &op_dex_o, &mode_imp_o,	2 },
	{ &op_sbx_o, &mode_imm_o,	2 }, /* $cb sbx #$ff */
	{ &op_cpy_o, &mode_abs_o,	4 },
	{ &op_cmp_o, &mode_abs_o,	4 },
	{ &op_dec_o, &mode_abs_o,	6 },
	{ &op_dcp_o, &mode_abs_o,	6 }, /* $cf dcp $ffff */
	/* 0xd0 */
	{ &op_bne_o, &mode_rel_o,	2 },
	{ &op_cmp_o, &mode_indy_o,	5 },
	NULL_OP,
	{ &op_dcp_o, &mode_indy_o,	8 }, /* $d3 dcp ($ff),y */
	{ &op_nop_o, &mode_zpx_o,	4 },  /* $d4 nop $ff,x */
	{ &op_cmp_o, &mode_zpx_o,	4 },
	{ &op_dec_o, &mode_zpx_o,	6 },
	{ &op_dcp_o, &mode_zpx_o,	6 }, /* $d7 dcp $ff,x */
	{ &op_cld_o, &mode_imp_o,	2 },
	{ &op_cmp_o, &mode_absy_o,	4 },
	{ &op_nop_o, &mode_imp_o,	2 },  /* $da nop */
	{ &op_dcp_o, &mode_absy_o,	7 }, /* $db dcp $ffff,y */
	{ &op_nop_o, &mode_absx_o,	4 }, /* $dc nop $ffff,x */
	{ &op_cmp_o, &mode_absx_o,	4 },
	{ &op_dec_o, &mode_absx_o,	7 },
	{ &op_dcp_o, &mode_absx_o,	7 }, /* $df dcp $ffff,x */
	/* 0xe0 */
	{ &op_cpx_o, &mode_imm_o,	2 },
	{ &op_sbc_o, &mode_indx_o,	6 },
	{ &op_nop_o, &mode_imm_o,	2 },  /* $e2 nop #$ff */
	{ &op_isb_o, &mode_indx_o,	8 }, /* $e3 isb ($ff,x) */
	{ &op_cpx_o, &mode_zp_o,	3 },
	{ &op_sbc_o, &mode_zp_o,	3 },
	{ &op_inc_o, &mode_zp_o,	5 },
	{ &op_isb_o, &mode_zp_o,	5 }, /* $e7 isb $ff */
	{ &op_inx_o, &mode_imp_o,	2 },
	{ &op_sbc_o, &mode_imm_o,	2 },
	{ &op_nop_o, &mode_imp_o,	2 },
	{ &op_sbc_o, &mode_imm_o,	2 }, /* $eb sbc #$ff */
	{ &op_cpx_o, &mode_abs_o,	4 },
	{ &op_sbc_o, &mode_abs_o,	4 },
	{ &op_inc_o, &mode_abs_o,	6 },
	{ &op_isb_o, &mode_abs_o,	6 }, /* $ef isb $ffff */
	/* 0xf0 */
	{ &op_beq_o, &mode_rel_o,	2 },
	{ &op_sbc_o, &mode_indy_o,	5 },
	NULL_OP,
	{ &op_isb_o, &mode_indy_o,	8 }, /* $f3 isb ($ff),y */
	{ &op_nop_o, &mode_zpx_o,	4 },  /* $f4 nop $ff,x */
	{ &op_sbc_o, &mode_zpx_o,	4 },
	{ &op_inc_o, &mode_zpx_o,	6 },
	{ &op_isb_o, &mode_zpx_o,	6 }, /* $f7 isb $ff,x */
	{ &op_sed_o, &mode_imp_o,	2 },
	{ &op_sbc_o, &mode_absy_o,	4 },
	{ &op_nop_o, &mode_imp_o,	2 },  /* $fa nop */
	{ &op_isb_o, &mode_absy_o,	7 }, /* $fb isb $ffff,y */
	{ &op_nop_o, &mode_absx_o,	7 }, /* $fc nop $ffff,x */
	{ &op_sbc_o, &mode_absx_o,	4 },
	{ &op_inc_o, &mode_absx_o,	7 },
	{ &op_isb_o, &mode_absx_o,	7 }, /* $ff isb $ffff,x */
};

int flipfire(void) {
	_G(_retfire) ^= 0x90;
	return _G(_retfire);
}
int flipspace(void) {
	_G(_retspace) ^= 0x10;
	return _G(_retspace);
}

int nextInst(CpuCtx* r) {
	InstArg arg[1];
	int oldpc = r->_pc;
	int opCode = r->_mem[r->_pc];
	InstInfo *info = g_ops + opCode;
	int mode, WriteToIO = 0;
	int bt = 0, br;
	if (info->_op == nullptr) {
		return 1;
	}
	mode = info->_mode->_f(r, arg);

	/* iAN: only if RAM is not visible there! */
	if (((r->_mem[1] & 0x7) >= 0x5) && ((r->_mem[1] & 0x07) <= 0x7)) {
		if (arg->_ea._value >= 0xd000 && arg->_ea._value < 0xe000) {
			/* is this an absolute sta, stx or sty to the IO-area? */
			/* iAN: added all possible writing opcodes */
			if (opCode == 0x0E || /*ASL $ffff  */
				opCode == 0x1E || /*ASL $ffff,X*/
				opCode == 0xCE || /*DEC $ffff  */
				opCode == 0xDE || /*DEC $ffff,X*/
				opCode == 0xEE || /*INC $ffff  */
				opCode == 0xFE || /*INC $ffff,X*/
				opCode == 0x4E || /*LSR $ffff  */
				opCode == 0x5E || /*LSR $ffff,X*/
				opCode == 0x2E || /*ROL $ffff  */
				opCode == 0x3E || /*ROL $ffff,X*/
				opCode == 0x6E || /*ROR $ffff  */
				opCode == 0x7E || /*ROR $ffff,X*/
				opCode == 0x8D || /*STA $ffff  */
				opCode == 0x9D || /*STA $ffff,X*/
				opCode == 0x99 || /*STA $ffff,Y*/
				opCode == 0x91 || /*STA ($ff),Y*/
				opCode == 0x8E || /*STX $ffff  */
				opCode == 0x8C    /*STY $ffff  */
			) {
				r->_cycles += g_ops[opCode]._cycles;
				/* ignore it, its probably an effect */
				/* try to keep updated at least a copy of $d011 */
				if (arg->_ea._value == 0xd011) {
					switch (opCode) {
					case 0x8D:
						_G(_byted011)[0] = r->_a & 0x7f;
					case 0x8E:
						_G(_byted011)[0] = r->_x & 0x7f;
					case 0x8C:
						_G(_byted011)[0] = r->_y & 0x7f;
					}
				}
				WriteToIO = 1;
			} else {
				_G(_byted011)[1] = (r->_cycles / 0x3f) % 0x157;
				_G(_byted011)[0] = (_G(_byted011)[0] & 0x7f) | ((_G(_byted011)[1] & 0x100) >> 1);
				_G(_byted011)[1] &= 0xff;
				switch (opCode) {
				case 0xad:
				case 0xaf: /* lda $ffff / lax $ffff */

					if ((arg->_ea._value == 0xd011) || (arg->_ea._value == 0xd012)) {
						r->_cycles += g_ops[opCode]._cycles;
						r->_a = _G(_byted011)[arg->_ea._value - 0xd011];
						if (opCode == 0xaf)
							r->_x = r->_a;
						updateFlagsNz(r, r->_a);
						WriteToIO = 5;
						break;
					}

					/* intros: simulate space */
					if (arg->_ea._value == 0xdc00 || arg->_ea._value == 0xdc01) {
						r->_cycles += g_ops[opCode]._cycles;
						if (arg->_ea._value == 0xdc00)
							r->_a = flipfire();
						else
							r->_a = flipspace();
						if (opCode == 0xaf)
							r->_x = r->_a;
						updateFlagsNz(r, r->_a);
						WriteToIO = 6;
						break;
					}

					if (arg->_ea._value >= 0xdd01 && arg->_ea._value <= 0xdd0f) {
						r->_cycles += g_ops[opCode]._cycles;
						r->_a = 0xff;
						if (opCode == 0xaf)
							r->_x = r->_a;
						updateFlagsNz(r, r->_a);
						WriteToIO = 2;
						break;
					}
					break;
				case 0x2d: /* and $ffff */

					/* intros: simulate space */
					if (arg->_ea._value == 0xdc00 || arg->_ea._value == 0xdc01) {
						r->_cycles += g_ops[opCode]._cycles;
						if (arg->_ea._value == 0xdc00)
							r->_a &= flipfire();
						else
							r->_a &= flipspace();

						updateFlagsNz(r, r->_a);
						WriteToIO = 6;
						break;
					}
					break;
				case 0xae: /* ldx $ffff */

					if ((arg->_ea._value == 0xd011) || (arg->_ea._value == 0xd012)) {
						r->_cycles += g_ops[opCode]._cycles;
						r->_x = _G(_byted011)[arg->_ea._value - 0xd011];
						updateFlagsNz(r, r->_x);
						WriteToIO = 5;
						break;
					}
					if (arg->_ea._value == 0xdc00 || arg->_ea._value == 0xdc01) {
						r->_cycles += g_ops[opCode]._cycles;
						if (arg->_ea._value == 0xdc00)
							r->_x = flipfire();
						else
							r->_x = flipspace();
						updateFlagsNz(r, r->_x);
						WriteToIO = 6;
						break;
					}
					break;
				case 0xac: /* ldy $ffff */

					if ((arg->_ea._value == 0xd011) || (arg->_ea._value == 0xd012)) {
						r->_cycles += g_ops[opCode]._cycles;
						r->_y = _G(_byted011)[arg->_ea._value - 0xd011];
						updateFlagsNz(r, r->_y);
						WriteToIO = 5;
						break;
					}
					if (arg->_ea._value == 0xdc00 || arg->_ea._value == 0xdc01) {
						r->_cycles += g_ops[opCode]._cycles;
						if (arg->_ea._value == 0xdc00)
							r->_y = flipfire();
						else
							r->_y = flipspace();
						updateFlagsNz(r, r->_y);
						WriteToIO = 6;
						break;
					}
					break;

				case 0x2c: /* bit $d011 */

					if ((arg->_ea._value == 0xd011) || (arg->_ea._value == 0xd012)) {
						r->_cycles += g_ops[opCode]._cycles;
						bt = _G(_byted011)[arg->_ea._value - 0xd011];
						r->_flags &= ~(FLAG_N | FLAG_V | FLAG_Z);
						r->_flags |= (bt & FLAG_N) != 0 ? FLAG_N : 0;
						r->_flags |= (bt & FLAG_V) != 0 ? FLAG_V : 0;
						r->_flags |= (bt & r->_a) == 0 ? FLAG_Z : 0;
						WriteToIO = 3;
					}
					break;

				case 0xcd: /* cmp $ffff */
				case 0xec: /* cpx $ffff */
				case 0xcc: /* cpy $ffff */
					if ((arg->_ea._value == 0xd011) || (arg->_ea._value == 0xd012)) {
						r->_cycles += g_ops[opCode]._cycles;
						bt = _G(_byted011)[arg->_ea._value - 0xd011];
						br = r->_a;
						if (opCode == 0xec)
							br = r->_x;
						if (opCode == 0xcc)
							br = r->_y;
						subtract(r, 1, br, bt);
						WriteToIO = 4;
						break;
					}
					/* intros: simulate space */
					if (arg->_ea._value == 0xdc00 || arg->_ea._value == 0xdc01) {
						r->_cycles += g_ops[opCode]._cycles;
						br = r->_a;
						if (opCode == 0xec)
							br = r->_x;
						if (opCode == 0xcc)
							br = r->_y;
						if (arg->_ea._value == 0xdc00)
							bt = flipfire();
						else
							bt = flipspace();
						subtract(r, 1, br, bt);
						WriteToIO = 6;
						break;
					}
					break;
				}
			}
		}
	}
	if (WriteToIO) {
		return 0;
	}

	info->_op->_f(r, mode, arg);
	r->_cycles += info->_cycles;
	if (opCode == 0) {
		return 1;
	}
	return 0;
}

} // End of namespace Scott
} // End of namespace Glk
