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

#include "common/scummsys.h"
#include "mm/mm1/views_enh/button_container.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {
namespace ViewsEnh {

void UIButton::draw(bool pressed) {
	_sprites->draw(g_events->getScreen(),
		pressed ? _selectedFrame : _frameNum,
		Common::Point(_bounds.left, _bounds.top));
}

ButtonContainer::ButtonContainer(const Common::String &name,
		UIElement *owner) : Views::TextView(name, owner) {
}

void ButtonContainer::saveButtons() {
	_savedButtons.push(_buttons);
	clearButtons();
}

void ButtonContainer::clearButtons() {
	_buttons.clear();
}

void ButtonContainer::restoreButtons() {
	_buttons = _savedButtons.pop();
}

void ButtonContainer::addButton(const Common::Rect &bounds, KeybindingAction action,
	Shared::Xeen::SpriteResource *sprites) {
	_buttons.push_back(UIButton(this, bounds, action, _buttons.size() * 2, sprites, sprites != nullptr));
}

void ButtonContainer::addButton(const Common::Rect &bounds, KeybindingAction action,
	int frameNum, Shared::Xeen::SpriteResource *sprites) {
	_buttons.push_back(UIButton(this, bounds, action, frameNum, sprites, sprites != nullptr));
}

bool ButtonContainer::msgMouseDown(const MouseDownMessage &msg) {
	_selectedAction = KEYBIND_NONE;

	if (msg._button == MouseMessage::MB_LEFT) {
		// Check whether any button is selected
		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._bounds.contains(msg._pos) &&
					_buttons[i]._action != KEYBIND_NONE) {
				_selectedAction = _buttons[i]._action;

				g_events->redraw();
				g_events->drawElements();
				return true;
			}
		}
	}

	return false;
}

bool ButtonContainer::msgMouseUp(const MouseUpMessage &msg) {
	KeybindingAction action = _selectedAction;
	_selectedAction = KEYBIND_NONE;

	if (msg._button == MouseMessage::MB_LEFT && action != KEYBIND_NONE) {
		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._action == action) {
				if (_buttons[i]._action == action)
					g_events->send(ActionMessage(action));

				g_events->redraw();
				g_events->drawElements();
				return true;
			}
		}
	}

	return false;
}

void ButtonContainer::draw() {
	TextView::draw();

	for (uint btnIndex = 0; btnIndex < _buttons.size(); ++btnIndex) {
		UIButton &btn = _buttons[btnIndex];
		if (btn._draw && btn._sprites)
			btn.draw(btn._action != KEYBIND_NONE &&
				btn._action == _selectedAction);
	}
}

bool ButtonContainer::doScroll(bool fadeIn) {
	// TODO
	return false;
}

} // namespace ViewsEnh
} // End of namespace MM1
} // End of namespace MM
