/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef NUVIE_CORE_COMMAND_BAR_H
#define NUVIE_CORE_COMMAND_BAR_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/misc/call_back.h"

namespace Ultima {
namespace Nuvie {

class NuvieIO;
class Events;
class Game;
class GUI_Button;
class GUI_CallBack;
class Font;
class U6Shape;

#define COMMANDBAR_USE_BUTTON 1
#define COMMANDBAR_ACTION_BUTTON 3

/* U6 command buttons.
 * [Attack][Cast][Talk][Look][Get][Drop][Move][Use][Rest][Combat]
 * Left click: New action if none is pending.
 * Right click: Set default action.
 * Information: [A]M-DD-YYYY Wind: W[B]
 */
class CommandBar: public GUI_Widget, public CallBack {
protected:
	Game *game;
	Events *event;
	Font *font;
	const Tile *icon[13];
	U6Shape *background; // used to display the WoU command bar backgrounds

	U6Shape *lever_up;	  // The lever in the up state (MD only)
	U6Shape *lever_down;  // The lever in the down state (MD only)

	sint8 selected_action; // underlined icon (-1 = none)
	sint8 active_action_num; // the last action that was activated (for MD levers)
	bool combat_mode; // state of combat icon
	Std::string wind; // wind direction
	void fill_square(uint8 pal_index);

	uint8 bg_color, font_color;
	uint16 offset;

	virtual void display_information();
	virtual GUI_status hit(uint8 num);

public:
	CommandBar();
	CommandBar(Game *g);
	~CommandBar() override;
	virtual bool init_buttons();

	void Display(bool full_redraw) override;
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	void update() {
		update_display = true;
	}

	void select_action(sint8 activate);
	void set_combat_mode(bool mode);
//    void set_wind(Std::string dir)        { wind = dir; update_display = true; }
	void set_selected_action(sint8 action) {
		selected_action = action;
		update_display = true;
	}
	bool try_selected_action(sint8 command_num = -1);
	sint8 get_selected_action() const {
		return selected_action;
	}

	// Called when a mode is being changed from here
	// *or* keyboard - so MD can update levers
	void on_new_action(EventMode action);

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override {
		return GUI_PASS;
	}
	uint16 callback(uint16 msg, CallBack *caller, void *data) override;
	bool load(NuvieIO *objlist);
	bool save(NuvieIO *objlist);
	bool drag_accept_drop(int x, int y, int message, void *data) override; // needed for !orig_style
	void drag_perform_drop(int x, int y, int message, void *data) override; // needed for !orig_style
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
