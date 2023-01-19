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

#include "mm/mm1/views/character_info.h"
#include "mm/mm1/views/combat.h"
#include "mm/mm1/game/spells_party.h"
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

void CharacterInfo::draw() {
	assert(g_globals->_currCharacter);
	CharacterBase::draw();

	MetaEngine::setKeybindingMode(
		_state == DISPLAY || _state == TRADE_WITH ?
		KeybindingMode::KBMODE_PARTY_MENUS :
		KeybindingMode::KBMODE_MENUS
	);

	switch (_state) {
	case DISPLAY:
		writeString(0, 21, STRING["dialogs.character.legend1"]);
		writeString(0, 22, STRING["dialogs.character.legend2"]);
		writeString(0, 23, STRING["dialogs.character.legend3"]);
		writeString(0, 24, STRING["dialogs.character.legend4"]);
		break;

	case EQUIP:
		writeString(0, 20, STRING["dialogs.character.equip"]);
		escToGoBack(0);
		break;

	case REMOVE:
		writeString(0, 20, STRING["dialogs.character.remove"]);
		escToGoBack(0);
		break;

	case SHARE:
		writeString(8, 20, STRING["dialogs.character.share_all"]);
		drawGemsGoldFood();
		break;

	case TRADE_WITH:
		// Print party
		clearLines(13, 24);
		for (uint i = 0; i < g_globals->_party.size(); ++i) {
			const Character &c = g_globals->_party[i];
			_textPos.x = (i % 2) == 0 ? 1 : 22;
			_textPos.y = 15 + (i / 2);
			writeChar(c._condition ? '*' : ' ');
			writeChar('1' + i);
			writeString(") ");
			writeString(c._name);
		}

		writeString(10, 20, Common::String::format(
			STRING["dialogs.character.trade_with"].c_str(),
			(int)g_globals->_party.size()
		));
		break;

	case TRADE_KIND:
		writeString(6, 20, STRING["dialogs.character.trade_which"]);
		drawGemsGoldFood();
		writeString(20, 23, STRING["dialogs.character.item"]);
		escToGoBack(0);
		break;

	case TRADE_ITEM:
		writeString(10, 20, STRING["dialogs.character.which"]);
		escToGoBack(0);
		break;

	case USE:
		g_globals->_combatEffectCtr = 0;
		g_globals->_nonCombatEffectCtr = 0;
		writeString(7, 20, STRING["dialogs.character.use_what"]);
		escToGoBack(0);
		break;

	default:
		break;
	}
}

void CharacterInfo::timeout() {
	switch (_state) {
	case USE:
		if (dynamic_cast<Views::Combat *>(g_events->priorView()) != nullptr) {
			close();
		} else {
			_state = DISPLAY;
			redraw();
		}
		break;
	default:
		return CharacterBase::timeout();
	}
}

