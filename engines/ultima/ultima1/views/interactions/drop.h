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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_DROP_H
#define ULTIMA1_VIEWS_INTERACTIONS_DROP_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Handler for the Drop command. Only usable in cities and castles
 */
class Drop : public Interaction {
	enum Mode {
		DROP_AREA, DROP_PENCE, DROP_WEAPON, DROP_ARMOUR
	};
private:
	Mode _mode = DROP_AREA;
	Common::String _amount;

	void none();
	void finish();
	void redrawStats();

	/**
	 * Handles the pence amount entry, digit by digit
	 */
	void penceKeypress(const KeypressMessage &msg);

	/**
	 * Called once a pence amount has been entered (or aborted)
	 */
	void dropPence();

	/**
	 * Bonus given for dropping pence on the special tile found in some
	 * cities - "wishing well"-style: usually gives a Hits bonus, and a
	 * small chance of a bonus spell charge
	 */
	void dropPenceCityBonus(int total);

	/**
	 * Bonus given for dropping pence on the special tiles found in
	 * castles - depending on the specific tile, gives Food, a random
	 * weapon, or an attribute bonus
	 */
	void dropPenceCastleBonus(int total);

	/**
	 * Handles a letter keypress made while selecting an owned item to
	 * drop. Unlike Ready's equivalent, an invalid/unowned choice just
	 * aborts with "none" rather than keeping a prior selection
	 */
	void selectItem(int16 *items, const char *const *names, int maxIndex,
		int16 &equippedIndex, Common::KeyCode keycode);

	/**
	 * Draws the centered, lettered list of owned items droppable in the
	 * current mode
	 */
	void drawItemList(const int16 *items, const char *const *names, int maxIndex, int col);

public:
	Drop();
	~Drop() override {
	}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
