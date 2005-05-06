/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// Video script opcodes for Simon1/Simon2
#include "stdafx.h"
#include "simon/simon.h"
#include "simon/intern.h"
#include "simon/vga.h"

#include "common/system.h"

namespace Simon {

typedef void (SimonEngine::*VgaOpcodeProc) ();

// Opcode tables
static const VgaOpcodeProc vga_opcode_table[] = {
	NULL,
	&SimonEngine::vc_1_dummy_op,
	&SimonEngine::vc_2_call,
	&SimonEngine::vc_3_new_sprite,
	&SimonEngine::vc_4_dummy_op,
	&SimonEngine::vc_5_skip_if_neq,
	&SimonEngine::vc_6_skip_ifn_sib_with_a,
	&SimonEngine::vc_7_skip_if_sib_with_a,
	&SimonEngine::vc_8_skip_if_parent_is,
	&SimonEngine::vc_9_skip_if_unk3_is,
	&SimonEngine::vc_10_draw,
	&SimonEngine::vc_11_clear_pathfind_array,
	&SimonEngine::vc_12_delay,
	&SimonEngine::vc_13_set_sprite_offset_x,
	&SimonEngine::vc_14_set_sprite_offset_y,
	&SimonEngine::vc_15_wakeup_id,
	&SimonEngine::vc_16_sleep_on_id,
	&SimonEngine::vc_17_set_pathfind_item,
	&SimonEngine::vc_18_jump_rel,
	&SimonEngine::vc_19_chain_to_script,
	&SimonEngine::vc_20_set_code_word,
	&SimonEngine::vc_21_jump_if_code_word,
	&SimonEngine::vc_22_set_sprite_palette,
	&SimonEngine::vc_23_set_sprite_priority,
	&SimonEngine::vc_24_set_sprite_xy,
	&SimonEngine::vc_25_halt_sprite,
	&SimonEngine::vc_26_set_window,
	&SimonEngine::vc_27_reset,
	&SimonEngine::vc_28_dummy_op,
	&SimonEngine::vc_29_stop_all_sounds,
	&SimonEngine::vc_30_set_base_delay,
	&SimonEngine::vc_31_set_palette_mode,
	&SimonEngine::vc_32_copy_var,
	&SimonEngine::vc_33_force_unlock,
	&SimonEngine::vc_34_force_lock,
	&SimonEngine::vc_35,
	&SimonEngine::vc_36_saveload_thing,
	&SimonEngine::vc_37_set_sprite_offset_y,
	&SimonEngine::vc_38_skip_if_var_zero,
	&SimonEngine::vc_39_set_var,
	&SimonEngine::vc_40_var_add,
	&SimonEngine::vc_41_var_sub,
	&SimonEngine::vc_42_delay_if_not_eq,
	&SimonEngine::vc_43_skip_if_bit_clear,
	&SimonEngine::vc_44_skip_if_bit_set,
	&SimonEngine::vc_45_set_sprite_x,
	&SimonEngine::vc_46_set_sprite_y,
	&SimonEngine::vc_47_add_var_f,
	&SimonEngine::vc_48,
	&SimonEngine::vc_49_set_bit,
	&SimonEngine::vc_50_clear_bit,
	&SimonEngine::vc_51_clear_hitarea_bit_0x40,
	&SimonEngine::vc_52_play_sound,
	&SimonEngine::vc_53_no_op,
	&SimonEngine::vc_54_no_op,
	&SimonEngine::vc_55_offset_hit_area,
	&SimonEngine::vc_56_delay,
	&SimonEngine::vc_57_no_op,
	&SimonEngine::vc_58,
	&SimonEngine::vc_59,
	&SimonEngine::vc_60_kill_sprite,
	&SimonEngine::vc_61_sprite_change,
	&SimonEngine::vc_62_palette_thing,
	&SimonEngine::vc_63_palette_thing_2,
	&SimonEngine::vc_64_skip_if_no_speech,
	&SimonEngine::vc_65_palette_thing_3,
	&SimonEngine::vc_66_skip_if_nz,
	&SimonEngine::vc_67_skip_if_ge,
	&SimonEngine::vc_68_skip_if_le,
	&SimonEngine::vc_69_play_track,
	&SimonEngine::vc_70_queue_music,
	&SimonEngine::vc_71_check_music_queue,
	&SimonEngine::vc_72_play_track_2,
	&SimonEngine::vc_73_set_op189_flag,
	&SimonEngine::vc_74_clear_op189_flag,
};

// Script parser
void SimonEngine::run_vga_script() {
	for (;;) {
		uint opcode;

		if (_continousVgaScript) {
			if (_vcPtr != (const byte *)&_vc_get_out_of_code) {
				fprintf(_dumpFile, "%.5d %.5X: %5d %4d ", _vgaTickCounter, _vcPtr - _curVgaFile1, _vgaCurSpriteId, _vgaCurFileId);
				dump_video_script(_vcPtr, true);
			}
		}

		if (_game & GF_SIMON1) {
			opcode = READ_BE_UINT16(_vcPtr);
			_vcPtr += 2;
		} else {
			opcode = *_vcPtr++;
		}

		if (opcode >= NUM_VIDEO_OP_CODES)
			error("Invalid VGA opcode '%d' encountered", opcode);

		if (opcode == 0)
			return;

		(this->*vga_opcode_table[opcode]) ();
	}
}

int SimonEngine::vc_read_var_or_word() {
	int16 var = vc_read_next_word();
	if (var < 0)
		var = vc_read_var(-var);
	return var;
}

uint SimonEngine::vc_read_next_word() {
	uint a = READ_BE_UINT16(_vcPtr);
	_vcPtr += 2;
	return a;
}

uint SimonEngine::vc_read_next_byte() {
	return *_vcPtr++;
}

void SimonEngine::vc_skip_next_instruction() {
	static const byte opcode_param_len_simon1[] = {
		0, 6, 2, 10, 6, 4, 2, 2,
		4, 4, 10, 0, 2, 2, 2, 2,
		2, 0, 2, 0, 4, 2, 4, 2,
		8, 0, 10, 0, 8, 0, 2, 2,
		4, 0, 0, 4, 4, 2, 2, 4,
		4, 4, 4, 2, 2, 2, 2, 4,
		0, 2, 2, 2, 2, 4, 6, 6,
		0, 0, 0, 0, 2, 6, 0, 0,
	};

	static const byte opcode_param_len_simon2[] = {
		0, 6, 2, 12, 6, 4, 2, 2,
		4, 4, 9, 0, 1, 2, 2, 2,
		2, 0, 2, 0, 4, 2, 4, 2,
		7, 0, 10, 0, 8, 0, 2, 2,
		4, 0, 0, 4, 4, 2, 2, 4,
		4, 4, 4, 2, 2, 2, 2, 4,
		0, 2, 2, 2, 2, 4, 6, 6,
		2, 0, 6, 6, 4, 6, 0, 0,
		0, 0, 4, 4, 4, 4, 4, 0,
		4, 2, 2
	};

	if (_game & GF_SIMON2) {
		uint opcode = vc_read_next_byte();
		_vcPtr += opcode_param_len_simon2[opcode];
	} else {
		uint opcode = vc_read_next_word();
		_vcPtr += opcode_param_len_simon1[opcode];
	}

	if (_continousVgaScript)
		fprintf(_dumpFile, "; skipped\n");
}

void SimonEngine::o_read_vgares_23() {
	// Simon1 Only
	if (_vgaRes328Loaded == true) {
		_vgaRes328Loaded = false;
		_lockWord |= 0x4000;
		read_vga_from_datfile_1(23);
		_lockWord &= ~0x4000;
	}
}

void SimonEngine::o_read_vgares_328() {
	// Simon1 Only
	if (_vgaRes328Loaded == false) {
		_vgaRes328Loaded = true;
		_lockWord |= 0x4000;
		read_vga_from_datfile_1(328);
		_lockWord &= ~0x4000;
	}
}

// VGA Script commands
void SimonEngine::vc_1_dummy_op() {
	/* dummy opcode */
	_vcPtr += 6;
}

void SimonEngine::vc_2_call() {
	VgaPointersEntry *vpe;
	uint num;
	uint res;
	byte *old_file_1, *old_file_2;
	byte *b, *bb;
	const byte *vc_ptr_org;

	num = vc_read_var_or_word();

	old_file_1 = _curVgaFile1;
	old_file_2 = _curVgaFile2;

	for (;;) {
		res = num / 100;
		vpe = &_vgaBufferPointers[res];

		_curVgaFile1 = vpe->vgaFile1;
		_curVgaFile2 = vpe->vgaFile2;
		if (vpe->vgaFile1 != NULL)
			break;
		if (_vgaCurFile2 != res)
			_videoVar7 = _vgaCurFile2;

		ensureVgaResLoaded(res);
		_videoVar7 = 0xFFFF;
	}


	bb = _curVgaFile1;
	b = bb + READ_BE_UINT16(&((VgaFile1Header *) bb)->hdr2_start);
	b = bb + READ_BE_UINT16(&((VgaFile1Header2 *) b)->unk2_offs);

	while (READ_BE_UINT16(&((VgaFile1Struct0x8 *) b)->id) != num)
		b += sizeof(VgaFile1Struct0x8);

	vc_ptr_org = _vcPtr;

	_vcPtr = _curVgaFile1 + READ_BE_UINT16(&((VgaFile1Struct0x8 *) b)->script_offs);

	//dump_vga_script(_vcPtr, res, num);
	run_vga_script();

	_curVgaFile1 = old_file_1;
	_curVgaFile2 = old_file_2;

	_vcPtr = vc_ptr_org;
}

void SimonEngine::vc_3_new_sprite() {
	uint16 paletteMode, vga_res, base_color, x, y, vgaSpriteId;
	uint16 res;
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;
	byte *old_file_1;

	paletteMode = vc_read_next_word();		/* 0 */

	if (_game & GF_SIMON2) {
		vga_res = vc_read_next_word();		/* 0 */
		vgaSpriteId = vc_read_next_word();	/* 2 */
	} else {
		vgaSpriteId = vc_read_next_word();	/* 2 */
		vga_res = vgaSpriteId / 100;
	}

	x = vc_read_next_word();			/* 4 */
	y = vc_read_next_word();			/* 6 */
	base_color = vc_read_next_word();		/* 8 */

	/* 2nd param ignored with simon1 */
	if (has_vga_sprite_with_id(vgaSpriteId, vga_res))
		return;

	vsp = _vgaSprites;
	while (vsp->id)
		vsp++;

	vsp->base_color = base_color;
	vsp->paletteMode = paletteMode;
	vsp->priority = 0;
	vsp->flags = 0;
	vsp->image = 0;
	vsp->x = x;
	vsp->y = y;
	vsp->id = vgaSpriteId;
	vsp->unk7 = res = vga_res;

	old_file_1 = _curVgaFile1;
	for (;;) {
		vpe = &_vgaBufferPointers[res];
		_curVgaFile1 = vpe->vgaFile1;

		if (vpe->vgaFile1 != NULL)
			break;
		if (_vgaCurFile2 != res)
			_videoVar7 = _vgaCurFile2;

		ensureVgaResLoaded(res);
		_videoVar7 = 0xFFFF;
	}

	pp = _curVgaFile1;
	p = pp + READ_BE_UINT16(&((VgaFile1Header *) pp)->hdr2_start);
	p = pp + READ_BE_UINT16(&((VgaFile1Header2 *) p)->id_table);

	while (READ_BE_UINT16(&((VgaFile1Struct0x6 *) p)->id) != vgaSpriteId)
		p += sizeof(VgaFile1Struct0x6);

#ifdef DUMP_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_FILE_NR && !dumped) {
			dumped = true;
			dump_vga_file(_curVgaFile1);
		}
	}
