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

#ifndef MM1_VIEWS_CAST_SPELL_H
#define MM1_VIEWS_CAST_SPELL_H

#include "mm/mm1/views/text_view.h"
#include "mm/mm1/views/text_entry.h"
#include "mm/mm1/game/spell_casting.h"

namespace MM {
namespace MM1 {
namespace Views {

class CastSpell : public TextView, public MM1::Game::SpellCasting {
	enum State {
		SELECT_SPELL, SELECT_NUMBER, SELECT_CHAR, PRESS_ENTER,
		ENDING
	};
private:
	State _state = SELECT_SPELL;
	TextEntry _textEntry;
	int _spellLevel = -1;
private:
	/**
	 * Set the current state
	 */
	void setState(State state);

	/**
	 * Start the selection of spell to cast
	 */
	void startCasting();

	/**
	 * Spell level number entered
	 */
	void spellLevelEntered(uint level);

	/**
	 * Spell number entered
	 */
	void spellNumberEntered(uint num);

	/**
	 * Spell done or failed
	 */
	void spellDone();
public:
	CastSpell();
	virtual ~CastSpell() {}

	bool msgValue(const ValueMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void timeout() override;
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
