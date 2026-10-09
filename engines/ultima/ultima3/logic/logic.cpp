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

#include "ultima/ultima3/logic/logic.h"
#include "ultima/ultima3/ultima3.h"

namespace Ultima {
namespace Ultima3 {

int Logic::_healCounter = 9;
int Logic::_upkeepCounter = 4;

constexpr int HUNGER_STEP = 10;
constexpr int STARVATION_DAMAGE = 5;

// Magic points regenerate up to half of the attribute they depend on
static int maxMagicPoints(byte attribute) {
	return Data::fromBcd(attribute) / 2;
}

void Logic::incrementMoveCounter(int amount) {
	Data::Savegame &save = _G(savegame);
	int carry = (amount < 0) ? save._partySize : amount;

	for (int i = 0; i < 4 && carry; ++i) {
		int total = Data::fromBcd(save._moveCount[i]) + carry;
		save._moveCount[i] = Data::toBcd(total % 100);
		carry = total / 100;
	}
}

void Logic::regenerateMagicPoint(Data::RosterEntry &e) {
	if (e.isAlive())
		e._magicPoints = Data::toBcd(Data::fromBcd(e._magicPoints) + 1);
}

void Logic::applyHunger(int slot) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);

	// Food only drops by one every few turns, tracked in a hidden counter
	int counter = Data::fromBcd(e._foodSubCounter) - HUNGER_STEP;
	int borrow = 0;
	if (counter < 0) {
		counter += 100;
		borrow = 1;
	}
	e._foodSubCounter = Data::toBcd(counter);

	int food = Data::fromBcdWord(e._food) - borrow;
	if (food < 0) {
		e._food = 0;
		_G(messages).print("Starving!\n");
		_G(effects).flashSlot(slot);
		g_engine->playSoundEffect(0xF7);
		damageCharacter(slot, STARVATION_DAMAGE);
	} else {
		e._food = Data::toBcdWord(food);
	}
}

bool Logic::damageCharacter(int slot, int amount) {
	Data::RosterEntry &e = _G(savegame).partyMember(slot);
	int hitPoints = Data::fromBcdWord(e._hitPoints) - amount;

	if (hitPoints < 0) {
		e._status = Data::STATUS_DEAD;
		e._hitPoints = 0;
		return true;
	}

	e._hitPoints = Data::toBcdWord(hitPoints);
	return false;
}

void Logic::processPartyTurnEffects(bool everyTurn) {
	if (!everyTurn) {
		if (--_upkeepCounter != 0)
			return;
		_upkeepCounter = 4;
	}

	if (--_healCounter < 0)
		_healCounter = 9;

	Data::Savegame &save = _G(savegame);

	for (int slot = save._partySize - 1; slot >= 0; --slot) {
		Data::RosterEntry &e = save.partyMember(slot);
		if (!e.isAlive())
			continue;

		int magic = Data::fromBcd(e._magicPoints);
		int intelligence = Data::fromBcd(e._intelligence);
		int wisdom = Data::fromBcd(e._wisdom);

		if (e._class == 'W' && magic < intelligence)
			regenerateMagicPoint(e);

		if (e._class == 'C' && Data::fromBcd(e._magicPoints) < wisdom)
			regenerateMagicPoint(e);

		if ((e._class == 'L' || e._class == 'D' || e._class == 'A') &&
				maxMagicPoints(e._intelligence) > Data::fromBcd(e._magicPoints))
			regenerateMagicPoint(e);

		if ((e._class == 'P' || e._class == 'I' || e._class == 'D') &&
				maxMagicPoints(e._wisdom) > Data::fromBcd(e._magicPoints))
			regenerateMagicPoint(e);

		if (e._class == 'R' && maxMagicPoints(e._wisdom) > Data::fromBcd(e._magicPoints) &&
				maxMagicPoints(e._intelligence) > Data::fromBcd(e._magicPoints))
			regenerateMagicPoint(e);

		applyHunger(slot);

		if (e._status == Data::STATUS_POISONED) {
			damageCharacter(slot, 1);
			_G(effects).flashSlot(slot);
			_G(messages).print("Poisoned!\n");
		}

		if (_healCounter == 0 && Data::fromBcdWord(e._hitPoints) < Data::fromBcdWord(e._maxHitPoints))
			e._hitPoints = Data::toBcdWord(Data::fromBcdWord(e._hitPoints) + 1);
	}
}

} // namespace Ultima3
} // namespace Ultima