#endif

#ifdef DUMP_BITMAPS_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_BITMAPS_FILE_NR && !dumped) {
			dumped = true;
			dump_vga_bitmaps(_curVgaFile2, _curVgaFile1, res);
		}
	}
#endif

	if (_startVgaScript)
		dump_vga_script(_curVgaFile1 + READ_BE_UINT16(&((VgaFile1Struct0x6*)p)->script_offs), res, vgaSpriteId);

	add_vga_timer(VGA_DELAY_BASE, _curVgaFile1 + READ_BE_UINT16(&((VgaFile1Struct0x6 *) p)->script_offs), vgaSpriteId, res);
	_curVgaFile1 = old_file_1;
}

void SimonEngine::vc_4_dummy_op() {
	/* dummy opcode */
	_vcPtr += 6;
}

void SimonEngine::vc_5_skip_if_neq() {
	uint var = vc_read_next_word();
	uint value = vc_read_next_word();
	if (vc_read_var(var) != value)
		vc_skip_next_instruction();
}

void SimonEngine::vc_6_skip_ifn_sib_with_a() {
	if (!itemIsSiblingOf(vc_read_next_word()))
		vc_skip_next_instruction();
}

void SimonEngine::vc_7_skip_if_sib_with_a() {
	if (itemIsSiblingOf(vc_read_next_word()))
		vc_skip_next_instruction();
}

void SimonEngine::vc_8_skip_if_parent_is() {		
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	if (!itemIsParentOf(a, b))
		vc_skip_next_instruction();
}

void SimonEngine::vc_9_skip_if_unk3_is() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	if (!vc_maybe_skip_proc_1(a, b))
		vc_skip_next_instruction();
}

