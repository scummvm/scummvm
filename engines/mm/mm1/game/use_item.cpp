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
#include "mm/mm1/game/equip_remove.h"
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

	if (!item->_tempBonus_id) {	
		msg = STRING["dialogs.character.use_combat.no_special_power"];

	} else if (item->_constBonus_id == IS_EQUIPPABLE || isEquipped) {
		if (invEntry._charges) {
			g_globals->_combatEffectCtr++;
			inv.removeCharge(&invEntry);

			if (item->_tempBonus_id == 0xff) {
				setSpell(item->_spellId, 0, 0);
				Game::SpellsParty::cast(_spellIndex, g_globals->_currCharacter);

			} else {
				// TODO: find out area of Character _effectId is used as an offset for
				//error("TODO: _effectId used as a character offset to increase attribute?");
				//add temporary equip bonus to character parameters
				applyItemBonus(item->_tempBonus_id, item->_tempBonus_value);

				if (g_globals->_combatEffectCtr)
					(isEquipped ? &g_globals->_currCharacter->_equipped :
						&g_globals->_currCharacter->_backpack)->removeCharge(&invEntry);

				g_globals->_party.updateAC();
				msg = STRING["dialogs.character.use_combat.done"];
				return "";
			}
		} else {
			msg = STRING["dialogs.character.use_combat.no_charges_left"];
		}
	} else {
		msg = STRING["dialogs.character.use_combat.not_equipped"];
	}

	g_events->send("Combat", GameMessage("DISABLE_ATTACKS"));

	return msg;
}

Common::String UseItem::nonCombatUseItem(Inventory &inv, Inventory::Entry &invEntry,
		bool isEquipped) {
	Common::String msg;
	Item *item = g_globals->_items.getItem(invEntry._id);

	if (!item->_tempBonus_id) {
		msg = STRING["dialogs.character.use_noncombat.no_special_power"];

	} else if (item->_constBonus_id == IS_EQUIPPABLE || isEquipped) {
		if (invEntry._charges) {
			g_globals->_nonCombatEffectCtr++;
			inv.removeCharge(&invEntry);

			if (item->_tempBonus_id== 0xff) {
				setSpell(item->_spellId, 0, 0);
				SpellResult result = Game::SpellsParty::cast(_spellIndex, g_globals->_currCharacter);
				switch (result) {
				case SR_SUCCESS_DONE:
					msg = STRING["spells.done"];
					break;
				case SR_FAILED:
					msg = STRING["spells.failed"];
					break;
				case SR_SUCCESS_SILENT:
					break;
				}
			} else {
				// Add temorary equip bonus to character parameters
				applyItemBonus (item->_tempBonus_id, item->_tempBonus_value);

				if (g_globals->_nonCombatEffectCtr)
					(isEquipped ? &g_globals->_currCharacter->_equipped :
						&g_globals->_currCharacter->_backpack)->removeCharge(&invEntry);

				g_globals->_party.updateAC();
				msg = STRING["spells.done"];
			}
		} else {
			msg = STRING["dialogs.character.use_noncombat.no_charges_left"];
		}
	} else {
		msg = STRING["dialogs.character.use_noncombat.not_equipped"];
	}

	return msg;
}

void UseItem::applyItemBonus(int id, int value){
	if ((id<2)||(id>=0xff)) return;
	Character &c = *g_globals->_currCharacter;
	switch (id) {
		case 24: c._might._current += value; break;
		case 30: c._speed._current += value; break;
		case 32: c._accuracy._current += value; break;
		case 34: c._luck._current += value; break;
		case 36: c._level._current += value; break;
		case 37: c._age._current += value; break;
		case 43: c._sp._current += value; break;
		case 48: c._spellLevel._current += value; break;
		case 49: c._gems += value; break;
		case 58: c._gold += 255*value; break;
		case 62: c._food += value; break;
		case 89: c._resistances._s._magic._current += value; break;
		case 99: c._resistances._s._fear._current += value; break;
	}
}


} // namespace Game
} // namespace MM1
} // namespace MM
