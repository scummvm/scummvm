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

#include "ultima/shared/gfx/gfx_surface.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

constexpr int COLOR_NOTCH = 0;   // black - corner notch pixels
constexpr int COLOR_BORDER = 1;  // blue - thick outer band
constexpr int COLOR_EDGE = 15;   // white - thin inner highlight line

void GfxSurface::writeString(const Common::Point &pt, const Common::String &str,
		Graphics::TextAlign align) {
	_textPos = pt;
	writeString(str, align);
}

void GfxSurface::writeString(const Common::String &str, Graphics::TextAlign align) {
	size_t strSize = 0;
	for (const char *p = str.c_str(); *p; ++p)
		strSize += Common::isPrint(*p) ? 1 : 0;

	switch (align) {
	case Graphics::kTextAlignCenter:
		_textPos.x -= strSize / 2;
		break;
	case Graphics::kTextAlignRight:
		_textPos.x -= strSize;
		break;
	case Graphics::kTextAlignLeft:
	default:
		break;
	}

	for (const char *p = str.c_str(); *p; ++p) {
		if (*p == '\n') {
			assert(align == Graphics::kTextAlignLeft);
			newLine();
		} else {
			writeChar(*p);
		}
	}
}

void GfxSurface::writeChar(uint32 chr) {
	const int fontW = _font.getMaxCharWidth();
	const int fontH = _font.getFontHeight();
	const int textW = this->w / fontW;

	if (chr != '\n') {
		fillRect(Common::Rect(_textPos.x * fontW, _textPos.y * fontH, (_textPos.x + 1) * fontW, (_textPos.y + 1) * fontH), _bgColor);

		if (chr == 16)
			drawRightArrow();
		else if (chr == 17)
			drawLeftArrow();
		else
			_font.drawChar(this, chr, _textPos.x * fontW, _textPos.y * fontH, _textColor);

		_textPos.x++;
	}

	if (_textPos.x >= textW || chr == '\n') {
		newLine();
	}
}

// Per-column vertical extent of an 8x8 right-pointing triangle, tip at
// column 7 - column 0 is the full-height flat edge, tapering down to a
// single pixel at the tip. Used to fill the triangle solid before the
// diagonal edge highlights are drawn on top
static const byte ARROW_TOP[8] = { 0, 0, 1, 1, 2, 2, 3, 3 };
static const byte ARROW_BOTTOM[8] = { 7, 6, 6, 5, 5, 4, 4, 3 };

void GfxSurface::drawRightArrow() {
	int px = _textPos.x * 8, py = _textPos.y * 8;

	for (int i = 0; i < 8; ++i)
		drawLine(px + i, py + ARROW_TOP[i], px + i, py + ARROW_BOTTOM[i], COLOR_BORDER);

	drawLine(px, py, px + 7, py + 3, COLOR_EDGE);
	drawLine(px + 7, py + 3, px, py + 7, COLOR_EDGE);
}

void GfxSurface::drawLeftArrow() {
	int px = _textPos.x * 8, py = _textPos.y * 8;

	for (int i = 0; i < 8; ++i)
		drawLine(px + 7 - i, py + ARROW_TOP[i], px + 7 - i, py + ARROW_BOTTOM[i], COLOR_BORDER);

	drawLine(px + 7, py, px, py + 3, COLOR_EDGE);
	drawLine(px, py + 3, px + 7, py + 7, COLOR_EDGE);
}

void GfxSurface::newLine() {
	_textPos.x = 0;
	_textPos.y++;

	const int fontH = _font.getFontHeight();
	const int textH = this->h / fontH;
	if (_textPos.y >= textH) {
		_textPos.y = textH - 1;

		if (_scrollable) {
			// Scroll the screen contents up
			blitFrom(*this, Common::Rect(0, fontH, this->w, this->h), Common::Point(0, 0));
			fillRect(Common::Rect(0, this->h - fontH, this->w, this->h), 0);
		}
	}
}

void GfxSurface::setTextPos(const Common::Point &pt) {
	_textPos = pt;
}

int GfxSurface::getStringWidth(const Common::String &str) const {
	return str.size() * _font.getMaxCharWidth();
}

byte GfxSurface::setColor(byte color) {
	byte oldColor = _textColor;
	_textColor = color;
	return oldColor;
}

void GfxSurface::setColor(byte fgColor, byte bgColor) {
	_textColor = fgColor;
	_bgColor = bgColor;
}

void GfxSurface::reverseColor() {
	SWAP(_textColor, _bgColor);
}

void GfxSurface::xorBlitFrom(const ManagedSurface &src, const Common::Point &destPos) {
	Graphics::Surface area = getSubArea(Common::Rect(destPos.x, destPos.y,
		destPos.x + src.w, destPos.y + src.h));

	for (int yCtr = 0; yCtr < src.h; ++yCtr) {
		const byte *pSrc = (const byte *)src.getBasePtr(0, yCtr);
		byte *pDest = (byte *)area.getBasePtr(0, yCtr);

		for (int xCtr = 0; xCtr < src.w; ++xCtr, ++pSrc, ++pDest)
			*pDest ^= *pSrc;
	}
}

} // namespace Gfx
} // namespace Shared
} // namespace Ultima