byte *vc_10_depack_column(VC10_state * vs) {
	int8 a = vs->depack_cont;
	const byte *src = vs->depack_src;
	byte *dst = vs->depack_dest;
	byte dh = vs->dh;
	byte color;

	if (a == -0x80)
		a = *src++;

	for (;;) {
		if (a >= 0) {
			color = *src++;
			do {
				*dst++ = color;
				if (!--dh) {
					if (--a < 0)
						a = -0x80;
					else
						src--;
					goto get_out;
				}
			} while (--a >= 0);
		} else {
			do {
				*dst++ = *src++;
				if (!--dh) {
					if (++a == 0)
						a = -0x80;
					goto get_out;
				}
			} while (++a != 0);
		}
		a = *src++;
	}

get_out:;
	vs->depack_src = src;
	vs->depack_cont = a;
	return vs->depack_dest + vs->y_skip;
}

void vc_10_skip_cols(VC10_state *vs) {
	vs->depack_cont = -0x80;
	while (vs->x_skip) {
		vc_10_depack_column(vs);
		vs->x_skip--;
	}
}

byte *SimonEngine::vc_10_depack_swap(const byte *src, uint w, uint h) {
	w <<= 3;

	{
		byte *dst_org = _videoBuf1 + w;
		byte color;
		int8 cur = -0x80;
		uint w_cur = w;

		do {
			byte *dst = dst_org;
			uint h_cur = h;

			if (cur == -0x80)
				cur = *src++;

			for (;;) {
				if (cur >= 0) {
					/* rle_same */
					color = *src++;
					do {
						*dst = color;
						dst += w;
						if (!--h_cur) {
							if (--cur < 0)
								cur = -0x80;
							else
								src--;
							goto next_line;
						}
					} while (--cur >= 0);
				} else {
					/* rle_diff */
					do {
						*dst = *src++;
						dst += w;
						if (!--h_cur) {
							if (++cur == 0)
								cur = -0x80;
							goto next_line;
						}
					} while (++cur != 0);
				}
				cur = *src++;
			}
		next_line:
			dst_org++;
		} while (--w_cur);
	}

	{
		byte *dst_org, *src_org;
		uint i;

		src_org = dst_org = _videoBuf1 + w;

		do {
			byte *dst = dst_org;
			for (i = 0; i != w; ++i) {
				byte b = src_org[i];
				b = (b >> 4) | (b << 4);
				*--dst = b;
			}

			src_org += w;
			dst_org += w;
		} while (--h);

	}

	return _videoBuf1;
}

byte *SimonEngine::vc_10_no_depack_swap(const byte *src, uint w, uint h) {
	if (src == _vc10BasePtrOld)
		return _videoBuf1;

	_vc10BasePtrOld = src;
	h *= 8;
	byte *dst = _videoBuf1 + h - 1;

	uint h_cur = h;
	do {
		do {
			*dst = *src << 4;
			(*dst--) |= (*src++) >> 4;
		} while (--h_cur != 0);
		h_cur = h;
		dst += h * 2;
	} while (--w != 0);

	return _videoBuf1;
}

/* must not be const */
static uint16 _video_windows[128] = {
	0, 0, 20, 200,
	0, 0, 3, 136,
	17, 0, 3, 136,
	0, 0, 20, 200,
	0, 0, 20, 134
};

/* simon2 specific */
void SimonEngine::decodeStripA(byte *dst, const byte *src, int height) {
	const uint pitch = _dxSurfacePitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dst_org = dst;
	uint h = height, w = 8;

	for (;;) {
		reps = *src++;
		if (reps >= 0) {
			color = *src++;

			do {
				*dst = color;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = height;
				}
			} while (--reps >= 0);
		} else {

			do {
				*dst = *src++;
				dst += pitch;

				/* reached bottom? */
				if (--h == 0) {
					/* reached right edge? */
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = height;
				}
			} while (++reps != 0);
		}
	}
}

