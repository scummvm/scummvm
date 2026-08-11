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

#ifndef NUVIE_GUI_GUI_FONT_H
#define NUVIE_GUI_GUI_FONT_H

#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

enum GuiFont {
	GUI_FONT_DEFAULT = 0,
	GUI_FONT_6X8     = 1,
	GUI_FONT_GUMP    = 2
};

class GUI_Font {
protected:
	/* the font source surface */
	Graphics::ManagedSurface *_fontStore;

	/* flags */
	bool _transparent;
	DisposeAfterUse::Flag _disposeFont;

	/* dimensions */
	int _charH, _charW;

	const byte *_wData;
public:
	/* use default 8x8 font */
	GUI_Font(uint8 fontType = GUI_FONT_DEFAULT);

	/* open named BMP file */
	GUI_Font(const char *name);

	/* use given YxY surface */
	GUI_Font(Graphics::ManagedSurface *bitmap);

	GUI_Font(const GUI_Font &font) = delete;

	virtual ~GUI_Font();

	/* determine drawing style */
	virtual void setTransparency(bool on);

	/* determine foreground and background color values RGB*/
	virtual void setColoring(uint8 fr, uint8 fg, uint8 fb, uint8 br = 255, uint8 bg = 255, uint8 bb = 255);
	/* Two color font */
	virtual void setColoring(uint8 fr, uint8 fg, uint8 fb, uint8 fr1, uint8 fg1, uint8 fb1, uint8 br, uint8 bg, uint8 bb);

	/* yields the pixel height of a single character */
	inline virtual int charHeight() const {
		return _charH - 1;
	}

	/* yields the pixel width of a single character */
	inline virtual int charWidth() const {
		return _charW;
	}

	/* put the text onto the given surface using the preset mode and colors */
	virtual void textOut(Graphics::ManagedSurface *context, int x, int y, const char *text, int line_wrap = 0);

	/* yields pixel width and height of a string when printed with this font */
	void textExtent(const char *text, int *w, int *h, int line_wrap = 0);

	/* yields the center (of width) where the text should be placed or 0 if larger than width */
	uint16 getCenter(const char *text, uint16 width);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
