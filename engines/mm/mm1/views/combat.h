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

#include "mm/mm1/events.h"
#include "mm/mm1/data/party.h"
#include "mm/mm1/game/encounter.h"
#include "mm/mm1/views/text_view.h"

namespace MM {
namespace MM1 {
namespace Views {

class Combat : public TextView {
private:
	enum Mode {
		BATTLE
	};
	Mode _mode = BATTLE;
	int _monstersCount = 0;
	Common::Array<Character *> _party;
	Monster *_monsterP;
	byte _arr1[MAX_COMBAT_MONSTERS];
	byte _arr2[MAX_COMBAT_MONSTERS];
	byte _arr3[MAX_COMBAT_MONSTERS];
	byte _arr4[MAX_PARTY_SIZE];
	int _val1, _val8, _val9, _val10;
	int _monsterIndex;

	void clear();
	void loadArrays();
	void monsterIndexOf();
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
