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
#include "mm/mm1/utils/mouse.h"

namespace MM {
namespace MM1 {

#define FRAME_RATE 20
#define FRAME_DELAY (1000 / FRAME_RATE)

class Events;

/**
 * Implements a thunk layer around an element's
 * bounds, allowing access to it as if it were
 * a simple Common::Rect, but any changes to it
 * will also be applied to a linked inner bounds
 */
struct Bounds {
private:
	Common::Rect _bounds;
	Common::Rect &_innerBounds;
	int _borderSize = 0;
public:
	const int16 &left;
	const int16 &top;
	const int16 &right;
	const int16 &bottom;
public:
	Bounds(Common::Rect &innerBounds);
	operator const Common::Rect &() const { return _bounds; }
	Bounds &operator=(const Common::Rect &r);
	void setBorderSize(size_t borderSize);
	size_t borderSize() const { return _borderSize; }
	int16 width() const { return _bounds.width(); }
	int16 height() const { return _bounds.height(); }
};

/**
 * User interface element
 */
class UIElement {
	friend class Events;
private:
	int _timeoutCtr = 0;
protected:
	UIElement *_parent;
	Common::Array<UIElement *> _children;
	Common::Rect _innerBounds;
	Bounds _bounds;
	bool _needsRedraw = true;
	Common::String _name;
protected:
	Common::Rect getLineBounds(int line1, int line2) const {
		return Common::Rect(0, line1 * 8, 320, (line2 + 1) * 8);
	}

	/**
	 * Set a delay countdown in seconds
	 */
	void delaySeconds(uint seconds);

	/**
	 * Set a delay countdown in frames
	 */
	void delayFrames(uint frames);

	/**
	 * Returns true if a delay is active
	 */
	bool isDelayActive() const {
		return _timeoutCtr != 0;
	}

	/**
	 * Cancels any active delay
	 */
	void cancelDelay() {
		_timeoutCtr = 0;
	}

	/**
	 * Ends an active delay and calls timeout
	 */
	bool endDelay();

	/**
	 * Called when an active timeout countdown expired
	 */
	virtual void timeout();

private:
	/**
	 * Outer method for doing drawing
	 *
	 */
	void drawElements();

	/**
	 * Finds a view globally
	 */
	static UIElement *findViewGlobally(const Common::String &name);
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
	virtual void close();

	/*
	 * Returns true if the view is focused
	 */
	bool isFocused() const;

	/**
	 * Sets the focus to a new view
	 */
	void replaceView(UIElement *ui, bool replaceAllViews = false);
	void replaceView(const Common::String &name, bool replaceAllViews = false);

	/**
	 * Adds a focused view to the view stack without replacing current one
	 */
	void addView(UIElement *ui);
	void addView(const Common::String &name);
	void addView();
	void open() { addView(); }

	/**
	 * Returns a random number
	 */
	int getRandomNumber(int minNumber, int maxNumber);
	int getRandomNumber(int maxNumber);

	/**
	 * Sets the element's bounds
	 */
	virtual void setBounds(const Common::Rect &r) {
		_bounds = r;
	}

	/**
	 * Gets the element's bounds
	 */
	Common::Rect getBounds() const {
		return _bounds;
	}

	/**
	 * Returns a surface for drawing the element
	 */
	Graphics::ManagedSurface getSurface() const;

	/**
	 * Clear the surface
	 */
	virtual void clearSurface();

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
	protected: \
		virtual bool msg##NAME(const NAME##Message &e) { \
			for (Common::Array<UIElement *>::iterator it = _children.begin(); \
					it != _children.end(); ++it) { \
				if ((*it)->msg##NAME(e)) return true; \
			} \
			return false; \
		} \
	public: \
		bool send(const Common::String &viewName, const NAME##Message &msg) { \
			UIElement *view = UIElement::findViewGlobally(viewName); \
			assert(view); \
			return view->msg##NAME(msg); \
		} \
		bool send(const NAME##Message &msg) { \
			return send("Root", msg); \
		} \

	MESSAGE(Focus);
	MESSAGE(Unfocus);
	MESSAGE(Keypress);
	MESSAGE(MouseDown);
	MESSAGE(MouseUp);
	MESSAGE(Action);
	MESSAGE(Game);
	MESSAGE(Header);
	MESSAGE(Info);
	MESSAGE(Value);
	MESSAGE(DrawGraphic);
	#undef MESSAGE
};

/**
 * Main events and view manager
 */
class Events : public UIElement, public Mouse {
private:
	Graphics::Screen *_screen = nullptr;
	Common::Stack<UIElement *> _views;
	bool _enhancedMode;
protected:
	/**
	 * Process an event
	 */
	void processEvent(Common::Event &ev);

	/**
	 * Overrides events we want to only go to the focused view
	 */
	#define MESSAGE(NAME) \
		bool msg##NAME(const NAME##Message &e) override { \
			return !_views.empty() ? focusedView()->msg##NAME(e) : false; \
		}
	MESSAGE(Action);
	MESSAGE(Focus);
	MESSAGE(Unfocus);
	MESSAGE(Keypress);
	MESSAGE(MouseDown);
	MESSAGE(MouseUp);
	MESSAGE(DrawGraphic);
	#undef MESSAGE
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
	void replaceView(UIElement *ui, bool replaceAllViews = false);
	void replaceView(const Common::String &name, bool replaceAllViews = false);

	/**
	 * Adds a focused view to the view stack without replacing current one
	 */
	void addView(UIElement *ui);
	void addView(const Common::String &name);

	/**
	 * Clears the view list
	 */
	void clearViews();

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

	/**
	 * Returns the view prior to the current view, if any
	 */
	UIElement *priorView() const {
		return _views.size() < 2 ? nullptr :
			_views[_views.size() - 2];
	}

	/**
	 * Returns true if a view of a given name is present
	 * at all in the visible view stack
	 */
	bool isPresent(const Common::String &name) const;

	/**
	 * Returns true if combat is active
	 */
	bool isInCombat() const {
		return isPresent("Combat");
	}

	Graphics::Screen *getScreen() const {
		return _screen;
	}

	void drawElements() {
		if (!_views.empty())
			focusedView()->drawElements();
	}

	/**
	 * Add a keypress to the event queue
	 */
	void addKeypress(const Common::KeyCode kc);

	/**
	 * Add a action to the event queue
	 */
	void addAction(KeybindingAction action);

	/**
	 * Checks whether a keypress is pending
	 */
	bool isKeypressPending() const;

	void draw() override {}

	bool tick() override {
		return !_views.empty() ? focusedView()->tick() : false;
	}

	/**
	 * Calling the close method for g_events closes the active window
	 */
	void close() override {
		focusedView()->close();
	}
};

extern Events *g_events;

} // namespace MM1
} // namespace MM

#endif
