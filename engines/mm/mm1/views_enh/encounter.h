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

#ifndef MM1_VIEWS_ENH_ENCOUNTER_H
#define MM1_VIEWS_ENH_ENCOUNTER_H

#include "mm/mm1/views_enh/yes_no.h"
#include "mm/mm1/events.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class Encounter : public YesNo {
private:
	enum Mode {
		ALERT, SURPRISED_BY_MONSTERS, SURPRISED_MONSTERS,
		ENCOUNTER_OPTIONS, NOWHERE_TO_RUN, SURROUNDED,
		SURRENDER_FAILED, NO_RESPONSE, BRIBE, NOT_ENOUGH,
		COMBAT, BATTLE
	};
	Mode _mode = ALERT;
	enum BribeType { BRIBE_GOLD, BRIBE_GEMS, BRIBE_FOOD };
	BribeType _bribeType = BRIBE_GOLD;
	Common::String _bribeTypeStr;

	/**
	 * Set display mode
	 */
	void setMode(Mode newMode);

	/**
	 * Sets the display area
	 */
	void setDisplayArea(bool largeArea);

	/**
	 * Handles the end of the encounter
	 */
	void encounterEnded();

	/**
	 * Initiates the combat
	 */
	void attack();

	/**
	 * Try and bribe the enemies
	 */
	void bribe();

	/**
	 * Try and retreat
	 */
	void retreat();

	/**
	 * Try and surrender
	 */
	void surrender();

	/**
	 * Ends an encounter
	 */
	void flee();

	/**
	 * Decreases the alignment counter, gradually turning
	 * EVIL to NEUTRAL to GOOD
	 */
	void decreaseAlignments();

	/**
	 * Increases the alignment counter, gradually turning
	 * GOOD to NEUTRAL to EVIL
	 */
	void increaseAlignments();
public:
	Encounter();
	virtual ~Encounter() {}

	/**
	 * Called when the view is focused
	 */
	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Draw the encounter details overlayed on
	 * the existing game screen
	 */
	void draw() override;

	/**
	 * Handles delay timeouts
	 */
	void timeout() override;

	/**
	 * Handles keypresses
	 */
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
