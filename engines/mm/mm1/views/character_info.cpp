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
#include "mm/mm1/utils/strings.h"
#include "mm/mm1/globals.h"
#include "mm/mm1/sound.h"

namespace MM {
namespace MM1 {
namespace Views {

void CharacterInfo::draw() {
	assert(g_globals->_currCharacter);
	CharacterBase::draw();

	switch (_state) {
	case DISPLAY:
		writeString(0, 21, STRING["dialogs.character.legend1"]);
		writeString(0, 22, STRING["dialogs.character.legend2"]);
		writeString(0, 23, STRING["dialogs.character.legend3"]);
		writeString(0, 24, STRING["dialogs.character.legend4"]);
		break;

	case EQUIP:
		writeString(0, 20, STRING["dialogs.chracter.equip"]);
		escToGoBack(0);
		break;

	case REMOVE:
		writeString(0, 20, STRING["dialogs.chracter.remove"]);
		escToGoBack(0);
		break;
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
		case Common::KEYCODE_e:
			if (!g_globals->_currCharacter->_backpack.empty())
				_state = EQUIP;
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
		break;

	default:
		break;
	}

	return true;
}

bool CharacterInfo::msgAction(const ActionMessage &msg) {
	if (msg._action >= KEYBIND_VIEW_PARTY1 &&
			msg._action <= KEYBIND_VIEW_PARTY6 &&
			_state == DISPLAY) {
		g_globals->_currCharacter = &g_globals->_party[
			msg._action - KEYBIND_VIEW_PARTY1];
		addView();
		return true;
	}

	return false;
}

void CharacterInfo::equipItem(uint index) {
	Character &c = *g_globals->_currCharacter;
	uint itemId = c._backpack[index];
	uint item14 = c._backpack14[index];

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

	getItem(itemId);
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
		if (item._equipMode == NOT_EQUIPPABLE) {
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
		c._backpack14.removeAt(index);

		int freeIndex = c._equipped.getFreeSlot();
		c._equipped[freeIndex] = itemId;
		c._equipped14[freeIndex] = item14;

		if (item._equipMode != EQUIPMODE_0) {
			if (item._equipMode == NOT_EQUIPPABLE) {
				equipError = STRING["dialogs.character.not_equipped"];
				_textPos.x = 10;
			} else if (item._equipMode == EQUIP_CURSED) {
				c._equipped14[freeIndex] += item._val10;
			}
		}
	}

	if (equipError.empty()) {
		switch (getItemCategory(itemId)) {
		case ITEMCAT_WEAPON:
		case ITEMCAT_TWO_HANDED:
			c._v68 = item._val16;
			c._v69 = item._val17;
			break;
		case ITEMCAT_MISSILE:
			c._v6a = item._val16;
			c._v6b = item._val17;
			break;
			c._v68 = item._val16;
			c._v69 = item._val17;
			break;
		case ITEMCAT_ARMOR:
		case ITEMCAT_SHIELD:
			c._acBase += item._val17;
			break;
		default:
			break;
		}

		warning("TODO: c field58");
		c.updateAttributes();
		c.updateAC();

	} else {
		clearLines(20, 24);
		_textPos.y = 21;
		writeString(equipError);

		Sound::sound(SOUND_2);
		delaySeconds(5);
	}

	_state = DISPLAY;
}

void CharacterInfo::removeItem(uint index) {

}

} // namespace Views
} // namespace MM1
} // namespace MM
