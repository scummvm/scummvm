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

#ifndef MM1_VIEWS_SPELLS_CAST_SPELL_H
#define MM1_VIEWS_SPELLS_CAST_SPELL_H

#include "mm/mm1/views/spells/spell_view.h"
#include "mm/mm1/views/text_entry.h"
#include "mm/mm1/game/spell_casting.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

class CastSpell : public SpellView, public MM1::Game::SpellCasting {
	enum State {
		NONE, SELECT_SPELL, SELECT_NUMBER, SELECT_CHAR, PRESS_ENTER, ENDING
	};
private:
	State _state = SELECT_SPELL;
	TextEntry _textEntry;
	int _spellLevel = -1;
	int _spellNumber = -1;
	Common::String _spellResult;
	int _spellResultX = -1;

private:
	/**
	 * Set the current state
	 */
	void setState(State state);

	/**
	 * Spell level number entered
	 */
	void spellLevelEntered(uint level);

	/**
	 * Spell number entered
	 */
	void spellNumberEntered(uint num);

	/**
	 * Cast the spell
	 */
	void performSpell(Character *chr = nullptr);

	/**
	 * Spell done or failed
	 */
	void spellDone();
	void spellDone(const Common::String &msg, int xp);
public:
	CastSpell();
	virtual ~CastSpell() {
	}

	bool msgGame(const GameMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgAction(const ActionMessage &msg) override;
	void timeout() override;
};

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
