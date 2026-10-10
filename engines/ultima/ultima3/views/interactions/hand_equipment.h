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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_HAND_EQUIPMENT_H
#define ULTIMA3_VIEWS_INTERACTIONS_HAND_EQUIPMENT_H

#include "ultima/ultima3/views/interactions/log_input.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * Passes food, gold or belongings from one party member to another
 */
class HandEquipment : public Interaction {
private:
	enum Stage {
		CHOOSE_GIVER,
		CHOOSE_RECEIVER,
		CHOOSE_KIND,
		CHOOSE_ITEM,
		CHOOSE_EQUIPMENT,
		ENTER_AMOUNT
	};

	// What is being passed on
	enum Kind {
		FOOD,
		GOLD,
		GEMS,
		KEYS,
		POWDER,
		TORCHES,
		WEAPONS,
		ARMOUR
	};

	Stage _stage = CHOOSE_GIVER;
	Kind _kind = FOOD;
	int _giver = 0, _receiver = 0;

	// For weapons and armour, which of them
	int _which = 0;

	PlayerChooser _players;
	LogMenu _menu;
	LogNumber _number;
	char _keys[20];
	const char *_words[19];

	void startAmount(int digits, bool rejectLetters);
	void startEquipmentMenu();

	/**
	 * Passes the amount entered, saying how it went
	 */
	void transfer();

public:
	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
