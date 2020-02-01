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
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_types.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/gui/gui_text.h"
#include "ultima/nuvie/gui/gui_text_toggle_button.h"
#include "ultima/nuvie/gui/gui_callback.h"
#include "ultima/nuvie/gui/gui_area.h"
#include "ultima/nuvie/gui/gui_dialog.h"
#include "ultima/nuvie/menus/input_dialog.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/views/party_view.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/gui/widgets/command_bar_new_ui.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

#define ID_WIDTH 280
#define ID_HEIGHT 166

InputDialog::InputDialog(GUI_CallBack *callback)
	: GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - ID_WIDTH) / 2,
	             Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - ID_HEIGHT) / 2,
	             ID_WIDTH, ID_HEIGHT, 244, 216, 131, GUI_DIALOG_UNMOVABLE) {
	callback_object = callback;
	init();
	grab_focus();
}

bool InputDialog::init() {
	uint8 textY = 11;
	uint8 buttonY = 9;
	int colX[] = { 9, 239 };
	int height = 12;
	uint8 row_h = 13;
	int yesno_width = 32;
	b_index_num = -1;
	last_index = 0;
	GUI_Widget *widget;
	GUI *gui = GUI::get_gui();
	GUI_Font *font = gui->get_font();
	Game *game = Game::get_game();
	MapWindow *map_window = game->get_map_window();

	widget = (GUI_Widget *) new GUI_Text(colX[0], textY, 0, 0, 0, "Interface:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Dragging enabled:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Direction selects target:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Look on left_click:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Walk with left button:", font);
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Enable doubleclick:", font);
	AddWidget(widget);
	if (game->get_game_type() == NUVIE_GAME_U6) {
		widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Allow free balloon movement:", font);
		AddWidget(widget);
	}
	if (!game->is_new_style()) {
		widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Doubleclick opens containers:", font);
		AddWidget(widget);
	}
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Use new command bar:", font);
	AddWidget(widget);
	if (!game->is_new_style()) {
		widget = (GUI_Widget *) new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Party view targeting:", font);
		AddWidget(widget);
	}

	Configuration *config = game->get_config();
	int interface;
	Std::string interface_str;
	config->value("config/input/interface", interface_str, "normal"); // get cfg variable because hackmove changes InterfaceType
	if (interface_str == "ignore_block")
		interface = 2;
	else if (interface_str == "fullscreen")
		interface = 1;
	else // normal
		interface = 0;

	const char *const yesno_text[] = { "no", "yes" };
	const char *const interface_text[] = { "Normal", "U7 like", "ignores obstacles" };

	interface_button = new GUI_TextToggleButton(this, 129, buttonY, 142, height, interface_text, 3, interface, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(interface_button);
	button_index[last_index] = interface_button;

	dragging_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, game->is_dragging_enabled(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(dragging_button);
	button_index[last_index += 1] = dragging_button;

	direction_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, game->get_event()->is_direction_selecting_targets(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(direction_button);
	button_index[last_index += 1] = direction_button;

	look_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, map_window->will_look_on_left_click(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(look_button);
	button_index[last_index += 1] = look_button;

	walk_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, map_window->will_walk_with_left_button(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(walk_button);
	button_index[last_index += 1] = walk_button;

	doubleclick_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, map_window->is_doubleclick_enabled(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(doubleclick_button);
	button_index[last_index += 1] = doubleclick_button;

	if (game->get_game_type() == NUVIE_GAME_U6) {
		balloon_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, game->has_free_balloon_movement(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
		AddWidget(balloon_button);
		button_index[last_index += 1] = balloon_button;
	} else
		balloon_button = NULL;
	if (!Game::get_game()->is_new_style()) {
		open_container_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, game->doubleclick_opens_containers(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
		AddWidget(open_container_button);
		button_index[last_index += 1] = open_container_button;
	}
	command_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, game->get_new_command_bar() != NULL, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(command_button);
	button_index[last_index += 1] = command_button;

	if (!Game::get_game()->is_new_style()) {
		bool party_view_targeting;
		config->value("config/input/party_view_targeting", party_view_targeting, false);
		party_targeting_button = new GUI_TextToggleButton(this, colX[1], buttonY += row_h, yesno_width, height, yesno_text, 2, party_view_targeting,  font, BUTTON_TEXTALIGN_CENTER, this, 0);
		AddWidget(party_targeting_button);
		button_index[last_index += 1] = party_targeting_button;
	} else
		open_container_button = party_targeting_button = NULL;
	cancel_button = new GUI_Button(this, 83, ID_HEIGHT - 20, 54, height, "Cancel", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(cancel_button);
	button_index[last_index += 1] = cancel_button;

	save_button = new GUI_Button(this, 154, ID_HEIGHT - 20, 40, height, "Save", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(save_button);
	button_index[last_index += 1] = save_button;

	return true;
}

InputDialog::~InputDialog() {
}

GUI_status InputDialog::close_dialog() {
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	callback_object->callback(0, this, this);
	return GUI_YUM;
}

GUI_status InputDialog::KeyDown(const Common::KeyState &key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case NORTH_KEY:
	case WEST_KEY:
		if (b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if (b_index_num <= 0)
			b_index_num = last_index;
		else
			b_index_num = b_index_num - 1;
		button_index[b_index_num]->set_highlighted(true);
		break;
	case SOUTH_KEY:
	case EAST_KEY:
		if (b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if (b_index_num == last_index)
			b_index_num = 0;
		else
			b_index_num += 1;
		button_index[b_index_num]->set_highlighted(true);
		break;
	case DO_ACTION_KEY:
		if (b_index_num != -1) return button_index[b_index_num]->Activate_button();
		break;
	case CANCEL_ACTION_KEY:
		return close_dialog();
	default:
		keybinder->handle_always_available_keys(a);
		break;
	}
	return GUI_YUM;
}

GUI_status InputDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (caller == cancel_button) {
		return close_dialog();
	} else if (caller == save_button) {
		Game *game = Game::get_game();
		MapWindow *map_window = game->get_map_window();
		Configuration *config = Game::get_game()->get_config();

		Std::string interface_str;
		if (interface_button->GetSelection() == 2)
			interface_str = "ignore_block";
		else if (interface_button->GetSelection() == 1)
			interface_str = "fullscreen";
		else // 0
			interface_str = "normal";
		config->set("config/input/interface", interface_str);
		map_window->set_interface(); // must come after you set cfg

		game->set_dragging_enabled(dragging_button->GetSelection());
		config->set("config/input/enabled_dragging", dragging_button->GetSelection() ? "yes" : "no");

		game->get_event()->set_direction_selects_target(direction_button->GetSelection());
		config->set("config/input/direction_selects_target", direction_button->GetSelection() ? "yes" : "no");

		map_window->set_look_on_left_click(look_button->GetSelection());
		config->set("config/input/look_on_left_click", look_button->GetSelection() ? "yes" : "no");

		map_window->set_walk_with_left_button(walk_button->GetSelection());
		config->set("config/input/walk_with_left_button", walk_button->GetSelection() ? "yes" : "no");

		map_window->set_enable_doubleclick(doubleclick_button->GetSelection());
		config->set("config/input/enable_doubleclick", doubleclick_button->GetSelection() ? "yes" : "no");

		map_window->set_use_left_clicks(); // allow or disallow left clicks - Must come after look_on_left_click and enable_doubleclick

		if (game->get_game_type() == NUVIE_GAME_U6) {
			game->set_free_balloon_movement(balloon_button->GetSelection() == 1);
			config->set(config_get_game_key(config) + "/free_balloon_movement", balloon_button->GetSelection() ? "yes" : "no");
		}
		if (open_container_button) {
			game->set_doubleclick_opens_containers(open_container_button->GetSelection());
			config->set("config/input/doubleclick_opens_containers", open_container_button->GetSelection() ? "yes" : "no");
		}
		if (command_button->GetSelection())
			game->init_new_command_bar();
		else
			game->delete_new_command_bar();
		config->set("config/input/new_command_bar", command_button->GetSelection() ? "yes" : "no");
		if (party_targeting_button) {
			game->get_view_manager()->get_party_view()->set_party_view_targeting(party_targeting_button->GetSelection());
			config->set("config/input/party_view_targeting", party_targeting_button->GetSelection() ? "yes" : "no");
		}

		config->write();
		close_dialog();
		return GUI_YUM;
	}

	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
