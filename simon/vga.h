/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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

#ifndef SIMON_VGA_H
#define SIMON_VGA_H

namespace Simon {

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif

struct VgaFile1Header {
	uint16 x_1, x_2;
	uint16 hdr2_start;
	uint16 x_3, x_4;
} GCC_PACK;

struct VgaFile1Header2 {
	uint16 x_1;
	uint16 unk1;
	uint16 x_2;
	uint16 id_count;
	uint16 x_3;
	uint16 unk2_offs;
	uint16 x_4;
	uint16 id_table;
	uint16 x_5;
} GCC_PACK;

struct VgaFile1Struct0x8 {
	uint16 id;
	uint16 x_1;
	uint16 x_2;
	uint16 script_offs;
} GCC_PACK;

struct VgaFile1Struct0x6 {
	uint16 id;
	uint16 x_2;
	uint16 script_offs;
} GCC_PACK;


#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct VC10_state {
	int image;
	uint16 flags;
	int x, y;

	byte base_color;

	uint draw_width, draw_height;
	uint x_skip, y_skip;

	byte *surf2_addr;
	uint surf2_pitch;

	byte *surf_addr;
	uint surf_pitch;

	byte dl, dh;

	const byte *depack_src;
	int8 depack_cont;

	byte depack_dest[200];
};

byte *vc_10_depack_column(VC10_state *vs);

} // End of namespace Simon

#endif
