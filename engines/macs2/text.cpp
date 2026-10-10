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

#include "macs2/text.h"
#include "common/util.h"

namespace Macs2 {

void GlyphData::readFromeFile(Common::File &file) {
	_ascii = file.readByte();
	_width = file.readUint16LE();
	_height = file.readUint16LE();
	_data.resize(_width * _height);
	file.read(_data.data(), _width * _height);
}

void GlyphData::readFromMemory(Common::SeekableReadStream *stream) {
	_ascii = stream->readByte();
	_width = stream->readUint16LE();
	_height = stream->readUint16LE();
	_data.resize(_width * _height);
	stream->read(_data.data(), _width * _height);
}

bool Text::findGlyph(char c, GlyphData &out, const GlyphData *glyphs, uint16 numGlyphs) const {
	for (uint16 i = 0; i < numGlyphs; i++) {
		if (glyphs[i]._ascii == c) {
			out = glyphs[i];
			return true;
		}
	}

	return false;
}

void Text::addDialogueFontFallbacks() {
	// Swedish CP850 characters missing from the original DOS dialogue font.
	// Match its transparent/outline/foreground palette entries and cell
	// dimensions to preserve text alignment and line spacing.
	static const struct {
		byte character;
		char baseCharacter;
		uint16 height;
		const char *pixels;
	} svFallbackGlyphs[] = {
		{ 0x86, 'a', 14, // a with ring above
			"..####.."
			".#X##X#."
			"..#XX#.."
			"...##..."
			"...###.."
			"..#XXX#."
			".#XX#XX#"
			"#XX##XX#"
			"#XX##XX#"
			"#XX##XX#"
			"#XX#XXX#"
			"##XXX#X#"
			".#######"
			"..###.#." },
		{ 0x8f, 'A', 13, // A with ring above
			"..####.."
			".#X##X#."
			"..#XX#.."
			"..#XXXX#"
			".#XX#XX#"
			"##X##XX#"
			"#XXXXXX#"
			"#XXXXXX#"
			"#XX##XX#"
			"#XX##XX#"
			"#XX##XX#"
			"########"
			".##..##." }
	};
	for (const auto &fallback : svFallbackGlyphs) {
		GlyphData existing;
		if (_numGlyphs >= kMaxGlyphs || findGlyph((char)fallback.character, existing))
			continue;
		// Only supplement the matching DOS font style.
		GlyphData base;
		if (!findGlyph(fallback.baseCharacter, base) || base._width != 8 || base._height != fallback.height)
			continue;
		GlyphData &glyph = _glyphs[_numGlyphs++];
		glyph._ascii = fallback.character;
		glyph._width = 8;
		glyph._height = fallback.height;
		glyph._data.resize(8 * fallback.height);
		for (uint j = 0; j < glyph._data.size(); j++) {
			const char pixel = fallback.pixels[j];
			glyph._data[j] = pixel == 'X' ? 255 : pixel == '#' ? 224 : 0;
		}
		_maxGlyphHeight = MAX(_maxGlyphHeight, glyph._height);
	}
}

void Text::addIntroFontFallbacks() {
	// Identify the DOS credits font once at load time. Resource indices are
	// local to a scene/object; the other overlay font has 79 dialogue glyphs.
	if (numOverlayGlyphs != 26)
		return;

	// only for the intro font - hacky - but does the job
	// maybe it would be better to check the resource id - but i am not sure if this is the same for all supported versions
	for (uint i = 0; i < 26; i++) {
		const GlyphData &glyph = _overlayGlyphs[i];
		if (glyph._ascii != (char)('A' + i) || glyph._width != 15 || glyph._height != 18)
			return;
	}

	static const struct {
		byte character;
		char baseCharacter;
		uint16 accentX;
		const char *accent;
	} svIntroFallbackGlyphs[] = {
		{ 0x8f, 'A', 1, ".###." "#L.R#" "#L.R#" ".###." }, // ring
		{ 0x8e, 'A', 0, "###.###" "#L#.#R#" "###.###" "......." },
		{ 0x99, 'O', 4, "###.###" "#L#.#R#" "###.###" "......." }
	};
	for (const auto &fallback : svIntroFallbackGlyphs) {
		const GlyphData &base = _overlayGlyphs[fallback.baseCharacter - 'A'];
		GlyphData &glyph = _overlayGlyphs[numOverlayGlyphs++];
		glyph = base;
		glyph._ascii = fallback.character;
		Common::fill(glyph._data.begin(), glyph._data.end(), 0);
		// Reserve four rows for the accent, keeping the original cell size
		// and bottom row so overlay alignment and line spacing do not change.
		for (uint y = 0; y < 14; y++) {
			const uint sourceY = y * 17 / 13;
			for (uint x = 0; x < 15; x++)
				glyph._data[(y + 4) * 15 + x] = base._data[sourceY * 15 + x];
		}
		const uint accentWidth = fallback.character == 0x8f ? 5 : 7;
		for (uint y = 0; y < 4; y++) {
			for (uint x = 0; x < accentWidth; x++) {
				const char pixel = fallback.accent[y * accentWidth + x];
				glyph._data[y * 15 + fallback.accentX + x] =
					pixel == '#' ? 224 : pixel == 'L' ? 196 : pixel == 'R' ? 198 : 0;
			}
		}
	}

	// Translated credits can use lowercase letters. This title font has
	// only capitals, so provide aliases for A-Z and the three Swedish letters.
	const uint16 capitalCount = numOverlayGlyphs;
	for (uint16 i = 0; i < capitalCount; i++) {
		const GlyphData &capital = _overlayGlyphs[i];
		byte character = (byte)capital._ascii;
		if (character >= 'A' && character <= 'Z')
			character += 'a' - 'A';
		else if (character == 0x8f)
			character = 0x86;
		else if (character == 0x8e)
			character = 0x84;
		else if (character == 0x99)
			character = 0x94;
		GlyphData &glyph = _overlayGlyphs[numOverlayGlyphs++];
		assert(numOverlayGlyphs <= kMaxGlyphs);
		glyph = capital;
		glyph._ascii = character;
	}
}

bool Text::findGlyph(char c, GlyphData &out) const {
	return findGlyph(c, out, _glyphs, _numGlyphs);
}

int Text::measureString(const Common::String &s) const {
	int width = 0;
	GlyphData currentGlyph;
	uint16 widestGlyph = 0;
	for (auto current = s.begin(); current != s.end(); current++) {
		if (findGlyph(*current, currentGlyph)) {
			widestGlyph = MAX(widestGlyph, currentGlyph._width);
		}
	}

	for (auto current = s.begin(); current != s.end(); current++) {
		if (findGlyph(*current, currentGlyph)) {
			width += currentGlyph._width + 1;
		} else {
			width += widestGlyph;
		}
	}
	return width;
}

int Text::measureString(const Common::String &s, const GlyphData *glyphs, uint16 numGlyphs) const {
	int width = 0;
	uint16 widestGlyph = 1;
	for (uint i = 0; i < numGlyphs; i++) {
		widestGlyph = MAX(widestGlyph, glyphs[i]._width);
	}
	for (auto current = s.begin(); current != s.end(); current++) {
		GlyphData currentGlyph;
		if (findGlyph(*current, currentGlyph, glyphs, numGlyphs)) {
			width += currentGlyph._width + 1;
		} else {
			width += widestGlyph;
		}
	}
	return width;
}

int Text::measureStrings(const Common::StringArray &sa) const {
	int max = -1;
	for (auto iter = sa.begin(); iter != sa.end(); iter++) {
		max = MAX(measureString(*iter), max);
	}
	return max;
}

int Text::measureStringsVertically(const Common::StringArray &sa, int lineHeight) const {
	return (int)sa.size() * lineHeight;
}

} // namespace Macs2
