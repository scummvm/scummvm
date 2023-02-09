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

#ifndef MM1_VIEWS_SPELLS_DUPLICATION_H
#define MM1_VIEWS_SPELLS_DUPLICATION_H

#include "mm/mm1/views/spells/spell_view.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Spells {

class Duplication : public SpellView {
private:
	enum Mode {
		SELECT_ITEM, CAST
	};
	Mode _mode = SELECT_ITEM;
	char _direction = '\0';
	int _squares = 0;

	/**
	 * Handle the Duplicationing
	 */
	void duplicate();
public:
	/**
	 * Show the view
	 */
	static void show();
public:
	/**
	 * Constructor
	 */
	Duplication();

	/**
	 * Destructor
	 */
	virtual ~Duplication() {}

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
};

} // namespace Spells
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
