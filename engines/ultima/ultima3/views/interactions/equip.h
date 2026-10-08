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

#ifndef ULTIMA3_VIEWS_INTERACTIONS_EQUIP_H
#define ULTIMA3_VIEWS_INTERACTIONS_EQUIP_H

#include "ultima/ultima3/views/interactions/interaction.h"
#include "ultima/ultima3/views/menu_input.h"

namespace Ultima {
namespace Ultima3 {
namespace Views {
namespace Interactions {

/**
 * Has a party member take up a weapon, or put on a set of armour
 */
class Equip : public Interaction {
private:
	bool _weapons;
	PlayerChooser _chooser;
	bool _choosing = true;
	MenuChoice _menu;
	char _keys[20];
	const char *_words[19];

	/**
	 * Sets up the menu of what can be picked once a party member is chosen
	 */
	void startMenu();

	/**
	 * Makes the item picked in the menu the one in use, if allowed
	 */
	void equip(char letter);

public:
	/**
	 * Constructor
	 * @param weapons	True for weapons, false for armour
	 */
	Equip(bool weapons) : _weapons(weapons) {}

	bool keypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima3
} // namespace Ultima

#endif
