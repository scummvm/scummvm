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

#include "tesoro/core/textqueue.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "tesoro/core/animbank.h"

namespace Tesoro {

static const int kSpaceGlyph = 94;		// the glyph of 0x7f, whose width is the width of a space

uint16 TextQueue::add(const byte *text, uint32 colour, int32 centreX, int32 y, int32 maxWidth, uint16 tag, bool centred, int32 lineHeight) {
	if (!_font || _font->stepCount() <= kSpaceGlyph) {
		warning("TextQueue: the font is not available");
		return tag;
	}
	const AnimBank &font = *_font;
	const int32 glyphs = font.imageCount();
	const int32 spaceWidth = font.step(kSpaceGlyph).hitW + 2;

	if (_is555)
		colour = (colour & 0x1f) | ((colour >> 1) & 0x7fe0);
	if (lineHeight == 0)
		lineHeight = (font.step(kSpaceGlyph).hitH * 3) / 2;

	const int32 widest = (kWidth / 2 - spaceWidth) * 2;
	if (maxWidth > widest) {
		centreX = kWidth / 2;
		maxWidth = widest;
	}

	const uint32 first = _lines.size();
	Line current;
	int32 width = 0;
	bool onlyGlyphs = true;
	const byte *p = text;
	const byte *lineStart = text;
	while (*p) {
		const int32 index = (int32)*p - 0x21;
		if (index < 0 || index >= glyphs || index >= (int32)font.stepCount()) {
			current.chars.push_back(0x7f);
			width += spaceWidth;
			onlyGlyphs = false;
		} else {
			current.chars.push_back(*p);
			width += 2 + font.step(index).hitW;
		}

		const byte *next = p + 1;
		if (maxWidth < width || *next == 0) {
			if (!onlyGlyphs && width > maxWidth) {
				// Back to the last space and break there; the space itself is dropped.
				const byte *space = p;
				while (space > lineStart && *space != 0x20)
					--space;
				if (*space == 0x20 && space >= lineStart) {
					current.chars.resize(current.chars.size() - (p - space) - 1);
					next = space + 1;
				}
			}

			int32 lineWidth = maxWidth;
			if (centred) {
				lineWidth = 0;
				for (uint i = 0; i < current.chars.size(); ++i)
					lineWidth += 2 + font.step(current.chars[i] - 0x21).hitW;
			}
			const int32 left = centreX - lineWidth / 2;
			if (left < spaceWidth)
				current.x = spaceWidth;
			else if (kWidth - spaceWidth < centreX + lineWidth / 2)
				current.x = (kWidth - spaceWidth) - lineWidth;
			else
				current.x = left;

			current.y = 0;
			current.colour = (uint16)colour;
			_lines.push_back(current);
			current.chars.clear();
			width = 0;
			onlyGlyphs = true;
			lineStart = next;
		}
		p = next;
	}

	const int32 count = _lines.size() - first;
	if (centred)
		y -= (count * lineHeight) / 2;
	int32 top = lineHeight;
	if (lineHeight <= y) {
		top = y;
		if (kHeight - lineHeight < count * lineHeight + y)
			top = kHeight - (count + 1) * lineHeight;
	}
	for (uint32 i = first; i < _lines.size(); ++i) {
		_lines[i].y = top;
		top += lineHeight;
	}
	return tag;
}

void TextQueue::shiftLines(uint32 first, int32 dy) {
	for (uint32 i = first; i < _lines.size(); ++i)
		_lines[i].y += dy;
}

void TextQueue::draw(uint16 *frame) const {
	if (!_font)
		return;
	const AnimBank &font = *_font;
	for (uint32 n = 0; n < _lines.size(); ++n) {
		const Line &line = _lines[n];
		const int32 baseline = font.step(kSpaceGlyph).imageY;
		int32 x = line.x;

		for (uint i = 0; i < line.chars.size(); ++i) {
			const int32 index = line.chars[i] - 0x21;
			if (index < 0 || index >= (int32)font.stepCount())
				continue;
			const AnimStep &seq = font.step(index);
			if (!seq.hasImage() || (uint32)seq.imageIndex >= font.imageCount() || !font.image(seq.imageIndex).isLoaded())
				continue;

			const byte *blob = font.image(seq.imageIndex).data();
			const uint32 size = font.image(seq.imageIndex).size();
			const int32 width = READ_LE_UINT16(blob + 8);
			const int32 height = READ_LE_UINT16(blob + 10);
			const uint16 type = READ_LE_UINT16(blob + 12);
			int32 row = line.y + baseline - seq.imageY;

			if (index != kSpaceGlyph) {
				for (int32 r = 0; r < height; ++r, ++row) {
					const uint32 header = 0x10 + 8 * r;
					if (header + 8 > size)
						break;
					const uint32 itemsAt = READ_LE_UINT32(blob + header);
					const uint32 items = READ_LE_UINT16(blob + header + 4);
					int32 dx = x;
					for (uint32 k = 0; k < items; ++k) {
						const uint32 at = itemsAt + 8 * k;
						if (at + 8 > size)
							break;
						const uint32 value = READ_LE_UINT32(blob + at);
						const int32 gap = READ_LE_UINT16(blob + at + 4);
						const int32 length = READ_LE_UINT16(blob + at + 6);
						dx += gap;
						for (int32 px = 0; px < length; ++px) {
							// Like the original, a glyph past the right edge continues on the next row.
							const int32 pixel = row * kWidth + dx + px;
							if (pixel < 0 || pixel >= kWidth * kHeight)
								continue;
							if (type == 1) {
								if (value + 2 * px + 2 <= size)
									frame[pixel] = READ_LE_UINT16(blob + value + 2 * px);
							} else if (type == 3) {
								frame[pixel] = value == 0 ? 0 : line.colour;
							}
						}
						dx += length;
					}
				}
			}
			x += 2 + width;
		}
	}
}

} // End of namespace Tesoro