void SimonEngine::vc_10_draw() {
	byte *p2;
	uint width, height;
	byte flags;
	const uint16 *vlut;
	VC10_state state;

	int cur;

	state.image = (int16)vc_read_next_word();
	if (state.image == 0)
		return;

	state.base_color = (_vcPtr[1] << 4);
	_vcPtr += 2;
	state.x = (int16)vc_read_next_word();
	if (_game & GF_SIMON2) {
		state.x -= _xScroll;
	}
	state.y = (int16)vc_read_next_word();

	if (!(_game & GF_SIMON2)) {
		state.flags = vc_read_next_word();
	} else {
		state.flags = vc_read_next_byte();
	}

	if (state.image < 0)
		state.image = vc_read_var(-state.image);

	p2 = _curVgaFile2 + state.image * 8;
	state.depack_src = _curVgaFile2 + READ_BE_UINT32(p2);

	width = READ_BE_UINT16(p2 + 6) >> 4;
	height = p2[5];
	flags = p2[4];

	if (height == 0 || width == 0)
		return;

	if (_dumpImages)
		dump_single_bitmap(_vgaCurFileId, state.image, state.depack_src, width * 16, height,
											 state.base_color);

	if (flags & 0x80 && !(state.flags & 0x10)) {
		if (state.flags & 1) {
			state.flags &= ~1;
			state.flags |= 0x10;
		} else {
			state.flags |= 0x8;
		}
	}

	if (_game & GF_SIMON2 && width >= 21) {
		const byte *src;
		byte *dst;
		uint w;

		_vgaVar1 = width * 2 - 40;
		_vgaVar7 = state.depack_src;
		_spriteHeight = height;
		if (_variableArray[34] == -1)
			state.x = _variableArray[251];

		_xScroll = state.x;

		vc_write_var(0xfb, _xScroll);

		dst = dx_lock_attached();
		src = state.depack_src + _xScroll * 4;

		for (w = 0; w < 40; w++) {
			decodeStripA(dst, src + READ_BE_UINT32(src), height);
			dst += 8;
			src += 4;
		}

		dx_unlock_attached();

		return;
	}

	if (state.flags & 0x10) {
		state.depack_src = vc_10_depack_swap(state.depack_src, width, height);
	} else if (state.flags & 1) {
		state.depack_src = vc_10_no_depack_swap(state.depack_src, width, height);
	}

	vlut = &_video_windows[_videoPaletteMode * 4];

	state.draw_width = width << 1;	/* cl */
	state.draw_height = height;		/* ch */

	state.x_skip = 0;							/* colums to skip = bh */
	state.y_skip = 0;							/* rows to skip   = bl */

	cur = state.x;
	if (cur < 0) {
		do {
			if (!--state.draw_width)
				return;
			state.x_skip++;
		} while (++cur);
	}
	state.x = cur;

	cur += state.draw_width - (vlut[2] << 1);
	if (cur > 0) {
		do {
			if (!--state.draw_width)
				return;
		} while (--cur);
	}

	cur = state.y;
	if (cur < 0) {
		do {
			if (!--state.draw_height)
				return;
			state.y_skip++;
		} while (++cur);
	}
	state.y = cur;

	cur += state.draw_height - vlut[3];
	if (cur > 0) {
		do {
			if (!--state.draw_height)
				return;
		} while (--cur);
	}

	assert(state.draw_width != 0 && state.draw_height != 0);

	state.draw_width <<= 2;

	state.surf2_addr = dx_lock_2();
	state.surf2_pitch = _dxSurfacePitch;

	state.surf_addr = dx_lock_attached();
	state.surf_pitch = _dxSurfacePitch;

	{
		uint offs, offs2;
		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		if ((_game & GF_SIMON1) && _subroutine == 2926) {
			offs = ((vlut[0]) * 2 + state.x) * 8;
			offs2 = (vlut[1] + state.y);
		} else {
			offs = ((vlut[0] - _video_windows[16]) * 2 + state.x) * 8;
			offs2 = (vlut[1] - _video_windows[17] + state.y);
		}

		state.surf2_addr += offs + offs2 * state.surf2_pitch;
		state.surf_addr += offs + offs2 * state.surf_pitch;
	}

	if (state.flags & 0x20) {
		byte *mask, *src, *dst;
		byte h;
		uint w;

		state.x_skip <<= 2;
		state.dl = width;
		state.dh = height;

		vc_10_skip_cols(&state);

		w = 0;
		do {
			mask = vc_10_depack_column(&state);	/* esi */
			src = state.surf2_addr + w * 2;	/* ebx */
			dst = state.surf_addr + w * 2;	/* edi */

			h = state.draw_height;
			if ((_game & GF_SIMON1) && vc_get_bit(88)) {
				/* transparency */
				do {
					if (mask[0] & 0xF0) {
						if ((dst[0] & 0x0F0) == 0x20)
							dst[0] = src[0];
					}
					if (mask[0] & 0x0F) {
						if ((dst[1] & 0x0F0) == 0x20)
							dst[1] = src[1];
					}
					mask++;
					dst += state.surf_pitch;
					src += state.surf2_pitch;
				} while (--h);
			} else {
				/* no transparency */
				do {
					if (mask[0] & 0xF0)
						dst[0] = src[0];
					if (mask[0] & 0x0F)
						dst[1] = src[1];
					mask++;
					dst += state.surf_pitch;
					src += state.surf2_pitch;
				} while (--h);
			}
		} while (++w != state.draw_width);

		/* vc_10_helper_5 */
	} else if (_lockWord & 0x20 && state.base_color == 0 || state.base_color == 0xC0) {
		const byte *src;
		byte *dst;
		uint h, i;

		if (!(state.flags & 8)) {
			src = state.depack_src + (width * state.y_skip << 4) + (state.x_skip << 3);
			dst = state.surf_addr;

			state.draw_width *= 2;

			if (state.flags & 2) {
				/* no transparency */
				h = state.draw_height;
				do {
					memcpy(dst, src, state.draw_width);
					dst += 320;
					src += width * 16;
				} while (--h);
			} else {
				/* transparency */
				h = state.draw_height;
				do {
					for (i = 0; i != state.draw_width; i++)
						if (src[i])
							dst[i] = src[i];
					dst += 320;
					src += width * 16;
				} while (--h);
			}

		} else {
			byte *dst_org = state.surf_addr;
			src = state.depack_src;
			/* AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD EEEEEEEE
			 * aaaaabbb bbcccccd ddddeeee efffffgg ggghhhhh
			 */

			if (state.flags & 2) {
				/* no transparency */
				do {
					uint count = state.draw_width >> 2;

					dst = dst_org;
					do {
						uint32 bits = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3]);

						dst[0] = (byte)((bits >> (32 - 5)) & 31);
						dst[1] = (byte)((bits >> (32 - 10)) & 31);
						dst[2] = (byte)((bits >> (32 - 15)) & 31);
						dst[3] = (byte)((bits >> (32 - 20)) & 31);
						dst[4] = (byte)((bits >> (32 - 25)) & 31);
						dst[5] = (byte)((bits >> (32 - 30)) & 31);

						bits = (bits << 8) | src[4];

						dst[6] = (byte)((bits >> (40 - 35)) & 31);
						dst[7] = (byte)((bits) & 31);

						dst += 8;
						src += 5;
					} while (--count);
					dst_org += 320;
				} while (--state.draw_height);
			} else {
				/* transparency */
				do {
					uint count = state.draw_width >> 2;

					dst = dst_org;
					do {
						uint32 bits = (src[0] << 24) | (src[1] << 16) | (src[2] << 8) | (src[3]);
						byte tmp;

						tmp = (byte)((bits >> (32 - 5)) & 31);
						if (tmp)
							dst[0] = tmp;
						tmp = (byte)((bits >> (32 - 10)) & 31);
						if (tmp)
							dst[1] = tmp;
						tmp = (byte)((bits >> (32 - 15)) & 31);
						if (tmp)
							dst[2] = tmp;
						tmp = (byte)((bits >> (32 - 20)) & 31);
						if (tmp)
							dst[3] = tmp;
						tmp = (byte)((bits >> (32 - 25)) & 31);
						if (tmp)
							dst[4] = tmp;
						tmp = (byte)((bits >> (32 - 30)) & 31);
						if (tmp)
							dst[5] = tmp;

						bits = (bits << 8) | src[4];

						tmp = (byte)((bits >> (40 - 35)) & 31);
						if (tmp)
							dst[6] = tmp;
						tmp = (byte)((bits) & 31);
						if (tmp)
							dst[7] = tmp;

						dst += 8;
						src += 5;
					} while (--count);
					dst_org += 320;
				} while (--state.draw_height);
			}
		}
		/* vc_10_helper_4 */
	} else {
		if (_game & GF_SIMON2 && state.flags & 0x4 && _bitArray[10] & 0x800) {
			state.surf_addr = state.surf2_addr;
			state.surf_pitch = state.surf2_pitch;
		}

		if (state.flags & 0x8) {
			uint w, h;
			byte *src, *dst, *dst_org;

			state.x_skip <<= 2;				/* reached */
			state.dl = width;
			state.dh = height;

			vc_10_skip_cols(&state);

			if (state.flags & 2) {
				dst_org = state.surf_addr;
				w = 0;
				do {
					src = vc_10_depack_column(&state);
					dst = dst_org;

					h = 0;
					do {
						dst[0] = (*src >> 4) | state.base_color;
						dst[1] = (*src & 15) | state.base_color;
						dst += 320;
						src++;
					} while (++h != state.draw_height);
					dst_org += 2;
				} while (++w != state.draw_width);
			} else {
				dst_org = state.surf_addr;
				if (state.flags & 0x40) {		/* reached */
					dst_org += vc_read_var(252);
				}
				w = 0;
				do {
					byte color;

					src = vc_10_depack_column(&state);
					dst = dst_org;

					h = 0;
					do {
						color = (*src >> 4);
						if (color)
							dst[0] = color | state.base_color;
						color = (*src & 15);
						if (color)
							dst[1] = color | state.base_color;
						dst += 320;
						src++;
					} while (++h != state.draw_height);
					dst_org += 2;
				} while (++w != state.draw_width);
			}
			/* vc_10_helper_6 */
		} else {
			const byte *src;
			byte *dst;
			uint count;

			src = state.depack_src + (width * state.y_skip) * 8;
			dst = state.surf_addr;
			state.x_skip <<= 2;
			if (state.flags & 2) {
				do {
					for (count = 0; count != state.draw_width; count++) {
						dst[count * 2] = (src[count + state.x_skip] >> 4) | state.base_color;
						dst[count * 2 + 1] = (src[count + state.x_skip] & 15) | state.base_color;
					}
					dst += 320;
					src += width * 8;
				} while (--state.draw_height);
			} else {
				do {
					for (count = 0; count != state.draw_width; count++) {
						byte color;
						color = (src[count + state.x_skip] >> 4);
						if (color)
							dst[count * 2] = color | state.base_color;
						color = (src[count + state.x_skip] & 15);
						if (color)
							dst[count * 2 + 1] = color | state.base_color;
					}
					dst += 320;
					src += width * 8;
				} while (--state.draw_height);

			}

			/* vc_10_helper_7 */
		}
	}

	dx_unlock_2();
	dx_unlock_attached();

}

