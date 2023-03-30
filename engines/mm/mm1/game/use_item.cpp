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

#include "mm/mm1/game/use_item.h"
#include "mm/mm1/game/combat.h"
#include "mm/mm1/game/spells_party.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {
namespace Game {


Common::String UseItem::combatUseItem(Inventory &inv, Inventory::Entry &invEntry,
		bool isEquipped) {
	Common::String msg;
	Item *item = g_globals->_items.getItem(invEntry._id);

	if (!item->_effectId) {
		msg = STRING["dialogs.character.use_combat.no_special_power"];

	} else if (item->_equipMode == IS_EQUIPPABLE || isEquipped) {
		if (invEntry._charges) {
			g_globals->_combatEffectCtr++;
			inv.removeCharge(&invEntry);

			if (item->_effectId == 0xff) {
				setSpell(item->_spellId, 0, 0);
				Game::SpellsParty::cast(_spellIndex, g_globals->_currCharacter);

			} else {
				// TODO: find out area of Character _effectId is used as an offset for
				error("TODO: _effectId used as a character offset to increase attribute?");

				if (g_globals->_combatEffectCtr)
					(isEquipped ? &g_globals->_currCharacter->_equipped :
						&g_globals->_currCharacter->_backpack)->removeCharge(&invEntry);

				g_globals->_party.updateAC();
				msg = STRING["dialogs.character.use_combat.done"];
				return false;
			}
		} else {
			msg = STRING["dialogs.character.use_combat.no_charges_left"];
		}
	} else {
		msg = STRING["dialogs.character.use_combat.not_equipped"];
	}

	Game::Combat *combat = dynamic_cast<Game::Combat *>(g_events->priorView());
	assert(combat);
	combat->disableAttacks();

	return msg;
}

Common::String UseItem::nonCombatUseItem(Inventory &inv, Inventory::Entry &invEntry,
		bool isEquipped) {
	Common::String msg;
	Item *item = g_globals->_items.getItem(invEntry._id);

	if (!item->_effectId) {
		msg = STRING["dialogs.character.use_noncombat.no_special_power"];

	} else if (item->_equipMode == IS_EQUIPPABLE || isEquipped) {
		if (invEntry._charges) {
			g_globals->_nonCombatEffectCtr++;
			inv.removeCharge(&invEntry);

			if (item->_effectId == 0xff) {
				setSpell(item->_spellId, 0, 0);
				Game::SpellsParty::cast(_spellIndex, g_globals->_currCharacter);

			} else {
				// TODO: find out area of Character _effectId is used as an offset for
				error("TODO: _effectId used as a character offset to increase attribute?");

				if (g_globals->_nonCombatEffectCtr)
					(isEquipped ? &g_globals->_currCharacter->_equipped :
						&g_globals->_currCharacter->_backpack)->removeCharge(&invEntry);

				g_globals->_party.updateAC();
				msg = STRING["spells.done"];
				return false;
			}
		} else {
			msg = STRING["dialogs.character.use_noncombat.no_charges_left"];
		}
	} else {
		msg = STRING["dialogs.character.use_noncombat.not_equipped"];
	}

	Game::Combat *combat = dynamic_cast<Game::Combat *>(g_events->priorView());
	assert(combat);
	combat->disableAttacks();

	return msg;
}

} // namespace Game
} // namespace MM1
} // namespace MM
