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
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "dune/sprite_sheet.h"

#include "dune/dune.h"
#include "dune/graphics.h"

#include "common/debug.h"
#include "common/endian.h"
#include "common/memstream.h"
#include "common/util.h"

namespace Dune {

SpriteSheet::SpriteSheet(DuneEngine *vm, Resource _r)
	: _vm(vm), r(_r)
{
	indexOffset = r->readUint16LE();
	if (indexOffset > 2) {
		_vm->applyPaletteBlock(&*_r);
	}

	r->seek(indexOffset);
	uint16 firstChunkOffset = r->readUint16LE();

	count = firstChunkOffset / 2;

	offsets.reserve(count);

	for (int i = 0; i != count; ++i) {
		r->seek(indexOffset + 2 * i);
		uint16 offset = r->readUint16LE();
		offsets.push_back(indexOffset + offset);
	}
	offsets.push_back(r->size());
}

/*
 * Decompresses a run-length encoded 4bpp image
 * into a non-run-length encoded 4bpp image.
 */

void decompress_rle_4bpp_image(Common::ReadStream *src, int w, int h, byte *dst) {
	byte *odst = dst;
	for (int y = 0; y != h; ++y) {
		int pitch = 2 * ((w + 3) / 4);
		do {
			byte cmd = src->readByte();
			int count;

			if (cmd & 0x80) {
				count = 257 - cmd;
				byte value = src->readByte();
				for (int i = 0; i != count; ++i) {
					*dst++ = value;
				}
			} else {
				count = cmd + 1;
				for (int i = 0; i != count; ++i) {
					*dst++ = src->readByte();
				}
			}
			pitch -= count;
		} while (pitch != 0);
	}

	FILE *f = fopen("out.ppm", "w");
	int pitch = 2 * ((w + 3) / 4);
	fprintf(f, "P6 %d %d 255\n", pitch, h);
	for (int i = 0; i != pitch*h; ++i) {
		byte b = odst[i];
		fwrite(&b, 1, 1, f);
		fwrite(&b, 1, 1, f);
		fwrite(&b, 1, 1, f);
	}
	fclose(f);
}

void SpriteSheet::draw(int index, int x, int y, byte draw_flags) {
	if (index >= count) {
		debug("SpriteSheet::draw %d >= %d", index, count);
		return;
	}
	assert(index < count);

	r->seek(offsets[index]);

	uint16 w0 = r->readUint16LE();
	uint16 w1 = r->readUint16LE();

	byte flags      = (w0 & 0xff00) >> 8;
	int  width      = (w0 & 0x7fff);
	int  height     = (w1 & 0x00ff);
	byte pal_offset = (w1 & 0xff00) >> 8;

	flags |= draw_flags;

	byte scale = (flags & 0x1c) >> 2;

	if (scale == 0) {
		_vm->_graphics.blitGraphics(&*r, x, y, width, height, flags, pal_offset);
		return;
	}

	const uint16 scale_factor_fp = READ_LE_UINT16(_vm->_staticState.begin() + 0x2774 + 2 * scale);

	uint16 dst_w = (width  << 8) / scale_factor_fp;
	uint16 dst_h = (height << 8) / scale_factor_fp;

	if (flags & 0x80) {
		int pitch = 2 * ((width + 3) / 4);

		byte *tmp = new byte[height * pitch];
		decompress_rle_4bpp_image(&*r, width, height, tmp);

		_vm->_graphics.copyScaled(tmp, pitch, x, y, dst_w, dst_h, flags, pal_offset, scale_factor_fp);
		delete[] tmp;
	}
}

} // End of namespace Dune
