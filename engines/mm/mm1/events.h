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

#ifndef MM1_EVENTS_H
#define MM1_EVENTS_H

#include "common/array.h"
#include "common/events.h"
#include "graphics/screen.h"

namespace MM {
namespace MM1 {

class UIElement {
protected:
	UIElement *_parent;
	Common::Array<UIElement *> _children;
public:
	UIElement(UIElement *uiParent = nullptr);
	virtual ~UIElement() {}

	/**
	 * Returns the game view
	 */
	virtual Graphics::Screen *getScreen() const {
		return _parent ? _parent->getScreen() : nullptr;
	}

	/**
	 * Draws the element
	 */
	virtual void draw();

	/**
	 * Called for game frame ticks
	 */
	virtual bool tick();

	/**
	 * Handles a keypress
	 */
	virtual bool keypressEvent(const Common::Event &e) {
		return false;
	}
};

class Events : public UIElement {
private:
	Graphics::Screen *_screen = nullptr;
	UIElement *_focusedElement;
protected:
	/**
	 * Process an event
	 */
	void processEvent(Common::Event &ev);
public:
	Events() {}
	virtual ~Events() {}

	/**
	 * Main game loop
	 */
	void runGame();

	/**
	 * Sets the focus to a new view
	 */
	void focusElement(UIElement *ui) {
		_focusedElement = ui;
	}

	Graphics::Screen *getScreen() const override {
		return _screen;
	}


	void draw() override {
		if (_focusedElement)
			_focusedElement->draw();
	}

	bool tick() override {
		return _focusedElement ? _focusedElement->tick() : false;
	}
};

} // namespace MM1
} // namespace MM

#endif
