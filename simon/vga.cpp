/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001/2002 The ScummVM project
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

typedef void (SimonState::*VgaOpcodeProc) ();
static const uint16 vc_get_out_of_code = 0;

// Opcode tables
static const VgaOpcodeProc vga_opcode_table[] = {
	NULL,
	&SimonState::vc_1_dummy_op,
	&SimonState::vc_2_call,
	&SimonState::vc_3_new_thread,
	&SimonState::vc_4_dummy_op,
	&SimonState::vc_5_skip_if_neq,
	&SimonState::vc_6_skip_ifn_sib_with_a,
	&SimonState::vc_7_skip_if_sib_with_a,
	&SimonState::vc_8_skip_if_parent_is,
	&SimonState::vc_9_skip_if_unk3_is,
	&SimonState::vc_10_draw,
	&SimonState::vc_11_clear_pathfind_array,
	&SimonState::vc_12_delay,
	&SimonState::vc_13_offset_x,
	&SimonState::vc_14_offset_y,
	&SimonState::vc_15_wakeup_id,
	&SimonState::vc_16_sleep_on_id,
	&SimonState::vc_17_set_pathfind_item,
	&SimonState::vc_18_jump_rel,
	&SimonState::vc_19_chain_to_script,
	&SimonState::vc_20_set_code_word,
	&SimonState::vc_21_jump_if_code_word,
	&SimonState::vc_22_set_pal,
	&SimonState::vc_23_set_pri,
	&SimonState::vc_24_set_image_xy,
	&SimonState::vc_25_halt_thread,
	&SimonState::vc_26_set_window,
	&SimonState::vc_27_reset,
	&SimonState::vc_28_dummy_op,
	&SimonState::vc_29_stop_all_sounds,
	&SimonState::vc_30_set_base_delay,
	&SimonState::vc_31_set_palette_mode,
	&SimonState::vc_32_copy_var,
	&SimonState::vc_33_force_unlock,
	&SimonState::vc_34_force_lock,
	&SimonState::vc_35,
	&SimonState::vc_36_saveload_thing,
	&SimonState::vc_37_offset_y_f,
	&SimonState::vc_38_skip_if_var_zero,
	&SimonState::vc_39_set_var,
	&SimonState::vc_40_var_add,
	&SimonState::vc_41_var_sub,
	&SimonState::vc_42_delay_if_not_eq,
	&SimonState::vc_43_skip_if_bit_clear,
	&SimonState::vc_44_skip_if_bit_set,
	&SimonState::vc_45_set_x,
	&SimonState::vc_46_set_y,
	&SimonState::vc_47_add_var_f,
	&SimonState::vc_48,
	&SimonState::vc_49_set_bit,
	&SimonState::vc_50_clear_bit,
	&SimonState::vc_51_clear_hitarea_bit_0x40,
	&SimonState::vc_52_play_sound,
	&SimonState::vc_53_no_op,
	&SimonState::vc_54_no_op,
	&SimonState::vc_55_offset_hit_area,
	&SimonState::vc_56_no_op,
	&SimonState::vc_57_no_op,
	&SimonState::vc_58,
	&SimonState::vc_59,
	&SimonState::vc_60_kill_thread,
	&SimonState::vc_61_sprite_change,
	&SimonState::vc_62_palette_thing,
	&SimonState::vc_63_palette_thing_2,
	&SimonState::vc_64_skip_if_text,
	&SimonState::vc_65_palette_thing_3,
	&SimonState::vc_66_skip_if_nz,
	&SimonState::vc_67_skip_if_ge,
	&SimonState::vc_68_skip_if_le,
	&SimonState::vc_69,
	&SimonState::vc_70,
	&SimonState::vc_71,
	&SimonState::vc_72,
	&SimonState::vc_73_set_op189_flag,
	&SimonState::vc_74_clear_op189_flag,
};

// Script parser
void SimonState::run_vga_script()
{
	for (;;) {
		uint opcode;

if (_continous_vgascript) {
		if ((void *)_vc_ptr != (void *)&vc_get_out_of_code) {
			fprintf(_dump_file, "%.5d %.5X: %5d %4d ", _vga_tick_counter, _vc_ptr - _cur_vga_file_1, _vga_cur_sprite_id, _vga_cur_file_id);
			dump_video_script(_vc_ptr, true);
		}
}

		if (!(_game & GAME_SIMON2)) {
			opcode = READ_BE_UINT16_UNALIGNED(_vc_ptr);
			_vc_ptr += 2;
		} else {
			opcode = *_vc_ptr++;
		}

		if (opcode >= gss->NUM_VIDEO_OP_CODES)
			error("Invalid VGA opcode '%d' encountered", opcode);

		if (opcode == 0)
			return;

		(this->*vga_opcode_table[opcode]) ();
	}
}

int SimonState::vc_read_var_or_word()
{
	int16 var = vc_read_next_word();
	if (var < 0)
		var = vc_read_var(-var);
	return var;
}

uint SimonState::vc_read_next_word()
{
	uint a = READ_BE_UINT16_UNALIGNED(_vc_ptr);
	_vc_ptr += 2;
	return a;
}

uint SimonState::vc_read_next_byte()
{
	return *_vc_ptr++;
}


