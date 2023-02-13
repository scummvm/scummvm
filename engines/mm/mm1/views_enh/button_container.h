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
#include "mm/shared/xeen/sprites.h"
#include "mm/mm1/views/text_view.h"
#include "mm/mm1/events.h"
#include "mm/mm1/metaengine.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

class ButtonContainer;

class UIButton {
public:
	Common::Rect _bounds;
	Shared::Xeen::SpriteResource *_sprites;
	KeybindingAction _action;
	uint _frameNum, _selectedFrame;
	bool _draw;

	/**
	 * Constructor
	 */
	UIButton(ButtonContainer *owner, const Common::Rect &bounds,
			KeybindingAction action, uint frameNum, Shared::Xeen::SpriteResource *sprites,
			bool draw) :
			_bounds(bounds), _action(action), _frameNum(frameNum),
			_selectedFrame(frameNum | 1), _sprites(sprites), _draw(draw) {
	}

	/**
	 * Constructor
	 */
	UIButton() : _action(KEYBIND_NONE), _frameNum(0), _selectedFrame(0),
		_sprites(nullptr), _draw(false) {
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
	KeybindingAction _selectedAction = KEYBIND_NONE;
protected:
	Common::Array<UIButton> _buttons;

	bool doScroll(bool fadeIn);
public:
	ButtonContainer(const Common::String &name, UIElement *owner);

	/**
	 * Saves the current list of buttons
	 */
	void saveButtons();

	void clearButtons();

	void restoreButtons();

	void addButton(const Common::Rect &bounds, KeybindingAction action,
		Shared::Xeen::SpriteResource *sprites = nullptr);
	void addButton(const Common::Rect &bounds, KeybindingAction action,
		int frameNum, Shared::Xeen::SpriteResource *sprites = nullptr);

	void draw() override;
	bool msgMouseDown(const MouseDownMessage &msg) override;
	bool msgMouseUp(const MouseUpMessage &msg) override;
};

} // namespace ViewsEnh
} // namespace MM1
} // namespace MM

#endif
