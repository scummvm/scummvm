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

#ifndef MM1_GAME_SPELL_CASTING_H
#define MM1_GAME_SPELL_CASTING_H

#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {
namespace Game {

enum SpellState {
	SS_OK, SS_NOT_ENOUGH_SP, SS_NOT_ENOUGH_GEMS,
	SS_COMBAT_ONLY, SS_NONCOMBAT_ONLY, SS_DOESNT_WORK,
	SS_OUTDOORS_ONLY
};

/**
 * Support class for handling spell casting logic
 */
class SpellCasting {
protected:
	int _spellIndex = 0;
	int _requiredSp = 0, _requiredGems = 0;
	SpellState _spellState = SS_OK;

	/**
	 * Returns true if combat is in progress
	 */
	bool isInCombat() const;

public:
	/**
	 * Sets the current spell
	 */
	void setSpell(const Character *chr, int lvl, int num);

	/**
	 * Get the index in the spell array for a given spell
	 */
	static int getSpellIndex(const Character *chr, int lvl, int num);

	/**
	 * Get the spell level and number from spell index
	 */
	static void getSpellLevelNum(int spellIndex, int &lvl, int &num);

	/**
	 * Sets a spell directly by index
	 */
	void setSpell(int spellIndex, int requiredSp, int requiredGems);

	/**
	 * Returns true if the spell requires a target
	 */
	bool hasCharTarget() const;

	/**
	 * Returns true if a spell can be cast, with the exception
	 * of magic not being allowed at the current location.
	 */
	bool canCast() const {
		return _spellState == SS_OK;
	}

	/**
	 * Returns if magic is allowed at the current location
	 */
	bool isMagicAllowed() const;

	/**
	 * Returns the spell error
	 */
	SpellState getSpellState() const {
		return _spellState;
	}

	/**
	 * Returns the error message
	 */
	Common::String getSpellError() const;
};

} // namespace Game
} // namespace MM1
} // namespace MM

#endif