void SimonEngine::vc_11_clear_pathfind_array() {
	memset(&_pathFindArray, 0, sizeof(_pathFindArray));
}

void SimonEngine::vc_12_delay() {
	VgaSprite *vsp = find_cur_sprite();
	uint num;

	if (_game & GF_SIMON1) {
		num = vc_read_var_or_word();
	} else {
		num = vc_read_next_byte() * _vgaBaseDelay;
	}

	// Work around to allow inventory arrows to be
	// shown in some versions of Simon the Sorcerer 1
	if ((_game & GF_SIMON1) && vsp->id == 0x80)
		num = 0;
	else
		num += VGA_DELAY_BASE;

	add_vga_timer(num, _vcPtr, _vgaCurSpriteId, _vgaCurFileId);
	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void SimonEngine::vc_13_set_sprite_offset_x() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->x += (int16)vc_read_next_word();
	_vgaSpriteChanged++;
}

void SimonEngine::vc_14_set_sprite_offset_y() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->y += (int16)vc_read_next_word();
	_vgaSpriteChanged++;
}

void SimonEngine::vc_15_wakeup_id() {
	VgaSleepStruct *vfs = _vgaSleepStructs, *vfs_tmp;
	uint16 id = vc_read_next_word();
	while (vfs->ident != 0) {
		if (vfs->ident == id) {
			add_vga_timer(VGA_DELAY_BASE, vfs->code_ptr, vfs->sprite_id, vfs->cur_vga_file);
			vfs_tmp = vfs;
			do {
				memcpy(vfs_tmp, vfs_tmp + 1, sizeof(VgaSleepStruct));
				vfs_tmp++;
			} while (vfs_tmp->ident != 0);
		} else {
			vfs++;
		}
	}

	/* clear a wait event */
	if (id == _vgaWaitFor)
		_vgaWaitFor = 0;
}

void SimonEngine::vc_16_sleep_on_id() {
	VgaSleepStruct *vfs = _vgaSleepStructs;
	while (vfs->ident)
		vfs++;

	vfs->ident = vc_read_next_word();
	vfs->code_ptr = _vcPtr;
	vfs->sprite_id = _vgaCurSpriteId;
	vfs->cur_vga_file = _vgaCurFileId;

	_vcPtr = (byte *)&_vc_get_out_of_code;
}

void SimonEngine::vc_17_set_pathfind_item() {
	uint a = vc_read_next_word();
	_pathFindArray[a - 1] = (const uint16 *)_vcPtr;
	while (READ_BE_UINT16(_vcPtr) != 999)
		_vcPtr += 4;
	_vcPtr += 2;
}

void SimonEngine::vc_18_jump_rel() {
	int16 offs = vc_read_next_word();
	_vcPtr += offs;
}

/* chain to script? */
void SimonEngine::vc_19_chain_to_script() {
	/* unused */
	error("vc_19_chain_to_script: not implemented");
}

/* helper routines */

void SimonEngine::vc_20_set_code_word() {
	/* FIXME: This opcode is somewhat strange: it first reads a BE word from 
	 * the script (advancing the script pointer in doing so); then it writes
	 * back the same word, this time as LE, into the script.
	 */
	uint16 a = vc_read_next_word();
	WRITE_LE_UINT16(const_cast<byte *>(_vcPtr), a);
	_vcPtr += 2;
}

void SimonEngine::vc_21_jump_if_code_word() {
	int16 a = vc_read_next_word();
	const byte *tmp = _vcPtr + a;
	if (_game & GF_SIMON2)
		tmp += 3;
	else
		tmp += 4;

	uint16 val = READ_LE_UINT16(tmp);
	if (val != 0) {
		// Decrement counter
		WRITE_LE_UINT16(const_cast<byte *>(tmp), val - 1);
		_vcPtr = tmp + 2;
	}
}

void SimonEngine::vc_22_set_sprite_palette() {
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	uint num = a == 0 ? 0x20 : 0x10;
	byte *palptr, *src;

	palptr = &_palette[(a << 6)];

	src = _curVgaFile1 + 6 + b * 96;

	do {
		palptr[0] = src[0] << 2;
		palptr[1] = src[1] << 2;
		palptr[2] = src[2] << 2;
		palptr[3] = 0;

		palptr += 4;
		src += 3;
	} while (--num);

	_videoVar9 = 2;
	_vgaSpriteChanged++;
}

void SimonEngine::vc_23_set_sprite_priority() {
	VgaSprite *vsp = find_cur_sprite(), *vus2;
	uint16 pri = vc_read_next_word();
	VgaSprite bak;

	if (vsp->id == 0)
		return;

	memcpy(&bak, vsp, sizeof(bak));
	bak.priority = pri;
	bak.paletteMode |= 0x8000;

	vus2 = vsp;

	if (vsp != _vgaSprites && pri < vsp[-1].priority) {
		do {
			vsp--;
		} while (vsp != _vgaSprites && pri < vsp[-1].priority);
		do {
			memcpy(vus2, vus2 - 1, sizeof(VgaSprite));
		} while (--vus2 != vsp);
		memcpy(vus2, &bak, sizeof(VgaSprite));
	} else if (vsp[1].id != 0 && pri >= vsp[1].priority) {
		do {
			vsp++;
		} while (vsp[1].id != 0 && pri >= vsp[1].priority);
		do {
			memcpy(vus2, vus2 + 1, sizeof(VgaSprite));
		} while (++vus2 != vsp);
		memcpy(vus2, &bak, sizeof(VgaSprite));
	} else {
		vsp->priority = pri;
	}
	_vgaSpriteChanged++;
}

