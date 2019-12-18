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

#include <string.h>

#include "SDL.h"

#include "ultima/ultima6/core/nuvie_defs.h"
#include "GUI_button.h"
#include "GUI_loadimage.h"

namespace Ultima {
namespace Ultima6 {

/* the check marks bitmap */
//#include "the_checker.h"
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
	if (x >= 0)
		area.x = x;
	if (y >= 0)
		area.y = y;
	if (w >= 0)
		area.w = w;
	if (h >= 0)
		area.h = h;

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
		/*
		**** OLD VERSION WITH TEXT CHECKMARK ****
		      if (checked)
		      {
		        buttonFont->SetTransparency(1);
		        buttonFont->SetColoring(255,0,0);
		        buttonFont->TextOut(surface,area.x+4,area.y+4,textmark);
		      }
		*/

		src.x = 8 - (checked * 8);
		src.y = 0;
		src.w = 8;
		src.h = 10;
		dest.x += 4;
		dest.y += 4;
		dest.w = 8;
		dest.h = 10;
		SDL_BlitSurface(checkmarks, &src, surface, &dest);
	}
	if (!enabled) {
		Uint8 *pointer;
		int pixel = SDL_MapRGB(surface->format, 0, 0, 0);;
		Uint8 bytepp = surface->format->BytesPerPixel;

		if (!SDL_LockSurface(surface)) {
			for (int y = 0; y < area.h; y += 2) {
				pointer = (Uint8 *)surface->pixels + surface->pitch * (area.y + y) + (area.x * bytepp);
				for (int x = 0; x<area.w >> 1; x++) {
					switch (bytepp) {
					case 1:
						*((Uint8 *)(pointer)) = (Uint8)pixel;
						pointer += 2;
						break;
					case 2:
						*((Uint16 *)(pointer)) = (Uint16)pixel;
						pointer += 4;
						break;
					case 3:  /* Format/endian independent */
						Uint8 r, g, b;

						r = (pixel >> surface->format->Rshift) & 0xFF;
						g = (pixel >> surface->format->Gshift) & 0xFF;
						b = (pixel >> surface->format->Bshift) & 0xFF;
						*((pointer) + surface->format->Rshift / 8) = r;
						*((pointer) + surface->format->Gshift / 8) = g;
						*((pointer) + surface->format->Bshift / 8) = b;
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
GUI_status GUI_Button:: MouseDown(int x, int y, int button) {
//	if(button == SDL_BUTTON_WHEELUP || button == SDL_BUTTON_WHEELDOWN)
//	  return GUI_PASS;
	if (enabled && (button == 1 || button == 3)) {
		pressed[0] = 1;
		Redraw();
	}
	return GUI_YUM;
}

GUI_status GUI_Button::MouseUp(int x, int y, int button) {
//	if (button==SDL_BUTTON_WHEELUP || button==SDL_BUTTON_WHEELDOWN)
//		return GUI_PASS;
	if ((button == 1 || button == 3) && (pressed[0])) {
		pressed[0] = 0;
		return Activate_button(x, y, button);
	}
	return GUI_YUM;
}

GUI_status GUI_Button::Activate_button(int x, int y, int button) {
	if (x >= 0 && y >= 0) {
		if (callback_object && callback_object->callback(BUTTON_CB, this, widget_data) == GUI_QUIT)
			return GUI_QUIT;
	}
	Redraw();
	return GUI_YUM;
}

GUI_status GUI_Button::MouseMotion(int x, int y, Uint8 state) {
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

//  Graphics::ManagedSurface *img=SDL_AllocSurface(SDL_SWSURFACE,area.w,area.h,
//				    16,31 << 11,63 << 5,31,0);
	Graphics::ManagedSurface *img = SDL_CreateRGBSurface(SDL_SWSURFACE, area.w, area.h,
	                                        16, 31 << 11, 63 << 5, 31, 0);

	if (img == NULL) return NULL;

	uint32 color1 = SDL_MapRGB(img->format, BL_R, BL_G, BL_B);
	uint32 color2 = SDL_MapRGB(img->format, BS_R, BS_G, BS_B);
	uint32 color3 = SDL_MapRGB(img->format, BF_R, BF_G, BF_B);
	uint32 color4 = SDL_MapRGB(img->format, BI2_R, BI2_G, BI2_B);


	buttonFont->SetColoring(0, 0, 0);
	buttonFont->SetTransparency(1);
	buttonFont->TextExtent(text, &tw, &th);
	if (tw > (area.w - (4 + is_checkable * 16))) {
		int n = (area.w - (4 + is_checkable * 16)) / buttonFont->CharWidth();
		duptext = new char[n + 1];
		strncpy(duptext, text, n);
		duptext[n] = 0;
		text = duptext;
		buttonFont->TextExtent(text, &tw, &th);
	}
	if (th > (area.h - 4)) {
		text = "";
	}
	switch (alignment) {
	case BUTTON_TEXTALIGN_LEFT:
		tx = 4 + (is_checkable * 16);
		break;
	case BUTTON_TEXTALIGN_CENTER:
		tx = (area.w - tw) >> 1;
		break;
	case BUTTON_TEXTALIGN_RIGHT:
		tx = area.w - 5 - tw;
		break;
	}
	ty = (area.h - th) >> 1;

	switch (style) {
	case BUTTON3D_UP:
		fillrect.x = 0;
		fillrect.y = 0;
		fillrect.w = area.w;
		fillrect.h = 2;
		SDL_FillRect(img, &fillrect, color1);
		fillrect.y = area.h - 2;
		SDL_FillRect(img, &fillrect, color2);
		fillrect.x = 0;
		fillrect.y = 0;
		fillrect.w = 2;
		fillrect.h = area.h;
		SDL_FillRect(img, &fillrect, color1);
		fillrect.x = area.w - 2;
		SDL_FillRect(img, &fillrect, color2);
		fillrect.h = 1;
		fillrect.w = 1;
		SDL_FillRect(img, &fillrect, color1);
		fillrect.x = 1;
		fillrect.y = area.h - 1;
		SDL_FillRect(img, &fillrect, color2);
		fillrect.x = 2;
		fillrect.y = 2;
		fillrect.w = area.w - 4;
		fillrect.h = area.h - 4;
		SDL_FillRect(img, &fillrect, color3);
		buttonFont->TextOut(img, tx, ty, text);
		break;
	case BUTTON3D_DOWN:
		fillrect.x = 0;
		fillrect.y = 0;
		fillrect.w = area.w;
		fillrect.h = area.h;
		SDL_FillRect(img, &fillrect, color3);
		buttonFont->TextOut(img, tx + 1, ty + 1, text);
		break;
	case BUTTON2D_UP:
		fillrect.x = 0;
		fillrect.y = 0;
		fillrect.w = area.w;
		fillrect.h = area.h;
		SDL_FillRect(img, &fillrect, color3);
		buttonFont->TextOut(img, tx, ty, text);
		break;
	case BUTTON2D_DOWN:
		fillrect.x = 0;
		fillrect.y = 0;
		fillrect.w = area.w;
		fillrect.h = area.h;
		SDL_FillRect(img, &fillrect, color4);
		buttonFont->SetTransparency(0);
		buttonFont->SetColoring(BI1_R, BI1_G, BI1_B, BI2_R, BI2_G, BI2_B);
		buttonFont->TextOut(img, tx, ty, text);
		break;
	}

	delete[] duptext;

	return img;
}

} // End of namespace Ultima6
} // End of namespace Ultima
