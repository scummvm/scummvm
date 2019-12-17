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

#ifndef ULTIMA6_SAVE_SAVE_DIALOG_H
#define ULTIMA6_SAVE_SAVE_DIALOG_H

#include "GUI_Dialog.h"
#include "TileManager.h"

namespace Ultima {
namespace Ultima6 {

class GUI;
class GUI_CallBack;
class GUI_Button;
class GUI_Scroller;
class SaveSlot;

// Callback message types

#define SAVEDIALOG_CB_SAVE    0x1
#define SAVEDIALOG_CB_LOAD    0x2
#define SAVEDIALOG_CB_CANCEL  0x3
#define SAVEDIALOG_CB_DELETE  0x4

class SaveDialog : public GUI_Dialog {
protected:

	GUI_Scroller *scroller;

	GUI_CallBack *callback_object;
	GUI_Button *save_button, *load_button, *cancel_button;

	SaveSlot *selected_slot;
	uint8 index, save_index, cursor_loc;
	uint16 cursor_x, cursor_y;
	const Tile *cursor_tile;
	bool show_cursor;
	void set_cursor_pos(uint8 index_num);

public:

	void Display(bool full_redraw);
	SaveDialog(GUI_CallBack *callback);

	~SaveDialog();

	bool init(const char *save_directory, const char *search_prefix);

	GUI_status close_dialog();
	GUI_status KeyDown(SDL_Keysym key);
	GUI_status MouseDown(int x, int y, int button);
	GUI_status MouseWheel(sint32 x, sint32 y);
	GUI_Scroller *get_scroller() {
		return scroller;
	}

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data);
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
