
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

#ifndef ULTIMA2_LOGIC_MONDAIN_LOGIC_H
#define ULTIMA2_LOGIC_MONDAIN_LOGIC_H

#include "ultima/ultima1/logic/logic.h"

namespace Ultima {
namespace Ultima1 {
namespace Logic {

class MondainLogic : public Logic {
private:
	// tick() is called once per game frame, but the original only advanced
	// Mondain's animations once every few frames - this counts frames
	// between updates
	int _tickCounter = 0;

	// Which spell useSelectedItem/cast() started casting - remembered so
	// castSpellAttack (called back later once a direction's been chosen
	// via the Direction view) knows which of the 3 direction-needing
	// spells to resolve
	int _castSpell = 0;

	/**
	 * Returns true if Mondain is standing in any of the 8 cells surrounding
	 * the player's current position
	 */
	bool isAdjacentToMondain() const;

	/**
	 * Returns true if the player is within a Manhattan distance of 7 of
	 * Mondain - used to gate the Kill/Magic Missile spells
	 */
	bool isWithinRange7() const;

	/**
	 * Rolls whether a spell succeeds at all - based on intelligence, with
	 * wizards always succeeding
	 */
	bool rollSpellSuccess();

	/**
	 * Returns the magic "strike" power of the currently readied weapon,
	 * used by the Magic Missile spell - a random value up to the player's
	 * intelligence, doubled/tripled/halved again depending on whether a
	 * wand, amulet, staff, or triangle is readied
	 */
	int getMagicWeaponPower();

	/**
	 * Blink spell (SPELL_BLINK) - teleports the player to a random open
	 * floor tile. Doesn't need a direction
	 */
	bool castTeleport();

	/**
	 * Kill spell (SPELL_KILL) - rather than harming Mondain, it backfires
	 * and doubles his remaining hit points (capped at 15000). Doesn't need
	 * a direction
	 */
	bool castInterficioNunc();

	/**
	 * Create spell (SPELL_CREATE) - places a barrier on the empty tile in
	 * the given direction, provided it isn't in line with Mondain
	 */
	bool castPlaceBarrier(Data::Direction dir);

	/**
	 * Destroy spell (SPELL_DESTROY) - removes a barrier tile in the given
	 * direction
	 */
	bool castRemoveBarrier(Data::Direction dir);

	/**
	 * Magic Missile spell (SPELL_MAGIC_MISSILE) - attacks Mondain in the
	 * given direction, provided he's within range
	 */
	bool castMagicMissile(Data::Direction dir);

protected:
	bool move(Data::Direction dir) override;

	/**
	 * Handles updating creatures/NPCs
	 */
	void updateCreatures() override;

	/**
	 * Resolves a wand attack - scans up to maxDistance tiles in the given
	 * direction for Mondain, then rolls to hit and applies damage. Unlike
	 * the main game's damage(), there's no city/castle branch here at all -
	 * this is the whole of the original's scanAndDamageAlongDirection
	 */
	void damage(Data::Direction dir, int effectNum, int maxDistance, int strike, int hitChance, int tileId) override;

	/**
	 * Cast a spell - only Magic Missile, Blink, Create, Destroy, and Kill
	 * have any effect here; everything else just fails
	 */
	bool cast() override;

	/**
	 * Called back by the Direction view once a direction's been chosen for
	 * whichever spell cast() deferred to it (_castSpell)
	 */
	void castSpellAttack(Data::Direction dir) override;

	bool get() override;
	bool inform() override;
	bool quit() override;
	bool steal() override;
	bool transact() override;

public:
	MondainLogic();
	~MondainLogic() override {
	}

	void entering() override;

	/**
	 * Called once per game frame - advances Mondain's hit-reaction and
	 * idle-pose animations
	 */
	void tick() override;
};

} // namespace Logic
} // namespace Ultima1
} // namespace Ultima

#endif
