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

namespace Kyra {

void Screen_EoB::selectPC98Palette(int palID, Palette &dest, int brightness, bool set) {
	if (palID < 0 || palID > 9)
		return;
	if (!_use16ColorMode)
		return;

	int temp = 0;
	const uint8 *pal16c = _vm->staticres()->loadRawData(kEoB1Palettes16c, temp);
	if (!pal16c)
		return;

	uint8 pal[48];
	for (int i = 0; i < 48; ++i)
		pal[i] = CLIP<int>(pal16c[palID * 48 + i] + brightness, 0, 15);
	loadPalette(pal, dest, 48);
	
	if (set)
		setScreenPalette(dest);
}

void Screen_EoB::decodeBIN(const uint8 *src, uint8 *dst, uint16 inSize) {
	const uint8 *end = src + inSize;
	memset(_dsTempPage, 0, 2048);
	int tmpDstOffs = 0;

	while (src < end) {
		uint8 code = *src++;
		if (!(code & 0x80)) {
			int offs = code << 4;
			code = *src++;
			offs |= (code >> 4);
			int len = (code & 0x0F) + 2;
			int tmpSrcOffs = (tmpDstOffs - offs) & 0x7FF;
			const uint8 *tmpSrc2 = dst;

			for (int len2 = len; len2; len2--) {
				*dst++ = _dsTempPage[tmpSrcOffs++];
				tmpSrcOffs &= 0x7FF;
			}

			while (len--) {
				_dsTempPage[tmpDstOffs++] = *tmpSrc2++;
				tmpDstOffs &= 0x7FF;
			}

		} else if (code & 0x40) {
			int len = code & 7;
			if (code & 0x20)
				len = (len << 8) | *src++;
			len += 2;

			int planes = ((code >> 3) & 3) + 1;
			while (len--) {
				for (int i = 0; i < planes; ++i) {
					*dst++ = _dsTempPage[tmpDstOffs++] = src[i];
					tmpDstOffs &= 0x7FF;
				}
			}
			src += planes;
		} else {
			for (int len = (code & 0x3F) + 1; len; len--) {
				*dst++ = _dsTempPage[tmpDstOffs++] = *src++;
				tmpDstOffs &= 0x7FF;
			}
		}
	}
}

void Screen_EoB::decodePC98PlanarBitmap(uint8 *srcDstBuffer, uint8 *tmpBuffer, uint16 size) {
	assert(tmpBuffer != srcDstBuffer);
	memcpy(tmpBuffer, srcDstBuffer, size);
	const uint8 *src = tmpBuffer;
	uint8 *dst1 = srcDstBuffer;
	uint8 *dst2 = srcDstBuffer + 4;
	size >>= 3;
	while (size--) {
		for (int i = 0; i < 4; ++i) {
			uint8 col1 = 0;
			uint8 col2 = 0;
			for (int ii = 0; ii < 4; ++ii) {
				col1 |= ((src[ii] >> (7 - i)) & 1) << ii;
				col2 |= ((src[ii] >> (3 - i)) & 1) << ii;
			}
			*dst1++ = col1;
			*dst2++ = col2;
		}
		src += 4;
		dst1 += 4;
		dst2 += 4;
	}
}

void Screen_EoB::initPC98PaletteCycle(int palID, PalCycleData *data) {
	if (!_use16ColorMode || !_cyclePalette)
		return;
	if (palID < 0 || palID > 9)
		return;

	_activePalCycle = data;
	int temp = 0;
	const uint8 *pal16c = _vm->staticres()->loadRawData(kEoB1Palettes16c, temp);
	if (!pal16c)
		return;

	if (data)
		memcpy(_cyclePalette, &pal16c[palID * 48], 48);
	else
		memset(_cyclePalette, 0, 48);
}

void Screen_EoB::updatePC98PaletteCycle(int brightness) {
	if (_activePalCycle) {
		for (int i = 0; i < 48; ++i) {
			if (--_activePalCycle[i].delay)
				continue;
			for (int8 in = 32; in == 32; ) {
				in = *_activePalCycle[i].data++;
				if (in < 16 && in > -16) {
					_cyclePalette[i] += in;
					_activePalCycle[i].delay = *_activePalCycle[i].data++;
				} else if (in < 32) {
					_cyclePalette[i] = in - 16;
					_activePalCycle[i].delay = *_activePalCycle[i].data++;
				} else if (in == 32)
					_activePalCycle[i].data += READ_BE_INT16(_activePalCycle[i].data);
			}
		}
	}

	uint8 pal[48];
	for (int i = 0; i < 48; ++i)
		pal[i] = CLIP<int>(_cyclePalette[i] + brightness, 0, 15);
	loadPalette(pal, *_palettes[0], 48);
	setScreenPalette(*_palettes[0]);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
