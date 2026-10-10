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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_CAST_SPELL_H
#define ULTIMA3_VIEWS_INTERACTIONS_CAST_SPELL_H

#include "ultima/ultima3/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * Has a party member cast a wizard or cleric spell, which costs magic points
 * and may need a direction or a party member to aim it at
 */
class CastSpell : public Interaction {
private:
	enum Stage {
		CHOOSE_CASTER,
		CHOOSE_TYPE,
		CHOOSE_SPELL,
		DIRECTION,
		TARGET,
		OVERVIEW
	};

	// The things the spells do
	enum Effect {
		SLAY_ORCS, MISSILE, LIGHT, DESCEND, ASCEND, FIREBALL, TELEPORT, MIND_BLAST,
		GREAT_LIGHT, WIZARD_TO_CLERIC, FIRE_STORM, ANNIHILATE, HOLD_TIME, MIND_STORM,
		DRAIN, DEATH_STORM, SLAY_SKELETONS, UNLOCK, HEAL_SMALL, RECALL_FLOOR,
		CURE, LEAVE_DUNGEON, HEAL_LARGE, VISION, RESURRECT, RECALL
	};

	Stage _stage = CHOOSE_CASTER;
	bool _inCombat;
	bool _finished = false;
	bool _cleric = false;
	int _slot;
	int _cost = 0;
	int _spell = 0;
	Effect _effect = SLAY_ORCS;
	int _amount = 0;
	PlayerChooser _players;
	DirectionChooser _directions;

	/**
	 * Gets going once a caster is known
	 */
	void begin();

	/**
	 * Carries out a spell that has been paid for
	 * @returns		True if it is finished
	 */
	bool perform(Effect effect);

	bool fail();
	void fanfare();

	/**
	 * Carries out a spell on a party member chosen as its target
	 */
	bool applyToPlayer(int target);

public:
	/**
	 * Constructor
	 * @param slot		The party member casting, or -1 to ask who
	 */
	CastSpell(int slot = -1);
	~CastSpell() override;

	bool isFinished() const override {
		return _finished;
	}

	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
