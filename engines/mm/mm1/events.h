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
#include "common/stack.h"
#include "graphics/screen.h"
#include "mm/mm1/messages.h"

namespace MM {
namespace MM1 {

class Events;

class UIElement {
	friend class Events;
protected:
	UIElement *_parent;
	Common::Array<UIElement *> _children;
	Common::Rect _bounds;
	bool _needsRedraw = true;
	Common::String _name;
protected:
	Common::Rect getLineBounds(int line1, int line2) const {
		return Common::Rect(0, line1 * 8, 320, (line2 + 1) * 8);
	}
private:
	/**
	 * Outer method for doing drawing
	 *
	 */
	void drawElements();
public:
	UIElement(const Common::String &name, UIElement *uiParent);
	virtual ~UIElement() {}

	/**
	 * Returns true if the elements needs to be redrawn
	 */
	bool needsRedraw() const { return _needsRedraw; }

	/**
	 * Sets that the element needs to be redrawn
	 */
	void redraw();

	/**
	 * Focuses the element as the current view
	 */
	void focus();

	/**
	 * Closes the current view. The view must have been added
	 * via addView, so there's a remaining view afterwards
	 */
	void close();

	/**
	 * Sets the focus to a new view
	 */
	void replaceView(UIElement *ui);
	void replaceView(const Common::String &name);

	/**
	 * Adds a focused view to the view stack without replacing current one
	 */
	void addView(UIElement *ui);
	void addView(const Common::String &name);
	void addView();

	/**
	 * Sets the element's bounds
	 */
	void setBounds(const Common::Rect &r) {
		_bounds = r;
	}

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
	#define MESSAGE(NAME) \
		virtual bool msg##NAME(const NAME##Message &e) { \
			for (Common::Array<UIElement *>::iterator it = _children.begin(); \
					it != _children.end(); ++it) { \
				if ((*it)->msg##NAME(e)) return true; \
			} \
			return false; \
		}

	MESSAGE(Focus);
	MESSAGE(Unfocus);
	MESSAGE(Keypress);
	MESSAGE(Action);
	MESSAGE(Business);
	MESSAGE(Game);
	MESSAGE(Info);
	#undef MESSAGE
};

class Events : public UIElement {
private:
	Graphics::Screen *_screen = nullptr;
	Common::Stack<UIElement *> _views;
	bool _enhancedMode;
protected:
	/**
	 * Process an event
	 */
	void processEvent(Common::Event &ev);
public:
	Events(bool enhancedMode);
	virtual ~Events();

	/**
	 * Main game loop
	 */
	void runGame();

	/**
	 * Sets the focus to a new view
	 */
	void replaceView(UIElement *ui);
	void replaceView(const Common::String &name);

	/**
	 * Adds a focused view to the view stack without replacing current one
	 */
	void addView(UIElement *ui);
	void addView(const Common::String &name);

	/**
	 * Pops a view from the view stack
	 */
	void popView();

	/**
	 * Returns the currently focused view, if any
	 */
	UIElement *focusedView() const {
		return _views.empty() ? nullptr : _views.top();
	}

	Graphics::Screen *getScreen() const {
		return _screen;
	}

	void drawElements() {
		if (!_views.empty())
			focusedView()->drawElements();
	}

	void draw() override {}

	bool tick() override {
		return !_views.empty() ? focusedView()->tick() : false;
	}

	/**
	 * Overrides events we want to only go to the focused view
	 */
	#define MESSAGE(NAME) \
		bool msg##NAME(const NAME##Message &e) override { \
			return !_views.empty() ? focusedView()->msg##NAME(e) : false; \
		}
	MESSAGE(Focus);
	MESSAGE(Unfocus);
	MESSAGE(Keypress);
	#undef MESSAGE
};

extern Events *g_events;

} // namespace MM1
} // namespace MM

#endif
