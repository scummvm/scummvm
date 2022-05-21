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

#include "common/system.h"
#include "graphics/screen.h"
#include "mm/mm1/events.h"

namespace MM {
namespace MM1 {

#define FRAME_RATE 20
#define FRAME_DELAY (1000 / FRAME_RATE)

void Events::runGame() {
	uint currTime, nextFrameTime = 0;
	_screen = new Graphics::Screen();

	Common::Event e;
	for (;;) {
		while (g_system->getEventManager()->pollEvent(e)) {
			if (e.type == Common::EVENT_QUIT)
				return;
			else
				processEvent(e);
		}

		g_system->delayMillis(10);
		if ((currTime = g_system->getMillis()) >= nextFrameTime) {
			nextFrameTime = currTime + FRAME_DELAY;
			tick();
			draw();
			_screen->update();
		}
	}

	delete _screen;
}

void Events::processEvent(Common::Event &ev) {
	switch (ev.type) {
	case Common::EVENT_KEYDOWN:
		msgKeypress(KeypressMessage(ev.kbd));
		break;
	default:
		break;
	}
}

UIElement::UIElement(UIElement *uiParent) : _parent(uiParent) {
	if (_parent)
		_parent->_children.push_back(this);
}

void UIElement::draw() {
	for (size_t i = 0; i < _children.size(); ++i)
		_children[i]->draw();
}

bool UIElement::tick() {
	for (size_t i = 0; i < _children.size(); ++i) {
		if (_children[i]->tick())
			return true;
	}

	return false;
}

} // namespace MM1
} // namespace MM
