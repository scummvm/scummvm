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

#ifndef MM1_VIEWS_COMBAT_H
#define MM1_VIEWS_COMBAT_H

#include "mm/mm1/game/combat.h"
#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

class Combat : public TextView, public Game::Combat {
private:
	void writeOptions();
	void writeAttackOptions();
	void writeCastOption();
	void writeShootOption();
	void resetBottom();

	/**
	 * Write the encounter handicap
	 */
	void writeHandicap();

	/**
	 * Write out all the static content
	 */
	void writeStaticContent();

	/**
	 * Write out the round number
	 */
	void writeRound();

	/**
	 * Writes out the party member numbers,
	 * with a plus next to each if they can attack
	 */
	void writePartyNumbers();

	/**
	 * Write the monsters list
	 */
	void writeMonsters();

	/**
	 * Write out a monster's status
	 */
	void writeMonsterStatus(int monsterNum);

	/**
	 * Write out a series of dots
	 */
	void writeDots();

	/**
	 * Writes out the party members
	 */
	void writeParty();

	/**
	 * Clears the party area
	 */
	void clearPartyArea();

	void writeDefeat();

	/**
	 * Highlight the round number indicator
	 */
	void highlightNextRound();

protected:
	/**
	 * Sets a new display mode
	 */
	void setMode(Mode newMode) override;
public:
	Combat();
	virtual ~Combat() {}

	/**
	 * Called when the view is focused
	 */
	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Draw the Combat details overlayed on
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

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