void SimonState::vc_skip_next_instruction()
{
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

	if (_game & GAME_SIMON2) {
		uint opcode = vc_read_next_byte();
		_vc_ptr += opcode_param_len_simon2[opcode];
	} else {
		uint opcode = vc_read_next_word();
		_vc_ptr += opcode_param_len_simon1[opcode];
	}

	if (_continous_vgascript)
		fprintf(_dump_file, "; skipped\n");
}

void SimonState::o_read_vgares_23()
{																// Simon1 Only
	if (_vga_res_328_loaded == true) {
		_vga_res_328_loaded = false;
		_lock_word |= 0x4000;
		read_vga_from_datfile_1(23);
		_lock_word &= ~0x4000;
	}
}

void SimonState::o_read_vgares_328()
{																// Simon1 Only
	if (_vga_res_328_loaded == false) {
		_vga_res_328_loaded = true;
		_lock_word |= 0x4000;
		read_vga_from_datfile_1(328);
		_lock_word &= ~0x4000;
	}
}


// VGA Script commands
void SimonState::vc_1_dummy_op()
{
	/* dummy opcode */
	_vc_ptr += 6;
}

void SimonState::vc_2_call()
{
	VgaPointersEntry *vpe;
	uint num;
	uint res;
	byte *old_file_1, *old_file_2;
	byte *b, *bb, *vc_ptr_org;

	num = vc_read_var_or_word();

	old_file_1 = _cur_vga_file_1;
	old_file_2 = _cur_vga_file_2;

	for (;;) {
		res = num / 100;
		vpe = &_vga_buffer_pointers[res];

		_cur_vga_file_1 = vpe->vgaFile1;
		_cur_vga_file_2 = vpe->vgaFile2;
		if (vpe->vgaFile1 != NULL)
			break;
		if (_vga_cur_file_2 != res)
			_video_var_7 = _vga_cur_file_2;

		ensureVgaResLoaded(res);
		_video_var_7 = 0xFFFF;
	}


	bb = _cur_vga_file_1;
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header *) bb)->hdr2_start);
	b = bb + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2 *) b)->unk2_offs);

	while (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8 *) b)->id) != num)
		b += sizeof(VgaFile1Struct0x8);

	vc_ptr_org = _vc_ptr;

	_vc_ptr = _cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x8 *) b)->script_offs);

	//dump_vga_script(_vc_ptr, res, num);
	run_vga_script();

	_cur_vga_file_1 = old_file_1;
	_cur_vga_file_2 = old_file_2;

	_vc_ptr = vc_ptr_org;
}

void SimonState::vc_3_new_thread()
{
	uint16 a, b, c, d, e, f;
	uint16 res;
	VgaSprite *vsp;
	VgaPointersEntry *vpe;
	byte *p, *pp;

	a = vc_read_next_word();			/* 0 */

	if (_game & GAME_SIMON2) {
		f = vc_read_next_word();		/* 0 */
		b = vc_read_next_word();		/* 2 */
	} else {
		b = vc_read_next_word();		/* 2 */
		f = b / 100;
	}

	c = vc_read_next_word();			/* 4 */
	d = vc_read_next_word();			/* 6 */
	e = vc_read_next_word();			/* 8 */

	/* 2nd param ignored with simon1 */
	if (has_vgastruct_with_id(b, f))
		return;

	vsp = _vga_sprites;
	while (vsp->id)
		vsp++;

	vsp->base_color = e;
	vsp->unk6 = a;
	vsp->priority = 0;
	vsp->unk4 = 0;
	vsp->image = 0;
	vsp->x = c;
	vsp->y = d;
	vsp->id = b;
	vsp->unk7 = res = f;

	for (;;) {
		vpe = &_vga_buffer_pointers[res];
		_cur_vga_file_1 = vpe->vgaFile1;

		if (vpe->vgaFile1 != NULL)
			break;
		if (res != _vga_cur_file_2)
			_video_var_7 = res;

		ensureVgaResLoaded(res);
		_video_var_7 = 0xFFFF;
	}

	pp = _cur_vga_file_1;
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header *) pp)->hdr2_start);
	p = pp + READ_BE_UINT16_UNALIGNED(&((VgaFile1Header2 *) p)->id_table);

	while (READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6 *) p)->id) != b)
		p += sizeof(VgaFile1Struct0x6);

#ifdef DUMP_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_FILE_NR && !dumped) {
			dumped = true;
			dump_vga_file(_cur_vga_file_1);
		}
	}
#endif

#ifdef DUMP_BITMAPS_FILE_NR
	{
		static bool dumped = false;
		if (res == DUMP_BITMAPS_FILE_NR && !dumped) {
			dumped = true;
			dump_vga_bitmaps(_cur_vga_file_2, _cur_vga_file_1, res);
		}
	}
#endif

	//dump_vga_script(_cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6*)p)->script_offs), res, b);

	add_vga_timer(gss->VGA_DELAY_BASE, _cur_vga_file_1 + READ_BE_UINT16_UNALIGNED(&((VgaFile1Struct0x6 *) p)->script_offs), b, res);
}

void SimonState::vc_4_dummy_op()
{
	/* dummy opcode */
	_vc_ptr += 6;
}

void SimonState::vc_5_skip_if_neq()
{
	uint var = vc_read_next_word();
	uint value = vc_read_next_word();
	if (vc_read_var(var) != value)
		vc_skip_next_instruction();
}

void SimonState::vc_6_skip_ifn_sib_with_a()			// vc_6_maybe_skip_3_inv
{
	if (!vc_maybe_skip_proc_3(vc_read_next_word()))
		vc_skip_next_instruction();
}

