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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_SPELL_H
#define ULTIMA_ULTIMA1_U1DIALOGS_SPELL_H

#include "ultima/shared/core/character.h"
#include "ultima/ultima1/maps/map_base.h"
#include "ultima/ultima1/maps/map_dungeon.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;

namespace Spells {
	
enum SpellId {
	SPELL_PRAYER = 0, SPELL_OPEN = 1, SPELL_UNLOCK = 2, SPELL_MAGIC_MISSILE = 3, SPELL_STEAL = 4,
	SPELL_LADDER_DOWN = 5, SPELL_LADDER_UP = 6, SPELL_BLINK = 7, SPELL_CREATE = 8,
	SPELL_DESTROY = 9, SPELL_KILL = 10
};

/**
 * Base class for Ultima 1 spells
 */
class Spell : public Shared::Spell {
protected:
	Ultima1Game *_game;
	SpellId _spellId;
protected:
	/**
	 * Adds a text string to the info area
	 * @param text			Text to add
	 * @param newLine		Whether to apply a newline at the end
	 */
	void addInfoMsg(const Common::String &text, bool newLine = true);
protected:
	/**
	 * Constructor
	 */
	Spell(SpellId spellId) : _game(nullptr), _spellId(spellId) {}
public:
	/**
	 * Sets the game
	 */
	void setGame(Ultima1Game *game);

	/**
	 * Cast the spell outside of dungeons
	 */
	virtual void cast(Maps::MapBase *map);

	/**
	 * Cast the spell in dungeons
	 */
	virtual void dungeonCast(Maps::MapDungeon *map);
};

} // End of  namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
