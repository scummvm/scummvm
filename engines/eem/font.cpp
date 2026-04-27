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

#include "common/debug.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/tokenizer.h"

#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "eem/detection.h"
#include "eem/font.h"

namespace EEM {

bool EEMFont::load(const Common::Path &path) {
	Common::File f;
	if (!f.open(path)) {
		warning("EEMFont::load: cannot open %s", path.toString().c_str());
		return false;
	}

	const uint16 numChars = f.readUint16LE();
	if (numChars == 0 || numChars > 256) {
		warning("EEMFont::load: %s reports %u chars", path.toString().c_str(), numChars);
		return false;
	}
	_glyphs.resize(numChars);

	for (uint i = 0; i < numChars; i++) {
		FontGlyph &g = _glyphs[i];
		g.height    = f.readByte();
		g.widthBits = f.readByte();
		g.sizeBytes = f.readByte();
		if (g.sizeBytes > 0) {
			g.bitmap.resize(g.sizeBytes);
			if (f.read(g.bitmap.data(), g.sizeBytes) != g.sizeBytes) {
				warning("EEMFont::load: short bitmap read at glyph %u", i);
				return false;
			}
		}
		if (g.height > _maxHeight)
			_maxHeight = g.height;
	}

	debugC(1, kDebugGfx, "Font %s loaded: %u glyphs, max h=%u",
		   path.toString().c_str(), numChars, _maxHeight);
	return true;
}

int EEMFont::charWidth(byte c) const {
	if (c >= _glyphs.size())
		return 0;
	return _glyphs[c].widthBits;
}

int EEMFont::stringWidth(const Common::String &s) const {
	int w = 0;
	for (uint i = 0; i < s.size(); i++)
		w += charWidth((byte)s[i]);
	return w;
}

int EEMFont::drawChar(Graphics::ManagedSurface *dst, int x, int y, byte c, byte color) const {
	if (!dst || c >= _glyphs.size())
		return 0;
	const FontGlyph &g = _glyphs[c];
	if (g.bitmap.empty())
		return g.widthBits;

	const uint bytesPerRow = (g.widthBits + 7) / 8;
	for (uint row = 0; row < g.height; row++) {
		const int dstY = y + (int)row;
		if (dstY < 0 || dstY >= dst->h)
			continue;
		const byte *srcRow = g.bitmap.data() + row * bytesPerRow;
		byte *dstRow = (byte *)dst->getBasePtr(0, dstY);
		for (uint bit = 0; bit < g.widthBits; bit++) {
			const int dstX = x + (int)bit;
			if (dstX < 0 || dstX >= dst->w)
				continue;
			const byte mask = (byte)(0x80 >> (bit & 7));
			if (srcRow[bit / 8] & mask)
				dstRow[dstX] = color;
		}
	}
	return g.widthBits;
}

int EEMFont::drawString(Graphics::ManagedSurface *dst, int x, int y,
						const Common::String &s, byte color) const {
	int penX = x;
	for (uint i = 0; i < s.size(); i++)
		penX += drawChar(dst, penX, y, (byte)s[i], color);
	return penX - x;
}

int EEMFont::drawWordWrapped(Graphics::ManagedSurface *dst, int x, int y, int width,
							 const Common::String &s, byte color) const {
	Common::StringTokenizer tok(s, " \t");
	int penY = y;
	Common::String line;

	while (!tok.empty()) {
		const Common::String word = tok.nextToken();
		Common::String trial = line.empty() ? word : line + " " + word;
		if (stringWidth(trial) <= width) {
			line = trial;
		} else {
			drawString(dst, x, penY, line, color);
			penY += _maxHeight + 1;
			line = word;
		}
	}
	if (!line.empty())
		drawString(dst, x, penY, line, color);
	return penY + _maxHeight - y;
}

} // End of namespace EEM
