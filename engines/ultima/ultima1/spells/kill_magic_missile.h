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

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_KILL_MAGIC_MISSILE_H
#define ULTIMA_ULTIMA1_U1DIALOGS_KILL_MAGIC_MISSILE_H

#include "ultima/ultima1/spells/spell.h"
#include "ultima/shared/engine/messages.h"

namespace Ultima {
namespace Ultima1 {
namespace Spells {

using Shared::CCharacterInputMsg;

/**
 * Common intermediate base class for both the Kill and Magic Missile spells
 */
class KillMagicMIssile : public Spell {
	DECLARE_MESSAGE_MAP;
	bool CharacterInputMsg(CCharacterInputMsg &msg);
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	KillMagicMIssile(Ultima1Game *game, Character *c, SpellId spellId) : Spell(game, c, spellId) {}

	/**
	 * Cast the spell outside a dungeon
	 */
	void cast(Maps::MapBase *map) override;
};

/**
 * Kill spell
 */
class Kill : public KillMagicMIssile {
public:
	/**
	 * Constructor
	 */
	Kill(Ultima1Game *game, Character *c);

	/**
	 * Cast the spell within dungeons
	 */
	void dungeonCast(Maps::MapDungeon *map) override;
};

/**
 * Magic Missile spell
 */
class MagicMissile : public KillMagicMIssile {
public:
	/**
	 * Constructor
	 */
	MagicMissile(Ultima1Game *game, Character *c);

	/**
	 * Cast the spell within dungeons
	 */
	void dungeonCast(Maps::MapDungeon *map) override;
};

} // End of  namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
