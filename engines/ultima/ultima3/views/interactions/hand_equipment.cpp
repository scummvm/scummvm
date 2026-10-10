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

#include "ultima/ultima3/views/interactions/hand_equipment.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

constexpr char ESCAPE_KEY = 0x1B;
constexpr int MAX_QUANTITY = 9999;
constexpr int MAX_ITEMS = 99;

void HandEquipment::startAmount(int digits, bool rejectLetters) {
	_G(messages).print(digits == 4 ? "\nHow much? " : (_kind == WEAPONS || _kind == ARMOUR) ?
		"\nHow many? " : "\nHow much? ");
	_number.setup(digits, rejectLetters);
	_stage = ENTER_AMOUNT;
}

void HandEquipment::startEquipmentMenu() {
	const Data::Savegame &save = _G(savegame);
	const bool weapons = _kind == WEAPONS;
	int letters;

	if (weapons)
		letters = save._allWeapons ? 16 : (save._plusTwoWeapons ? 15 : 8);
	else
		letters = save._allArmour ? 8 : (save._plusTwoArmour ? 7 : 5);

	_keys[0] = 'Q';
	_keys[1] = ESCAPE_KEY;
	_words[0] = _words[1] = "Quit";

	for (int i = 0; i < letters; ++i) {
		_keys[2 + i] = 'A' + i;
		_words[2 + i] = weapons ? Data::WEAPON_NAMES[i] : Data::ARMOUR_NAMES[i];
	}

	_G(messages).print("\n");
	_menu.setup(_keys, _words, 2 + letters);
	_stage = CHOOSE_EQUIPMENT;
}

void HandEquipment::transfer() {
	Data::Savegame &save = _G(savegame);
	Data::RosterEntry &from = save.partyMember(_giver);
	Data::RosterEntry &to = save.partyMember(_receiver);

	int amount = _number.value();
	int have, room;
	uint16 *wordFrom = nullptr, *wordTo = nullptr;
	byte *byteFrom = nullptr, *byteTo = nullptr;
	int limit = MAX_ITEMS;

	switch (_kind) {
	case FOOD:
		wordFrom = &from._food;
		wordTo = &to._food;
		break;
	case GOLD:
		wordFrom = &from._gold;
		wordTo = &to._gold;
		break;
	case GEMS:
		byteFrom = &from._gems;
		byteTo = &to._gems;
		break;
	case KEYS:
		byteFrom = &from._keys;
		byteTo = &to._keys;
		break;
	case POWDER:
		byteFrom = &from._powder;
		byteTo = &to._powder;
		break;
	case TORCHES:
		byteFrom = &from._torches;
		byteTo = &to._torches;
		break;
	case WEAPONS:
		byteFrom = &from._weaponOwned[_which - 1];
		byteTo = &to._weaponOwned[_which - 1];
		break;
	default:
		byteFrom = &from._armourOwned[_which - 1];
		byteTo = &to._armourOwned[_which - 1];
		break;
	}

	if (wordFrom) {
		have = Data::fromBcdWord(*wordFrom);
		room = Data::fromBcdWord(*wordTo);
		limit = MAX_QUANTITY;
	} else {
		have = Data::fromBcd(*byteFrom);
		room = Data::fromBcd(*byteTo);
	}

	if (amount > have) {
		_G(messages).print("\nNot enough!");
		g_engine->playSoundEffect(0xFE);
		_G(messages).print("\n");
	} else if (room + amount > limit) {
		_G(messages).print("\nNo more room!\n");
		g_engine->playSoundEffect(0xFE);
	} else {
		if (wordFrom) {
			*wordFrom = Data::toBcdWord(have - amount);
			*wordTo = Data::toBcdWord(room + amount);
		} else {
			*byteFrom = Data::toBcd(have - amount);
			*byteTo = Data::toBcd(room + amount);
		}

		_G(messages).print("\nDone!\n");
	}
}

bool HandEquipment::keypress(const KeypressMessage &msg) {
	switch (_stage) {
	case CHOOSE_GIVER: {
		PlayerChooser::Result result = _players.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		_giver = _players.slot();
		_G(messages).print("  To Player: ");
		_stage = CHOOSE_RECEIVER;
		return false;
	}

	case CHOOSE_RECEIVER: {
		PlayerChooser::Result result = _players.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		_receiver = _players.slot();
		if (_receiver == _giver) {
			_G(messages).print("<-What?\n");
			g_engine->playSoundEffect(0xFE);
			return true;
		}

		static const char *const WORDS[7] = { "Food", "Gold", "Equipment", "Weapons", "Armour", "Quit", "Quit" };
		_G(messages).print("F, G, E, W, A:\n");
		_menu.setup("FGEWAQ\033", WORDS, 7);
		_stage = CHOOSE_KIND;
		return false;
	}

	case CHOOSE_KIND: {
		if (!_menu.handleKey(msg))
			return false;

		switch (_menu.key()) {
		case 'F':
		case 'G':
			_kind = (_menu.key() == 'F') ? FOOD : GOLD;
			startAmount(4, true);
			return false;

		case 'E': {
			static const char *const WORDS[6] = { "Gems", "Keys", "Powder", "Torch", "Quit", "Quit" };
			_G(messages).print("\nG, K, P, T:\n");
			_menu.setup("GKPTQ\033", WORDS, 6);
			_stage = CHOOSE_ITEM;
			return false;
		}

		case 'W':
		case 'A':
			_kind = (_menu.key() == 'W') ? WEAPONS : ARMOUR;
			startEquipmentMenu();
			return false;

		default:
			_G(messages).print("\n");
			return true;
		}
	}

	case CHOOSE_ITEM: {
		if (!_menu.handleKey(msg))
			return false;

		switch (_menu.key()) {
		case 'G': _kind = GEMS; break;
		case 'K': _kind = KEYS; break;
		case 'P': _kind = POWDER; break;
		case 'T': _kind = TORCHES; break;
		default:
			_G(messages).print("\n");
			return true;
		}

		startAmount(2, false);
		return false;
	}

	case CHOOSE_EQUIPMENT: {
		if (!_menu.handleKey(msg))
			return false;

		char key = _menu.key();
		if (key == 'Q' || key == ESCAPE_KEY) {
			_G(messages).print("\n");
			return true;
		}
		if (key == 'A') {
			_G(messages).print("<-What?\n");
			g_engine->playSoundEffect(0xFE);
			return true;
		}

		_which = key - 'A';
		startAmount(2, false);
		return false;
	}

	default: {
		if (!_number.handleKey(msg))
			return false;

		if (_number.hasLetters()) {
			g_engine->playSoundEffect(0xFE);
			_G(messages).print("\n");
			return true;
		}

		// Weapons and armour need to be amounts of more than none, and
		// the one in use can't be passed on
		if (_kind == WEAPONS || _kind == ARMOUR) {
			if (_number.value() == 0) {
				_G(messages).print("\n");
				return true;
			}

			const Data::RosterEntry &from = _G(savegame).partyMember(_giver);
			if ((_kind == WEAPONS ? from._weaponIndex : from._armourIndex) == _which) {
				g_engine->playSoundEffect(0xFE);
				_G(messages).print("\nIn use!\n");
				return true;
			}
		}

		transfer();
		return true;
	}
	}
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
