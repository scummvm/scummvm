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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/gui/gui_load_image.h"

namespace Ultima {
namespace Nuvie {

/* the check marks bitmap */
Graphics::ManagedSurface *checkmarks = NULL;


GUI_Button:: GUI_Button(void *data, int x, int y, Graphics::ManagedSurface *image,
                        Graphics::ManagedSurface *image2, GUI_CallBack *callback, bool free_surfaces)
	: GUI_Widget(data, x, y, image->w, image->h) {
	callback_object = callback;

	button = image;
	button2 = image2;
	freebutton = free_surfaces;
	for (int i = 0; i < 3; ++i) {
		pressed[i] = 0;
	}
	enabled = 1;
	buttonFont = NULL;
	freefont = 0;
	flatbutton = 0;
	is_checkable = 0;
	checked = 0;
	is_highlighted = false;
}

GUI_Button:: GUI_Button(void *data, int x, int y, int w, int h,
                        GUI_CallBack *callback)
	: GUI_Widget(data, x, y, w, h) {
	callback_object = callback;

	button = NULL;
	button2 = NULL;
	freebutton = 0;
	for (int i = 0; i < 3; ++i) {
		pressed[i] = 0;
	}
	enabled = 1;
	buttonFont = NULL;
	freefont = 0;
	flatbutton = 0;
	is_checkable = 0;
	checked = 0;
	is_highlighted = false;
}

GUI_Button::GUI_Button(void *data, int x, int y, int w, int h, const char *text,
                       GUI_Font *font, int alignment, int is_checkbutton,
                       GUI_CallBack *callback, int flat)
	: GUI_Widget(data, x, y, w, h) {
	callback_object = callback;

	if (font != NULL) {
		buttonFont = font;
		freefont = 0;
	} else {
		buttonFont = new GUI_Font();
		freefont = 1;
	}
	flatbutton = flat;
	freebutton = 1;
	button = NULL;
	button2 = NULL;

	is_checkable = is_checkbutton;
	checked = 0;
	is_highlighted = false;
	/*
	  if (is_checkable &&(checkmarks==NULL))
	  {
	    checkmarks=GUI_LoadImage(checker_w,checker_h,checker_pal,checker_data);
	    SDL_SetColorKey(checkmarks,SDL_SRCCOLORKEY,0);
	  }
	*/
	ChangeTextButton(-1, -1, -1, -1, text, alignment);

	for (int i = 0; i < 3; ++i) {
		pressed[i] = 0;
	}
	enabled = 1;
}

GUI_Button::~GUI_Button() {
	if (freebutton) {
		if (button)
			SDL_FreeSurface(button);
		if (button2)
			SDL_FreeSurface(button2);
	}
	if (freefont)
		delete buttonFont;
}

/* Resize/reposition/change text */
void GUI_Button::ChangeTextButton(int x, int y, int w, int h, const char *text, int alignment) {
	if (x != -1 || y != -1) {
		assert(x >= 0 && y >= 0);
		area.moveTo(x, y);
	}

	if (w != -1 || h != -1) {
		assert(w >= 0 && h >= 0);
		area.setWidth(w);
		area.setHeight(h);
	}

	if (freebutton) {
		if (button)
			SDL_FreeSurface(button);
		if (button2)
			SDL_FreeSurface(button2);
		if (flatbutton) {
			button = CreateTextButtonImage(BUTTON2D_UP, text, alignment);
			button2 = CreateTextButtonImage(BUTTON2D_DOWN, text, alignment);
		} else {
			button = CreateTextButtonImage(BUTTON3D_UP, text, alignment);
			button2 = CreateTextButtonImage(BUTTON3D_DOWN, text, alignment);
		}
	}
}

/* Show the widget  */
void GUI_Button:: Display(bool full_redraw) {
	Common::Rect src, dest = area;

	if (button) {
		if ((button2 != NULL) && ((pressed[0]) == 1 || is_highlighted))
			SDL_BlitSurface(button2, NULL, surface, &dest);
		else
			SDL_BlitSurface(button, NULL, surface, &dest);
	}
	if (is_checkable) {
		src.left = 8 - (checked * 8);
		src.top = 0;
		src.setWidth(8);
		src.setHeight(10);
		dest.left += 4;
		dest.top += 4;
		dest.setWidth(8);
		dest.setHeight(10);
		SDL_BlitSurface(checkmarks, &src, surface, &dest);
	}
	if (!enabled) {
		uint8 *pointer;
		int pixel = SDL_MapRGB(surface->format, 0, 0, 0);
		uint8 bytepp = surface->format.bytesPerPixel;

		if (!SDL_LockSurface(surface)) {
			for (int y = 0; y < area.height(); y += 2) {
				pointer = (uint8 *)surface->getPixels() + surface->pitch * (area.top + y) + (area.left * bytepp);
				for (int x = 0; x<area.width() >> 1; x++) {
					switch (bytepp) {
					case 1:
						*((uint8 *)(pointer)) = (uint8)pixel;
						pointer += 2;
						break;
					case 2:
						*((uint16 *)(pointer)) = (uint16)pixel;
						pointer += 4;
						break;
					case 3:  /* Format/endian independent */
						uint8 r, g, b;

						r = (pixel >> surface->format.rShift) & 0xFF;
						g = (pixel >> surface->format.gShift) & 0xFF;
						b = (pixel >> surface->format.bShift) & 0xFF;
						*((pointer) + surface->format.rShift / 8) = r;
						*((pointer) + surface->format.gShift / 8) = g;
						*((pointer) + surface->format.bShift / 8) = b;
						pointer += 6;
						break;
					case 4:
						*((uint32 *)(pointer)) = (uint32)pixel;
						pointer += 8;
						break;
					}
				}
			}
			SDL_UnlockSurface(surface);
		}
	}

	DisplayChildren();
}

/* Mouse hits activate us */
GUI_status GUI_Button:: MouseDown(int x, int y, Shared::MouseButton btn) {
//	if(btn == SDL_BUTTON_WHEELUP || btn == SDL_BUTTON_WHEELDOWN)
//	  return GUI_PASS;
	if (enabled && (btn == Shared::BUTTON_LEFT || btn == Shared::BUTTON_RIGHT)) {
		pressed[0] = 1;
		Redraw();
	}
	return GUI_YUM;
}

GUI_status GUI_Button::MouseUp(int x, int y, Shared::MouseButton btn) {
//	if (btn==SDL_BUTTON_WHEELUP || btn==SDL_BUTTON_WHEELDOWN)
//		return GUI_PASS;
	if ((btn == Shared::BUTTON_LEFT || btn == Shared::BUTTON_RIGHT) && (pressed[0])) {
		pressed[0] = 0;
		return Activate_button(x, y, btn);
	}
	return GUI_YUM;
}

GUI_status GUI_Button::Activate_button(int x, int y, Shared::MouseButton btn) {
	if (x >= 0 && y >= 0) {
		if (callback_object && callback_object->callback(BUTTON_CB, this, widget_data) == GUI_QUIT)
			return GUI_QUIT;
	}
	Redraw();
	return GUI_YUM;
}

GUI_status GUI_Button::MouseMotion(int x, int y, uint8 state) {
	if ((pressed[0] == 1) && ((x < 0) || (y < 0))) {
		pressed[0] = 2;
		Redraw();
	}
	if ((pressed[0] == 2) && (x >= 0) && (y >= 0)) {
		pressed[0] = 1;
		Redraw();
	}
	return GUI_YUM;
}

void GUI_Button::Disable() {
	enabled = 0;
	Redraw();
}

void GUI_Button::Enable(int flag) {
	enabled = flag;
	Redraw();
}

Graphics::ManagedSurface *GUI_Button::CreateTextButtonImage(int style, const char *text, int alignment) {
	Common::Rect fillrect;
	int th, tw;
	int tx = 0, ty = 0;
	char *duptext = 0;

	Graphics::ManagedSurface *img = new Graphics::ManagedSurface(area.width(), area.height(),
		Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	if (img == NULL)
		return NULL;

	uint32 color1 = SDL_MapRGB(img->format, BL_R, BL_G, BL_B);
	uint32 color2 = SDL_MapRGB(img->format, BS_R, BS_G, BS_B);
	uint32 color3 = SDL_MapRGB(img->format, BF_R, BF_G, BF_B);
	uint32 color4 = SDL_MapRGB(img->format, BI2_R, BI2_G, BI2_B);


	buttonFont->setColoring(0, 0, 0);
	buttonFont->setTransparency(true);
	buttonFont->textExtent(text, &tw, &th);
	if (tw > (area.width() - (4 + is_checkable * 16))) {
		int n = (area.width() - (4 + is_checkable * 16)) / buttonFont->charWidth();
		duptext = new char[n + 1];
		strncpy(duptext, text, n);
		duptext[n] = 0;
		text = duptext;
		buttonFont->textExtent(text, &tw, &th);
	}
	if (th > (area.height() - 4)) {
		text = "";
	}
	switch (alignment) {
	case BUTTON_TEXTALIGN_LEFT:
		tx = 4 + (is_checkable * 16);
		break;
	case BUTTON_TEXTALIGN_CENTER:
		tx = (area.width() - tw) >> 1;
		break;
	case BUTTON_TEXTALIGN_RIGHT:
		tx = area.width() - 5 - tw;
		break;
	}
	ty = (area.height() - th) >> 1;

	if (!area.isEmpty()) {
		switch (style) {
		case BUTTON3D_UP:
			fillrect = Common::Rect(area.width(), 2);
			SDL_FillRect(img, &fillrect, color1);
			fillrect.moveTo(0, area.height() - 2);
			SDL_FillRect(img, &fillrect, color2);

			fillrect = Common::Rect(2, area.height());
			SDL_FillRect(img, &fillrect, color1);
			fillrect.moveTo(area.width() - 2, 0);
			SDL_FillRect(img, &fillrect, color2);

			fillrect.setHeight(1);
			fillrect.setWidth(1);
			SDL_FillRect(img, &fillrect, color1);
			fillrect.moveTo(1, area.height() - 1);
			SDL_FillRect(img, &fillrect, color2);

			fillrect = Common::Rect(2, 2, area.width() - 2, area.height() - 2);
			SDL_FillRect(img, &fillrect, color3);

			buttonFont->textOut(img, tx, ty, text);
			break;
		case BUTTON3D_DOWN:
			fillrect = Common::Rect(area.width(), area.height());
			SDL_FillRect(img, &fillrect, color3);
			buttonFont->textOut(img, tx + 1, ty + 1, text);
			break;
		case BUTTON2D_UP:
			fillrect = Common::Rect(area.width(), area.height());
			SDL_FillRect(img, &fillrect, color3);
			buttonFont->textOut(img, tx, ty, text);
			break;
		case BUTTON2D_DOWN:
			fillrect = Common::Rect(area.width(), area.height());
			SDL_FillRect(img, &fillrect, color4);
			buttonFont->setTransparency(false);
			buttonFont->setColoring(BI1_R, BI1_G, BI1_B, BI2_R, BI2_G, BI2_B);
			buttonFont->textOut(img, tx, ty, text);
			break;
		}
	}

	delete[] duptext;

	return img;
}

} // End of namespace Nuvie
} // End of namespace Ultima
