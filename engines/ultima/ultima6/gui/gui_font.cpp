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

#include <stdlib.h>
#include <cmath>
#include <misc/ultima/ultima6/misc/u6_misc.h>
#include <misc/SDLUtils.h>

#include "GUI_font.h"
#include "GUI_loadimage.h"

namespace Ultima {
namespace Ultima6 {

/* use default 8x8 font */
GUI_Font::GUI_Font(Uint8 fontType) {
	Graphics::ManagedSurface *temp;

	w_data = NULL;

	if (fontType == GUI_FONT_6X8)
		temp = GUI_Font6x8();
	else if (fontType == GUI_FONT_GUMP) {
		temp = GUI_FontGump();
		w_data = GUI_FontGumpWData();
	} else
		temp = GUI_DefaultFont();


	fontStore = SDL_ConvertSurface(temp, temp->format, SDL_SWSURFACE);
	charh = fontStore->h / 16;
	charw = fontStore->w / 16;
	freefont = 1;
	SetTransparency(1);
}

/* open named BMP file */
GUI_Font::GUI_Font(char *name) {
	fontStore = SDL_LoadBMP(name);
	if (fontStore != NULL) {
		charh = fontStore->h / 16;
		charw = fontStore->w / 16;
		freefont = 1;
	} else {
		freefont = 0;
		DEBUG(0, LEVEL_EMERGENCY, "Could not load font.\n");
		exit(1);
	}
	SetTransparency(1);
	w_data = NULL;
}

/* use given YxY surface */
GUI_Font::GUI_Font(Graphics::ManagedSurface *bitmap) {
	if (bitmap == NULL)
		fontStore = GUI_DefaultFont();
	else
		fontStore = bitmap;
	charh = fontStore->h / 16;
	charw = fontStore->w / 16;
	freefont = 0;
	SetTransparency(1);
	w_data = NULL;
}

/* copy constructor */
GUI_Font::GUI_Font(GUI_Font &font) {
	Graphics::ManagedSurface *temp = font.fontStore;
	fontStore = SDL_ConvertSurface(temp, temp->format, SDL_SWSURFACE);
	charh = fontStore->h / 16;
	charw = fontStore->w / 16;
	freefont = 1;
	SetTransparency(1);
	w_data = NULL;
}

GUI_Font::~GUI_Font() {
	if (freefont)
		SDL_FreeSurface(fontStore);
}

/* determine drawing style */
void GUI_Font::SetTransparency(int on) {
	if ((transparent = on))  // single "=" is correct
		SDL_SetColorKey(fontStore, SDL_TRUE, 0);
	else
		SDL_SetColorKey(fontStore, 0, 0);
}

/* determine foreground and background color values RGB*/
void GUI_Font::SetColoring(Uint8 fr, Uint8 fg, Uint8 fb, Uint8 br, Uint8 bg, Uint8 bb) {
	SDL_Color colors[3] = {{br, bg, bb, 0}, {fr, fg, fb, 0}};
	SDL_SetColors(fontStore, colors, 0, 2);
}

void GUI_Font::SetColoring(Uint8 fr, Uint8 fg, Uint8 fb, Uint8 fr1, Uint8 fg1, Uint8 fb1, Uint8 br, Uint8 bg, Uint8 bb) {
	SDL_Color colors[4] = {{br, bg, bb, 0}, {fr, fg, fb, 0}, {fr1, fg1, fb1, 0}};
	SDL_SetColors(fontStore, colors, 0, 3);
}

/* put the text onto the given surface using the preset mode and colors */
void GUI_Font::TextOut(Graphics::ManagedSurface *context, int x, int y, const char *text, int line_wrap) {
	int i;
	int j;
	Uint8 ch;
	Common::Rect src;
	Common::Rect dst;

	src.w = charw;
	src.h = charh - 1;
	dst.w = charw;
	dst.h = charh - 1;
	i = 0;
	j = 0;
	while ((ch = text[i])) { // single "=" is correct!
		if (line_wrap && j == line_wrap) {
			j = 0;
			y += charh;
		}

		src.x = (ch % 16) * charw;
		src.y = (ch / 16) * charh;
		if (w_data) {
			dst.x = x;
			dst.w = w_data[ch];
			x += dst.w;
		} else
			dst.x = x + (j * charw);
		dst.y = y;
		SDL_BlitSurface(fontStore, &src, context, &dst);
		i++;
		j++;
	}
}

void GUI_Font:: TextExtent(const char *text, int *w, int *h, int line_wrap) {
	int len = strlen(text);
	if (w_data) { //variable width font.
		//FIXME we're not handling line_wrap properly for variable width fonts!
		*w = 0;
		for (int i = 0; i < len; i++) {
			*w += w_data[text[i]];
		}
	} else {
		if (line_wrap && len > line_wrap)
			*w = line_wrap * charw;
		else
			*w = len * charw;
	}

	if (line_wrap && len > line_wrap) {
		*h = (int)ceil((float)len / (float)line_wrap);
		*h *= charh - 1;
	} else
		*h = charh - 1;
	return;
}

uint16 GUI_Font::get_center(const char *text, uint16 width) {
	int w, h;
	TextExtent(text, &w, &h);
	if (w < width)
		return ((width - w) / 2);
	else
		return 0;
}

} // End of namespace Ultima6
} // End of namespace Ultima
