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

#ifndef MM1_VIEWS_ENH_SEARCH_H
#define MM1_VIEWS_ENH_SEARCH_H

#include "mm/mm1/views_enh/select_number.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class Search : public SelectNumber {
private:
	enum Mode {
		INITIAL, OPTIONS, RESPONSE, FOCUS_GET_TREASURE,
		GET_TREASURE, GET_ITEMS, GET_ITEMS_DONE
	};
	Mode _mode = INITIAL;
	bool _removing = false;
	byte _val1 = 0;
	int _lineNum = 0;
	Shared::Xeen::SpriteResource _escSprite;

	void setMode(Mode mode);

	/**
	 * Open the container
	 */
	void openContainer();
	void openContainer2();

	/**
	 * Find/remove trap
	 */
	void findRemoveTrap();
	void findRemoveTrap2();

	/**
	 * Detect magic/trap
	 */
	void detectMagicTrap();

	/**
	 * Select which user to try and remove trap or detect magic
	 */
	bool whoWillTry();

	/**
	 * Finally give the treasure
	 */
	void getTreasure();

	/**
	 * Draw the treasure result
	 */
	void drawTreasure();

	/**
	 * Get any treasure items
	 */
	void drawItem();

public:
	Search();
	virtual ~Search() {}

	bool msgGame(const GameMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	void timeout() override;

	/**
	 * Called after a selection of who will try is done
	 */
	void whoWillTry(int charNum);
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
