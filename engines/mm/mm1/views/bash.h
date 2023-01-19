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

#ifndef MM1_VIEWS_BASH_H
#define MM1_VIEWS_BASH_H

#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

class Bash : public TextView {
private:
	/**
	 * Handle bashing the door
	 */
	void bashDoor();

	/**
	 * When there's no door to bash, the bash action
	 * is transformed into a simple forwards movement
	 */
	void forwards();
public:
	Bash();
	virtual ~Bash() {}

	/**
	 * Handles game messages
	 */
	bool msgGame(const GameMessage &msg) override;

	/**
	 * The view doesn't actually have any rendering,
	 * so the draw method has no implementation
	 */
	void draw() override {}
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
