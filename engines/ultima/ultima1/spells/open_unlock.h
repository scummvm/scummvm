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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_OPEN_UNLOCK_H
#define ULTIMA_ULTIMA1_U1DIALOGS_OPEN_UNLOCK_H

#include "ultima/ultima1/spells/spell.h"
#include "ultima/ultima1/widgets/dungeon_item.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

/**
 * Open/unlock common spell base class
 */
class OpenUnlock : public Spell {
private:
	/**
	 * Open a given widget
	 */
	void openItem(Maps::MapDungeon *map, Widgets::DungeonItem *item);
public:
	/**
	 * Constructor
	 */
	OpenUnlock(Ultima1Game *game, Character *c, SpellId spellId) : Spell(game, c, spellId) {}

	/**
	 * Cast the spell within dungeons
	 */
	void dungeonCast(Maps::MapDungeon *map) override;
};

/**
 * Open spell
 */
class Open : public OpenUnlock {
public:
	/**
	 * Constructor
	 */
	Open(Ultima1Game *game, Character *c) : OpenUnlock(game, c, SPELL_OPEN) {}
};

/**
 * Open spell
 */
class Unlock : public OpenUnlock {
public:
	/**
	 * Constructor
	 */
	Unlock(Ultima1Game *game, Character *c) : OpenUnlock(game, c, SPELL_UNLOCK) {}
};

} // End of  namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
