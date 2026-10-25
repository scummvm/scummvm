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

#ifndef MM1_VIEWS_ENH_BESTIARY_H
#define MM1_VIEWS_ENH_BESTIARY_H

#include "common/array.h"
#include "mm/mm1/views_enh/scroll_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

/**
 * Lists every monster in the game alphabetically. Monsters the party
 * hasn't encountered yet are greyed out
 */
class Bestiary : public ScrollView {
private:
	int _topIndex = 0;
	int _selectedIndex = 0;
	Common::Array<int> _monsterIds;

	/**
	 * Builds the alphabetical list. Only the display order is sorted, so
	 * discovery bits and image records keep their original IDs
	 */
	void buildOrder();

	/**
	 * Selects a list entry, scrolling it into view
	 */
	void select(int index);

	/**
	 * Scrolls the list by a number of entries
	 */
	void scroll(int delta);

	/**
	 * Opens the detail view for the selected monster
	 */
	void openImage();

public:
	Bestiary();
	~Bestiary() override {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
};

/**
 * Shows a monster's picture and, once it's been encountered, its statistics
 */
class BestiaryImage : public ScrollView {
private:
	int _monsterId = 1;
	int _statsPage = 0;
	Graphics::ManagedSurface _image;

	/**
	 * Switches between the stats and defenses pages
	 */
	void changePage(int delta);

public:
	BestiaryImage();
	~BestiaryImage() override {}

	bool msgGame(const GameMessage &msg) override;
	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
