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

#ifndef MM1_VIEWS_LOCATIONS_INN_H
#define MM1_VIEWS_LOCATIONS_INN_H

#include "mm/mm1/views/text_view.h"
#include "mm/mm1/views/character_view.h"
#include "mm/mm1/data/int_array.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Locations {

class Inn : public TextView {
private:
	CharacterView _characterView;
	Common::Array<uint> _charNums;
	IntArray _partyChars;
public:
	Inn() : TextView("Inn") {
	}
	virtual ~Inn() {
	}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Locations
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
