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

#include "engines/engine.h"
#include "graphics/fonts/ttf.h"
#include "mm/mm1/globals.h"
#include "mm/utils/engine_data.h"
#include "graphics/fontman.h"

namespace MM {
namespace MM1 {

Globals *g_globals;

Globals::Globals() {
	g_globals = this;
}

Globals::~Globals() {
	g_globals = nullptr;
}

bool Globals::load() {
	// Initialise engine data for the game
	Common::U32String errMsg;
	if (!load_engine_data("mm1", 1, 0, errMsg)) {
		GUIErrorMessage(errMsg);
		return false;
	}

	if (!_strings.load("strings_en.yml"))
		return false;

	// Load roster
	_roster.load();

	// Load the font
	_font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);

	return true;
}

} // namespace MM1
} // namespace MM