void SimonEngine::vc_24_set_sprite_xy() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->image = vc_read_var_or_word();

	vsp->x += (int16)vc_read_next_word();
	vsp->y += (int16)vc_read_next_word();
	if (_game & GF_SIMON1) {
		vsp->flags = vc_read_next_word();
	} else {
		vsp->flags = vc_read_next_byte();
	}

	_vgaSpriteChanged++;
}

void SimonEngine::vc_25_halt_sprite() {
	VgaSprite *vsp = find_cur_sprite();
	while (vsp->id != 0) {
		memcpy(vsp, vsp + 1, sizeof(VgaSprite));
		vsp++;
	}
	_vcPtr = (byte *)&_vc_get_out_of_code;
	_vgaSpriteChanged++;
}

void SimonEngine::vc_26_set_window() {
	uint16 *as = &_video_windows[vc_read_next_word() * 4];
	as[0] = vc_read_next_word();
	as[1] = vc_read_next_word();
	as[2] = vc_read_next_word();
	as[3] = vc_read_next_word();
}

void SimonEngine::vc_27_reset() {
	VgaSprite bak, *vsp;
	VgaSleepStruct *vfs;
	VgaTimerEntry *vte, *vte2;

	_lockWord |= 8;

	memset(&bak, 0, sizeof(bak));

	vsp = _vgaSprites;
	while (vsp->id) {
		if ((_game & GF_SIMON1) && vsp->id == 0x80) {
			memcpy(&bak, vsp, sizeof(VgaSprite));
		}
		vsp->id = 0;
		vsp++;
	}

	if (bak.id != 0)
		memcpy(_vgaSprites, &bak, sizeof(VgaSprite));

	vfs = _vgaSleepStructs;
	while (vfs->ident) {
		vfs->ident = 0;
		vfs++;
	}

	vte = _vgaTimerList;
	while (vte->delay) {
		if ((_game & GF_SIMON1) && vsp->id == 0x80) {
			vte++;
		} else {
			vte2 = vte;
			while (vte2->delay) {
				memcpy(vte2, vte2 + 1, sizeof(VgaTimerEntry));
				vte2++;
			}
		}
	}

	vc_write_var(0xFE, 0);

	_lockWord &= ~8;
}

void SimonEngine::vc_28_dummy_op() {
	/* unused */
	_vcPtr += 8;
}

void SimonEngine::vc_29_stop_all_sounds() {
	_sound->stopAll();
}

void SimonEngine::vc_30_set_base_delay() {
	_vgaBaseDelay = vc_read_next_word();
}

void SimonEngine::vc_31_set_palette_mode() {
	_videoPaletteMode = vc_read_next_word();
}

uint SimonEngine::vc_read_var(uint var) {
	assert(var < 255);
	return (uint16)_variableArray[var];
}

void SimonEngine::vc_write_var(uint var, int16 value) {
	_variableArray[var] = value;
}

void SimonEngine::vc_32_copy_var() {
	uint16 a = vc_read_var(vc_read_next_word());
	vc_write_var(vc_read_next_word(), a);
}

void SimonEngine::vc_33_force_unlock() {
	if (_lockCounter != 0) {
		_lockCounter = 1;
		unlock();
	}
}

void SimonEngine::vc_34_force_lock() {
	lock();
	_lockCounter = 200;
	_leftButtonDown = 0;
}

void SimonEngine::vc_35() {
	/* unused */
	_vcPtr += 4;
	_vgaSpriteChanged++;
}

void SimonEngine::vc_36_saveload_thing() {
	_videoVar8 = false;
	uint vga_res = vc_read_next_word();
	uint mode = vc_read_next_word();

	if (_game & GF_SIMON1) {
		if (mode == 16) {
			_copyPartialMode = 2;
		} else {
			set_video_mode_internal(mode, vga_res);
		}
	} else {
		set_video_mode_internal(mode, vga_res);
	}
}

void SimonEngine::vc_37_set_sprite_offset_y() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->y += vc_read_var(vc_read_next_word());
	_vgaSpriteChanged++;
}

void SimonEngine::vc_38_skip_if_var_zero() {
	uint var = vc_read_next_word();
	if (vc_read_var(var) == 0)
		vc_skip_next_instruction();
}

void SimonEngine::vc_39_set_var() {
	uint var = vc_read_next_word();
	int16 value = vc_read_next_word();
	vc_write_var(var, value);
}

void SimonEngine::vc_40_var_add() {
	uint var = vc_read_next_word();
	int16 value = vc_read_var(var) + vc_read_next_word();

	if ((_game & GF_SIMON2) && var == 0xF && !(_bitArray[5] & 1)) {
		int16 tmp;

		if (_vgaVar2 != 0) {
			if (_vgaVar2 >= 0)
				goto no_scroll;
			_vgaVar2 = 0;
		} else {
			if (_xScrollStep != 0)
				goto no_scroll;
		}

		if (value - _xScroll >= 30) {
			_vgaVar2 = 20;
			tmp = _vgaVar1 - _xScroll;
			if (tmp < 20)
				_vgaVar2 = tmp;
			add_vga_timer(6, NULL, 0, 0);	/* special timer */
		}
	}
no_scroll:;

	vc_write_var(var, value);
}

void SimonEngine::vc_41_var_sub() {
	uint var = vc_read_next_word();
	int16 value = vc_read_var(var) - vc_read_next_word();

	if ((_game & GF_SIMON2) && var == 0xF && !(_bitArray[5] & 1)) {
		int16 tmp;

		if (_vgaVar2 != 0) {
			if (_vgaVar2 < 0)
				goto no_scroll;
			_vgaVar2 = 0;
		} else {
			if (_xScrollStep != 0)
				goto no_scroll;
		}

		if ((uint16)(value - _xScroll) < 11) {
			_vgaVar2 = -20;
			tmp = _vgaVar1 - _xScroll;
			if (_xScroll < 20)
				_vgaVar2 = -_xScroll;
			add_vga_timer(6, NULL, 0, 0);	/* special timer */
		}
	}
no_scroll:;

	vc_write_var(var, value);
}

void SimonEngine::vc_42_delay_if_not_eq() {
	uint val = vc_read_var(vc_read_next_word());
	if (val != vc_read_next_word()) {

		add_vga_timer(_vgaBaseDelay + 1, _vcPtr - 4, _vgaCurSpriteId, _vgaCurFileId);
		_vcPtr = (byte *)&_vc_get_out_of_code;
	}
}

