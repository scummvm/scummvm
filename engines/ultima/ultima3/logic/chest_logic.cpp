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

#include "ultima/ultima3/logic/chest_logic.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {

int ChestLogic::rollBelow(int limit) {
	return Graphics::Views::g_events->getRandomNumber(limit - 1);
}

bool ChestLogic::evadesTrap(int slot) {
	const Data::RosterEntry &e = _G(savegame).partyMember(slot);
	int chance = Data::fromBcd(e._dexterity);

	switch (e._class) {
	case 'T':
		chance += 0x80;
		break;
	case 'B':
	case 'I':
	case 'R':
		chance += 0x40;
		break;
	default:
		break;
	}

	return rollBelow(255) < chance;
}

void ChestLogic::damageParty() {
	Data::Savegame &save = _G(savegame);

	for (int slot = 0; slot < save._partySize; ++slot) {
		if (!save.partyMember(slot).isAlive())
			continue;

		_G(effects).flashSlot(slot);
		g_engine->playSoundEffect(0xF7);

		damageCharacter(slot, Data::fromBcd(rollBelow(255) & 0x77));
		damageCharacter(slot, Data::fromBcd(8));
	}
}

void ChestLogic::loot(int slot) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);

	int gold = rollBelow(100) | 0x30;
	if (gold >= 100)
		gold -= 100;

	_G(messages).print(Common::String::format("Gold+%02X\n", Data::toBcd(gold)).c_str());
	e._gold = Data::toBcdWord(MIN(Data::fromBcdWord(e._gold) + gold, 9999));

	if (rollBelow(255) >= 0x40)
		return;

	// Sometimes a weapon is found as well, or failing that some armour
	int roll = rollBelow(255);
	if (roll < 0x80) {
		int weapon = rollBelow(255) & roll & 7;

		if (weapon != 0) {
			_G(messages).print("and a ");
			_G(messages).print(Data::WEAPON_NAMES[weapon]);
			_G(messages).print("\n");
			e._weaponOwned[weapon - 1] = Data::toBcd(MIN(Data::fromBcd(e._weaponOwned[weapon - 1]) + 1, 99));
			return;
		}
	}

	roll = rollBelow(255);
	if (roll >= 0x80)
		return;

	int armour = rollBelow(255) & roll & 3;
	if (armour != 0) {
		_G(messages).print("and ");
		_G(messages).print(Data::ARMOUR_NAMES[armour]);
		_G(messages).print("\n");
		e._armourOwned[armour - 1] = Data::toBcd(MIN(Data::fromBcd(e._armourOwned[armour - 1]) + 1, 99));
	}
}

void ChestLogic::open(int slot) {
	Data::Savegame &save = _G(savegame);
	Data::RosterEntry &e = save.partyMember(slot);

	// Half of them are trapped
	if (rollBelow(255) < 0x80) {
		loot(slot);
		return;
	}

	int first = rollBelow(255);
	int second = rollBelow(255);
	bool acid = (first & second & 3) == 0;

	if (acid)
		_G(messages).print("Acid trap!\n");
	else if (first == 1)
		_G(messages).print("Poison trap!\n");
	else if (first == 2)
		_G(messages).print("Gas trap!\n");
	else
		_G(messages).print("Bomb trap!\n");

	if (evadesTrap(slot)) {
		_G(messages).print("Trap evaded!\n");
		g_engine->playSoundEffect(0xFA);
	} else if (acid) {
		_G(effects).flashSlot(slot);
		g_engine->playSoundEffect(0xF7);
		damageCharacter(slot, Data::fromBcd(rollBelow(255) & 0x37));
	} else if (first == 1) {
		_G(effects).flashSlot(slot);
		g_engine->playSoundEffect(0xF7);
		e._status = Data::STATUS_POISONED;
	} else if (first == 2) {
		for (int i = 0; i < save._partySize; ++i) {
			Data::RosterEntry &member = save.partyMember(i);

			if (member.isAlive()) {
				_G(effects).flashSlot(i);
				g_engine->playSoundEffect(0xF7);
				member._status = Data::STATUS_POISONED;
			}
		}
	} else {
		damageParty();
	}

	loot(slot);
}

} // namespace Ultima3
} // namespace Ultima
