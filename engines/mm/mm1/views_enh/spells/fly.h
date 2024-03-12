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

#ifndef MM1_VIEWS_ENH_SPELLS_FLY_H
#define MM1_VIEWS_ENH_SPELLS_FLY_H

#include "mm/mm1/views_enh/scroll_view.h"
#include "mm/mm1/game/fly.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {
namespace Spells {

class Fly : public ScrollView, public MM1::Game::Fly {
private:
	enum Mode { SELECT_X, SELECT_Y, CAST };
	Mode _mode = SELECT_X;
	int _xIndex = 0, _yIndex = 0;

public:
	/**
	 * Constructor
	 */
	Fly();

	/**
	 * Destructor
	 */
	virtual ~Fly() {}

	/**
	 * Show the view
	 */
	bool msgFocus(const FocusMessage &) override;

	/**
	 * Draw the view contents
	 */
	void draw() override;

	/**
	 * Keypress handler
	 */
	bool msgKeypress(const KeypressMessage &msg) override;

	/**
	 * Action handler
	 */
	bool msgAction(const ActionMessage &msg) override;

	/**
	 * Timeout handler
	 */
	void timeout() override;
};

} // namespace Spells
} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
