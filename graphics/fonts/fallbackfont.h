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

#ifndef GRAPHICS_FONTS_FALLBACKFONT_H
#define GRAPHICS_FONTS_FALLBACKFONT_H

#include "common/array.h"
#include "common/hashmap.h"
#include "common/noncopyable.h"
#include "graphics/font.h"

namespace Graphics {

/**
 * @brief Font implementation that selects the first font containing each requested glyph.
 *
 * Fonts are queried in array order. A @ref Graphics::GlyphPresence::kPresent result takes precedence over unknown coverage.
 * If no font reports the glyph present, the first font reporting @ref Graphics::GlyphPresence::kUnknown is used.
 * The U+FFFD replacement character and then '?' are considered only when every font reports the requested glyph absent.
 *
 * The first font is the primary font and provides the line metrics.
 * Keeping one set of metrics prevents line height and baseline changes when adjacent characters resolve to different fonts.
 * Glyphs from later fonts are shifted vertically to keep their baselines aligned with the primary font.
 *
 * @note Reliable fallback selection requires every font in the chain to implement @ref Graphics::Font::hasGlyph() accurately.
 * A font that reports @ref Graphics::GlyphPresence::kUnknown for a missing glyph may be selected before replacement fallback is attempted.
 *
 * @note This class is non-copyable because it owns every font in its chain.
 * A shallow copy would make multiple instances delete the same @ref Graphics::Font objects.
 */
class FallbackFont : public Font, public Common::NonCopyable {
public:
	/**
	 * Construct a fallback font from an ordered list of fonts.
	 *
	 * This object takes ownership of every font in @p fonts and deletes them when destroyed.
	 *
	 * @param fonts  Fonts to query in fallback order. The array must not be empty.
	 */
	explicit FallbackFont(const Common::Array<Font *> &fonts);

	/**
	 * Destroy the fallback font and every font passed to the constructor.
	 */
	~FallbackFont() override;

	/**
	 * Return the height of the first font in the chain.
	 *
	 * The primary font defines the line height so that selecting a fallback glyph does not change line spacing.
	 *
	 * @return Font height in pixels.
	 */
	int getFontHeight() const override;

	/**
	 * Return the name of the first font in the chain.
	 *
	 * The composite uses the primary font name because fallback fonts are implementation details of glyph selection.
	 *
	 * @return Font name, or an empty string if the first font has no name.
	 */
	Common::String getFontName() const override;

	/**
	 * Return the ascent of the first font in the chain.
	 *
	 * The primary ascent defines the common baseline used to align fallback glyphs.
	 *
	 * @return Font ascent in pixels, or -1 if it is unknown.
	 */
	int getFontAscent() const override;

	/**
	 * Return the descent of the first font in the chain.
	 *
	 * The primary descent keeps the line extent independent of the selected glyph source.
	 *
	 * @return Font descent in pixels, or -1 if it is unknown.
	 */
	int getFontDescent() const override;

	/**
	 * Return the leading of the first font in the chain.
	 *
	 * The primary leading keeps the distance between lines independent of the selected glyph source.
	 *
	 * @return Font leading in pixels, or -1 if it is unknown.
	 */
	int getFontLeading() const override;

	/**
	 * Return the largest maximum character width reported by any font in the chain.
	 *
	 * @return Maximum character width in pixels.
	 */
	int getMaxCharWidth() const override;

	/**
	 * Return the combined glyph presence reported by the font chain.
	 *
	 * @param chr  Character to query.
	 *
	 * @return @ref Graphics::GlyphPresence::kPresent if any font reports the glyph present,
	 * @ref Graphics::GlyphPresence::kUnknown if none reports it present and at least one reports unknown,
	 * or @ref Graphics::GlyphPresence::kAbsent if every font reports it absent.
	 */
	GlyphPresence hasGlyph(uint32 chr) const override;

	/**
	 * Return the width of the resolved glyph.
	 *
	 * @param chr  Character to resolve and query.
	 *
	 * @return Resolved glyph width in pixels.
	 */
	int getCharWidth(uint32 chr) const override;

	/**
	 * Return the kerning offset between two resolved glyphs.
	 *
	 * Kerning is applied only when both glyphs resolve to the same font.
	 *
	 * @param left   Left character. Can be 0.
	 * @param right  Right character. Can be 0.
	 *
	 * @return Horizontal displacement, or 0 when the glyphs resolve to different fonts.
	 */
	int getKerningOffset(uint32 left, uint32 right) const override;

	/**
	 * Return the bounding box of the resolved glyph with baseline alignment applied.
	 *
	 * @param chr  Character to resolve and query.
	 *
	 * @return Bounding box of the resolved glyph.
	 */
	Common::Rect getBoundingBox(uint32 chr) const override;

	/**
	 * Draw a resolved character on a surface.
	 *
	 * @param dst    Surface to draw on.
	 * @param chr    Character to resolve and draw.
	 * @param x      Horizontal drawing coordinate.
	 * @param y      Vertical drawing coordinate. Fallback glyphs are offset to align with the primary baseline.
	 * @param color  Character color.
	 */
	void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	/** @overload */
	void drawChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override;

	/**
	 * Draw a resolved character on a surface while preserving its alpha channel.
	 *
	 * @param dst    Surface to draw on.
	 * @param chr    Character to resolve and draw.
	 * @param x      Horizontal drawing coordinate.
	 * @param y      Vertical drawing coordinate. Fallback glyphs are offset to align with the primary baseline.
	 * @param color  Character color.
	 */
	void drawAlphaChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const override;

	/** @overload */
	void drawAlphaChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const override;

private:
	/** Resolved font and character pair used for rendering and metrics. */
	struct GlyphSource {
		/** Construct an empty source. */
		GlyphSource() {}

		/**
		 * Construct a resolved source.
		 *
		 * @param fontIdx    Index of the font in the fallback chain.
		 * @param character  Character to use from that font.
		 */
		GlyphSource(uint fontIdx, uint32 character) : fontIndex(fontIdx), chr(character) {}

		/** Index of the selected font in @ref Graphics::FallbackFont::_fonts. */
		uint fontIndex = 0;

		/** Character to query or render with the selected font. */
		uint32 chr = 0;
	};

	/**
	 * Find the best source for a character without applying replacement characters.
	 *
	 * @param chr     Character to resolve.
	 * @param source  Receives the selected font and character.
	 *
	 * @return True if a font reports the character present or unknown, otherwise false.
	 */
	bool findGlyphSource(uint32 chr, GlyphSource &source) const;

	/**
	 * Resolve and cache a source for a character, including replacement fallback.
	 *
	 * @param chr  Character to resolve.
	 *
	 * @return Selected font and character.
	 */
	GlyphSource resolveGlyph(uint32 chr) const;

	/**
	 * Calculate the vertical offset required to align a font with the primary baseline.
	 *
	 * @param font  Font to align.
	 *
	 * @return Vertical drawing offset in pixels, or 0 if either ascent is unknown.
	 */
	int getBaselineOffset(const Font *font) const;

	/** Ordered list of owned fonts. */
	Common::Array<Font *> _fonts;

	/** Cached source for each requested character. */
	mutable Common::HashMap<uint32, GlyphSource> _glyphSources;
};

} // End of namespace Graphics

#endif
