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
#include "ultima/nuvie/gui/gui_text_toggle_button.h"

namespace Ultima {
namespace Nuvie {

GUI_TextToggleButton::GUI_TextToggleButton(void *data, int x, int y, int w, int h,
        const char *const *texts_, int count_, int selection_,
        GUI_Font *font, int alignment_,
        GUI_CallBack *callback, int flat)
	: GUI_Button(data, x, y, w, h, "", font, alignment_, 0, callback, flat) {
	count = count_;
	assert(count > 0);

	selection = selection_;
	assert(selection >= 0 && selection < count);

	alignment = alignment_;

	texts = new char *[count];
	for (int i = 0; i < count; ++i) {
		int l = strlen(texts_[i]);
		texts[i] = new char[l + 1];
		strcpy(texts[i], texts_[i]);
	}

	ChangeTextButton(-1, -1, -1, -1, texts[selection], alignment);
}

GUI_TextToggleButton::~GUI_TextToggleButton() {
	for (int i = 0; i < count; ++i)
		delete[] texts[i];
	delete[] texts;
	texts = 0;
}

GUI_status GUI_TextToggleButton::MouseUp(int x, int y, Shared::MouseButton button_) {
	if ((button_ == 1 || button_ == 3) && (pressed[0])) {
		pressed[0] = 0;
		return Activate_button(x, y, button_);
	}
	return GUI_Button::MouseUp(x, y, button_);
}

GUI_status GUI_TextToggleButton::Activate_button(int x, int y, Shared::MouseButton button_) {
	selection = (selection + (button_ == Shared::BUTTON_LEFT ? 1 : -1)) % count;
	if (selection < 0)
		selection = count - 1;

	if (x >= 0 && y >= 0) {
		if (callback_object && callback_object->callback(BUTTON_CB, this, widget_data) == GUI_QUIT)
			return GUI_QUIT;
	}

	ChangeTextButton(-1, -1, -1, -1, texts[selection], alignment);
	Redraw();

	return GUI_YUM;
}

int GUI_TextToggleButton::GetSelection() const {
	return selection;
}

} // End of namespace Nuvie
} // End of namespace Ultima
