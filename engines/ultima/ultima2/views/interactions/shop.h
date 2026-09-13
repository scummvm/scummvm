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

#ifndef ULTIMA2_VIEWS_INTERACTIONS_SHOP_H
#define ULTIMA2_VIEWS_INTERACTIONS_SHOP_H

#include "ultima/ultima2/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima2 {
namespace Views {
namespace Interactions {

/**
 * A generic numbered-menu shop: prints an item list, reads a digit, and
 * on a nonzero digit computes the price (Savegame::computeItemPrice),
 * charges gold, and grants the item. Covers the weapon/armour/cleric-
 * spell/wizard-spell shoppes, which all share this exact shape and only
 * differ in their item list, price-index mapping, and target array
 */
class Shop : public Interaction {
public:
	enum Kind { WEAPON, ARMOR, SPELL_CLERIC, SPELL_WIZARD };

private:
	Kind _kind;

	int priceIndexForDigit(int digit) const;

public:
	Shop(Kind kind, const Common::String &name);
	~Shop() override {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima2
} // namespace Ultima

#endif