void SimonEngine::vc_43_skip_if_bit_clear() {
	if (!vc_get_bit(vc_read_next_word())) {
		vc_skip_next_instruction();
	}
}

void SimonEngine::vc_44_skip_if_bit_set() {
	if (vc_get_bit(vc_read_next_word())) {
		vc_skip_next_instruction();
	}
}

void SimonEngine::vc_45_set_sprite_x() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->x = vc_read_var(vc_read_next_word());
	_vgaSpriteChanged++;
}

void SimonEngine::vc_46_set_sprite_y() {
	VgaSprite *vsp = find_cur_sprite();
	vsp->y = vc_read_var(vc_read_next_word());
	_vgaSpriteChanged++;
}

void SimonEngine::vc_47_add_var_f() {
	uint var = vc_read_next_word();
	vc_write_var(var, vc_read_var(var) + vc_read_var(vc_read_next_word()));
}

void SimonEngine::vc_48() {
	uint a = (uint16)_variableArray[12];
	uint b = (uint16)_variableArray[13];
	int c = _variableArray[14];
	const uint16 *p = _pathFindArray[a - 1];
	int step;
	int y1, y2;
	int16 *vp;

	p += b * 2 + 1;

	step = 2;
	if (c < 0) {
		c = -c;
		step = -2;
	}

	vp = &_variableArray[20];

	do {
		y2 = READ_BE_UINT16(p);
		p += step;
		y1 = READ_BE_UINT16(p) - y2;

		vp[0] = y1 >> 1;
		vp[1] = y1 - (y1 >> 1);

		vp += 2;
	} while (--c);
}

void SimonEngine::vc_set_bit_to(uint bit, bool value) {
	uint16 *bits = &_bitArray[bit >> 4];
	*bits = (*bits & ~(1 << (bit & 15))) | (value << (bit & 15));
}

bool SimonEngine::vc_get_bit(uint bit) {
	uint16 *bits = &_bitArray[bit >> 4];
	return (*bits & (1 << (bit & 15))) != 0;
}

void SimonEngine::vc_49_set_bit() {
	vc_set_bit_to(vc_read_next_word(), true);
}

void SimonEngine::vc_50_clear_bit() {
	vc_set_bit_to(vc_read_next_word(), false);
}

void SimonEngine::vc_51_clear_hitarea_bit_0x40() {
	clear_hitarea_bit_0x40(vc_read_next_word());
}

void SimonEngine::vc_52_play_sound() {
	uint16 sound_id = vc_read_next_word();

	if (_game & GF_SIMON2) {
		if (sound_id >= 0x8000) {
			sound_id = -sound_id;
			_sound->playAmbient(sound_id);
		} else {
			_sound->playEffects(sound_id);
		}
	} else if (_game & GF_TALKIE) {
		_sound->playEffects(sound_id);
	} else {
		playSting(sound_id);
	}
}

void SimonEngine::vc_53_no_op() {
	/* unused */
	_vcPtr += 4;
}

void SimonEngine::vc_54_no_op() {
	/* unused */
	_vcPtr += 6;
}

void SimonEngine::vc_55_offset_hit_area() {
	HitArea *ha = _hitAreas;
	uint count = ARRAYSIZE(_hitAreas);
	uint16 id = vc_read_next_word();
	int16 x = vc_read_next_word();
	int16 y = vc_read_next_word();

	for (;;) {
		if (ha->id == id) {
			ha->x += x;
			ha->y += y;
			break;
		}
		ha++;
		if (!--count)
			break;
	}

	_needHitAreaRecalc++;
}

void SimonEngine::vc_56_delay() {
	if (_game & GF_SIMON2) {
		uint num = vc_read_var_or_word() * _vgaBaseDelay;

		add_vga_timer(num + VGA_DELAY_BASE, _vcPtr, _vgaCurSpriteId, _vgaCurFileId);
		_vcPtr = (byte *)&_vc_get_out_of_code;
	}
}

void SimonEngine::vc_59() {
	if (_game & GF_SIMON2) {
		uint file = vc_read_next_word();
		uint start = vc_read_next_word();
		uint end = vc_read_next_word() + 1;

		do {
			vc_kill_sprite(file, start);
		} while (++start != end);
	} else {
		if (!_mixer->isSoundHandleActive(_sound->_voice_handle))
			vc_skip_next_instruction();
	}
}

void SimonEngine::vc_58() {
	uint sprite = _vgaCurSpriteId;
	uint file = _vgaCurFileId;
	const byte *vc_ptr_org;
	uint16 tmp;

	_vgaCurFileId = vc_read_next_word();
	_vgaCurSpriteId = vc_read_next_word();

	tmp = TO_BE_16(vc_read_next_word());

	vc_ptr_org = _vcPtr;
	_vcPtr = (byte *)&tmp;
	vc_23_set_sprite_priority();

	_vcPtr = vc_ptr_org;
	_vgaCurSpriteId = sprite;
	_vgaCurFileId = file;
}

void SimonEngine::vc_57_no_op() {
	/* unused */
}

void SimonEngine::vc_kill_sprite(uint file, uint sprite) {
	uint16 old_sprite_id, old_cur_file_id;
	VgaSleepStruct *vfs;
	VgaSprite *vsp;
	VgaTimerEntry *vte;
	const byte *vc_ptr_org;

	old_sprite_id = _vgaCurSpriteId;
	old_cur_file_id = _vgaCurFileId;
	vc_ptr_org = _vcPtr;

	_vgaCurFileId = file;
	_vgaCurSpriteId = sprite;

	vfs = _vgaSleepStructs;
	while (vfs->ident != 0) {
		if ((_game & GF_SIMON1) || (vfs->sprite_id == _vgaCurSpriteId && vfs->cur_vga_file == _vgaCurFileId)) {
			while (vfs->ident != 0) {
				memcpy(vfs, vfs + 1, sizeof(VgaSleepStruct));
				vfs++;
			}
			break;
		}
		vfs++;
	}

	vsp = find_cur_sprite();
	if (vsp->id) {
		vc_25_halt_sprite();

		vte = _vgaTimerList;
		while (vte->delay != 0) {
			if ((_game & GF_SIMON1) || (vte->sprite_id == _vgaCurSpriteId && vte->cur_vga_file == _vgaCurFileId)) {
				delete_vga_timer(vte);
				break;
			}
			vte++;
		}
	}

	_vgaCurFileId = old_cur_file_id;
	_vgaCurSpriteId = old_sprite_id;
	_vcPtr = vc_ptr_org;
}

void SimonEngine::vc_60_kill_sprite() {
	uint file;

	if (_game & GF_SIMON2) {
		file = vc_read_next_word();
	} else {
		file = _vgaCurFileId;
	}
	uint sprite = vc_read_next_word();
	vc_kill_sprite(file, sprite);
}

