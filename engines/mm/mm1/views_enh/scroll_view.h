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
#define GLYPH_W 24
#define GLYPH_H 20

class ScrollView : public TextView {
	struct Button {
		Shared::Xeen::SpriteResource *_sprites;
		Common::Rect _bounds;
		int _frame = -1;
		Common::KeyState _key;
		KeybindingAction _action = KEYBIND_NONE;
		bool _enabled = true;
		bool _halfSize = false;

		Button(Shared::Xeen::SpriteResource *sprites,
			const Common::Point &pos, int frame,
			const Common::KeyState &key, bool halfSize = false) :
			_sprites(sprites), _frame(frame), _key(key), _halfSize(halfSize),
			_bounds(Common::Rect(pos.x, pos.y,
				pos.x + (halfSize ? GLYPH_W / 2 : GLYPH_W),
				pos.y + (halfSize ? GLYPH_H / 2 : GLYPH_H))
			) {
		}
		Button(Shared::Xeen::SpriteResource *sprites,
			const Common::Point &pos, int frame,
			KeybindingAction action, bool halfSize = false) :
			_sprites(sprites), _frame(frame), _action(action), _halfSize(halfSize),
			_bounds(Common::Rect(pos.x, pos.y,
				pos.x + (halfSize ? GLYPH_W / 2 : GLYPH_W),
				pos.y + (halfSize ? GLYPH_H / 2 : GLYPH_H))
			) {
		}
		Button(const Common::Rect &r, const Common::KeyState &key) :
			_sprites(nullptr), _bounds(r), _key(key) {
		}
		Button(const Common::Rect &r, const KeybindingAction action) :
			_sprites(nullptr), _bounds(r), _action(action) {
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

	/**
	 * Return the number of buttons
	 */
	size_t getButtonCount() const {
		return _buttons.size();
	}

	/**
	 * Draws the buttons
	 */
	void drawButtons();

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
	int addButton(Shared::Xeen::SpriteResource *sprites,
		const Common::Point &pos, int frame,
		const Common::KeyState &key, bool halfSize = false);

	/**
	 * Add a button for display
	 */
	int addButton(Shared::Xeen::SpriteResource *sprites,
		const Common::Point &pos, int frame, KeybindingAction action,
		bool halfSize = false);

	/**
	 * Add a button for display
	 */
	int addButton(const Common::Rect &r, const Common::KeyState &key);

	/**
	 * Add a button for display
	 */
	int addButton(const Common::Rect &r, KeybindingAction action);

	/**
	 * Set a button's enablement
	 */
	void setButtonEnabled(int buttonNum, bool enabled) {
		_buttons[buttonNum]._enabled = enabled;
	}

	/**
	 * Return true if a button is enabled
	 */
	bool isButtonEnabled(int buttonNum) const {
		return _buttons[buttonNum]._enabled;
	}

	/**
	 * Set a button's position
	 */
	void setButtonPos(int buttonNum, const Common::Point &pos) {
		_buttons[buttonNum]._bounds.moveTo(pos);
	}

	/**
	 * Delete buttons
	 */
	void removeButtons(int start, int end = -2);

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
