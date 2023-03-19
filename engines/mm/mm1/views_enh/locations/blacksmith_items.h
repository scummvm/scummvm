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

#ifndef MM1_VIEWS_ENH_LOCATIONS_BLACKSMITH_ITEMS_H
#define MM1_VIEWS_ENH_LOCATIONS_BLACKSMITH_ITEMS_H

#include "mm/mm1/views_enh/items_view.h"
#include "mm/mm1/data/locations.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

class BlacksmithItems : public ItemsView,
	public BlacksmithData, public BuyWeaponData,
	public BuyArmorData, public BuyMiscData {
private:
	enum BlacksmithMode {
		WEAPONS_MODE = 0, ARMOR_MODE = 1, MISC_MODE = 2,
		SELL_MODE = 3
	};
	BlacksmithMode _mode = WEAPONS_MODE;
	int _buySellItem = -1;

	/**
	 * Populates the list of items
	 */
	void populateItems();

	/**
	 * Displays the title row
	 */
	void drawTitle();

	/**
	 * Called if the buy/sell action has been confirmed
	 */
	void itemConfirmed();

protected:
	/**
	 * Get the text color for a line
	 */
	int getLineColor() const override;

	/**
	 * Called when an item is selected
	 */
	void itemSelected() override;

public:
	BlacksmithItems();
	virtual ~BlacksmithItems() {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Location
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