bool CharacterInfo::msgKeypress(const KeypressMessage &msg) {
	if (msg.keycode == Common::KEYCODE_ESCAPE) {
		if (_state != DISPLAY) {
			redraw();
		} else {
			close();
		}

		_state = DISPLAY;
		return true;
	}

	switch (_state) {
	case DISPLAY:
		switch (msg.keycode) {
		case Common::KEYCODE_c:
			send("CastSpell", ValueMessage(0));
			break;
		case Common::KEYCODE_e:
			if (!g_globals->_currCharacter->_backpack.empty())
				_state = EQUIP;
			redraw();
			break;
		case Common::KEYCODE_g:
			g_globals->_currCharacter->gatherGold();
			redraw();
			break;
		case Common::KEYCODE_q:
			replaceView("QuickRef");
			break;
		case Common::KEYCODE_r:
			if (!g_globals->_currCharacter->_equipped.empty())
				_state = REMOVE;
			redraw();
			break;
		case Common::KEYCODE_s:
			_state = SHARE;
			redraw();
			break;
		case Common::KEYCODE_t:
			_state = TRADE_WITH;
			redraw();
			break;
		case Common::KEYCODE_u:
			_state = USE;
			redraw();
			break;
		default:
			break;
		 }
		break;

	case EQUIP:
		if (msg.keycode >= Common::KEYCODE_a &&
				msg.keycode <= Common::KEYCODE_f)
			equipItem(msg.keycode - Common::KEYCODE_a);
		break;

	case REMOVE:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_6)
			removeItem(msg.keycode - Common::KEYCODE_1);
		break;

	case SHARE:
		if (msg.keycode >= Common::KEYCODE_1 &&
			msg.keycode <= Common::KEYCODE_3) {
			Party::share((TransferKind)(msg.keycode - Common::KEYCODE_0));
			_state = DISPLAY;
			redraw();
			break;
		}
		break;

	case TRADE_KIND:
		if (msg.keycode >= Common::KEYCODE_1 &&
				msg.keycode <= Common::KEYCODE_3) {
			_tradeKind = (TransferKind)(msg.keycode - Common::KEYCODE_0);
			tradeHowMuch();

		} else if (msg.keycode == Common::KEYCODE_4) {
			if (g_globals->_party[_tradeWith]._backpack.full()) {
				writeString(14, 21, STRING["dialogs.character.full"]);
				Sound::sound(SOUND_2);
				_state = DISPLAY;
				delaySeconds(3);
			} else {
				_state = TRADE_ITEM;
				redraw();
			}
		}
		break;

	case TRADE_ITEM:
		if (msg.keycode >= Common::KEYCODE_a &&
				msg.keycode <= Common::KEYCODE_f) {
			switch (g_globals->_currCharacter->trade(_tradeWith,
				msg.keycode - Common::KEYCODE_a)) {
			case Character::TRADE_FULL:
				writeString(14, 21, STRING["dialogs.character.full"]);
				_state = DISPLAY;
				delaySeconds(3);
				break;
			case Character::TRADE_SUCCESS:
				_state = DISPLAY;
				redraw();
				break;
			default:
				// Do nothing, no item at selected index
				break;
			}
		}
		break;

	case USE: {
		Character &c = *g_globals->_currCharacter;
		Inventory *inv;
		Inventory::Entry *invEntry;
		if (msg.keycode >= Common::KEYCODE_1 && msg.keycode <= Common::KEYCODE_6 &&
			(msg.keycode - Common::KEYCODE_1) < (int)c._equipped.size()) {
			inv = &c._equipped;
			invEntry = &c._equipped[msg.keycode - Common::KEYCODE_1];
		} else if (msg.keycode >= Common::KEYCODE_a && msg.keycode <= Common::KEYCODE_f &&
				(msg.keycode - Common::KEYCODE_a) < (int)c._backpack.size()) {
			inv = &c._backpack;
			invEntry = &c._backpack[msg.keycode - Common::KEYCODE_a];
		} else {
			break;
		}

		if (dynamic_cast<Views::Combat *>(g_events->priorView()) != nullptr)
			combatUseItem(*inv, *invEntry, msg.keycode >= Common::KEYCODE_a);
		else
			nonCombatUseItem(*inv, *invEntry, msg.keycode >= Common::KEYCODE_a);
		break;
	}
	default:
		break;
	}

	return true;
}

bool CharacterInfo::msgAction(const ActionMessage &msg) {
	if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6) {
		if (_state == DISPLAY) {
			g_globals->_currCharacter = &g_globals->_party[
				msg._action - KEYBIND_VIEW_PARTY1];
			redraw();
			return true;
		} else if (_state == TRADE_WITH) {
			_state = TRADE_KIND;
			_tradeWith = msg._action - KEYBIND_VIEW_PARTY1;
			redraw();
			return true;
		}
	}
	return false;
}

bool CharacterInfo::msgGame(const GameMessage &msg) {
	if (msg._name == "USE") {
		addView();
		_state = USE;
		redraw();
		return true;
	}

	return false;
}

