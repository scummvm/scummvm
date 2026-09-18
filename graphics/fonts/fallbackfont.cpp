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

#include "graphics/fonts/fallbackfont.h"

#include "graphics/managed_surface.h"
#include "graphics/surface.h"

namespace Graphics {

FallbackFont::FallbackFont(const Common::Array<Font *> &fonts) : _fonts(fonts) {
	assert(!_fonts.empty());
}

FallbackFont::~FallbackFont() {
	for (uint i = 0; i < _fonts.size(); i++)
		delete _fonts[i];
}

int FallbackFont::getFontHeight() const {
	// The primary font defines a stable line height for the entire chain.
	return _fonts[0]->getFontHeight();
}

Common::String FallbackFont::getFontName() const {
	// The composite identifies itself by the primary font rather than its fallback implementation details.
	return _fonts[0]->getFontName();
}

int FallbackFont::getFontAscent() const {
	// The primary ascent defines the common baseline used to align fallback glyphs.
	return _fonts[0]->getFontAscent();
}

int FallbackFont::getFontDescent() const {
	// The primary descent keeps line extents independent of the selected glyph source.
	return _fonts[0]->getFontDescent();
}

int FallbackFont::getFontLeading() const {
	// The primary leading keeps line spacing independent of the selected glyph source.
	return _fonts[0]->getFontLeading();
}

int FallbackFont::getMaxCharWidth() const {
	int maxWidth = 0;

	for (uint i = 0; i < _fonts.size(); i++)
		maxWidth = MAX(maxWidth, _fonts[i]->getMaxCharWidth());

	return maxWidth;
}

GlyphPresence FallbackFont::hasGlyph(uint32 chr) const {
	GlyphPresence result = GlyphPresence::kAbsent;

	for (uint i = 0; i < _fonts.size(); i++) {
		const GlyphPresence presence = _fonts[i]->hasGlyph(chr);
		if (presence == GlyphPresence::kPresent)
			return GlyphPresence::kPresent;
		if (presence == GlyphPresence::kUnknown)
			result = GlyphPresence::kUnknown;
	}

	return result;
}

bool FallbackFont::findGlyphSource(uint32 chr, GlyphSource &source) const {
	uint firstUnknownFont = _fonts.size();

	for (uint i = 0; i < _fonts.size(); i++) {
		const GlyphPresence presence = _fonts[i]->hasGlyph(chr);
		if (presence == GlyphPresence::kPresent) {
			source = GlyphSource(i, chr);
			return true;
		}
		if (presence == GlyphPresence::kUnknown && firstUnknownFont == _fonts.size())
			firstUnknownFont = i;
	}

	if (firstUnknownFont < _fonts.size()) {
		source = GlyphSource(firstUnknownFont, chr);
		return true;
	}

	return false;
}

FallbackFont::GlyphSource FallbackFont::resolveGlyph(uint32 chr) const {
	Common::HashMap<uint32, GlyphSource>::const_iterator cachedSource = _glyphSources.find(chr);
	if (cachedSource != _glyphSources.end())
		return cachedSource->_value;

	GlyphSource source;
	if (findGlyphSource(chr, source)) {
		_glyphSources[chr] = source;
		return source;
	}

	const uint32 replacementCharacters[] = {0xFFFD, '?'};
	for (uint replaceIdx = 0; replaceIdx < ARRAYSIZE(replacementCharacters); replaceIdx++) {
		const uint32 replacement = replacementCharacters[replaceIdx];
		if (replacement == chr)
			continue;

		if (findGlyphSource(replacement, source)) {
			_glyphSources[chr] = source;
			return source;
		}
	}

	source = GlyphSource(0, chr);
	_glyphSources[chr] = source;
	return source;
}

int FallbackFont::getBaselineOffset(const Font *font) const {
	const int primaryAscent = getFontAscent();
	const int fontAscent = font->getFontAscent();

	if (primaryAscent < 0 || fontAscent < 0)
		return 0;

	return primaryAscent - fontAscent;
}

int FallbackFont::getCharWidth(uint32 chr) const {
	const GlyphSource source = resolveGlyph(chr);
	return _fonts[source.fontIndex]->getCharWidth(source.chr);
}

int FallbackFont::getKerningOffset(uint32 left, uint32 right) const {
	const GlyphSource leftSource = resolveGlyph(left);
	const GlyphSource rightSource = resolveGlyph(right);

	if (leftSource.fontIndex != rightSource.fontIndex)
		return 0;

	return _fonts[leftSource.fontIndex]->getKerningOffset(leftSource.chr, rightSource.chr);
}

Common::Rect FallbackFont::getBoundingBox(uint32 chr) const {
	const GlyphSource source = resolveGlyph(chr);
	const Font *font = _fonts[source.fontIndex];
	Common::Rect boundingBox = font->getBoundingBox(source.chr);
	boundingBox.translate(0, getBaselineOffset(font));
	return boundingBox;
}

void FallbackFont::drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	const GlyphSource source = resolveGlyph(chr);
	const Font *font = _fonts[source.fontIndex];
	font->drawChar(dst, source.chr, x, y + getBaselineOffset(font), color);
}

void FallbackFont::drawChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const {
	const GlyphSource source = resolveGlyph(chr);
	const Font *font = _fonts[source.fontIndex];
	font->drawChar(dst, source.chr, x, y + getBaselineOffset(font), color);
}

void FallbackFont::drawAlphaChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const {
	const GlyphSource source = resolveGlyph(chr);
	const Font *font = _fonts[source.fontIndex];
	font->drawAlphaChar(dst, source.chr, x, y + getBaselineOffset(font), color);
}

void FallbackFont::drawAlphaChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const {
	const GlyphSource source = resolveGlyph(chr);
	const Font *font = _fonts[source.fontIndex];
	font->drawAlphaChar(dst, source.chr, x, y + getBaselineOffset(font), color);
}

} // End of namespace Graphics
