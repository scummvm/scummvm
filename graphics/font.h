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

#ifndef GRAPHICS_FONT_H
#define GRAPHICS_FONT_H

#include "common/str.h"
#include "common/ustr.h"
#include "common/rect.h"

namespace Common {
template<class T> class Array;
}

namespace Graphics {

/**
 * @defgroup graphics_font Fonts
 * @ingroup graphics
 *
 * @brief API for representing and managing fonts on the screen.
 *
 * @{
 */

struct Surface;
class ManagedSurface;

/** Text alignment modes. */
enum TextAlign {
	kTextAlignInvalid,  ///< Indicates invalid alignment.
	kTextAlignStart,    ///< Align the text to start of line (virtual).
	kTextAlignLeft,     ///< Align the text to the left.
	kTextAlignCenter,   ///< Center the text.
	kTextAlignEnd,      ///< Align the text to end of line (virtual).
	kTextAlignRight     ///< Align the text to the right.
};

/** Word wrapping modes. */
enum WordWrapMode {
	kWordWrapDefault				 = 0,		///< Default wrapping mode.
	kWordWrapEvenWidthLines 		 = 1 << 0,	///< Make the resulting line segments close to the same width.
	kWordWrapOnExplicitNewLines		 = 1 << 1,	///< Text is wrapped on new lines. Otherwise, treats them as single whitespace.
	kWordWrapAllowTrailingWhitespace = 1 << 2 	///< Allow any amount of trailing whitespace before wrapping as it won't be drawn.
};

/**
 * Convert virtual text alignments (start + end)
 * to actual text alignment (left + right + center) for drawing.
 *
 * If actual text alignment is provided, it is returned as-is.
 *
 * @param alignH  The horizontal alignment to convert.
 * @param rtl     Indicates whether this is an RTL (right-to-left) language (such as Hebrew),
 *                or a left-to-right language (such as English).
 */
TextAlign convertTextAlignH(TextAlign alignH, bool rtl);

/**
 * Instances of this class represent a distinct font, with a built-in renderer.
 *
 * @todo Maybe move the high-level methods (drawString etc.) to a separate
 *       FontRenderer class? That way, we could have different variants... ?
 */
class Font {
public:
	Font() {}
	virtual ~Font() {}

	/**
	 * Return the height of the font.
	 *
	 * @return Font height in pixels.
	 */
	virtual int getFontHeight() const = 0;

	/**
	 * Returns the font's name, if one is available
	 */
	virtual Common::String getFontName() const { return ""; }

	/**
	 * Return the ascent of the font.
	 *
	 * @return Font ascent in pixels. If it is unknown
	 * a value of -1 is returned.
	 */
	virtual int getFontAscent() const;

	/**
	 * Return the descent of the font.
	 *
	 * @return Font descent in pixels. If it is unknown
	 * a value of -1 is returned.
	 */
	virtual int getFontDescent() const;

	/**
	 * Return the leading of the font.
	 * This is the distance between the descent line
	 * and the ascent line below it.
	 *
	 * @return Font leading in pixels. If it is unknown
	 * a value of -1 is returned.
	 */
	virtual int getFontLeading() const;

	/**
	 * Return the maximum width of the font.
	 *
	 * @return Maximum font width in pixels.
	 */
	virtual int getMaxCharWidth() const = 0;

	/**
	 * Return the width of a specific character.
	 *
	 * @param chr  The character to query the width of.
	 *
	 * @return The width of the character in pixels.
	 */
	virtual int getCharWidth(uint32 chr) const = 0;

	/**
	 * Query the kerning offset between two characters.
	 *
	 * @param left   Left character. Can be 0.
	 * @param right  Right character. Can be 0.
	 *
	 * @return The horizontal displacement.
	 */
	virtual int getKerningOffset(uint32 left, uint32 right) const;

	/**
	 * Calculate the bounding box of a character.
	 *
	 * It is assumed that the character shall be drawn at position (0, 0).
	 *
	 * The idea here is that the character might be drawn outside the
	 * rect (0, 0) to (getCharWidth(chr), getFontHeight()) for some fonts.
	 * This is common among TTF fonts.
	 *
	 * The default implementation simply returns the rect with a width
	 * of getCharWidth(chr) and height of getFontHeight().
	 *
	 * @param chr  The character to draw.
	 *
	 * @return The bounding box of the drawn glyph.
	 */
	virtual Common::Rect getBoundingBox(uint32 chr) const;

	/**
	 * Return the bounding box of a string drawn with drawString.
	 *
	 * @param str     The drawn string.
	 * @param x       The x position where to start drawing.
	 * @param y       The y position where to start drawing.
	 * @param w       Width of the text area. This can be 0 to allow for
	 *                obtaining the whole bounding box for a string. Note that this
	 *                does not work with an align different than kTextAlignLeft or
	 *                with @p useEllipsis.
	 * @param align   Text alignment. This can be used to center a string
	 *                in the given area or to align it to the right.
	 * @param deltax  Offset to the x starting position of the string.
	 * @param useEllipsis  Try to fit the string in the area by inserting an
	 *                     ellipsis. Note that the default value is false for this
	 *                     argument, unlike for drawString.
	 *
	 * @return The actual area where the string is drawn.
	 */
	Common::Rect getBoundingBox(const Common::String &str, int x = 0, int y = 0, const int w = 0, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = false) const;
	/** @overload */
	Common::Rect getBoundingBox(const Common::U32String &str, int x = 0, int _y = 0, const int w = 0, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = false) const;

