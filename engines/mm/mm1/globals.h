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

#ifndef MM1_GLOBALS_H
#define MM1_GLOBALS_H

#include "common/serializer.h"
#include "graphics/font.h"
#include "mm/utils/strings_data.h"
#include "mm/utils/xeen_font.h"
#include "mm/xeen/sprites.h"
#include "mm/mm1/data/game_state.h"
#include "mm/mm1/data/items.h"
#include "mm/mm1/data/monsters.h"
#include "mm/mm1/data/roster.h"
#include "mm/mm1/data/treasure.h"
#include "mm/mm1/gfx/bitmap_font.h"
#include "mm/mm1/game/combat.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/maps/maps.h"

namespace MM {
namespace MM1 {

class Globals : public GameState {
public:
	StringsData _strings;
	Roster _roster;
	Common::Array<Character *> _combatParty;
	Character *_currCharacter = nullptr;
	Item _currItem;
	ItemsArray _items;
	Maps::TownId _startingTown = Maps::SORPIGAL;
	Maps::Maps _maps;
	Gfx::BitmapFont _font;
	bool _heardRumor = false;
	Game::Encounter _encounters;
	Game::Combat *_combat = nullptr;
	Monsters _monsters;
	Treasure _treasure;
	byte _delay = 5;
	int _nonCombatEffectCtr = 0, _combatEffectCtr = 0;

	// Console flags
	bool _intangible = false;
	bool _encountersOn = true;
	bool _allSpells = false;

public:
	// Enhanced mode globals
	Xeen::SpriteResource _mainIcons;
	Xeen::SpriteResource _confirmIcons;
	Xeen::SpriteResource _globalSprites;
	Xeen::SpriteResource _tileSprites;
	byte SYMBOLS[20][64];
	XeenFont _fontNormal;
	XeenFont _fontReduced;
public:
	Globals();
	virtual ~Globals();

	/**
	 * Loads data for the globals
	 */
	bool load(bool isEnhanced);

	/**
	 * Returns a string
	 */
	const Common::String &operator[](const Common::String &name) {
		assert(_strings.contains(name));
		return _strings[name];
	}

	/**
	 * Saves global data to/from savegames
	 */
	void synchronize(Common::Serializer &s);
};

extern Globals *g_globals;

#define STRING (*g_globals)

} // namespace MM1
} // namespace MM

#endif
