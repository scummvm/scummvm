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

#ifndef MM1_VIEWS_ENH_SCROLL_VIEW_H
#define MM1_VIEWS_ENH_SCROLL_VIEW_H

#include "mm/mm1/views_enh/text_view.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

#define FRAME_BORDER_SIZE 8

class ScrollView : public TextView {
	struct Button {
		Xeen::SpriteResource *_sprites;
		Common::Point _pos;
		int _frame;
		Common::KeyState _key;

		Button(Xeen::SpriteResource *sprites,
			const Common::Point &pos, int frame,
			const Common::KeyState &key) :
			_sprites(sprites), _pos(pos), _frame(frame), _key(key) {
		}
	};
private:
	Common::Array<Button> _buttons;
	int _selectedButton = -1;
protected:
	Common::Point _symbolPos;
protected:
	/**
	 * Draw the scroll frame
	 */
	void frame();

	/**
	 * Fill the scroll background
	 */
	void fill();

	/**
	 * Draw a special symbol
	 */
	void writeSymbol(int symbolId);

	/**
	 * Get the button at the given position
	 */
	int getButtonAt(const Common::Point &pos);
public:
	ScrollView(const Common::String &name);
	ScrollView(const Common::String &name, UIElement *owner);
	virtual ~ScrollView() {}

	/**
	 * Clear the buttons list
	 */
	void clearButtons() {
		_buttons.clear();
	}

	/**
	 * Add a button for display
	 */
	void addButton(Xeen::SpriteResource *sprites,
		const Common::Point &pos, int frame,
		const Common::KeyState &key);

	/**
	 * Reset selected button
	 */
	void resetSelectedButton();

	/**
	 * Draw the view
	 */
	void draw() override;

	/**
	 * View is focused
	 */
	bool msgFocus(const FocusMessage &msg) override;

	/**
	 * Mouse down messages
	 */
	bool msgMouseDown(const MouseDownMessage &msg) override;

	/**
	 * Mouse up messages
	 */
	bool msgMouseUp(const MouseUpMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
