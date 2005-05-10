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

#include "stdafx.h"

#include "common/file.h"

#include "simon/simon.h"
#include "simon/intern.h"

namespace Simon {

void SimonEngine::loadIconFile() {
	Common::File in;
	if (_game & GF_ACORN)
		in.open("ICONDATA");
	else if (_game & GF_AMIGA)
		in.open("icon.pkd");
	else
		in.open("ICON.DAT");
	uint size;

	if (in.isOpen() == false)
		error("Can't open icons file 'ICON.DAT'");

	size = in.size();

	_iconFilePtr = (byte *)malloc(size);
	if (_iconFilePtr == NULL)
		error("Out of icon memory");

	in.read(_iconFilePtr, size);
	in.close();
}

// Thanks to Stuart Caie for providing the original
// C conversion upon which this function is based.
void decompress_icon_amiga (byte *dst, byte *src, byte base, uint pitch) {
	byte icon_pln[288];
	byte *i, *o, x, y;

	// Decode RLE planar icon data
	i = src;
	o = icon_pln;
	while (o < &icon_pln[288]) {
		x = *i++;
		if (x < 128) {
			do {
				*o++ = *i++;
				*o++ = *i++;
				*o++ = *i++;
			} while (x-- > 0);
		} else {
			x = 256 - x;
			do {
				*o++ = i[0];
				*o++ = i[1];
				*o++ = i[2];
			} while (x-- > 0);
			i += 3;
		}
	}

	// Translate planar data to chunky (very slow method)
	for (y = 0; y < 24; y++) {
		for (x = 0; x < 24; x++) {
			byte pixel =
				  (icon_pln[((     y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 1 : 0)
				| (icon_pln[((24 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 2 : 0)
				| (icon_pln[((48 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 4 : 0)
				| (icon_pln[((72 + y) * 3) + (x >> 3)] & (1 << (7 - (x & 7))) ? 8 : 0);
			if (pixel)
				dst[x] = pixel | base;
		}
		dst += pitch;
	}
}

static void decompress_icon(byte *dst, byte *src, uint w, uint h_org, byte base, uint pitch) {
	int8 reps;
	byte color_1, color_2;
	byte *dst_org = dst;
	uint h = h_org;

	for (;;) {
		reps = *src++;
		if (reps < 0) {
			reps--;
			color_1 = *src >> 4;
			if (color_1 != 0)
				color_1 |= base;
			color_2 = *src++ & 0xF;
			if (color_2 != 0)
				color_2 |= base;

			do {
				if (color_1 != 0)
					*dst = color_1;
				dst += pitch;
				if (color_2 != 0)
					*dst = color_2;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (++reps != 0);
		} else {
			do {
				color_1 = *src >> 4;
				if (color_1 != 0)
					*dst = color_1 | base;
				dst += pitch;

				color_2 = *src++ & 0xF;
				if (color_2 != 0)
					*dst = color_2 | base;
				dst += pitch;

				// reached bottom?
				if (--h == 0) {
					// reached right edge?
					if (--w == 0)
						return;
					dst = ++dst_org;
					h = h_org;
				}
			} while (--reps >= 0);
		}
	}
}


void SimonEngine::draw_icon_c(FillOrCopyStruct *fcs, uint icon, uint x, uint y) {
	byte *dst;
	byte *src;

	_lockWord |= 0x8000;
	dst = dx_lock_2();

	if (!(_game & GF_SIMON2)) {
		// Simon 1
		dst += (x + fcs->x) * 8;
		dst += (y * 25 + fcs->y) * _dxSurfacePitch;

		if (_game & GF_AMIGA) {
			src = _iconFilePtr;
			src += READ_BE_UINT32(&((uint32 *)src)[icon]);
			decompress_icon_amiga (dst, src, 0xE0, _dxSurfacePitch);
		} else {
			src = _iconFilePtr;
			src += READ_LE_UINT16(&((uint16 *)src)[icon]);
			decompress_icon(dst, src, 24, 12, 0xE0, _dxSurfacePitch);
		}
	} else {
		// Simon 2
		dst += 110;
		dst += x;
		dst += (y + fcs->y) * _dxSurfacePitch;

		src = _iconFilePtr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 0]);
		decompress_icon(dst, src, 20, 10, 0xE0, _dxSurfacePitch);

		src = _iconFilePtr;
		src += READ_LE_UINT16(&((uint16 *)src)[icon * 2 + 1]);
		decompress_icon(dst, src, 20, 10, 0xD0, _dxSurfacePitch);
	}

	dx_unlock_2();
	_lockWord &= ~0x8000;
}

uint SimonEngine::setup_icon_hit_area(FillOrCopyStruct *fcs, uint x, uint y, uint icon_number,
																		 Item *item_ptr) {
	HitArea *ha;

	ha = findEmptyHitArea();

	if (!(_game & GF_SIMON2)) {
		ha->x = (x + fcs->x) << 3;
		ha->y = y * 25 + fcs->y;
		ha->item_ptr = item_ptr;
		ha->width = 24;
		ha->height = 24;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->layer = 100;
		ha->unk3 = 0xD0;
	} else {
		ha->x = x + 110;
		ha->y = fcs->y + y;
		ha->item_ptr = item_ptr;
		ha->width = 20;
		ha->height = 20;
		ha->flags = 0xB0;
		ha->id = 0x7FFD;
		ha->layer = 100;
		ha->unk3 = 0xD0;
	}

	return ha - _hitAreas;
}

} // End of namespace Simon