void CharacterInfo::equipItem(uint index) {
	Character &c = *g_globals->_currCharacter;
	uint itemId = c._backpack[index]._id;
	uint item14 = c._backpack[index]._charges;

	int classBit = 0;
	Common::String equipError;
	_textPos.x = 0;

	switch (c._class) {
	case KNIGHT:
		classBit = KNIGHT_BIT;
		break;
	case PALADIN:
		classBit = PALADIN_BIT;
		break;
	case ARCHER:
		classBit = ARCHER_BIT;
		break;
	case CLERIC:
		classBit = CLERIC_BIT;
		break;
	case SORCERER:
		classBit = SORCERER_BIT;
		break;
	case ROBBER:
		classBit = ROBBER_BIT;
		break;
	default:
		equipError = STRING["dialogs.character.wrong_class"];
		break;
	}

	g_globals->_items.getItem(itemId);
	const Item &item = g_globals->_currItem;

	if (equipError.empty() && (item._disablements & classBit))
		equipError = STRING["dialogs.character.wrong_class"];
	if (equipError.empty()) {
		int alignBit = 0;
		switch (c._alignment) {
		case GOOD:
			alignBit = GOOD_BIT;
			break;
		case NEUTRAL:
			alignBit = NEUTRAL_BIT;
			break;
		case EVIL:
			alignBit = EVIL_BIT;
			break;
		default:
			equipError = STRING["dialogs.character.wrong_alignment"];
			break;
		}

		if ((item._disablements & alignBit) && alignBit != NEUTRAL_BIT)
			equipError = STRING["dialogs.character.wrong_alignment"];
	}

	if (equipError.empty()) {
		if (item._equipMode == IS_EQUIPPABLE) {
			equipError = STRING["dialogs.character.not_equipped"];
			_textPos.x = 10;
		}
	}

	if (equipError.empty()) {
		if (isWeapon(itemId)) {
			if (c._equipped.hasWeapon() || c._equipped.hasTwoHanded())
				equipError = STRING["dialogs.character.have_weapon"];
		} else if (isMissile(itemId)) {
			if (c._equipped.hasMissile()) {
				equipError = STRING["dialogs.character.have_missile"];
				_textPos.x = 3;
			}
		} else if (isTwoHanded(itemId)) {
			if (c._equipped.hasShield()) {
				equipError = STRING["dialogs.character.cannot_with_shield"];
				_textPos.x = 7;
			} else if (c._equipped.hasWeapon()) {
				equipError = STRING["dialogs.character.have_weapon"];

			}
		} else if (isArmor(itemId)) {
			if (c._equipped.hasArmor()) {
				equipError = STRING["dialogs.character.have_armor"];
				_textPos.x = 5;
			}
		} else if (isShield(itemId)) {
			if (c._equipped.hasTwoHanded()) {
				equipError = STRING["dialogs.character.cannot_two_handed"];
				_textPos.x = 1;
			}
		} else if (itemId == 255) {
			equipError = STRING["dialogs.character.not_equipped"];
			_textPos.x = 10;
		}
	}

	if (equipError.empty() && c._equipped.full()) {
		equipError = STRING["dialogs.character.full"];
		_textPos.x = 14;
	}

	if (equipError.empty()) {
		// All checks passed, can equip item
		c._backpack.removeAt(index);
		uint freeIndex = c._equipped.add(itemId, item14);

		if (item._equipMode != EQUIPMODE_0) {
			if (item._equipMode == IS_EQUIPPABLE) {
				equipError = STRING["dialogs.character.not_equipped"];
				_textPos.x = 10;
			} else if (item._equipMode == EQUIP_CURSED) {
				c._equipped[freeIndex]._charges += item._val10;
			}
		}
	}

	if (equipError.empty()) {
		switch (getItemCategory(itemId)) {
		case ITEMCAT_WEAPON:
		case ITEMCAT_TWO_HANDED:
			c._physicalAttr._base = item._val16;
			c._physicalAttr._current = item._val17;
			break;
		case ITEMCAT_MISSILE:
			c._missileAttr._base = item._val16;
			c._missileAttr._current = item._val17;
			break;
		case ITEMCAT_ARMOR:
		case ITEMCAT_SHIELD:
			c._ac._base += item._val17;
			break;
		default:
			break;
		}

		c.updateResistances();
		c.updateAttributes();
		c.updateAC();

	} else {
		clearLines(20, 24);
		_textPos.y = 21;
		writeString(equipError);

		Sound::sound(SOUND_2);
		delaySeconds(3);
	}

	_state = DISPLAY;
}

