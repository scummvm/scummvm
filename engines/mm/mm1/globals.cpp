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
	_treasure.clear();
}

Globals::~Globals() {
	g_globals = nullptr;
}

bool Globals::load(bool isEnhanced) {
	// Initialise engine data for the game
	Common::U32String errMsg;
	if (!load_engine_data("mm1", 1, 0, errMsg)) {
		GUIErrorMessage(errMsg);
		return false;
	}

	if (!_strings.load("strings_en.yml"))
		return false;

	if (!_font.load("font.bmp"))
		return false;

	if (!_monsters.load() || !_items.load())
		return false;

	// Load roster
	_roster.load();

	if (isEnhanced) {
		_mainIcons.load("main.icn");
		_confirmIcons.load("confirm.icn");
		_globalSprites.load("global.icn");
		_tileSprites.load("town.til");

		{
			Common::File f;
			if (!f.open("symbols.bin"))
				error("Could not load symbols.bin");
			f.read(SYMBOLS, 20 * 64);
			f.close();
		}

		{
			Common::File f;
			if (!f.open("fnt"))
				error("Could not open font");
			_fontNormal.load(&f, 0, 0x1000);
			_fontReduced.load(&f, 0x800, 0x1080);
			f.close();
		}

		{
			Common::File f;
			if (!f.open("text_colors.bin"))
				error("Could not open text colors");
			XeenFont::loadColors(&f);
			XeenFont::setColors(0);
			f.close();
		}
	}

	return true;
}

void Globals::synchronize(Common::Serializer &s) {
	s.syncAsByte(_startingTown);

	// Sync the party
	int partySize = _party.size();
	s.syncAsByte(partySize);
	if (s.isLoading())
		_party.resize(partySize);

	for (int i = 0; i < partySize; ++i)
		_party[i].synchronize(s);

	// Sync map data and visited tiles
	_maps.synchronize(s);
	_maps.synchronizeCurrent(s);

	// Sync treasure data
	_treasure.synchronize(s);

	_currCharacter = nullptr;
}

} // namespace MM1
} // namespace MM
