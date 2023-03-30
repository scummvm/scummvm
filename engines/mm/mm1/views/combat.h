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
	LineArray _monsterSpellLines;
	uint _attackableCount = 0;
	InfoMessage _spellResult;
	// Combat options that have sub-option selection
	enum SelectedOption {
		OPTION_NONE, OPTION_DELAY, OPTION_EXCHANGE,
		OPTION_FIGHT, OPTION_SHOOT
	};
	SelectedOption _option = OPTION_NONE;

	/**
	 * Selects a combat option that requires a selection
	 */
	void setOption(SelectedOption option);

	void writeOptions();
	void writeAllOptions();
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

	/**
	 * Writes the result of defeating all the monsters
	 */
	void writeDefeat();

	/**
	 * Highlight the round number indicator
	 */
	void highlightNextRound();

	/**
	 * Write monster changes
	 */
	void writeMonsterEffects();

	/**
	 * Handles a monster action
	 */
	void writeMonsterAction(bool flees);

	/**
	 * Write out message from a monster casting a spell
	 */
	void writeMonsterSpell();

	/**
	 * Write out monster's attack
	 */
	void writeMonsterAttack();

	/**
	 * Write message for monster infiltrating the party
	 */
	void writeInfiltration();

	/**
	 * Write message for monster waits for an opening
	 */
	void writeWaitsForOpening();

	/**
	 * Writes the result of a spell
	 */
	void writeSpellResult();

	/**
	 * Whether there's messages remaining
	 */
	void checkMonsterSpellDone();

	/**
	 * Delay option
	 */
	void delay();

	/**
	 * Exchange option
	 */
	void exchange();

	/**
	 * Fight option
	 */
	void fight();

	/**
	 * Shoot option
	 */
	void shoot();

	/**
	 * Write message for characters attacking monsters
	 */
	void writeCharAttackDamage();

	/**
	 * Write message for character attack having no effect
	 */
	void writeCharAttackNoEffect();

	/**
	 * Get attack damage string
	 */
	Common::String getAttackString();

	/**
	 * Writes out a message
	 */
	void writeMessage();

	/**
	 * Writes text for delay number selection
	 */
	void writeDelaySelect();

	/**
	 * Write text for exchange party member
	 */
	void writeExchangeSelect();

	/**
	 * Having selected to fight, selects monster to attack
	 */
	void writeFightSelect();

	/**
	 * Having selected to shoot, selects monster to attack
	 */
	void writeShootSelect();

protected:
	/**
	 * Sets a new display mode
	 */
	void setMode(Mode newMode) override;

	/**
	 * Does final cleanup when combat is done
	 */
	void combatDone() override;

public:
	Combat();
	virtual ~Combat() {}

	void displaySpellResult(const InfoMessage &msg) override;

	/**
	 * Disable the flags for allowing attacks for
	 * the current character
	 */
	void disableAttacks() override;

	/**
	 * Called when the view is focused
	 */
	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Called when the view is unfocused
	 */
	bool msgUnfocus(const UnfocusMessage &msg) override;

	/**
	 * Called for game messages
	 */
	bool msgGame(const GameMessage &msg) override;

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

	/**
	 * Key binder actions
	 */
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
