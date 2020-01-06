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


#ifdef ENABLE_EOB

#include "kyra/resource/resource.h"
#include "common/memstream.h"

namespace Kyra {

static uint32 _decodeFrameAmiga_x = 0;

bool decodeFrameAmiga_readNextBit(const uint8 *&data, uint32 &code, uint32 &chk) {
	_decodeFrameAmiga_x = code & 1;
	code >>= 1;
	if (code)
		return _decodeFrameAmiga_x;

	data -= 4;
	code = READ_BE_UINT32(data);
	chk ^= code;
	_decodeFrameAmiga_x = code & 1;
	code = (code >> 1) | (1 << 31);

	return _decodeFrameAmiga_x;
}

uint32 decodeFrameAmiga_readBits(const uint8 *&data, uint32 &code, uint32 &chk, int count) {
	uint32 res = 0;
	while (count--) {
		decodeFrameAmiga_readNextBit(data, code, chk);
		uint32 bt1 = _decodeFrameAmiga_x;
		_decodeFrameAmiga_x = res >> 31;
		res = (res << 1) | bt1;
	}
	return res;
}

void Screen_EoB::loadSpecialAmigaCPS(const char *fileName, int destPage, bool isGraphics) {
	uint32 fileSize = 0;
	const uint8 *file = _vm->resource()->fileData(fileName, &fileSize);

	if (!file)
		error("Screen_EoB::loadSpecialAmigaCPS(): Failed to load file '%s'", file);

	uint32 inSize = READ_BE_UINT32(file);
	const uint8 *pos = file;

	// Check whether the file starts with the actual compression header.
	// If this is not the case, there should a palette before the header.
	// Unlike normal CPS files these files never have more than one palette.
	if (((inSize + 15) & ~3) != ((fileSize + 3) & ~3)) {
		Common::MemoryReadStream in(pos, 64);
		_palettes[0]->loadAmigaPalette(in, 0, 32);
		pos += 64;
	}

	inSize = READ_BE_UINT32(pos);
	uint32 outSize = READ_BE_UINT32(pos + 4);
	uint32 chk = READ_BE_UINT32(pos + 8);

	pos = pos + 8 + inSize;
	uint8 *dstStart = _pagePtrs[destPage];
	uint8 *dst = dstStart + outSize;

	uint32 val = READ_BE_UINT32(pos);
	_decodeFrameAmiga_x = 0;
	chk ^= val;

	while (dst > dstStart) {
		int para = -1;
		int para2 = 0;

		if (decodeFrameAmiga_readNextBit(pos, val, chk)) {
			uint32 code = decodeFrameAmiga_readBits(pos, val, chk, 2);

			if (code == 3) {
				para = para2 = 8;
			} else {
				int cnt = 0;
				if (code < 2) {
					cnt = 3 + code;
					para2 = 9 + code;
				} else {
					cnt = decodeFrameAmiga_readBits(pos, val, chk, 8) + 1;
					para2 = 12;
				}
					
				code = decodeFrameAmiga_readBits(pos, val, chk, para2);
				while (cnt--) {
					dst--;
					*dst = dst[code & 0xFFFF];
				}
			}
		} else {
			if (decodeFrameAmiga_readNextBit(pos, val, chk)) {
				uint32 code = decodeFrameAmiga_readBits(pos, val, chk, 8);
				dst--;
				*dst = dst[code & 0xFFFF];
				dst--;
				*dst = dst[code & 0xFFFF];

			} else {
				para = 3;				
			}
		}

		if (para > 0) {
			uint32 code = decodeFrameAmiga_readBits(pos, val, chk, para);
			uint32 cnt = (code & 0xFFFF) + para2 + 1;

			while (cnt--) {
				for (int i = 0; i < 8; ++i) {
					decodeFrameAmiga_readNextBit(pos, val, chk);
					uint32 bt1 = _decodeFrameAmiga_x;
					_decodeFrameAmiga_x = code >> 31;
					code = (code << 1) | bt1;
				}
				*(--dst) = code & 0xFF;
			}
		}
	}

	delete[] file;

	if (chk)
		error("Screen_EoB::loadSpecialAmigaCPS(): Checksum error");

	if (isGraphics)
		convertAmigaGfx(_pagePtrs[destPage], 320, 200);
}

void Screen_EoB::setDualPalettes(Palette &top, Palette &bottom) {
	// The original supports simultaneous fading of both palettes, but doesn't make any use of that
	// feature. The fade rate is always set to 0. So I see no need to implement that.
	_palettes[0]->copy(top, 0, 32, 0);
	_palettes[0]->copy(bottom, 0, 32, 32);
	setScreenPalette(*_palettes[0]);
	enableDualPaletteMode(120);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
