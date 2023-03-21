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
	switch (_state) {
	case DISPLAY:
		switch (msg.keycode) {
		case Common::KEYCODE_c:
			send("CastSpell", GameMessage("SPELL", 0));
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
	switch (msg._action) {
	case KEYBIND_ESCAPE:
		if (_state != DISPLAY) {
			redraw();
		} else {
			close();
		}

		_state = DISPLAY;
		return true;
	case KEYBIND_VIEW_PARTY1:
	case KEYBIND_VIEW_PARTY2:
	case KEYBIND_VIEW_PARTY3:
	case KEYBIND_VIEW_PARTY4:
	case KEYBIND_VIEW_PARTY5:
	case KEYBIND_VIEW_PARTY6:
		if (_state == DISPLAY) {
			g_globals->_currCharacter = &g_globals->_party[
				msg._action - KEYBIND_VIEW_PARTY1];
			redraw();
		} else if (_state == TRADE_WITH) {
			_state = TRADE_KIND;
			_tradeWith = msg._action - KEYBIND_VIEW_PARTY1;
			redraw();
		}
		return true;
	default:
		break;
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
	Common::String errMsg;
	_state = DISPLAY;

	if (!EquipRemove::equipItem(index, _textPos, errMsg)) {
		clearLines(20, 24);
		_textPos.y = 21;
		writeString(errMsg);

		Sound::sound(SOUND_2);
		delaySeconds(3);
	}
}

void CharacterInfo::removeItem(uint index) {
	Common::String errMsg;
	_state = DISPLAY;

	if (!EquipRemove::removeItem(index, _textPos, errMsg)) {
		clearLines(20, 24);
		_textPos.y = 21;
		writeString(errMsg);

		Sound::sound(SOUND_2);
		delaySeconds(3);
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
