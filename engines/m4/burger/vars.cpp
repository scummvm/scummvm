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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "m4/burger/vars.h"
#include "m4/burger/rooms/room.h"
#include "m4/adv_r/adv_file.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/mem/mem.h"
#include "m4/platform/keys.h"
#include "m4/detection.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {

Vars *g_vars;

const char *GAME_MODES[4] = { "WHOLE_GAME", "INTERACTIVE_DEMO", "MAGAZINE_DEMO", "WHOLE_GAME" };
int32 Vars::_wilbur_should = 0;

/**
 * Structure for accented character replacement
 */
struct ConverterEntry {
	const char *_find;
	byte _replace;
};
static const ConverterEntry ASCII_CONVERTERS[] = {
	{ "\xc4", 1 },
	{ "\xc9", 2 },
	{ "\xd6", 3 },
	{ "\xdc", 4 },
	{ "\xe1", 5 },
	{ "\xe4", 6 },
	{ "\xe9", 0x0B },
	{ "\xf6", 0x0C },
	{ "\xfc", 0x0E },
	{ "\xdf", 0x0F },
	{ "\xe2", 0x10 },
	{ "\xe0", 0x11 },
	{ "\xef", 0x12 },
	{ "\xee", 0x13 },
	{ "\xea", 0x14 },
	{ "\xe8", 0x15 },
	{ "\xeb", 0x16 },
	{ "\xf9", 0x17 },
	{ "\xfb", 0x18 },
	{ "\xe7", 0x19 },
	{ "\xc7", 0x1C },
	{ "\xf4", 0x1D },
	{ nullptr, 0 }
};

Vars::Vars() {
	g_vars = this;
	_resources.setUseLowercase(true);

	Inventory *inv = new Inventory();
	_inventory = inv;

	Common::fill(&_globals[0], &_globals[GLB_SHARED_VARS], 0);
}

Vars::~Vars() {
	g_vars = nullptr;
	delete _inventory;
}

void Vars::main_cold_data_init() {
	initMouseSeries("cursor", nullptr);

	_kernel.first_fade = 32;
	_G(custom_ascii_converter) = custom_ascii_converter_proc;
	debugC(1, kDebugCore, "executing - %s", GAME_MODES[_executing]);

	// Set up game mode and starting room
	switch (g_engine->isDemo()) {
	case GStyle_Demo:
		_executing = INTERACTIVE_DEMO;
		break;
	case GStyle_NonInteractiveDemo:
		_executing = MAGAZINE_DEMO;
		break;
	default:
		_executing = WHOLE_GAME;
		break;
	}

	switch (_executing) {
	case JUST_OVERVIEW:
		_game.setRoom(971);		// Burger overview starts right in at 971
		break;

	case INTERACTIVE_DEMO:
	case MAGAZINE_DEMO:
		// Burger Demo starts at the demo menu screen
		_game.setRoom(901);
		break;

	case WHOLE_GAME:
		_game.setRoom(ConfMan.getBool("seen_intro") || g_engine->savesExist() ? 903 : 951);
		break;
	}

	font_set_colors(2, 1, 3);
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
	inv_give_to_player("MONEY");

	// Reset the global variables
	_flags.reset();
	_flags.reset1();
	_flags.reset2();
	_flags.reset3();
	_flags.reset4();
	_flags.reset5();
	_flags.conv_reset_all();

	Rooms::Room::setWilburHotspot();
}

void Vars::custom_ascii_converter_proc(char *string) {
	char *str;

	for (const auto *entry = ASCII_CONVERTERS; entry->_find; entry++) {
		while ((str = strstr(string, entry->_find)) != nullptr)
			*str = entry->_replace;
	}
}

} // namespace Burger
} // namespace M4
