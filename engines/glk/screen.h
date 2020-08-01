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

#ifndef GLK_DRAW_H
#define GLK_DRAW_H

#include "common/archive.h"
#include "common/array.h"
#include "graphics/screen.h"
#include "graphics/font.h"
#include "glk/fonts.h"
#include "glk/utils.h"

namespace Glk {

#define FONTS_TOTAL 8

enum CaretShape {
	SMALL_DOT = 0, FAT_DOT = 1, THIN_LINE = 2, FAT_LINE = 3, BLOCK = 4
};

/**
 * Screen surface class
 */
class Screen : public Graphics::Screen {
private:
	/**
	 * Open the fonts archive and load all the fonts
	 */
	void loadFonts();

	/**
	 * Load a single font
	 */
	const Graphics::Font *loadFont(FACES face, Common::Archive *archive,
		double size, double aspect, int style);
protected:
	Common::Array<const Graphics::Font *> _fonts;
protected:
	/**
	 * Load the fonts
	 */
	virtual void loadFonts(Common::Archive *archive);
public:
	/**
	 * Return the font Id for a given name
	 */
	static FACES getFontId(const Common::String &name);

	/**
	 * Return the name for a given font Id
	 */
	static Common::String getFontName(FACES font);
public:
	/**
	 * Constructor
	 */
	Screen() : Graphics::Screen() {}

	/**
	 * Destructor
	 */
	~Screen() override;

	/**
	 * Initialize the screen
	 */
	void initialize();

	/**
	 * Fills the screen with a given color
	 */
	void fill(uint color);

	/**
	 * Fill a given area of the screen with a given color
	 */
	void fillRect(const Rect &box, uint color);

	/**
	 * Draws a string using the specified font at the given co-ordinates
	 * @param pos       Position for the bottom-left corner the text will be drawn with
	 * @param fontIdx   Which font to use
	 * @param color     Text color
	 * @param text      The text to draw
	 * @param spw       ??
	 */
	int drawString(const Point &pos, int fontIdx, uint color, const Common::String &text, int spw = 0);

	/**
	 * Draws a unicode string using the specified font at the given co-ordinates
	 * @param pos       Position for the bottom-left corner the text will be drawn with
	 * @param fontIdx   Which font to use
	 * @param color     Text color
	 * @param text      The text to draw
	 * @param spw       ??
	 */
	int drawStringUni(const Point &pos, int fontIdx, uint color, const Common::U32String &text, int spw = 0);

	/**
	 * Get the width in pixels of a string
	 * @param fontIdx   Which font to use
	 * @param text      Text to get the width of
	 * @param spw       Delta X
	 * @returns         Width of string multiplied by GLI_SUBPIX
	 */
	size_t stringWidth(int fontIdx, const Common::String &text, int spw = 0);

	/**
	 * Get the width in pixels of a unicode string
	 * @param fontIdx   Which font to use
	 * @param text      Text to get the width of
	 * @param spw       Delta X
	 * @returns         Width of string multiplied by GLI_SUBPIX
	 */
	size_t stringWidthUni(int fontIdx, const Common::U32String &text, int spw = 0);
};

} // End of namespace Glk

#endif