void SimonEngine::vc_61_sprite_change() {
	VgaSprite *vsp = find_cur_sprite();

	vsp->image = vc_read_var_or_word();

	vsp->x += vc_read_next_word();
	vsp->y += vc_read_next_word();
	vsp->flags = 0x24;

	_vgaSpriteChanged++;
}

void SimonEngine::vc_62_palette_thing() {
	uint i;

	vc_29_stop_all_sounds();

	if (!_videoVar3) {
		_videoVar3 = true;

		_videoNumPalColors = 256;
		if (_videoPaletteMode == 4)
			_videoNumPalColors = 208;

		memcpy(_videoBuf1, _paletteBackup, _videoNumPalColors * sizeof(uint32));
		for (i = NUM_PALETTE_FADEOUT; i != 0; --i) {
			palette_fadeout((uint32 *)_videoBuf1, _videoNumPalColors);
			_system->setPalette(_videoBuf1, 0, _videoNumPalColors);
			if (_fade)
				_system->updateScreen();
			delay(5);
		}

		if (_game & GF_SIMON1) {
			uint16 params[5];						/* parameters to vc_10_draw */
			VgaSprite *vsp;
			VgaPointersEntry *vpe;
			const byte *vc_ptr_org = _vcPtr;

			vsp = _vgaSprites;
			while (vsp->id != 0) {
				if (vsp->id == 0x80) {
					byte *old_file_1 = _curVgaFile1;
					byte *old_file_2 = _curVgaFile2;
					uint palmode = _videoPaletteMode;
	
					vpe = &_vgaBufferPointers[vsp->unk7];
					_curVgaFile1 = vpe->vgaFile1;
					_curVgaFile2 = vpe->vgaFile2;
					_videoPaletteMode = vsp->paletteMode;

					params[0] = READ_BE_UINT16(&vsp->image);
					params[1] = READ_BE_UINT16(&vsp->base_color);
					params[2] = READ_BE_UINT16(&vsp->x);
					params[3] = READ_BE_UINT16(&vsp->y);
					params[4] = READ_BE_UINT16(&vsp->flags);
					_vcPtr = (byte *)params;
					vc_10_draw();

					_videoPaletteMode = palmode;
					_curVgaFile1 = old_file_1;
					_curVgaFile2 = old_file_2;
					break;
				}
				vsp++;
			}
			_vcPtr = vc_ptr_org;
		}

		// Allow one section of Simon the Sorcerer 1 introduction to be displayed
		// in lower half of screen
		if ((_game & GF_SIMON1) && (_subroutine == 2923 || _subroutine == 2926))
			dx_clear_surfaces(200);
		else
			dx_clear_surfaces(_videoPaletteMode == 4 ? 134 : 200);
	}
	if (_game & GF_SIMON2) {
		if (_nextMusicToPlay != -1)
			loadMusic(_nextMusicToPlay);
	}

}

void SimonEngine::vc_63_palette_thing_2() {
	_paletteColorCount = 208;
	if (_videoPaletteMode != 4) {
		_paletteColorCount = 256;
	}
	_videoVar3 = false;
}

void SimonEngine::vc_64_skip_if_no_speech() {
	// Simon2
	if (!_mixer->isSoundHandleActive(_sound->_voice_handle) || (_subtitles && _language != 20))
		vc_skip_next_instruction();
}

void SimonEngine::vc_65_palette_thing_3() {
	// Simon2
	_paletteColorCount = 624;
	_videoNumPalColors = 208;
	if (_videoPaletteMode != 4) {
		_paletteColorCount = 768;
		_videoNumPalColors = 256;
	}
	_paletteColorCount |= 0x8000;
	_videoVar3 = false;
}

void SimonEngine::vc_66_skip_if_nz() {
	// Simon2
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) != vc_read_var(b))
		vc_skip_next_instruction();
}

void SimonEngine::vc_67_skip_if_ge() {
	// Simon2
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) >= vc_read_var(b))
		vc_skip_next_instruction();
}

void SimonEngine::vc_68_skip_if_le() {
	// Simon2
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) <= vc_read_var(b))
		vc_skip_next_instruction();
}

void SimonEngine::vc_69_play_track() {
	// Simon2
	int16 track = vc_read_next_word();
	int16 loop = vc_read_next_word();

	// Jamieson630:
	// This is a "play track". The original
	// design stored the track to play if one was
	// already in progress, so that the next time a
	// "fill MIDI stream" event occured, the MIDI
	// player would find the change and switch
	// tracks. We use a different architecture that
	// allows for an immediate response here, but
	// we'll simulate the variable changes so other
	// scripts don't get thrown off.
	// NOTE: This opcode looks very similar in function
	// to vc_72(), except that vc_72() may allow for
	// specifying a non-valid track number (999 or -1)
	// as a means of stopping what music is currently
	// playing.
	midi.setLoop(loop != 0);
	midi.startTrack(track);
}

void SimonEngine::vc_70_queue_music() {
	// Simon2
	uint16 track = vc_read_next_word();
	uint16 loop = vc_read_next_word();

	// Jamieson630:
	// This sets the "on end of track" action.
	// It specifies whether to loop the current
	// track and, if not, whether to switch to
	// a different track upon completion.
	if (track != 0xFFFF && track != 999)
		midi.queueTrack(track, loop != 0);
	else
		midi.setLoop(loop != 0);
}

void SimonEngine::vc_71_check_music_queue() {
	// Simon2
	// Jamieson630:
	// This command skips the next instruction
	// unless (1) there is a track playing, AND
	// (2) there is a track queued to play after it.
	if (!midi.isPlaying (true))
		vc_skip_next_instruction();
}

void SimonEngine::vc_72_play_track_2() {
	// Simon2
	// Jamieson630:
	// This is a "play or stop track". Note that
	// this opcode looks very similar in function
	// to vc_69(), except that this opcode may allow
	// for specifying a track of 999 or -1 in order to
	// stop the music. We'll code it that way for now.

	// NOTE: It's possible that when "stopping" a track,
	// we're supposed to just go on to the next queued
	// track, if any. Must find out if there is ANY
	// case where this is used to stop a track in the
	// first place.

	int16 track = vc_read_next_word();
	int16 loop = vc_read_next_word();

	if (track == -1 || track == 999) {
		midi.stop();
	} else {
		midi.setLoop (loop != 0);
		midi.startTrack (track);
	}
}

void SimonEngine::vc_73_set_op189_flag() {
	// Simon2
	vc_read_next_byte();
	_op189Flags |= 1 << vc_read_next_byte();
}

void SimonEngine::vc_74_clear_op189_flag() {
	// Simon2
	vc_read_next_byte();
	_op189Flags &= ~(1 << vc_read_next_byte());
}

} // End of namespace Simon
