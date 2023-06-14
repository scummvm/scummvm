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

#ifndef M4_GLOBALS_H
#define M4_GLOBALS_H

#include "m4/game.h"
#include "m4/kernel.h"
#include "m4/term.h"
#include "m4/fileio/sys_file.h"

namespace M4 {

#define CACHE_NOT_OVERRIDE_BY_FLAG_PARSE 2

struct Globals;

extern Globals *g_globals;

struct Globals {
	Globals() { g_globals = this; }
	~Globals() { g_globals = nullptr; }

	Game _game;
	Kernel _kernel;
	Term _term;
	Hag_Statics _hag;

	bool _system_shutting_down = false;
	size_t _mem_to_alloc = 0;
	void *_gameInterfaceBuff = nullptr;
	void *_custom_interface_setup = nullptr;
	void *_custom_interface_button_handler = nullptr;
	int _global_sound_room = 0;
	bool _interface_visible = false;
	bool _please_hyperwalk = false;
};

#define _G(X) (g_globals->_##X)

} // namespace M4

#endif
