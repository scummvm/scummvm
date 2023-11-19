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

#ifndef MM1_VIEWS_ENH_CAST_SPELL_H
#define MM1_VIEWS_ENH_CAST_SPELL_H

#include "mm/mm1/messages.h"
#include "mm/mm1/game/spell_casting.h"
#include "mm/mm1/views_enh/party_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

/**
 * Dialog for casting a spell
 */
class CastSpell : public PartyView, public MM1::Game::SpellCasting {
private:
	Shared::Xeen::SpriteResource _icons;

	/**
	 * Updates the data for the displayed spell
	 */
	void updateSelectedSpell();

	/**
	 * Casts the selected spell
	 */
	void castSpell(Character *target = nullptr);

	/**
	 * Handles spell errors
	 */
	void spellError();

protected:
	/**
	 * Return true if the selected character can be switched
	 */
	bool canSwitchChar() override {
		return !isInCombat();
	}

	/**
	 * Called when the selected character has been switched
	 */
	void charSwitched(Character *priorChar) override;

public:
	CastSpell();
	virtual ~CastSpell() {}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
};

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
