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
#include "ultima/nuvie/menus/cheats_dialog.h"
#include "ultima/nuvie/core/egg_manager.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

#define CD_WIDTH 212
#define CD_HEIGHT 101

CheatsDialog::CheatsDialog(GUI_CallBack *callback)
	: GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - CD_WIDTH) / 2,
	             Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - CD_HEIGHT) / 2,
	             CD_WIDTH, CD_HEIGHT, 244, 216, 131, GUI_DIALOG_UNMOVABLE) {
	callback_object = callback;
	init();
	grab_focus();
}

bool CheatsDialog::init() {
	int textY[] = { 11, 24, 37, 50, 63 };
	int buttonY[] = { 9, 22, 35, 48, 61, 80 };
	int colX[] = { 9, 163 };
	int height = 12;
	b_index_num = -1;
	last_index = 0;
	GUI_Widget *widget;
	GUI *gui = GUI::get_gui();

	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[0], 0, 0, 0, "Cheats:", gui->get_font());
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[1], 0, 0, 0, "Show eggs:", gui->get_font());
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[2], 0, 0, 0, "Enable hackmove:", gui->get_font());
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[3], 0, 0, 0, "Anyone will join:", gui->get_font());
	AddWidget(widget);
	widget = (GUI_Widget *) new GUI_Text(colX[0], textY[4], 0, 0, 0, "Minimum brightness:", gui->get_font());
	AddWidget(widget);

	bool party_all_the_time;
	Game *game = Game::get_game();
	Configuration *config = game->get_config();
	config->value("config/cheats/party_all_the_time", party_all_the_time);
	const char *const enabled_text[] = { "Disabled", "Enabled" };
	const char *const yesno_text[] = { "no", "yes" };
	char buff[4];
	int brightness_selection;
	int num_of_brightness = 8;
	uint8 min_brightness = game->get_map_window()->get_min_brightness();

	if (min_brightness == 255) {
		brightness_selection = 7;
	} else if (min_brightness % 20 == 0 && min_brightness <= 120) {
		brightness_selection = min_brightness / 20;
	} else {
		num_of_brightness = 9;
		brightness_selection = 8; // manually edited setting or old 128
		sprintf(buff, "%d", min_brightness);
	}
	const char *const brightness_text[] = { "0", "20", "40", "60", "80", "100", "120", "255", buff };

	cheat_button = new GUI_TextToggleButton(this, colX[1] - 30, buttonY[0], 70, height, enabled_text, 2, game->are_cheats_enabled(), gui->get_font(), BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(cheat_button);
	button_index[last_index] = cheat_button;

	egg_button = new GUI_TextToggleButton(this, colX[1], buttonY[1], 40, height, yesno_text, 2, game->get_obj_manager()->is_showing_eggs(), gui->get_font(), BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(egg_button);
	button_index[last_index += 1] = egg_button;

	hackmove_button = new GUI_TextToggleButton(this, colX[1], buttonY[2], 40, height, yesno_text, 2, game->using_hackmove(), gui->get_font(), BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(hackmove_button);
	button_index[last_index += 1] = hackmove_button;

	party_button = new GUI_TextToggleButton(this, colX[1], buttonY[3], 40, height, yesno_text, 2, party_all_the_time, gui->get_font(), BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(party_button);
	button_index[last_index += 1] = party_button;

	brightness_button = new GUI_TextToggleButton(this, colX[1], buttonY[4], 40, height, brightness_text, num_of_brightness, brightness_selection,  gui->get_font(), BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(brightness_button);
	button_index[last_index += 1] = brightness_button;

	cancel_button = new GUI_Button(this, 50, buttonY[5], 54, height, "Cancel", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(cancel_button);
	button_index[last_index += 1] = cancel_button;

	save_button = new GUI_Button(this, 121, buttonY[5], 40, height, "Save", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(save_button);
	button_index[last_index += 1] = save_button;

	return true;
}

CheatsDialog::~CheatsDialog() {
}

GUI_status CheatsDialog::close_dialog() {
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	callback_object->callback(0, this, this); // I don't think this does anything atm
	return GUI_YUM;
}

GUI_status CheatsDialog::KeyDown(const Common::KeyState &key) {
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

GUI_status CheatsDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (caller == cancel_button) {
		return close_dialog();
	} else if (caller == save_button) {
		Game *game = Game::get_game();
		Configuration *config = game->get_config();

		Std::string key = config_get_game_key(config);
		key.append("/show_eggs");
		config->set(key, egg_button->GetSelection() ? "yes" : "no");
		game->get_obj_manager()->set_show_eggs(egg_button->GetSelection());
		game->get_egg_manager()->set_egg_visibility(cheat_button->GetSelection() ? egg_button->GetSelection() : false);

		game->set_cheats_enabled(cheat_button->GetSelection());
		config->set("config/cheats/enabled", cheat_button->GetSelection() ? "yes" : "no");
		game->set_hackmove(hackmove_button->GetSelection());
		config->set("config/cheats/enable_hackmove", hackmove_button->GetSelection() ? "yes" : "no");
		game->get_converse()->set_party_all_the_time(party_button->GetSelection());
		config->set("config/cheats/party_all_the_time", party_button->GetSelection() ? "yes" : "no");

		int brightness = brightness_button->GetSelection();
		if (brightness < 8) {
			int min_brightness;
			if (brightness == 7)
				min_brightness = 255;
			else
				min_brightness = brightness * 20;
			config->set("config/cheats/min_brightness", min_brightness);
			game->get_map_window()->set_min_brightness(min_brightness);
			game->get_map_window()->updateAmbience();
		}

		config->write();
		return close_dialog();
	}

	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
