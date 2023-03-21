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

#ifndef MM1_VIEWS_CHARACTER_INFO_H
#define MM1_VIEWS_CHARACTER_INFO_H

#include "common/array.h"
#include "mm/mm1/views/character_base.h"
#include "mm/mm1/data/character.h"
#include "mm/mm1/data/items.h"
#include "mm/mm1/game/spell_casting.h"
#include "mm/mm1/game/equip_remove.h"
#include "mm/mm1/views/text_entry.h"

namespace MM {
namespace MM1 {
namespace Views {

/**
 * In-game character dialog
 */
class CharacterInfo : public CharacterBase, MM1::Game::SpellCasting,
		MM1::Game::EquipRemove {
private:
	enum ViewState {
		DISPLAY, EQUIP, GATHER, REMOVE, SHARE,
		TRADE_WITH, TRADE_KIND, TRADE_ITEM, USE };
	ViewState _state = DISPLAY;
	Common::String _newName;
private:
	int _tradeWith = -1;
	TransferKind _tradeKind = TK_GEMS;
	TextEntry _textEntry;
private:
	/**
	 * Equips the item at the given index
	 */
	void equipItem(uint index);

	/**
	 * Removes the equipped item at the given index
	 */
	void removeItem(uint index);

	/**
	 * Draw options for gems, gold, and food
	 */
	void drawGemsGoldFood();

	/**
	 * Selects how much for gems/gold/food trades
	 */
	void tradeHowMuch();

	/**
	 * Using an item during combat
	 */
	void combatUseItem(Inventory &inv, Inventory::Entry &invEntry, bool isEquipped);

	/**
	 * Using an item outside of combat
	 */
	void nonCombatUseItem(Inventory &inv, Inventory::Entry &invEntry, bool isEquipped);

public:
	CharacterInfo() : CharacterBase("CharacterInfo") {}
	virtual ~CharacterInfo() {}

	void draw() override;
	void timeout() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;

	/**
	 * How much entry aborted
	 */
	void howMuchAborted();

	/**
	 * How much entered for a trade
	 */
	void howMuchEntered(uint amount);
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