	/**
	 * Draw a character at a specific point on the surface.
	 *
	 * Note that the point describes the top left edge point where to draw
	 * the character. This can be different from the top left edge point of the
	 * character's bounding box. For example, TTF fonts sometimes move
	 * characters like 't' by one (or more) pixels to the left to create better
	 * visual results. To query the actual bounding box of a character, use
	 * getBoundingBox.
	 * @see getBoundingBox
	 *
	 * The Font implementation should take care of not drawing outside of the
	 * specified surface.
	 *
	 * @param dst   The surface to draw on.
	 * @param chr   The character to draw.
	 * @param x     The x coordinate where to draw the character.
	 * @param y     The y coordinate where to draw the character.
	 * @param color The color of the character.
	 */
	virtual void drawChar(Surface *dst, uint32 chr, int x, int y, uint32 color) const = 0;
	virtual void drawChar(ManagedSurface *dst, uint32 chr, int x, int y, uint32 color) const;

	/** @overload */

	/**
	 * Draw the given @p str string to the given @p dst surface.
	 *
	 * @param dst     The surface on which to draw the string.
	 * @param str     The string to draw.
	 * @param x       The x position where to start drawing.
	 * @param y       The y position where to start drawing.
	 * @param w       Width of the text area.
	 * @param color   The color with which to draw the string.
	 * @param align   Text alignment. This can be used to center the string in the given area or to align it to the right.
	 * @param deltax  Offset to the x starting position of the string.
	 * @param useEllipsis  Use ellipsis if needed to fit the string in the area.
	 *
	 */
	void drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = false) const;
	/** @overload */
	void drawString(Surface *dst, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = false) const;
	/** @overload */
	void drawString(ManagedSurface *dst, const Common::String &str, int x, int _y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = false) const;
	/** @overload */
	void drawString(ManagedSurface *dst, const Common::U32String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = false) const;

	/**
	 * Compute and return the width of the string @p str when rendered using this font.
	 *
	 * This describes the logical width of the string when drawn at (0, 0).
	 * This can be different from the actual bounding box of the string. Use
	 * getBoundingBox when you need the bounding box of a drawn string.
	 * @see getBoundingBox
	 * @see drawChar
	 */
	int getStringWidth(const Common::String &str) const;
	/** @overload */
	int getStringWidth(const Common::U32String &str) const;

	/**
	 * Word-wrap a text (that can contain newline characters) so that
	 * no text line is wider than @p maxWidth pixels.
	 *
	 * If necessary, additional line breaks are generated, preferably between
	 * words, where whitespace is. The resulting lines are appended
	 * to the @p lines string list. This returns the maximal width of any of the new
	 * lines (i.e. a value that is smaller or equal to maxWidth).
	 *
	 * @param str        The string to word-wrap.
	 * @param maxWidth   Maximum width that a line can have.
	 * @param lines      The string list to which the text lines from @p str are appended.
	 * @param initWidth  Starting width of the first line, for partially filled lines (optional).
	 * @param mode		 Wrapping mode. A bitfield of @c WordWrapMode values.
	 *
	 * @return The maximal width of any of the lines added to @p lines.
	 */
	int wordWrapText(const Common::String &str, int maxWidth, Common::Array<Common::String> &lines, int initWidth = 0, uint32 mode = kWordWrapOnExplicitNewLines) const;
	/** @overload */
	int wordWrapText(const Common::U32String &str, int maxWidth, Common::Array<Common::U32String> &lines, int initWidth = 0, uint32 mode = kWordWrapOnExplicitNewLines) const;
	/**
	 * @overload
	 * Word-wrap a text, and returns in lineCountination a list of lines where a word has
	 * been splitted into the next line.
	 *
	 * @param lineContinuation	Bool list. If the ith element of the list is true, then the ith
	 * line in lines contains a splitted word.
	 */
	int wordWrapText(const Common::U32String &str, int maxWidth, Common::Array<Common::U32String> &lines, Common::Array<bool> &lineContinuation, int initWidth = 0, uint32 mode = kWordWrapOnExplicitNewLines) const;

	/**
	 * Scales the single gylph at @p chr the given the @p scale and the pointer @p grayScaleMap to the grayscale array. It fills @p scaleSurface surface 
	 * and then we draw the character on @p scaleSurface surface. The @p scaleSUrface is magnified to grayScale array and then we change the @p scaleSurface using the 
	 * grayScale array and @p grayLevel.
	 *
	 * @param grayScaleMap   The pointer to the grayScale array.
	 * @param grayScaleMapSize  The size of the grayScale array.
	 * @param scaleSurface  The surface to where character is scaled and drawn.
	 * @param width     The width of the bouding box for scaled glyph.
	 * @param height    The height of the bounding box for scaled glyph.
	 * @param grayLevel The graylevel is the threshold value above which the pixel is considered
	 * @param chr       The character to scale.
	 * @param xOffset   The x offset of the bounding box for scaled glyph.
	 * @param yOffset   The y offset of the bounding box for scaled glyph.
	 * @param srcheight The height of the source glyph.
	 * @param srcwidth  The width of the source glyph bounding box.
	 * @param scale     The scale factor that is equal to @p newSize / @p srcSize of initializeScaling.
	 */
	void scaleSingleGlyph(Surface *scaleSurface, int *grayScaleMap, int grayScaleMapSize, int width, int height, int xOffset, int yOffset, int grayLevel, int chr, int srcheight, int srcwidth, float scale) const;

};
/** @} */
} // End of namespace Graphics

#endif
