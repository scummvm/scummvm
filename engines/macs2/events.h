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

#ifndef MACS2_EVENTS_H
#define MACS2_EVENTS_H

#include "common/array.h"
#include "common/stack.h"
#include "graphics/screen.h"
#include "macs2/messages.h"

namespace Macs2 {

class Events;

/**
 * Implements a thunk layer around an element's bounds,
 * allowing access to it as if it were a simple Common::Rect,
 * but any changes to it will also be applied to a linked inner bounds
 */
struct Bounds {
private:
	Common::Rect _bounds;
	Common::Rect &_innerBounds;

public:
	Bounds(Common::Rect &innerBounds);
	operator const Common::Rect &() const { return _bounds; }
	Bounds &operator=(const Common::Rect &r);
	int16 height() const { return _bounds.height(); }
};

/**
 * User interface element
 */
class UIElement {
	friend class Events;

protected:
	Common::Array<UIElement *> _children;
	Common::Rect _innerBounds;
	Bounds _bounds;
	bool _needsRedraw = true;
	Common::String _name;

private:
	/**
	 * Outer method for doing drawing
	 */
	void drawElements();

public:
	UIElement(const Common::String &name, UIElement *uiParent);
	UIElement(const Common::String &name);
	virtual ~UIElement() {}

	/**
	 * Sets that the element needs to be redrawn
	 */
	void redraw();

	/**
	 * Returns a surface for drawing the element
	 */
	Graphics::ManagedSurface getSurface() const;

	/**
	 * Draws the element
	 */
	virtual void draw();

	/**
	 * Called for game frame ticks
	 */
	virtual bool tick();

	/**
	 * Find a view by name
	 */
	virtual UIElement *findView(const Common::String &name);

/**
 * Handles events
 */
#define MESSAGE(NAME)                                                     \
protected:                                                                \
	virtual bool msg##NAME(const NAME##Message &e) {                      \
		for (Common::Array<UIElement *>::iterator it = _children.begin(); \
			 it != _children.end(); ++it) {                               \
			if ((*it)->msg##NAME(e))                                      \
				return true;                                              \
		}                                                                 \
		return false;                                                     \
	}

	MESSAGE(Focus);
	MESSAGE(Unfocus);
	MESSAGE(Keypress);
	MESSAGE(MouseDown);
	MESSAGE(MouseUp);
	MESSAGE(Action);
	MESSAGE(MouseMove);
#undef MESSAGE
};

/**
 * Main events and view manager. This is kept separate from the engine
 * class because the engine may add a lot of globals and bring in other
 * classes. So to save on compilation time, classes that only need to
 * access basic view management methods like addView only need to
 * include events.h rather than the whole engine.
 */
class Events : public UIElement {
private:
	Graphics::Screen *_screen = nullptr;
	Common::Stack<UIElement *> _views;

protected:
	/**
	 * Process an event
	 */
	void processEvent(Common::Event &ev);

	/**
	 * Returns true if the game should quit
	 */
	virtual bool shouldQuit() const = 0;

/**
 * Overrides events we want to only go to the focused view
 */
#define MESSAGE(NAME)                                                 \
	bool msg##NAME(const NAME##Message &e) override {                 \
		return !_views.empty() ? focusedView()->msg##NAME(e) : false; \
	}
	MESSAGE(Action);
	MESSAGE(Focus);
	MESSAGE(Unfocus);
	MESSAGE(Keypress);
	MESSAGE(MouseDown);
	MESSAGE(MouseUp);
#undef MESSAGE
public:
	Events();
	virtual ~Events();

	// TODO: Consider a better place
	uint32 currentMillis = 0;

	/**
	 * Main game loop
	 */
	void runGame();

	/**
	 * Adds a focused view to the view stack without replacing current one
	 */
	void addView(UIElement *ui);

	/**
	 * Returns the currently focused view, if any
	 */
	UIElement *focusedView() const {
		return _views.empty() ? nullptr : _views.top();
	}

	/**
	 * Returns the underlying screen
	 */
	Graphics::Screen *getScreen() const {
		return _screen;
	}

	/**
	 * Draws the focused view
	 */
	void drawElements() {
		if (!_views.empty())
			focusedView()->drawElements();
	}

	/**
	 * Events manager doesn't have any intrinsic drawing
	 */
	void draw() override {}

	/**
	 * Called once every game frame
	 */
	bool tick() override {
		return !_views.empty() ? focusedView()->tick() : false;
	}
};

extern Events *g_events;

} // namespace Macs2

#endif
