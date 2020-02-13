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

#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/utils.h"
#include "ultima/nuvie/gui/gui_font.h"
#include "ultima/nuvie/gui/gui_load_image.h"
#include "common/textconsole.h"

namespace Ultima {
namespace Nuvie {

/* use default 8x8 font */
GUI_Font::GUI_Font(uint8 fontType) {
	Graphics::ManagedSurface *temp;

	_wData = NULL;

	if (fontType == GUI_FONT_6X8)
		temp = GUI_Font6x8();
	else if (fontType == GUI_FONT_GUMP) {
		temp = GUI_FontGump();
		_wData = GUI_FontGumpWData();
	} else
		temp = GUI_DefaultFont();


	_fontStore = SDL_ConvertSurface(temp, temp->format, SDL_SWSURFACE);
	_charH = _fontStore->h / 16;
	_charW = _fontStore->w / 16;
	_disposeFont = DisposeAfterUse::YES;
	setTransparency(true);
}

/* open named BMP file */
GUI_Font::GUI_Font(char *name) {
	_fontStore = SDL_LoadBMP(name);
	if (_fontStore != NULL) {
		_charH = _fontStore->h / 16;
		_charW = _fontStore->w / 16;
		_disposeFont = DisposeAfterUse::YES;
	} else {
		::error("Could not load font");
	}

	setTransparency(true);
	_wData = NULL;
}

/* use given YxY surface */
GUI_Font::GUI_Font(Graphics::ManagedSurface *bitmap) {
	if (bitmap == NULL)
		_fontStore = GUI_DefaultFont();
	else
		_fontStore = bitmap;
	_charH = _fontStore->h / 16;
	_charW = _fontStore->w / 16;
	_disposeFont = DisposeAfterUse::NO;
	setTransparency(true);
	_wData = NULL;
}

/* copy constructor */
GUI_Font::GUI_Font(GUI_Font &font) {
	Graphics::ManagedSurface *temp = font._fontStore;
	_fontStore = SDL_ConvertSurface(temp, temp->format, SDL_SWSURFACE);
	_charH = _fontStore->h / 16;
	_charW = _fontStore->w / 16;
	_disposeFont = DisposeAfterUse::YES;
	setTransparency(true);
	_wData = NULL;
}

GUI_Font::~GUI_Font() {
	if (_disposeFont == DisposeAfterUse::YES)
		delete _fontStore;
}

/* determine drawing style */
void GUI_Font::setTransparency(bool on) {
	_transparent = on;
	
	if (_transparent)
		_fontStore->setTransparentColor(0);
	else
		_fontStore->clearTransparentColor();
}

/* determine foreground and background color values RGB*/
void GUI_Font::setColoring(uint8 fr, uint8 fg, uint8 fb, uint8 br, uint8 bg, uint8 bb) {
	const SDL_Color colors[2] = { MAKE_COLOR(br, bg, bb), MAKE_COLOR(fr, fg, fb) };	
	SDL_SetColors(_fontStore, colors, 0, 2);
}

void GUI_Font::setColoring(uint8 fr, uint8 fg, uint8 fb, uint8 fr1, uint8 fg1, uint8 fb1, uint8 br, uint8 bg, uint8 bb) {
	const SDL_Color colors[3] = {
		MAKE_COLOR(br, bg, bb), MAKE_COLOR(fr, fg, fb), MAKE_COLOR(fr1, fg1, fb1) };
	SDL_SetColors(_fontStore, colors, 0, 3);
}

/* put the text onto the given surface using the preset mode and colors */
void GUI_Font::textOut(Graphics::ManagedSurface *context, int x, int y, const char *text, int line_wrap) {
	int i;
	int j;
	uint8 ch;
	Common::Rect src(_charW, _charH - 1);
	Common::Rect dst(_charW, _charH - 1);

	i = 0;
	j = 0;
	while ((ch = text[i])) { // single "=" is correct!
		if (line_wrap && j == line_wrap) {
			j = 0;
			y += _charH;
		}

		src.moveTo((ch % 16) * _charW, (ch / 16) * _charH);
		if (_wData) {
			dst.left = x;
			dst.right = x + _wData[ch];
			x += dst.width();

		} else {
			dst.moveTo(x + (j * _charW), dst.top);
		}

		dst.moveTo(dst.left, y);
		SDL_BlitSurface(_fontStore, &src, context, &dst);
		i++;
		j++;
	}
}

void GUI_Font:: textExtent(const char *text, int *w, int *h, int line_wrap) {
	int len = strlen(text);
	if (_wData) { //variable width font.
		//FIXME we're not handling line_wrap properly for variable width fonts!
		*w = 0;
		for (int i = 0; i < len; i++) {
			*w += _wData[(byte)text[i]];
		}
	} else {
		if (line_wrap && len > line_wrap)
			*w = line_wrap * _charW;
		else
			*w = len * _charW;
	}

	if (line_wrap && len > line_wrap) {
		*h = (int)ceil((float)len / (float)line_wrap);
		*h *= _charH - 1;
	} else
		*h = _charH - 1;
	return;
}

uint16 GUI_Font::getCenter(const char *text, uint16 width) {
	int w, h;
	textExtent(text, &w, &h);
	if (w < width)
		return ((width - w) / 2);
	else
		return 0;
}

} // End of namespace Nuvie
} // End of namespace Ultima
