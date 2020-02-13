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

#ifndef NUVIE_GUI_TEXT_TOGGLE_BUTTON_H
#define NUVIE_GUI_TEXT_TOGGLE_BUTTON_H

#include "ultima/nuvie/gui/gui_button.h"

namespace Ultima {
namespace Nuvie {

// A button that cycles through a set of captions when clicked

class GUI_TextToggleButton : public GUI_Button {
public:
	/* Passed the button data, position, width, height,
	   captions, number of captions, initial selection,
	   a font, an alignment (see GUI_Button),
	   the callback and a flag if it should be 2D (1) or 3D (0).

	   The captions are copied into the class. */
	GUI_TextToggleButton(void *data, int x, int y, int w, int h,
	                     const char *const *texts, int count, int selection,
	                     GUI_Font *font, int alignment,
	                     GUI_CallBack *callback, int flat = 0);

	~GUI_TextToggleButton() override;

	virtual int GetSelection() const;

	GUI_status MouseUp(int x, int y, Shared::MouseButton button) override;
	GUI_status Activate_button(int x = 0, int y = 0, Shared::MouseButton button = Shared::BUTTON_LEFT) override;

protected:
	int selection;
	char **texts;
	int count;
	int alignment;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
