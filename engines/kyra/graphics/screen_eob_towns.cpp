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
#include "graphics/sjis.h"

namespace Kyra {

void Screen_EoB::decodeSHP(const uint8 *data, int dstPage) {
	int32 bytesLeft = READ_LE_UINT32(data);
	const uint8 *src = data + 4;
	uint8 *dst = getPagePtr(dstPage);

	if (bytesLeft < 0) {
		memcpy(dst, data, 64000);
		return;
	}

	while (bytesLeft > 0) {
		uint8 code = *src++;
		bytesLeft--;

		for (int i = 8; i; i--) {
			if (code & 0x80) {
				uint16 copyOffs = (src[0] << 4) | (src[1] >> 4);
				uint8 count = (src[1] & 0x0F) + 3;
				src += 2;
				bytesLeft -= 2;
				const uint8 *copySrc = dst - 1 - copyOffs;
				while (count--)
					*dst++ = *copySrc++;
			} else if (bytesLeft) {
				*dst++ = *src++;
				bytesLeft--;
			} else {
				break;
			}
			code <<= 1;
		}
	}
}

void Screen_EoB::convertToHiColor(int page) {
	if (!_16bitPalette)
		return;
	uint16 *dst = (uint16 *)getPagePtr(page);
	memcpy(_convertHiColorBuffer, dst, SCREEN_H * SCREEN_W);
	uint8 *src = _convertHiColorBuffer;
	for (int s = SCREEN_H * SCREEN_W; s; --s)
		*dst++ = _16bitPalette[*src++];
}

void Screen_EoB::shadeRect(int x1, int y1, int x2, int y2, int shadingLevel) {
	if (!_16bitPalette)
		return;

	int l = _16bitShadingLevel;
	_16bitShadingLevel = shadingLevel;

	if (_curPage == 0 || _curPage == 1)
		addDirtyRect(x1, y1, x2 - x1 + 1, y2 - y1 + 1);

	uint16 *dst = (uint16*)(getPagePtr(_curPage) + y1 * SCREEN_W * _bytesPerPixel + x1 * _bytesPerPixel);

	for (; y1 < y2; ++y1) {
		uint16 *ptr = dst;
		for (int i = 0; i < x2 - x1; i++) {
			*ptr = shade16bitColor(*ptr);
			ptr++;
		}
		dst += SCREEN_W;
	}

	_16bitShadingLevel = l;
}

SJISFontLarge::SJISFontLarge(Common::SharedPtr<Graphics::FontSJIS> &font) : SJISFont(font, 0, false, false, 0) {
	_sjisWidth = _font->getMaxFontWidth();
	_fontHeight = _font->getFontHeight();
	_asciiWidth = _font->getCharWidth('a');
}

void SJISFontLarge::drawChar(uint16 c, byte *dst, int pitch, int) const {
	_font->setDrawingMode(Graphics::FontSJIS::kDefaultMode);
	_font->toggleFatPrint(false);
	_font->drawChar(dst, c, 320, 1, _colorMap[1], _colorMap[0], 320, 200);
}

SJISFont12x12::SJISFont12x12(const uint16 *searchTable) : _height(6), _width(6), _data(0) {
	assert(searchTable);
	for (int i = 0; i < 148; i++)
		_searchTable[searchTable[i]] = i + 1;
}

bool SJISFont12x12::load(Common::SeekableReadStream &file) {
	delete[] _data;
	int size = 148 * 24;
	if (file.size() < size)
		return false;

	_data = new uint8[size];
	file.read(_data, size);

	return true;
}

void SJISFont12x12::unload() {
	delete[] _data;
	_data = 0;
	_searchTable.clear();
}

void SJISFont12x12::drawChar(uint16 c, byte *dst, int pitch, int) const {
	int offs = _searchTable[c];
	if (!offs)
		return;

	const uint8 *src = _data + (offs - 1) * 24;
	uint8 color1 = _colorMap[1];

	int bt = 0;
	uint16 chr = 0;

	for (int i = 0; i < 192; ++i) {
		if (!bt) {
			chr = *src++;
			bt = 8;
		}
		if (chr & 0x80)
			*dst = color1;
		dst++;
		if (--bt)
			chr <<= 1;
		else if (i & 8)
			dst += (pitch - 16);
	}
}

} // End of namespace Kyra

#endif // ENABLE_EOB
