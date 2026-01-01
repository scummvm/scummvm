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

#include "common/debug.h"
#include "m4/riddle/vars.h"
#include "m4/riddle/inventory.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"
#include "m4/m4.h"

namespace M4 {
namespace Riddle {

Vars *g_vars;

static void spanish_ascii_converter(char *str) {
	while (*str) {
		byte c = (byte)*str;
		switch (c) {
		case 0xA0: // á CP850
		case 0xE1: // á ISO-8859-1
			*str = 0x80;
			break;
		case 0x82: // é CP850
		case 0xE9: // é ISO-8859-1
			*str = 0x8F;
			break;
		case 0xA1: // í CP850
		case 0xED: // í ISO-8859-1
			*str = 0x91;
			break;
		case 0xA2: // ó CP850
		case 0xF3: // ó ISO-8859-1
			*str = 0x83;
			break;
		case 0xA3: // ú CP850
		case 0xFA: // ú ISO-8859-1
			*str = 0x84;
			break;
		case 0xA4: // ñ CP850
		case 0xF1: // ñ ISO-8859-1
			*str = 0x85;
			break;
		// Uppercase
		case 0xB5: // Á CP850
		case 0xC1: // Á ISO-8859-1
			*str = 0x86;
			break;
		case 0x90: // É CP850
		case 0xC9: // É ISO-8859-1
			*str = 0x87;
			break;
		case 0xD6: // Í CP850
		case 0xCD: // Í ISO-8859-1
			*str = 0x88;
			break;
		case 0xE0: // Ó CP850 (Wait, 0xE0 is alpha in CP437, but usually Ó in 850)
		case 0xD3: // Ó ISO-8859-1
			*str = 0x89;
			break;
		// case 0xE9: // Ú CP850 - Conflict with é (ISO). Prioritize é.
		// 	*str = 0x81; 
		// 	break;
		case 0xDA: // Ú ISO-8859-1
			*str = 0x8A;
			break;
		
		case 0xA5: // Ñ CP850
		case 0xD1: // Ñ ISO-8859-1
			*str = 0x8B;
			break;
		
		case 0x81: // ü CP850
		case 0xFC: // ü ISO-8859-1
			*str = 0x8C;
			break;
			
		case 0xAD: // ¡ CP850
		// case 0xA1: // ¡ ISO-8859-1 - Conflict with í (CP850). Prioritize í.
		// 	*str = 0x82; 
		// 	break;
		// Special case for ISO ¡ if distinct?
		// ISO ¡ is 0xA1.
		// CP850 ¡ is 0xAD.
			
		case 0xA8: // ¿ CP850
		case 0xBF: // ¿ ISO-8859-1
			*str = 0x8E;
			break;
		default:
			break;
		}
		str++;
	}
}

Vars::Vars() {
	g_vars = this;

	Inventory *inv = new Inventory();
	_inventory = inv;
}

Vars::~Vars() {
	g_vars = nullptr;
	delete _inventory;
}

void Vars::main_cold_data_init() {
	_player.walker_loads_first = false;
	_player.walker_visible = true;
	_global_sound_room = 997;
	_kernel.letter_box_y = LETTERBOX_Y;
	_kernel.first_fade = 0;
	_kernel.first_non_walker_cel_hash = 32;
	_i_just_hyperwalked = false;

	conv_set_default_hv(6, 4);
	_game.previous_room = 0;
	_game.room_id = 0;

	if (_game.new_room == 0)
		_game.setRoom(901);

	initMouseSeries("cursor", nullptr);
	conv_set_default_text_colour(7, 8);
}

void Vars::global_menu_system_init() {
	AddSystemHotkey(KEY_ESCAPE, Hotkeys::escape_key_pressed);
	AddSystemHotkey(KEY_F2, Hotkeys::saveGame);
	AddSystemHotkey(KEY_F3, Hotkeys::loadGame);

	if (_interface.init(0, 5, 6, 8, 9))
		static_cast<Inventory *>(_inventory)->init();
}

void Vars::initialize_game() {
	// Put all the inventory items back in their original scenes
	for (const auto &item : _inventory->_objects)
		inv_put_thing_in(item->name, item->scene);

	_G(flags)[V071] = 2;
	_G(flags)[V088] = 1;
	_G(flags)[V086] = 2;
	_G(flags)[V014] = 1;
	_G(flags)[V270] = 824;
	_G(flags)[V282] = 1;
	_G(flags)[V292] = 1;

	if (g_engine->getLanguage() == Common::ES_ESP) {
		_G(custom_ascii_converter) = spanish_ascii_converter;
	}
}

} // namespace Riddle
} // namespace M4
