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

#ifndef NUVIE_MENUS_GAME_MENU_DIALOG_H
#define NUVIE_MENUS_GAME_MENU_DIALOG_H

#include "ultima/nuvie/gui/gui_dialog.h"
#include "ultima/nuvie/core/nuvie_defs.h"

namespace Ultima {
namespace Nuvie {

#define GAMEMENUDIALOG_CB_DELETE 3

class GUI;
class GUI_CallBack;
class GUI_Button;

class GameMenuDialog : public GUI_Dialog {
protected:
	uint8 last_index;
	sint8 b_index_num;
	CallBack *callback_object;
	GUI_Button *load_button, *save_button, *video_button, *audio_button, *input_button,
		*gameplay_button, *cheats_button, *continue_button, *quit_button;
#ifdef HAVE_JOYSTICK_SUPPORT
	GUI_Button *joystick_button;
	GUI_Button *button_index[10]; // needs to be one bigger
#else
	GUI_Button *button_index[9]; // add to here when you add a button. Keep buttons in order by height
#endif
public:
	GameMenuDialog(CallBack *callback);
	~GameMenuDialog();
	bool init();

	GUI_status close_dialog();
	GUI_status KeyDown(const Common::KeyState &key);
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
