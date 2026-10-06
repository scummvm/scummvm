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

#ifndef MACS2_TEXT_H
#define MACS2_TEXT_H

#include "common/file.h"
#include "common/str.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "macs2/sprite.h"

namespace Macs2 {

struct GlyphData : public Sprite {
	char _ascii = 0;

	void readFromeFile(Common::File &file);
	void readFromMemory(Common::SeekableReadStream *stream);
};

class Text {
public:
	static constexpr uint kMaxGlyphs = 256;

	GlyphData _glyphs[kMaxGlyphs];
	GlyphData _panelGlyphs[kMaxGlyphs];
	GlyphData _overlayGlyphs[kMaxGlyphs];
	uint16 _numGlyphs = 79;
	uint16 _maxGlyphHeight = 0;
	uint16 numOverlayGlyphs = 0;
	uint16 maxOverlayGlyphHeight = 0;
	uint16 numPanelGlyphs = 0;
	uint16 maxPanelGlyphHeight = 0;
	uint16 amigaTextLinePitch = 0;

	bool findGlyph(char c, GlyphData &out) const;
	bool findGlyph(char c, GlyphData &out, const GlyphData *glyphs, uint16 numGlyphs) const;

	int measureString(const Common::String &s) const;
	int measureString(const Common::String &s, const GlyphData *glyphs, uint16 numGlyphs) const;
	int measureStrings(const Common::StringArray &sa) const;
	int measureStringsVertically(const Common::StringArray &sa, int lineHeight) const;
};

} // namespace Macs2

#endif
