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

#ifndef ULTIMA_ULTIMA1_CORE_PARTY_H
#define ULTIMA_ULTIMA1_CORE_PARTY_H

#include "ultima/shared/core/party.h"
#include "ultima/ultima1/spells/blink.h"
#include "ultima/ultima1/spells/create.h"
#include "ultima/ultima1/spells/destroy.h"
#include "ultima/ultima1/spells/kill.h"
#include "ultima/ultima1/spells/ladder_down.h"
#include "ultima/ultima1/spells/ladder_up.h"
#include "ultima/ultima1/spells/magic_missile.h"
#include "ultima/ultima1/spells/open_unlock.h"
#include "ultima/ultima1/spells/prayer.h"
#include "ultima/ultima1/spells/steal.h"

namespace Ultima {
namespace Ultima1 {

enum Weapon {
	WEAPON_HANDS = 0, WEAPON_DAGGER = 1, WEAPON_MACE = 2, WEAPON_AXE = 3, WEAPON_ROPE_SPIKES = 4,
	WEAPON_SWORD = 5, WEAPON_GREAT_SWORD = 6, WEAPON_BOW_ARROWS = 7, WEAPON_AMULET = 8,
	WEAPON_WAND = 9, WEAPON_STAFF = 10, WEAPON_TRIANGLE = 11, WEAPON_PISTOL = 12,
	WEAPON_LIGHT_SWORD = 13, WEAPON_PHAZOR = 14, WEAPON_BLASTER = 15
};

class Ultima1Game;

/**
 * Implements the data for a playable character within the game
 */
class Character : public Shared::Character {
private:
	Ultima1Game *_game;
	Spells::Blink _spellBlink;
	Spells::Create _spellCreate;
	Spells::Destroy _spellDestroy;
	Spells::Kill _spellKill;
	Spells::LadderDown _spellLadderDown;
	Spells::LadderUp _spellLadderUp;
	Spells::MagicMissile _spellMagicMissile;
	Spells::Open _spellOpen;
	Spells::Prayer _spellPrayer;
	Spells::Steal _spellSteal;
	Spells::Unlock _spellUnlock;
public:
	/**
	 * Constructor
	 */
	Character(Ultima1Game *game);

	/**
	 * Setup the party
	 */
	void setup();

	/**
	 * Gets the damage a given weapon does
	 */
	uint getWeaponDamage() const;
};

/**
 * Implements the party
 */
class Party : public Shared::Party {
public:
	/**
	 * Constructor
	 */
	Party(Ultima1Game *game);

	/**
	 * Setup the party
	 */
	void setup();
};

} // End of namespace Ultima1
} // End of namespace Ultima

#endif
