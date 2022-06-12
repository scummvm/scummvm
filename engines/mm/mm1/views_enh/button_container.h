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

#ifndef MM1_VIEWS_ENH_BUTTON_CONTAINER_H
#define MM1_VIEWS_ENH_BUTTON_CONTAINER_H

#include "common/array.h"
#include "common/stack.h"
#include "common/rect.h"
#include "mm/xeen/sprites.h"
#include "mm/mm1/views/text_view.h"
#include "mm/mm1/events.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class ButtonContainer;

class UIButton {
public:
	Common::Rect _bounds;
	Xeen::SpriteResource *_sprites;
	int _value;
	uint _frameNum, _selectedFrame;
	bool _draw;

	/**
	 * Constructor
	 */
	UIButton(ButtonContainer *owner, const Common::Rect &bounds,
			int value, uint frameNum, Xeen::SpriteResource *sprites,
			bool draw) :
			_bounds(bounds), _value(value), _frameNum(frameNum),
			_selectedFrame(frameNum | 1), _sprites(sprites), _draw(draw) {
	}

	/**
	 * Constructor
	 */
	UIButton() : _value(0), _frameNum(0), _selectedFrame(0), _sprites(nullptr), _draw(false) {
	}

	/**
	 * Set the frame
	 */
	void setFrame(uint frameNum) {
		_frameNum = frameNum;
		_selectedFrame = frameNum | 1;
	}

	/**
	 * Set the frame
	 */
	void setFrame(uint frameNum, uint selectedFrame) {
		_frameNum = frameNum;
		_selectedFrame = selectedFrame;
	}

	/**
	 * Draw the button
	 */
	void draw(bool pressed = false);
};

class ButtonContainer : public Views::TextView {
private:
	Common::Stack< Common::Array<UIButton> > _savedButtons;
protected:
	Common::Array<UIButton> _buttons;
	int _buttonValue;

	bool doScroll(bool fadeIn);
public:
	ButtonContainer(const Common::String &name, UIElement *owner);

	/**
	 * Saves the current list of buttons
	 */
	void saveButtons();

	void clearButtons();

	void restoreButtons();

	void addButton(const Common::Rect &bounds, int val,
		Xeen::SpriteResource *sprites = nullptr);
	void addButton(const Common::Rect &bounds, int val,
		int frameNum, Xeen::SpriteResource *sprites = nullptr);

//	void addPartyButtons(XeenEngine *vm);

	/**
	 * Clears any currently set button value
	 */
	void clearEvents() {
		_buttonValue = 0;
	}

	void draw() override;
	bool msgMouseClick(const MouseClickMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
