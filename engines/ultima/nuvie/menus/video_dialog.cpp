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


#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/gui_types.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/gui/gui_text.h"
#include "ultima/nuvie/gui/gui_text_toggle_button.h"
#include "ultima/nuvie/gui/gui_callback.h"
#include "ultima/nuvie/gui/gui_area.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/screen/dither.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/gui/gui_dialog.h"
#include "ultima/nuvie/menus/video_dialog.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/inventory_view.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

static const int VD_WIDTH = 311;
static const int VD_HEIGHT = 171; // add or subtract 13 if you add/remove a row

VideoDialog::VideoDialog(GUI_CallBack *callback)
	: GUI_Dialog(Game::get_game()->get_game_x_offset() + (Game::get_game()->get_game_width() - VD_WIDTH) / 2,
	             Game::get_game()->get_game_y_offset() + (Game::get_game()->get_game_height() - VD_HEIGHT) / 2,
	             VD_WIDTH, VD_HEIGHT, 244, 216, 131, GUI_DIALOG_UNMOVABLE), callback_object(callback),
	             non_square_pixels_button(nullptr) {
	init();
	grab_focus();
}

bool VideoDialog::init() {
	const int colX[] = { 9, 29, 63, 232, 270};
	int height = 12;
	int yesno_width = 32;
	int buttonY = 9;
	uint8 textY = 11;
	uint8 row_h = 13;
	last_index = 0;
	b_index_num = -1;
	GUI_Widget *widget;
	GUI *gui = GUI::get_gui();
	GUI_Font *font = gui->get_font();
	Game *game = Game::get_game();
	Screen *scr = game->get_screen();
	const char *const yesno_text[] = { "no", "yes" };
	only2x_button = nullptr;

	// fullscreen_toggle
	widget = new GUI_Text(colX[0], textY, 0, 0, 0, "Fullscreen:", gui->get_font());
	AddWidget(widget);

	fullscreen_button = new GUI_TextToggleButton(this, colX[4], buttonY, yesno_width, height, yesno_text, 2, scr->is_fullscreen(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(fullscreen_button);
	button_index[last_index] = fullscreen_button;

	widget = new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Non-square pixels:", gui->get_font());
	AddWidget(widget);
	non_square_pixels_button = new GUI_TextToggleButton(this, colX[4], buttonY += row_h, yesno_width, height, yesno_text, 2, scr->is_non_square_pixels(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(non_square_pixels_button);
	button_index[last_index += 1] = non_square_pixels_button;

	Configuration *config = Game::get_game()->get_config();

	// show roofs
	widget = new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Show roofs:", gui->get_font());
	AddWidget(widget);
	roof_button = new GUI_TextToggleButton(this, colX[4], buttonY += row_h, yesno_width, height, yesno_text, 2, game->is_roof_mode(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(roof_button);
	button_index[(last_index += 1)] = roof_button;

	// use_new_dolls
	if (game->is_new_style()) {
		doll_button = nullptr;
		old_use_new_dolls = true;
	} else {
		widget = new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Use new actor dolls:", gui->get_font());
		AddWidget(widget);
		bool use_new_dolls;
		config->value(config_get_game_key(config) + "/use_new_dolls", use_new_dolls, false);
		old_use_new_dolls = use_new_dolls;
		doll_button = new GUI_TextToggleButton(this, colX[4], buttonY += row_h, yesno_width, height, yesno_text, 2, use_new_dolls, font, BUTTON_TEXTALIGN_CENTER, this, 0);
		AddWidget(doll_button);
		button_index[last_index += 1] = doll_button;
	}

	// tile_lighting_b
	widget = new GUI_Text(colX[0], textY += row_h, 0, 0, 0, "Use lighting data from map tiles:", gui->get_font());
	AddWidget(widget);
	old_use_tile_lighting = game->get_map_window()->using_map_tile_lighting;
	tile_lighting_b = new GUI_TextToggleButton(this, colX[4], buttonY += row_h, yesno_width, height, yesno_text, 2, old_use_tile_lighting, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(tile_lighting_b);
	button_index[last_index += 1] = tile_lighting_b;

	// needs restart text
	widget = new GUI_Text(colX[0], textY += row_h * 2, 0, 0, 0, "The following require a restart:", gui->get_font());
	AddWidget(widget);

	// lighting (needs reset)
	widget = new GUI_Text(colX[1], textY += row_h, 0, 0, 0, "Lighting mode:", gui->get_font());
	AddWidget(widget);
	const char *const lighting_text[] = { "none", "smooth", "original" };
	lighting_button = new GUI_TextToggleButton(this, colX[3], buttonY += row_h * 3, 70, height, lighting_text, ARRAYSIZE(lighting_text), scr->get_old_lighting_style(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(lighting_button);
	button_index[last_index += 1] = lighting_button;

	// sprites (needs reset)
	widget = new GUI_Text(colX[1], textY += row_h, 0, 0, 0, "Use custom actor tiles:", gui->get_font());
	AddWidget(widget);
	const char *const sprite_text[] = { "no", "yes", "default" };
	Std::string custom_tile_str;
	int custom_tiles;
	config->value(config_get_game_key(config) + "/custom_actor_tiles", custom_tile_str, "default");
	if (custom_tile_str == "default")
		custom_tiles = 2;
	else
		custom_tiles = custom_tile_str == "yes" ? 1 : 0;
	sprites_b = new GUI_TextToggleButton(this, colX[3], buttonY += row_h, 70, height, sprite_text, ARRAYSIZE(sprite_text), custom_tiles, font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(sprites_b);
	button_index[last_index += 1] = sprites_b;

	// game_style (needs reset)
	const char *game_style_text[] = {
		"original style",
		"new style",
		"original+",
		"original+ full map"};
	widget = new GUI_Text(colX[1], textY += row_h, 0, 0, 0, "Game style:", gui->get_font());
	AddWidget(widget);
	game_style_button = new GUI_TextToggleButton(this, colX[3] - 84, buttonY += row_h, 154, height, game_style_text, ARRAYSIZE(game_style_text), game->get_game_style(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(game_style_button);
	button_index[last_index += 1] = game_style_button;

	// dithering (needs reset)
	widget = new GUI_Text(colX[1], textY += row_h, 0, 0, 0, "Old video graphics:", gui->get_font());
	AddWidget(widget);
	const char *const dither_text[] = { "no", "CGA", "EGA" };
	dither_button = new GUI_TextToggleButton(this, colX[4], buttonY += row_h, yesno_width, height, dither_text, ARRAYSIZE(dither_text), game->get_dither()->get_mode(), font, BUTTON_TEXTALIGN_CENTER, this, 0);
	AddWidget(dither_button);
	button_index[last_index += 1] = dither_button;

	// cancel/save buttons
	cancel_button = new GUI_Button(this, 95, VD_HEIGHT - 20, 54, height, "Cancel", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(cancel_button);
	button_index[last_index += 1] = cancel_button;
	save_button = new GUI_Button(this, 170, VD_HEIGHT - 20, 40, height, "Save", font, BUTTON_TEXTALIGN_CENTER, 0, this, 0);
	AddWidget(save_button);
	button_index[last_index += 1] = save_button;

	return true;
}

VideoDialog::~VideoDialog() {
}

GUI_status VideoDialog::close_dialog() {
	Delete(); // mark dialog as deleted. it will be freed by the GUI object
	callback_object->callback(0, this, this);
	return GUI_YUM;
}

GUI_status VideoDialog::KeyDown(const Common::KeyState &key) {
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

GUI_status VideoDialog::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (caller == cancel_button) {
		return close_dialog();
	} else if (caller == save_button) {
		Game *game = Game::get_game();
		Screen *scr = Game::get_game()->get_screen();
		Configuration *config = Game::get_game()->get_config();

		bool fullscreen = fullscreen_button ? fullscreen_button->GetSelection() : scr->is_fullscreen();
		if (fullscreen != scr->is_fullscreen())
			scr->toggle_fullscreen();
		bool non_square_pixels = non_square_pixels_button ? (bool)non_square_pixels_button->GetSelection() : false;
		scr->set_non_square_pixels(non_square_pixels);

		// fullscreen
		config->set("config/fullscreen", fullscreen ? "yes" : "no");
		game->get_screen()->set_fullscreen(fullscreen);

		// non-square pixels
		config->set("config/video/non_square_pixels", non_square_pixels ? "yes" : "no");
		// roof mode
		bool roof_mode = roof_button->GetSelection();
		game->set_roof_mode(roof_mode);
		game->get_map_window()->set_roof_mode(roof_mode);
		game->get_game_map()->set_roof_mode(roof_mode);
		config->set(config_get_game_key(config) + "/roof_mode", roof_mode ? "yes" : "no");
		// use_new_dolls
		if (doll_button && old_use_new_dolls != (doll_button->GetSelection() ? 1 : 0)) {
			config->set(config_get_game_key(config) + "/use_new_dolls", doll_button->GetSelection() ? "yes" : "no");
			ViewManager *vm = game->get_view_manager();
			InventoryView *iv = vm->get_inventory_view();
			if (vm->get_current_view() == iv) // showing a doll so need to reset
				iv->set_party_member(iv->get_party_member_num());
		}

		// tile_lighting_b
		if (old_use_tile_lighting != (bool)tile_lighting_b->GetSelection()) {
			config->set(config_get_game_key(config) + "/map_tile_lighting", tile_lighting_b->GetSelection() ? "yes" : "no");
			game->get_map_window()->using_map_tile_lighting = tile_lighting_b->GetSelection() == 1;
			game->get_map_window()->updateAmbience();
		}

		// lighting
		const char *lighting_char;
		int lighting = lighting_button->GetSelection();
		if (lighting == 0)
			lighting_char = "none";
		else if (lighting == 1)
			lighting_char = "smooth";
		else
			lighting_char = "original";
		config->set("config/general/lighting", lighting_char);

		// sprites
		const char *sprite_char;
		if (sprites_b->GetSelection() == 2)
			sprite_char = "default";
		else
			sprite_char = sprites_b->GetSelection() ? "yes" : "no";
		config->set(config_get_game_key(config) + "/custom_actor_tiles", sprite_char);

		// game_style
		const char *game_style_text[] = {
				"original",
				"new",
				"original+",
				"original+_full_map"};
		config->set("config/video/game_style", game_style_text[game_style_button->GetSelection()]);

		// dither
		const char *dither_char;
		uint8 dither = dither_button->GetSelection();
		if (dither == 0)
			dither_char = "none";
		else if (dither == 1)
			dither_char = "cga";
		else
			dither_char = "ega";
		config->set("config/general/dither_mode", dither_char);

		config->write();
		close_dialog();
		return GUI_YUM;
	}

	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
