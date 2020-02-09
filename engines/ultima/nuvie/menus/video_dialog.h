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

#ifndef NUVIE_MENUS_VIDEO_DIALOG_H
#define NUVIE_MENUS_VIDEO_DIALOG_H

#include "ultima/nuvie/gui/gui_dialog.h"

namespace Ultima {
namespace Nuvie {

class GUI;
class GUI_CallBack;
class GUI_Button;
class GUI_TextToggleButton;

class VideoDialog : public GUI_Dialog {
protected:
	uint8 last_index;
	sint8 b_index_num;
	bool old_use_new_dolls, old_use_tile_lighting;
	GUI_CallBack *callback_object;
	GUI_Button *save_button, *cancel_button, *only2x_button;
	GUI_TextToggleButton *scale_button, *scale_win_button, *scaler_button, *scaler_win_button,
	                     *fullscreen_button, *non_square_pixels_button, *roof_button, *lighting_button, *dither_button,
	                     *game_style_button, *doll_button, *tile_lighting_b, *sprites_b;
	GUI_Button *button_index[11]; // add to here when you add a button. Keep buttons in order by height
	void rebuild_buttons(bool init);
public:
	VideoDialog(GUI_CallBack *callback);
	~VideoDialog() override;
	bool init();

	GUI_status close_dialog();
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
