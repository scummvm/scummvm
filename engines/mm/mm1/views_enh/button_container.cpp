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
	Xeen::SpriteResource *sprites) {
	_buttons.push_back(UIButton(this, bounds, action, _buttons.size() * 2, sprites, sprites != nullptr));
}

void ButtonContainer::addButton(const Common::Rect &bounds, KeybindingAction action,
	int frameNum, Xeen::SpriteResource *sprites) {
	_buttons.push_back(UIButton(this, bounds, action, frameNum, sprites, sprites != nullptr));
}

bool ButtonContainer::msgMouseClick(const MouseClickMessage &msg) {
	if (msg._button == MouseClickMessage::MB_LEFT) {
		const Common::Point pt = msg._pos;

		// Check whether any button is selected
		for (uint i = 0; i < _buttons.size(); ++i) {
			if (_buttons[i]._bounds.contains(pt) && _buttons[i]._action != KEYBIND_NONE) {
				// Show the button briefly depressed
				_buttons[i].draw(true);
				g_engine->getScreen()->update();
				g_system->delayMillis(500);

				_buttons[i].draw(false);
				g_engine->getScreen()->update();

				// Trigger the button action
				KeybindingAction action = _buttons[i]._action;
				g_events->msgAction(ActionMessage(action));

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
		if (btn._draw)
			btn.draw();
	}
}

bool ButtonContainer::doScroll(bool fadeIn) {
	// TODO
	return false;
}

} // namespace ViewsEnh
} // End of namespace MM1
} // End of namespace MM