void CharacterInfo::removeItem(uint index) {
	Character &c = *g_globals->_currCharacter;
	uint itemId = c._equipped[index]._id;
	uint item14 = c._equipped[index]._charges;

	Common::String removeError;

	g_globals->_items.getItem(itemId);
	const Item &item = g_globals->_currItem;
	if (item._equipMode == EQUIP_CURSED) {
		removeError = STRING["dialogs.character.cursed"];
		_textPos.x = 13;
	} else if (c._backpack.full()) {
		removeError = STRING["dialogs.character.full"];
		_textPos.x = 14;
	}

	_state = DISPLAY;

	if (!removeError.empty()) {
		clearLines(20, 24);
		_textPos.y = 21;
		writeString(removeError);

		Sound::sound(SOUND_2);
		delaySeconds(3);
		return;
	}

	// Shift item to backpack
	c._equipped.removeAt(index);
	c._backpack.add(itemId, item14);

	if (item._val10) {
		// TODO: _equipMode is used as a character offset. Need to
		// find an example that calls it so I know what area of
		// the character updates are being done to
		error("TODO: item flag in remove item");
	}

	switch (getItemCategory(itemId)) {
	case ITEMCAT_WEAPON:
	case ITEMCAT_TWO_HANDED:
		c._physicalAttr.clear();
		break;
	case ITEMCAT_MISSILE:
		c._missileAttr.clear();
		break;
	case ITEMCAT_ARMOR:
	case ITEMCAT_SHIELD:
		c._ac._base = MAX((int)c._ac._base - (int)item._val17, 0);
		break;
	default:
		break;
	}
}

void CharacterInfo::drawGemsGoldFood() {
	writeString(20, 20, STRING["dialogs.character.gems"]);
	writeString(20, 21, STRING["dialogs.character.gold"]);
	writeString(20, 22, STRING["dialogs.character.food"]);
}

void CharacterInfo::howMuchAborted() {
	_state = TRADE_WITH;
	redraw();
}

void CharacterInfo::howMuchEntered(uint amount) {
	Character &src = *g_globals->_currCharacter;
	Character &dest = g_globals->_party[_tradeWith];

	switch (_tradeKind) {
	case TK_GEMS:
		if (amount > src._gems || ((int)dest._gems + amount) > 0xffff) {
			Sound::sound(SOUND_2);
		} else {
			src._gems -= amount;
			dest._gems += amount;
		}
		break;
	case TK_GOLD:
		if (amount > src._gold || (dest._gold + amount) > 0xffffff) {
			Sound::sound(SOUND_2);
		} else {
			src._gold -= amount;
			dest._gold += amount;
		}
		break;
	case TK_FOOD:
		if (amount > src._food || (dest._food + amount) > 40) {
			Sound::sound(SOUND_2);
		} else {
			src._food -= amount;
			dest._food += amount;
		}
		break;

	default:
		break;
	}

	_state = DISPLAY;
	redraw();
}

void CharacterInfo::tradeHowMuch() {
	clearLines(20, 24);
	escToGoBack(0);
	writeString(10, 20, STRING["dialogs.character.how_much"]);

	_textEntry.display(20, 20, 5, true,
		[]() {
			CharacterInfo *view =
				(CharacterInfo *)g_events->focusedView();
			view->howMuchAborted();
		},
		[](const Common::String &text) {
			CharacterInfo *view =
				(CharacterInfo *)g_events->focusedView();
			view->howMuchEntered(atoi(text.c_str()));
		}
	);
}

void CharacterInfo::combatUseItem(Inventory &inv, Inventory::Entry &invEntry, bool isEquipped) {
	Item *item = g_globals->_items.getItem(invEntry._id);
	Common::String msg;

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

				clearLines(20, 24);
				writeString(14, 22, STRING["dialogs.character.use_combat.done"]);
				Sound::sound(SOUND_2);
				g_globals->_party.updateAC();
				delaySeconds(2);
				return;
			}
		} else {
			msg = STRING["dialogs.character.use_combat.no_charges_left"];
		}
	} else {
		msg = STRING["dialogs.character.use_combat.not_equipped"];
	}

	clearLines(20, 24);
	static_cast<Views::Combat *>(g_events->priorView())->disableAttacks();

	writeString(8, 21, msg);
	delaySeconds(3);
}

void CharacterInfo::nonCombatUseItem(Inventory &inv, Inventory::Entry &invEntry, bool isEquipped) {
	Item *item = g_globals->_items.getItem(invEntry._id);
	Common::String msg;

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

				clearLines(20, 24);
				writeString(14, 22, STRING["spells.done"]);
				Sound::sound(SOUND_2);
				g_globals->_party.updateAC();
				delaySeconds(2);
				return;
			}
		} else {
			msg = STRING["dialogs.character.use_noncombat.no_charges_left"];
		}
	} else {
		msg = STRING["dialogs.character.use_noncombat.not_equipped"];
	}

	clearLines(20, 24);
	static_cast<Views::Combat *>(g_events->priorView())->disableAttacks();

	writeString(9, 21, msg);
	delaySeconds(3);
}

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM
