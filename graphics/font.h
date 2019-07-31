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

#ifndef GRAPHICS_FONT_H
#define GRAPHICS_FONT_H

#include "common/str.h"
#include "common/ustr.h"
#include "common/rect.h"

namespace Common {
template<class T> class Array;
}

namespace Graphics {

struct Surface;
class ManagedSurface;

/** Text alignment modes */
enum TextAlign {
	kTextAlignInvalid,
	kTextAlignLeft,     ///< Text should be aligned to the left
	kTextAlignCenter,   ///< Text should be centered
	kTextAlignRight     ///< Text should be aligned to the right
};

/**
 * Instances of this class represent a distinct font, with a built-in renderer.
 * @todo Maybe move the high-level methods (drawString etc.) to a separate
 *       FontRenderer class? That way, we could have different variants... ?
 */
class Font {
public:
	Font() {}
	virtual ~Font() {}

	/**
	 * Query the height of the font.
	 *
	 * @return font height.
	 */
	virtual int getFontHeight() const = 0;

	/**
	 * Query the maximum width of the font.
	 *
	 * @return maximum font width.
	 */
	virtual int getMaxCharWidth() const = 0;

	/**
	 * Query the width of a specific character.
	 *
	 * @param chr The character to query the width of.
	 * @return The character's width.
	 */
	virtual int getCharWidth(uint32 chr) const = 0;

	/**
	 * Query the kerning offset between two characters.
	 *
	 * @param left  The left character. May be 0.
	 * @param right The right character. May be 0.
	 * @return The horizontal displacement.
	 */
	virtual int getKerningOffset(uint32 left, uint32 right) const;

	/**
	 * Calculate the bounding box of a character. It is assumed that
	 * the character shall be drawn at position (0, 0).
	 *
	 * The idea here is that the character might be drawn outside the
	 * rect (0, 0) to (getCharWidth(chr), getFontHeight()) for some fonts.
	 * This is common among TTF fonts.
	 *
	 * The default implementation simply returns the rect with a width
	 * of getCharWidth(chr) and height of getFontHeight().
	 *
	 * @param chr The character to draw.
	 * @return The bounding box of the drawn glyph.
	 */
	virtual Common::Rect getBoundingBox(uint32 chr) const;

	/**
	 * Return the bounding box of a string drawn with drawString.
	 *
	 * @param x The x position where to start drawing
	 * @param y The y position where to start drawing
	 * @param w The width of the text area. This can be 0 to allow for
	 *          obtaining the whole bounding box for a string. Note that this
	 *          does not work with an align different from kTextAlignLeft or
	 *          with useEllipsis.
	 * @param align The text alignment. This can be used to center a string
	 *              in the given area or to align it to the right.
	 * @param delatx Offset to the x starting position of the string.
	 * @param useEllipsis Try to fit the string in the area by inserting an
	 *                    ellipsis. Be ware that the default is false for this
	 *                    one unlike for drawString!
	 * @return The actual area where the string is drawn.
	 */
	Common::Rect getBoundingBox(const Common::String &str, int x = 0, int y = 0, const int w = 0, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = false) const;
	Common::Rect getBoundingBox(const Common::U32String &str, int x = 0, int y = 0, const int w = 0, TextAlign align = kTextAlignLeft) const;

	/**
	 * Draw a character at a specific point on a surface.
	 *
	 * Note that the point describes the top left edge point where to draw
	 * the character. This can be different from top left edge point of the
	 * character's bounding box! For example, TTF fonts sometimes move
	 * characters like 't' one (or more) pixels to the left to create better
	 * visual results. To query the actual bounding box of a character use
	 * getBoundingBox.
	 * @see getBoundingBox
	 *
	 * The Font implemenation should take care of not drawing outside of the
	 * specified surface.
	 *
	 * @param dst The surface to drawn on.
	 * @param chr The character to draw.
	 * @param x   The x coordinate where to draw the character.
	 * @param y   The y coordinate where to draw the character.
	 * @param color The color of the character.
	 */
	virtual void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const = 0;
	void drawChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const;

	// TODO: Add doxygen comments to this
	void drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;
	void drawString(Surface *dst, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0) const;
	void drawString(ManagedSurface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;
	void drawString(ManagedSurface *dst, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0) const;

	/**
	 * Compute and return the width the string str has when rendered using this font.
	 * This describes the logical width of the string when drawn at (0, 0).
	 * This can be different from the actual bounding box of the string. Use
	 * getBoundingBox when you need the bounding box of a drawn string.
	 * @see getBoundingBox
	 * @see drawChar
	 */
	int getStringWidth(const Common::String &str) const;
	int getStringWidth(const Common::U32String &str) const;

	/**
	 * Take a text (which may contain newline characters) and word wrap it so that
	 * no text line is wider than maxWidth pixels. If necessary, additional line breaks
	 * are generated, preferably between words (i.e. where whitespaces are).
	 * The resulting lines are appended to the lines string list.
	 * It returns the maximal width of any of the new lines (i.e. a value which is less
	 * or equal to maxWidth).
	 *
	 * @param str       the string to word wrap
	 * @param maxWidth  the maximum width a line may have
	 * @param lines     the string list to which the text lines from str are appended
	 * @param initWidth the starting width of the first line, for partially filled lines (optional)
	 * @param evenWidthLinesModeEnabled if enabled, the resulting line segments will be close to the same width (optional)
	 * @param wrapOnExplicitNewLines if enabled, forces wrapping on new line characters, otherwise treats them as single white space (optional)
	 * @return the maximal width of any of the lines added to lines
	 */
	int wordWrapText(const Common::String &str, int maxWidth, Common::Array<Common::String> &lines, int initWidth = 0, bool evenWidthLinesModeEnabled = false, bool wrapOnExplicitNewLines = true) const;
	int wordWrapText(const Common::U32String &str, int maxWidth, Common::Array<Common::U32String> &lines, int initWidth = 0, bool evenWidthLinesModeEnabled = false, bool wrapOnExplicitNewLines = true) const;

private:
	Common::String handleEllipsis(const Common::String &str, int w) const;
};

} // End of namespace Graphics

#endif
