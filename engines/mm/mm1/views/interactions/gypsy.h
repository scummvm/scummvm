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

#ifndef MM1_VIEWS_MAPS_GYPSY_H
#define MM1_VIEWS_MAPS_GYPSY_H

#include "mm/mm1/views/text_view.h"
#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

class Gypsy : public TextView {
private:
	Character *_character = nullptr;

	void charSelected(uint charIndex);
	void modeChanged(bool allowSelection);
public:
	Gypsy();
	virtual ~Gypsy() {}

	bool msgFocus(const FocusMessage &msg) override;
	bool msgUnfocus(const UnfocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