void SimonState::vc_7_skip_if_sib_with_a()			// vc_7_maybe_skip_3
{
	if (vc_maybe_skip_proc_3(vc_read_next_word()))
		vc_skip_next_instruction();
}

void SimonState::vc_8_skip_if_parent_is()			// vc_8_maybe_skip_2			
{
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	if (!vc_maybe_skip_proc_2(a, b))
		vc_skip_next_instruction();
}

void SimonState::vc_9_skip_if_unk3_is()				// vc_9_maybe_skip
{
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	if (!vc_maybe_skip_proc_1(a, b))
		vc_skip_next_instruction();
}

byte *vc_10_depack_column(VC10_state * vs)
{
	int8 a = vs->depack_cont;
	byte *src = vs->depack_src;
	byte *dst = vs->depack_dest;
	byte dh = vs->dh;
	byte color;

	if (a != -0x80)
		goto start_here;

	for (;;) {
		a = *src++;
	start_here:;
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
	}

get_out:;
	vs->depack_src = src;
	vs->depack_cont = a;
	return vs->depack_dest + vs->y_skip;
}

void vc_10_skip_cols(VC10_state * vs)
{
	vs->depack_cont = -0x80;
	while (vs->x_skip) {
		vc_10_depack_column(vs);
		vs->x_skip--;
	}
}

byte *SimonState::vc_10_depack_swap(byte *src, uint w, uint h)
{
	w <<= 3;

	{
		byte *dst_org = _video_buf_1 + w;
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

		src_org = dst_org = _video_buf_1 + w;

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

	return _video_buf_1;

}

byte *vc_10_no_depack_swap(byte *src)
{
	error("vc_10_no_depack_swap unimpl");
	return NULL;
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
void SimonState::vc_10_helper_8(byte *dst, byte *src)
{
	const uint pitch = _dx_surface_pitch;
	int8 reps = (int8)0x80;
	byte color;
	byte *dst_org = dst;
	uint h = _vga_var5, w = 8;

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
					h = _vga_var5;
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
					h = _vga_var5;
				}
			} while (++reps != 0);
		}
	}
}

