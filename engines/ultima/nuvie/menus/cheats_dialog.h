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

#ifndef NUVIE_MENUS_CHEATS_DIALOG_H
#define NUVIE_MENUS_CHEATS_DIALOG_H

#include "ultima/nuvie/gui/gui_dialog.h"

namespace Ultima {
namespace Nuvie {

class GUI;
class GUI_CallBack;
class GUI_Button;
class GUI_TextToggleButton;

class CheatsDialog : public GUI_Dialog {
protected:
	uint8 last_index;
	sint8 b_index_num;
	GUI_CallBack *callback_object;
	GUI_Button *save_button, *cancel_button;
	GUI_TextToggleButton *brightness_button, *cheat_button, *egg_button, *hackmove_button, *party_button;
	GUI_Button *button_index[7]; // add to here when you add a button. Keep buttons in order by height

public:
	CheatsDialog(GUI_CallBack *callback);
	~CheatsDialog() override;
	bool init();

	GUI_status close_dialog();
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
