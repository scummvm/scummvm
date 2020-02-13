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

/* This is a very generic button widget - NOT ANY LONGER*/

#ifndef NUVIE_GUI_GUI_BUTTON_H
#define NUVIE_GUI_GUI_BUTTON_H

#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/gui/gui_font.h"

namespace Ultima {
namespace Nuvie {

class GUI_CallBack;

/* design constants */
#define BUTTON3D_UP 1
#define BUTTON3D_DOWN 2
#define BUTTON2D_UP 3
#define BUTTON2D_DOWN 4

/* alignment constants */
#define BUTTON_TEXTALIGN_LEFT 1
#define BUTTON_TEXTALIGN_CENTER 2
#define BUTTON_TEXTALIGN_RIGHT 3

// Callback message types

#define BUTTON_CB 0x1

/* color constants */

// Button face color
const uint8 BF_R = 183, BF_G = 185, BF_B = 150;
// Button light color
const uint8 BL_R = 245, BL_G = 247, BL_B = 201;
// Button shadow color
const uint8 BS_R = 115, BS_G = 116, BS_B = 94;
// 2D Button inverse text color
const uint8 BI1_R = 255, BI1_G = 255, BI1_B = 255;
// 2D Button inverse background color
const uint8 BI2_R = 0, BI2_G = 0, BI2_B = 0;

#define GUI_BUTTON_DONT_FREE_SURFACES false

/* This is the definition of the "I've been activated" callback */
typedef GUI_status(*GUI_ActiveProc)(void *data);

class GUI_Button : public GUI_Widget {

public:
	/* Passed the button data, position, images (pressed/unpressed) and callback */
	GUI_Button(void *data, int x, int y, Graphics::ManagedSurface *image,
	           Graphics::ManagedSurface *image2, GUI_CallBack *callback, bool free_surfaces = true);

	/* I don't know what this one is for */
	GUI_Button(void *data, int x, int y, int w, int h,
	           GUI_CallBack *callback);

	/* Passed the button data, position, width, height, a caption, a font,
	   an alignment (one of the constants above), if it should be a checkbutton (1/0),
	   the callback and a flag if it should be 2D (1) or 3D (0) */
	GUI_Button(void *data, int x, int y, int w, int h, const char *text,
	           GUI_Font *font, int alignment, int is_checkbutton,
	           GUI_CallBack *callback, int flat = 0);

	~GUI_Button() override;

	/* change features of a text button (if one of the dimensions is negativ, it's ignored) */
	virtual void ChangeTextButton(int x, int y, int w, int h, const char *text, int alignment);

	/* Show the widget  */
	void Display(bool full_redraw) override;

	/* Mouse hits activate us */
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseMotion(int x, int y, uint8 state) override;

	/* Clickable or not ... */
	virtual void Disable();
	virtual void Enable(int flag = 1);

	/* yields current state */
	virtual int Enabled() {
		return enabled;
	}

	/* yields flag if button is a checkbutton */
	virtual int IsCheckButton() {
		return is_checkable;
	}
	virtual void set_highlighted(bool val) {
		is_highlighted = val;
	}
	virtual GUI_status Activate_button(int x = 0, int y = 0, Shared::MouseButton button = Shared::BUTTON_LEFT);

protected:
	/* yields an appropriate image */
	virtual Graphics::ManagedSurface *CreateTextButtonImage(int style, const char *text, int alignment);

	/* The button font */
	GUI_Font *buttonFont;

	/* The button images */
	Graphics::ManagedSurface *button, *button2;

	/* The activation callback */
	GUI_CallBack *callback_object;

	/* remember me! - flags */
	int enabled;
	int flatbutton;
	int freebutton, freefont;

	/* Checkbutton flags */
	int is_checkable;
	int checked;
	bool is_highlighted;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
