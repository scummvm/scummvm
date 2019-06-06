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

#include "glk/advsys/advsys.h"
#include "common/translation.h"

namespace Glk {
namespace AdvSys {

void AdvSys::runGame() {
	if (!initialize()) {
		GUIErrorMessage(_("Could not start AdvSys game"));
		return;
	}

	// TODO: play game
	print("ADVINT v1.2 - Copyright (c) 1986, by David Betz\n");

	deinitialize();
}

bool AdvSys::initialize() {
	// Create a Glk window for the game
	_window = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	if (!_window)
		return false;

	// Load the game's header
	if (!Game::init(_gameFile))
		return false;

	return true;
}

void AdvSys::deinitialize() {
}

void AdvSys::print(const char *msg) {
	glk_put_string_stream(glk_window_get_stream(_window), msg);
}

Common::Error AdvSys::loadGameData(strid_t save) {
	return Common::kNoError;
}

Common::Error AdvSys::saveGameData(strid_t save, const Common::String& desc) {
	return Common::kNoError;
}

} // End of namespace AdvSys
} // End of namespace Glk
