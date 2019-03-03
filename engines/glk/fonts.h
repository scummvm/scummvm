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

#ifndef GLK_FONTS_H
#define GLK_FONTS_H

#include "glk/glk_types.h"
#include "glk/utils.h"

namespace Glk {

enum FACES { MONOR, MONOB, MONOI, MONOZ, PROPR, PROPB, PROPI, PROPZ, CUSTOM, CUSTOM2 };
enum TYPES { MONOF, PROPF };
enum STYLES { FONTR, FONTB, FONTI, FONTZ };

/**
 * Font configuration info
 */
struct FontInfo {
public:
	static uint _caretColor, _caretSave;
	static int _caretShape;
public:
	double _size;
	double _aspect;
	int _cellW, _cellH;
	int _leading;
	int _baseLine;
	uint _linkColor, _linkSave;
	uint _moreColor, _moreSave;
	int _linkStyle;
	FACES _moreFont;
	int _moreAlign;
	Common::String _morePrompt;
	int _caps;

	/**
	 * Constructor
	 */
	FontInfo();


	/**
	 * Draws the text input caret at the given position
	 * @remarks     The position specifies the caret's bottom-left corner,
	 *      and the X position is in multiples of GLI_SUBPIX
	 */
	void drawCaret(const Point &pos);
};

/**
 * Font info for mono (fixed size) fonts
 */
struct MonoFontInfo : public FontInfo {
};

/**
 * Font info for proportional (variable size) fonts
 */
struct PropFontInfo : public MonoFontInfo {
	int _justify;
	int _quotes;
	int _dashes;
	int _spaces;
	int _lineSeparation;

	/**
	 * Constructor
	 */
	PropFontInfo();
};

} // End of namespace Glk

#endif
