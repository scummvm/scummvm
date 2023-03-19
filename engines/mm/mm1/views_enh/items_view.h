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

#ifndef MM1_VIEWS_ENH_ITEMS_VIEW_H
#define MM1_VIEWS_ENH_ITEMS_VIEW_H

#include "mm/mm1/views_enh/party_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class ItemsView : public PartyView {
protected:
	enum CostMode { SHOW_COST, SHOW_VALUE, NO_COST };
	int _selectedItem = -1;
	CostMode _costMode = NO_COST;
	Common::Array<int> _items;
	const Common::Rect _buttonsArea;
	Shared::Xeen::SpriteResource _btnSprites;
	Common::StringArray _btnText;

	/**
	 * Add a button to the buttons bar
	 */
	void addButton(int frame, const Common::String &text,
		Common::KeyCode keycode);

	/**
	 * Display a message that the inventory is full
	 */
	void backpackFull();

	/**
	 * Display a message the character doesn't have enough gold
	 */
	void notEnoughGold();

	/**
	 * Display an arbitrary message
	 */
	void displayMessage(const Common::String &msg);

	/**
	 * Get the text color for a line
	 */
	virtual int getLineColor() const {
		return 0;
	}

	/**
	 * Called when an item is selected
	 */
	virtual void itemSelected() = 0;

public:
	ItemsView(const Common::String &name);
	virtual ~ItemsView() {}

	bool msgFocus(const FocusMessage &msg) override;
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	void timeout() override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