void SimonState::vc_10_draw()
{
	byte *p2;
	uint width, height;
	byte flags;
	const uint16 *vlut;
	VC10_state state;

	int cur;

	state.image = (int16)vc_read_next_word();
	if (state.image == 0)
		return;

	state.base_color = (_vc_ptr[1] << 4);
	_vc_ptr += 2;
	state.x = (int16)vc_read_next_word();
	if (_game & GAME_SIMON2) {
		state.x -= _x_scroll;
	}
	state.y = (int16)vc_read_next_word();

	if (!(_game & GAME_SIMON2)) {
		state.e = vc_read_next_word();
	} else {
		state.e = vc_read_next_byte();
	}

	if (state.image < 0)
		state.image = vc_read_var(-state.image);

	p2 = _cur_vga_file_2 + state.image * 8;
	state.depack_src = _cur_vga_file_2 + READ_BE_UINT32_UNALIGNED(&*(uint32 *)p2);

	width = READ_BE_UINT16_UNALIGNED(p2 + 6) >> 4;
	height = p2[5];
	flags = p2[4];

	if (height == 0 || width == 0)
		return;

#ifdef DUMP_DRAWN_BITMAPS
	dump_single_bitmap(_vga_cur_file_id, state.image, state.depack_src, width * 16, height,
										 state.base_color);
#endif

	if (flags & 0x80 && !(state.e & 0x10)) {
		if (state.e & 1) {
			state.e &= ~1;
			state.e |= 0x10;
		} else {
			state.e |= 0x8;
		}
	}

	if (_game & GAME_SIMON2 && width >= 21) {
		byte *src, *dst;
		uint w;

		_vga_var1 = width * 2 - 40;
		_vga_var7 = state.depack_src;
		_vga_var5 = height;
		if (_variableArray[34] == -1)
			state.x = _variableArray[502 / 2];

		_x_scroll = state.x;

		vc_write_var(0xfb, _x_scroll);

		dst = dx_lock_attached();
		src = state.depack_src + _x_scroll * 4;

		w = 40;
		do {
			vc_10_helper_8(dst, src + READ_BE_UINT32_UNALIGNED(&*(uint32 *)src));
			dst += 8;
			src += 4;
		} while (--w);

		dx_unlock_attached();


		return;
	}

	if (state.e & 0x10)
		state.depack_src = vc_10_depack_swap(state.depack_src, width, height);
	else if (state.e & 1)
		state.depack_src = vc_10_no_depack_swap(state.depack_src);


	vlut = &_video_windows[_video_palette_mode * 4];

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
	state.surf2_pitch = _dx_surface_pitch;

	state.surf_addr = dx_lock_attached();
	state.surf_pitch = _dx_surface_pitch;

	{
		uint offs = ((vlut[0] - _video_windows[16]) * 2 + state.x) * 8;
		uint offs2 = (vlut[1] - _video_windows[17] + state.y);

		state.surf2_addr += offs + offs2 * state.surf2_pitch;
		state.surf_addr += offs + offs2 * state.surf_pitch;
	}

	if (state.e & 0x20) {
		byte *mask, *src, *dst;
		byte h;
		uint w;

		state.x_skip <<= 2;
		state.dl = width;
		state.dh = height;

		vc_10_skip_cols(&state);

		/* XXX: implement transparency */

		w = 0;
		do {
			mask = vc_10_depack_column(&state);	/* esi */
			src = state.surf2_addr + w * 2;	/* ebx */
			dst = state.surf_addr + w * 2;	/* edi */

			h = state.draw_height;
			do {
				if (mask[0] & 0xF0)
					dst[0] = src[0];
				if (mask[0] & 0x0F)
					dst[1] = src[1];
				mask++;
				dst += state.surf_pitch;
				src += state.surf2_pitch;
			} while (--h);
		} while (++w != state.draw_width);

		/* vc_10_helper_5 */
	} else if (_lock_word & 0x20 && state.base_color == 0 || state.base_color == 0xC0) {
		byte *src, *dst;
		uint h, i;

		if (!(state.e & 8)) {
			src = state.depack_src + (width * state.y_skip << 4) + (state.x_skip << 3);
			dst = state.surf_addr;

			state.draw_width *= 2;

			if (state.e & 2) {
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

			if (state.e & 2) {
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
		if (_game & GAME_SIMON2 && state.e & 0x4 && _bit_array[10] & 0x800) {
			state.surf_addr = state.surf2_addr;
			state.surf_pitch = state.surf2_pitch;
			if (_debugMode)
				warning("vc_10_draw: (state.e&0x4)");
		}

		if (state.e & 0x8) {
			uint w, h;
			byte *src, *dst, *dst_org;

			state.x_skip <<= 2;				/* reached */
			state.dl = width;
			state.dh = height;

			vc_10_skip_cols(&state);

			if (state.e & 2) {
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
				if (state.e & 0x40) {		/* reached */
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
			byte *src, *dst;
			uint count;

			src = state.depack_src + (width * state.y_skip) * 8;
			dst = state.surf_addr;
			state.x_skip <<= 2;
			if (state.e & 2) {
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

void SimonState::vc_11_clear_pathfind_array()
{
	memset(&_pathfind_array, 0, sizeof(_pathfind_array));
}

void SimonState::vc_12_delay()					//vc_12_sleep_variable
{
	uint num;

	if (!(_game & GAME_SIMON2)) {
		num = vc_read_var_or_word();
	} else {
		num = vc_read_next_byte() * _vga_base_delay;
	}

	add_vga_timer(num + gss->VGA_DELAY_BASE, _vc_ptr, _vga_cur_sprite_id, _vga_cur_file_id);
	_vc_ptr = (byte *)&vc_get_out_of_code;
}

void SimonState::vc_13_offset_x()
{
	VgaSprite *vsp = find_cur_sprite();
	int16 a = vc_read_next_word();
	vsp->x += a;
	_vga_sprite_changed++;
}

void SimonState::vc_14_offset_y()
{
	VgaSprite *vsp = find_cur_sprite();
	int16 a = vc_read_next_word();
	vsp->y += a;
	_vga_sprite_changed++;
}

void SimonState::vc_15_wakeup_id()				//vc_15_start_funkystruct_by_id
{
	VgaSleepStruct *vfs = _vga_sleep_structs, *vfs_tmp;
	uint16 id = vc_read_next_word();
	while (vfs->ident != 0) {
		if (vfs->ident == id) {
			add_vga_timer(gss->VGA_DELAY_BASE, vfs->code_ptr, vfs->sprite_id, vfs->cur_vga_file);
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
	if (id == _vga_wait_for)
		_vga_wait_for = 0;
}


void SimonState::vc_16_sleep_on_id()				//vc_16_setup_funkystruct
{
	VgaSleepStruct *vfs = _vga_sleep_structs;
	while (vfs->ident)
		vfs++;

	vfs->ident = vc_read_next_word();
	vfs->code_ptr = _vc_ptr;
	vfs->sprite_id = _vga_cur_sprite_id;
	vfs->cur_vga_file = _vga_cur_file_id;

	_vc_ptr = (byte *)&vc_get_out_of_code;
}

void SimonState::vc_17_set_pathfind_item()
{
	uint a = vc_read_next_word();
	_pathfind_array[a - 1] = (uint16 *)_vc_ptr;
	while (READ_BE_UINT16_UNALIGNED(_vc_ptr) != 999)
		_vc_ptr += 4;
	_vc_ptr += 2;
}

void SimonState::vc_18_jump_rel()
{
	int16 offs = vc_read_next_word();
	_vc_ptr += offs;
}

/* chain to script? */
void SimonState::vc_19_chain_to_script()
{
	/* XXX: not implemented */
	error("vc_19_chain_to_script: not implemented");
}


/* helper routines */

/* write unaligned 16-bit */
static void write_16_le(void *p, uint16 a)
{
	((byte *)p)[0] = (byte)(a);
	((byte *)p)[1] = (byte)(a >> 8);
}

/* read unaligned 16-bit */
static uint16 read_16_le(void *p)
{
	return ((byte *)p)[0] | (((byte *)p)[1] << 8);
}

/* FIXME: unaligned access */
void SimonState::vc_20_set_code_word()
{
	uint16 a = vc_read_next_word();
	write_16_le(_vc_ptr, a);
	_vc_ptr += 2;
}

/* FIXME: unaligned access */
void SimonState::vc_21_jump_if_code_word()
{
	if (!(_game & GAME_SIMON2)) {
		int16 a = vc_read_next_word();
		byte *tmp = _vc_ptr + a;
		uint16 val = read_16_le(tmp + 4);

		if (val != 0) {
			write_16_le(tmp + 4, val - 1);
			_vc_ptr = tmp + 6;
		}
	} else {
		int16 a = vc_read_next_word();
		byte *tmp = _vc_ptr + a;
		uint16 val = read_16_le(tmp + 3);

		if (val != 0) {
			write_16_le(tmp + 3, val - 1);
			_vc_ptr = tmp + 5;
		}
	}
}

void SimonState::vc_22_set_pal()
{
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();
	uint num = a == 0 ? 0x20 : 0x10;
	byte *palptr, *src;

	palptr = &_palette[(a << 6)];

	src = _cur_vga_file_1 + 6 + b * 96;

	do {
		palptr[0] = src[0] << 2;
		palptr[1] = src[1] << 2;
		palptr[2] = src[2] << 2;
		palptr[3] = 0;

		palptr += 4;
		src += 3;
	} while (--num);

	_video_var_9 = 2;
	_vga_sprite_changed++;
}

void SimonState::vc_23_set_pri()
{
	VgaSprite *vsp = find_cur_sprite(), *vus2;
	uint16 pri = vc_read_next_word();
	VgaSprite bak;

	if (vsp->id == 0) {
		if (_debugMode)
			warning("tried to set pri for unknown id %d", _vga_cur_sprite_id);
		return;
	}

	memcpy(&bak, vsp, sizeof(bak));
	bak.priority = pri;
	bak.unk6 |= 0x8000;

	vus2 = vsp;

	if (vsp != _vga_sprites && pri < vsp[-1].priority) {
		do {
			vsp--;
		} while (vsp != _vga_sprites && pri < vsp[-1].priority);
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
	_vga_sprite_changed++;
}

void SimonState::vc_24_set_image_xy()
{
	VgaSprite *vsp = find_cur_sprite();
	vsp->image = vc_read_var_or_word();

	if (vsp->id == 0) {
		if (_debugMode)
			warning("Trying to set XY of nonexistent sprite '%d'", _vga_cur_sprite_id);
	}

	vsp->x += (int16)vc_read_next_word();
	vsp->y += (int16)vc_read_next_word();
	if (!(_game & GAME_SIMON2)) {
		vsp->unk4 = vc_read_next_word();
	} else {
		vsp->unk4 = vc_read_next_byte();
	}

	_vga_sprite_changed++;
}

void SimonState::vc_25_halt_thread()				//vc_25_del_sprite_and_get_out
{
	VgaSprite *vsp = find_cur_sprite();
	while (vsp->id != 0) {
		memcpy(vsp, vsp + 1, sizeof(VgaSprite));
		vsp++;
	}
	_vc_ptr = (byte *)&vc_get_out_of_code;
	_vga_sprite_changed++;
}

void SimonState::vc_26_set_window()
{
	uint16 *as = &_video_windows[vc_read_next_word() * 4];
	as[0] = vc_read_next_word();
	as[1] = vc_read_next_word();
	as[2] = vc_read_next_word();
	as[3] = vc_read_next_word();
}

void SimonState::vc_27_reset_simon1()
{
	VgaSprite bak, *vsp;
	VgaSleepStruct *vfs;
	VgaTimerEntry *vte, *vte2;

	_lock_word |= 8;

	memset(&bak, 0, sizeof(bak));

	vsp = _vga_sprites;
	while (vsp->id) {
		if (vsp->id == 128) {
			memcpy(&bak, vsp, sizeof(VgaSprite));
		}
		vsp->id = 0;
		vsp++;
	}

	if (bak.id != 0)
		memcpy(_vga_sprites, &bak, sizeof(VgaSprite));

	vfs = _vga_sleep_structs;
	while (vfs->ident) {
		vfs->ident = 0;
		vfs++;
	}


	vte = _vga_timer_list;
	while (vte->delay) {
		if (vte->sprite_id != 0x80) {
			vte2 = vte;
			while (vte2->delay) {
				memcpy(vte2, vte2 + 1, sizeof(VgaTimerEntry));
				vte2++;
			}
		} else {
			vte++;
		}
	}

	vc_write_var(0xFE, 0);

	_lock_word &= ~8;
}


void SimonState::vc_27_reset_simon2()
{
	_lock_word |= 8;

	{
		VgaSprite *vsp = _vga_sprites;
		while (vsp->id) {
			vsp->id = 0;
			vsp++;
		}
	}

	{
		VgaSleepStruct *vfs = _vga_sleep_structs;
		while (vfs->ident) {
			vfs->ident = 0;
			vfs++;
		}
	}

	{
		VgaTimerEntry *vte = _vga_timer_list;
		while (vte->delay) {
			VgaTimerEntry *vte2 = vte;
			while (vte2->delay) {
				memcpy(vte2, vte2 + 1, sizeof(VgaTimerEntry));
				vte2++;
			}
		}
	}

	vc_write_var(0xFE, 0);

	_lock_word &= ~8;
}

void SimonState::vc_27_reset()
{
	if (!(_game & GAME_SIMON2))
		vc_27_reset_simon1();
	else
		vc_27_reset_simon2();
}

void SimonState::vc_28_dummy_op()
{
	/* dummy opcode */
	_vc_ptr += 8;
}

void SimonState::vc_29_stop_all_sounds()
{
	_sound->stopAll();
}

void SimonState::vc_30_set_base_delay()
{
	_vga_base_delay = vc_read_next_word();
}

void SimonState::vc_31_set_palette_mode()
{
	_video_palette_mode = vc_read_next_word();
}

uint SimonState::vc_read_var(uint var)
{
	assert(var < 255);
	return (uint16)_variableArray[var];
}

void SimonState::vc_write_var(uint var, int16 value)
{
	_variableArray[var] = value;
}

void SimonState::vc_32_copy_var()
{
	uint16 a = vc_read_var(vc_read_next_word());
	vc_write_var(vc_read_next_word(), a);
}

void SimonState::vc_33_force_unlock()
{
	if (_lock_counter != 0) {
		_lock_counter = 1;
		unlock();
	}
}

void SimonState::vc_34_force_lock()
{
	lock();
	_lock_counter = 200;
	_left_button_down = 0;
}

void SimonState::vc_35()
{
	/* not used? */
	_vc_ptr += 4;
	_vga_sprite_changed++;
}

void SimonState::vc_36_saveload_thing()
{
	_video_var_8 = false;
	uint vga_res = vc_read_next_word();
	uint mode = vc_read_next_word();

	if (!(_game & GAME_SIMON2)) {
		if (mode == 16) {
			_copy_partial_mode = 2;
		} else {
			set_video_mode_internal(mode, vga_res);
		}
	} else {
		set_video_mode_internal(mode, vga_res);
	}
}

void SimonState::vc_37_offset_y_f()				//vc_37_sprite_unk3_add
{
	VgaSprite *vsp = find_cur_sprite();
	vsp->y += vc_read_var(vc_read_next_word());
	_vga_sprite_changed++;
}

void SimonState::vc_38_skip_if_var_zero()
{
	uint var = vc_read_next_word();
	if (vc_read_var(var) == 0)
		vc_skip_next_instruction();
}

void SimonState::vc_39_set_var()
{
	uint var = vc_read_next_word();
	int16 value = vc_read_next_word();
	vc_write_var(var, value);
}

void SimonState::vc_40_var_add()
{
	uint var = vc_read_next_word();
	int16 value = vc_read_var(var) + vc_read_next_word();

	if (_game & GAME_SIMON2 && var == 0xF && !(_bit_array[5] & 1)) {
		int16 tmp;

		if (_vga_var2 != 0) {
			if (_vga_var2 >= 0)
				goto no_scroll;
			_vga_var2 = 0;
		} else {
			if (_vga_var3 != 0)
				goto no_scroll;
		}

		if (value - _x_scroll >= 30) {
			_vga_var2 = 20;
			tmp = _vga_var1 - _x_scroll;
			if (tmp < 20)
				_vga_var2 = tmp;
			add_vga_timer(10, NULL, 0, 0);	/* special timer */
		}
	}
no_scroll:;

	vc_write_var(var, value);
}

void SimonState::vc_41_var_sub()
{
	uint var = vc_read_next_word();
	int16 value = vc_read_var(var) - vc_read_next_word();

	if (_game & GAME_SIMON2 && var == 0xF && !(_bit_array[5] & 1)) {
		int16 tmp;

		if (_vga_var2 != 0) {
			if (_vga_var2 < 0)
				goto no_scroll;
			_vga_var2 = 0;
		} else {
			if (_vga_var3 != 0)
				goto no_scroll;
		}

		if ((uint16)(value - _x_scroll) < 11) {
			_vga_var2 = -20;
			tmp = _vga_var1 - _x_scroll;
			if (_x_scroll < 20)
				_vga_var2 = -_x_scroll;
			add_vga_timer(10, NULL, 0, 0);	/* special timer */
		}
	}
no_scroll:;

	vc_write_var(var, value);
}

void SimonState::vc_42_delay_if_not_eq()
{
	uint val = vc_read_var(vc_read_next_word());
	if (val == vc_read_next_word()) {

		add_vga_timer(_vga_base_delay + 1, _vc_ptr - 4, _vga_cur_sprite_id, _vga_cur_file_id);
		_vc_ptr = (byte *)&vc_get_out_of_code;
	}
}

void SimonState::vc_43_skip_if_bit_clear()
{
	if (!vc_get_bit(vc_read_next_word())) {
		vc_skip_next_instruction();
	}
}

void SimonState::vc_44_skip_if_bit_set()
{
	if (vc_get_bit(vc_read_next_word())) {
		vc_skip_next_instruction();
	}
}

void SimonState::vc_45_set_x()
{
	VgaSprite *vsp = find_cur_sprite();
	vsp->x = vc_read_var(vc_read_next_word());
	_vga_sprite_changed++;
}

void SimonState::vc_46_set_y()
{
	VgaSprite *vsp = find_cur_sprite();
	vsp->y = vc_read_var(vc_read_next_word());
	_vga_sprite_changed++;
}

void SimonState::vc_47_add_var_f()
{
	uint var = vc_read_next_word();
	vc_write_var(var, vc_read_var(var) + vc_read_var(vc_read_next_word()));
}

void SimonState::vc_48()
{
	uint a = (uint16)_variableArray[12];
	uint b = (uint16)_variableArray[13];
	int c = _variableArray[14];
	uint16 *p = _pathfind_array[a - 1];
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
		y2 = READ_BE_UINT16_UNALIGNED(p);
		p += step;
		y1 = READ_BE_UINT16_UNALIGNED(p) - y2;

//    assert(READ_BE_UINT16_UNALIGNED(&p[1]) != 999);

		vp[0] = y1 >> 1;
		vp[1] = y1 - (y1 >> 1);

		vp += 2;
	} while (--c);

}

void SimonState::vc_set_bit_to(uint bit, bool value)
{
	uint16 *bits = &_bit_array[bit >> 4];
	*bits = (*bits & ~(1 << (bit & 15))) | (value << (bit & 15));
}

bool SimonState::vc_get_bit(uint bit)
{
	uint16 *bits = &_bit_array[bit >> 4];
	return (*bits & (1 << (bit & 15))) != 0;
}

void SimonState::vc_49_set_bit()
{
	vc_set_bit_to(vc_read_next_word(), true);
}

void SimonState::vc_50_clear_bit()
{
	vc_set_bit_to(vc_read_next_word(), false);
}

void SimonState::vc_51_clear_hitarea_bit_0x40()
{
	clear_hitarea_bit_0x40(vc_read_next_word());
}

void SimonState::vc_52_play_sound()
{
	uint16 a = vc_read_next_word();

	if (!(_game & GAME_SIMON2)) {
		_sound->playEffects(a);
	} else {
		if (a >= 0x8000) {
			a = -a;
			_sound->playAmbient(a);
		} else {
			_sound->playEffects(a);
		}
	}
}

void SimonState::vc_53_no_op()
{
	/* no op */
}

void SimonState::vc_54_no_op()
{
	/* no op */
}

void SimonState::vc_55_offset_hit_area()
{
	HitArea *ha = _hit_areas;
	uint count = ARRAYSIZE(_hit_areas);
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

	_need_hitarea_recalc++;
}

void SimonState::vc_56_no_op()
{
	/* no op in simon1 */
	if (_game & GAME_SIMON2) {
		uint num = vc_read_var_or_word() * _vga_base_delay;

		if (_continous_vgascript)
			fprintf(_dump_file, "; sleep_ex = %d\n", num + gss->VGA_DELAY_BASE);

		add_vga_timer(num + gss->VGA_DELAY_BASE, _vc_ptr, _vga_cur_sprite_id, _vga_cur_file_id);
		_vc_ptr = (byte *)&vc_get_out_of_code;
	}
}

void SimonState::vc_59()
{
	if (_game & GAME_SIMON2) {
		uint file = vc_read_next_word();
		uint start = vc_read_next_word();
		uint end = vc_read_next_word() + 1;

		do {
			vc_kill_thread(file, start);
		} while (++start != end);
	} else {
		if (vc_59_helper())
			vc_skip_next_instruction();
	}
}

void SimonState::vc_58()
{
	uint sprite = _vga_cur_sprite_id;
	uint file = _vga_cur_file_id;
	byte *vc_ptr;
	uint16 tmp;

	_vga_cur_file_id = vc_read_next_word();
	_vga_cur_sprite_id = vc_read_next_word();

	tmp = TO_BE_16(vc_read_next_word());

	vc_ptr = _vc_ptr;
	_vc_ptr = (byte *)&tmp;
	vc_23_set_pri();

	_vc_ptr = vc_ptr;
	_vga_cur_sprite_id = sprite;
	_vga_cur_file_id = file;
}

void SimonState::vc_57_no_op()
{
	/* no op */

}

void SimonState::vc_kill_thread(uint file, uint sprite)
{
	uint16 old_sprite_id, old_cur_file_id;
	VgaSleepStruct *vfs;
	VgaSprite *vsp;
	VgaTimerEntry *vte;
	byte *vc_org;

	old_sprite_id = _vga_cur_sprite_id;
	old_cur_file_id = _vga_cur_file_id;
	vc_org = _vc_ptr;

	_vga_cur_file_id = file;
	_vga_cur_sprite_id = sprite;

	vfs = _vga_sleep_structs;
	while (vfs->ident != 0) {
		if (vfs->sprite_id == _vga_cur_sprite_id
				&& (vfs->cur_vga_file == _vga_cur_file_id || !(_game & GAME_SIMON2))
			) {
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
		vc_25_halt_thread();

		vte = _vga_timer_list;
		while (vte->delay != 0) {
			if (vte->sprite_id == _vga_cur_sprite_id
					&& (vte->cur_vga_file == _vga_cur_file_id || !(_game & GAME_SIMON2))
				) {
				delete_vga_timer(vte);
				break;
			}
			vte++;
		}
	}

	_vga_cur_file_id = old_cur_file_id;
	_vga_cur_sprite_id = old_sprite_id;
	_vc_ptr = vc_org;
}

void SimonState::vc_60_kill_thread()
{
	uint file;

	if (_game & GAME_SIMON2) {
		file = vc_read_next_word();
	} else {
		file = _vga_cur_file_id;
	}
	uint sprite = vc_read_next_word();
	vc_kill_thread(file, sprite);
}

void SimonState::vc_61_sprite_change()
{
	VgaSprite *vsp = find_cur_sprite();

	vsp->image = vc_read_var_or_word();

	vsp->x += vc_read_next_word();
	vsp->y += vc_read_next_word();
	vsp->unk4 = 36;

	_vga_sprite_changed++;
}

void SimonState::vc_62_palette_thing()
{
	uint i;
	byte *vc_ptr_org = _vc_ptr;


	vc_29_stop_all_sounds();

	if (!_video_var_3) {
		if (_game & GAME_SIMON2) {
		//FIXME The screen should be cleared elsewhere.
		dx_clear_surfaces(_video_palette_mode == 4 ? 134 : 200);
			if (_midi_unk2 != 0xffff) {
				playMusic(999, _midi_unk2);
			}
		}
	return;
	}

	_video_var_3 = true;
	_video_num_pal_colors = 256;
	if (_video_palette_mode == 4)
		_video_num_pal_colors = 208;

	memcpy(_video_buf_1, _palette_backup, _video_num_pal_colors * sizeof(uint32));
	for (i = NUM_PALETTE_FADEOUT; i != 0; --i) {
		palette_fadeout((uint32 *)_video_buf_1, _video_num_pal_colors);
		_system->set_palette(_video_buf_1, 0, _video_num_pal_colors);
		_system->update_screen();
		delay(5);
	}

	if (!(_game & GAME_SIMON2)) {
		uint16 params[5];						/* parameters to vc_10_draw */
		VgaSprite *vsp;
		VgaPointersEntry *vpe;

		vsp = _vga_sprites;
		while (vsp->id != 0) {
			if (vsp->id == 128) {
				byte *f1 = _cur_vga_file_1;
				byte *f2 = _cur_vga_file_2;
				uint palmode = _video_palette_mode;

				vpe = &_vga_buffer_pointers[vsp->unk7];
				_cur_vga_file_1 = vpe->vgaFile1;
				_cur_vga_file_2 = vpe->vgaFile2;
				_video_palette_mode = vsp->unk6;

				params[0] = READ_BE_UINT16_UNALIGNED(&vsp->image);
				params[1] = READ_BE_UINT16_UNALIGNED(&vsp->base_color);
				params[2] = READ_BE_UINT16_UNALIGNED(&vsp->x);
				params[3] = READ_BE_UINT16_UNALIGNED(&vsp->y);
				params[4] = READ_BE_UINT16_UNALIGNED(&vsp->unk4);
				_vc_ptr = (byte *)params;
				vc_10_draw();

				_video_palette_mode = palmode;
				_cur_vga_file_1 = f1;
				_cur_vga_file_2 = f2;
				break;
			}
			vsp++;
		}
	}

	dx_clear_surfaces(_video_palette_mode == 4 ? 134 : 200);

	_vc_ptr = vc_ptr_org;
}

void SimonState::vc_63_palette_thing_2()
{
	_palette_color_count = 208;
	if (_video_palette_mode != 4) {
		_palette_color_count = 256;
	}
	_video_var_3 = false;
}

void SimonState::vc_64_skip_if_text()
{																// Simon2
	if (vc_59_helper())
		vc_skip_next_instruction();
}

void SimonState::vc_65_palette_thing_3()
{																// Simon2
	_palette_color_count = 0x270;
	_video_num_pal_colors = 0x0D0;
	if (_video_palette_mode != 4) {
		_palette_color_count = 0x300;
		_video_num_pal_colors = 0x100;
	}
	_palette_color_count |= 0x8000;
	_video_var_3 = false;
}

void SimonState::vc_66_skip_if_nz()
{																// Simon2
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) != vc_read_var(b))
		vc_skip_next_instruction();
}

void SimonState::vc_67_skip_if_ge()
{																// Simon2
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) >= vc_read_var(b))
		vc_skip_next_instruction();
}

void SimonState::vc_68_skip_if_le()
{																// Simon2
	uint a = vc_read_next_word();
	uint b = vc_read_next_word();

	if (vc_read_var(a) <= vc_read_var(b))
		vc_skip_next_instruction();
}

void SimonState::vc_69()
{																// Simon2
	uint16 a = vc_read_next_word();
	uint16 b = vc_read_next_word();

	if (_debugMode)
		warning("vc_69(%d,%d): music stuff?", a, b);

	if (_vc72_var1 == 999) {
		_vc70_var2 = b;
		midi.initialize();
		midi.play();
		_vc72_var1 = b;
	} else if (_vc72_var1 != 0xFFFF) {
		if (_vc72_var1 != a) {
			_vc72_var3 = a;
			_vc72_var2 = a;
		}
	}

}

void SimonState::vc_70()
{																// Simon2
	uint16 a = vc_read_next_word();
	uint16 b = vc_read_next_word();

	_vc70_var1 = a;
	_vc70_var2 = b;

	if (_debugMode)
		warning("vc_70(%d,%d): music stuff?", a, b);
}


void SimonState::vc_71()
{																// Simon2
	if (_vc72_var3 == 0xFFFF && _vc72_var1 == 0xFFFF)
		vc_skip_next_instruction();
}

void SimonState::vc_72()
{																// Simon2
	uint16 a = vc_read_next_word();
	uint16 b = vc_read_next_word();
	if (a != _vc72_var1) {
		_vc72_var2 = b;
		_vc72_var3 = a;
	}

	if (_debugMode)
		warning("vc_72(%d,%d): music stuff?", a, b);
}

void SimonState::vc_73_set_op189_flag()
{																// Simon2
	vc_read_next_byte();
	_op_189_flags |= 1 << vc_read_next_byte();
}

void SimonState::vc_74_clear_op189_flag()
{																// Simon2
	vc_read_next_byte();
	_op_189_flags &= ~(1 << vc_read_next_byte());
}
