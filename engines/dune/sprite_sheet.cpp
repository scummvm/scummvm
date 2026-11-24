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
#include "dune/sprite_sheet.h"

#include "dune/dune.h"
#include "dune/graphics.h"

#include "common/debug.h"
#include "common/memstream.h"
#include "common/util.h"

namespace Dune {

SpriteSheet::SpriteSheet(DuneEngine *vm, Resource _r, bool takeItsPalette)
	: _vm(vm), r(_r)
{
	indexOffset = r->readUint16LE();

	// A bank of people carries colours of its own, and reading them in merely
	// because the bank was opened to draw somebody puts them over the room's:
	// Chani's bank sets two and forty of them, from the twentieth up, which is
	// enough to turn a sietch purple.  The palette is taken only when the
	// caller means to change it.
	if (indexOffset > 2) {
		if (takeItsPalette)
			_vm->applyPaletteBlock(&*_r);
		else
			r->seek(indexOffset);
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

void SpriteSheet::applyPalette() {
	if (indexOffset <= 2)
		return;
	const int32 position = r->pos();
	r->seek(2);
	_vm->applyPaletteBlock(&*r);
	r->seek(position);
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

}

bool SpriteSheet::read(int index, uint32 skip, byte *into, uint32 size) {
	if (index < 0 || index >= count)
		return false;

	const uint32 at = offsets[index] + skip;
	if (at + size > (uint32)r->size())
		return false;

	r->seek(at);

	return r->read(into, size) == size;
}

bool SpriteSheet::size(int index, int *width, int *height) {
	if (index < 0 || index >= count)
		return false;

	r->seek(offsets[index]);

	const uint16 w0 = r->readUint16LE();
	const uint16 w1 = r->readUint16LE();

	*width = w0 & 0x01ff;
	*height = w1 & 0x00ff;

	return true;
}

void SpriteSheet::drawMapSprite(int index, int x, int y, const Common::Rect &clip) {
	if (index < 0 || index >= count)
		return;
	r->seek(offsets[index]);
	const uint16 first = r->readUint16LE();
	const uint16 second = r->readUint16LE();
	const int width = first & 0x1ff, height = second & 0xff;
	const uint pitch = 2 * ((width + 3) / 4);
	Common::Array<byte> pixels(height * pitch);
	// ONMAP has no flipped entries. DNVGA 1372 paints compressed entries
	// through 10E2/11EE, without the plain painter's transparent-nibble quirk.
	if (first & 0x8000)
		decompress_rle_4bpp_image(&*r, width, height, pixels.begin());
	else
		r->read(pixels.begin(), pixels.size());
	_vm->_graphics.copyMapSprite(pixels.begin(), pitch, x, y, width, height, second >> 8, clip, first & 0x8000);
}

void SpriteSheet::draw(int index, int x, int y, uint16 word, byte palOverride) {
	if (index < 0 || index >= count) {
		debug("SpriteSheet::draw asked for sprite %d of %d", index, count);
		return;
	}

	r->seek(offsets[index]);

	uint16 w0 = r->readUint16LE();
	uint16 w1 = r->readUint16LE();

	byte flags      = (w0 & 0xff00) >> 8;
	// The width is nine bits and the flags the other seven; taking fifteen
	// bits for it made the flags part of the width.
	int  width      = (w0 & 0x01ff);
	int  height     = (w1 & 0x00ff);
	byte pal_offset = (w1 & 0xff00) >> 8;

	// 0xC22F takes nothing of the caller's word into the sprite's own flags
	// but the two flips: `and ch,0x60' before it ors them in.  Taking the
	// whole byte put the caller's bits where the sprite keeps its size and
	// its run length flag.
	flags |= (byte)((word >> 8) & 0x60);

	// A scene gives each sprite a palette offset of its own, and where it is
	// not nought it stands in place of the one in the sprite's own record.
	if (palOverride != 0)
		pal_offset = palOverride;

	// Which of the eight sizes to draw at is the caller's, out of bits 10 to
	// 12 of its word, and not the sprite's: 0xC25E masks the word with
	// 0x1C00.  Reading it out of the sprite's flags instead drew every figure
	// of a room one size too large.
	byte scale = (word >> 10) & 7;

	if (scale == 0) {
		_vm->_graphics.blitGraphics(&*r, x, y, width, height, flags, pal_offset);
		return;
	}

	const uint16 scale_factor_fp = READ_LE_UINT16(_vm->_staticState.begin() + 0x2774 + 2 * scale);

	uint16 dst_w = (width  << 8) / scale_factor_fp;
	uint16 dst_h = (height << 8) / scale_factor_fp;

	int pitch = 2 * ((width + 3) / 4);
	byte *tmp = new byte[height * pitch];

	if (flags & 0x80) {
		decompress_rle_4bpp_image(&*r, width, height, tmp);
	} else {
		// A sprite that is not run length encoded is scaled straight out of
		// its rows; 0xC292 only unpacks the ones that are.  Refusing to draw
		// it left holes where scenes had scaled a plain sprite.
		r->read(tmp, height * pitch);
	}

	_vm->_graphics.copyScaled(tmp, pitch, x, y, dst_w, dst_h, flags, pal_offset, scale_factor_fp);
	delete[] tmp;
}

} // End of namespace Dune
