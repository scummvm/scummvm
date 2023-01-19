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

#ifndef MM1_VIEWS_COMBAT_H
#define MM1_VIEWS_COMBAT_H

#include "mm/mm1/game/combat.h"
#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

class Combat : public TextView, public Game::Combat {
private:
	enum Mode {
		BATTLE
	};
	Mode _mode = BATTLE;

	void writeOptions();
	void writeAttackOptions();
	void writeCastOption();
	void writeShootOption();
	void resetBottom();
	void writeHandicap();

public:
	Combat();
	virtual ~Combat() {}

	/**
	 * Called when the view is focused
	 */
	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Draw the Combat details overlayed on
	 * the existing game screen
	 */
	void draw() override;

	/**
	 * Handles delay timeouts
	 */
	void timeout() override;

	/**
	 * Handles keypresses
	 */
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Views
} // namespace MM1
} // namespace MM

#endif
