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

#include "ultima/ultima3/views/interactions/equip.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

// The last weapon and armour letters, plus one, that each class may use
static const char CLASS_ORDER[] = "FCWTPBLIDAR";
static const char WEAPON_LIMITS[] = "QDCHQQQDDCL";
static const char ARMOUR_LIMITS[] = "IECDFDCDCCH";

constexpr char ESCAPE_KEY = 0x1B;
constexpr char EXOTIC_WEAPON = 'P';
constexpr char EXOTIC_ARMOUR = 'H';

Equip::Equip(bool weapons, int slot) : _weapons(weapons), _slot(slot) {
	if (slot >= 0) {
		_choosing = false;
		_G(messages).print(_weapons ? "Weapon:\n" : "Armour:\n");
		startMenu();
	}
}

void Equip::startMenu() {
	const Data::Savegame &save = _G(savegame);
	int letters;

	if (_weapons) {
		letters = save._allWeapons ? 16 : (save._plusTwoWeapons ? 15 : 8);
	} else {
		letters = save._allArmour ? 8 : (save._plusTwoArmour ? 7 : 5);
	}

	_keys[0] = 'Q';
	_keys[1] = ESCAPE_KEY;
	_words[0] = _words[1] = "Quit";

	for (int i = 0; i < letters; ++i) {
		_keys[2 + i] = 'A' + i;
		_words[2 + i] = _weapons ? Data::WEAPON_NAMES[i] : Data::ARMOUR_NAMES[i];
	}

	_keys[2 + letters] = '\0';
	_menu.setup(_keys, _words, 2 + letters);
}

void Equip::equip(char letter) {
	Data::RosterEntry &e = _G(savegame).partyMember(_slot);

	const char *classPos = strchr(CLASS_ORDER, e._class);
	int classNum = (classPos && *classPos) ? classPos - CLASS_ORDER : 0;
	char limit = (_weapons ? WEAPON_LIMITS : ARMOUR_LIMITS)[classNum];
	char exotic = _weapons ? EXOTIC_WEAPON : EXOTIC_ARMOUR;

	if (letter != exotic && letter >= limit) {
		_G(messages).print("\nNot allowed!\n");
		g_engine->playSoundEffect(0xFF);
		return;
	}

	int index = letter - 'A';
	byte *owned = _weapons ? e._weaponOwned : e._armourOwned;
	byte &current = _weapons ? e._weaponIndex : e._armourIndex;

	// The first item, bare hands or skin, is always held
	if (index != 0 && owned[index - 1] == 0) {
		_G(messages).print("\nNone owned\n");
		g_engine->playSoundEffect(0xFF);
		return;
	}

	_G(messages).print("\nReadied!\n");
	current = index;
}

bool Equip::keypress(const KeypressMessage &msg) {
	if (_choosing) {
		PlayerChooser::Result result = _chooser.handleKey(msg);
		if (result == PlayerChooser::PENDING)
			return false;
		if (result == PlayerChooser::CANCELLED)
			return true;

		_slot = _chooser.slot();
		if (!_G(savegame).partyMember(_slot).isAlive()) {
			_G(messages).print("Incapacitated!\n");
			g_engine->playSoundEffect(0xFF);
			return true;
		}

		_choosing = false;
		_G(messages).print(_weapons ? "Weapon:\n" : "Armour:\n");
		startMenu();
		return false;
	}

	if (!_menu.handleKey(msg))
		return false;

	char letter = _menu.key();
	if (letter == 'Q' || letter == ESCAPE_KEY)
		_G(messages).print("\n");
	else
		equip(letter);

	return true;
}

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima
