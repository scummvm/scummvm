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

#ifndef MM1_VIEWS_ENH_LOCATIONS_TEMPLE_H
#define MM1_VIEWS_ENH_LOCATIONS_TEMPLE_H

#include "mm/mm1/views_enh/locations/location.h"
#include "mm/mm1/data/character.h"
#include "mm/mm1/data/locations.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Locations {

class Temple : public Location, public TempleData {
private:
	bool _isEradicated = false;
	int _healCost = 0, _uncurseCost = 0;
	int _alignmentCost = 0, _donateCost = 0;
private:
	void updateCosts();
	void restoreHealth();
	void uncurseItems();
	void restoreAlignment();
	void donate();

public:
	Temple();

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgGame(const GameMessage &msg) override;
};

} // namespace Locations
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
