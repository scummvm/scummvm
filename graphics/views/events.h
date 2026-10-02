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

#ifndef GRAPHICS_VIEWS_EVENTS_H
#define GRAPHICS_VIEWS_EVENTS_H

#include "common/array.h"
#include "common/random.h"
#include "common/stack.h"
#include "graphics/font.h"
#include "graphics/views/gfx_surface.h"
#include "graphics/views/messages.h"
#include "graphics/views/rect.h"
#include "graphics/screen.h"

namespace Graphics {
namespace Views {

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
	int _borderSize = 0;

public:
	const int16 &left;
	const int16 &top;
	const int16 &right;
	const int16 &bottom;

public:
	Bounds(Common::Rect &innerBounds);
	operator const Common::Rect &() const {
		return _bounds;
	}
	Bounds &operator=(const Common::Rect &r);
	void setBorderSize(size_t borderSize);
	size_t borderSize() const {
		return _borderSize;
	}
	int16 width() const {
		return _bounds.width();
	}
	int16 height() const {
		return _bounds.height();
	}
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
#ifdef USE_TTS
	Common::String _previousSaid;
#endif

protected:
	/**
	 * Set a delay countdown in seconds, after which timeout() is called
	 */
	void delaySeconds(uint seconds);

	/**
	 * Set a delay countdown in frames, after which timeout() is called
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
	 * Called when an active timeout countdown expired
	 */
	virtual void timeout();

private:
	/**
	 * Outer method for doing drawing
	 *
	 */
	virtual void drawElements();

	/**
	 * Finds a view globally
	 */
	static UIElement *findViewGlobally(const Common::String &name);

public:
	UIElement(const Common::String &name, UIElement *uiParent);
	UIElement(const Common::String &name);
	virtual ~UIElement() {
	}

	/**
	 * Returns true if the elements needs to be redrawn
	 */
	bool needsRedraw() const {
		return _needsRedraw;
	}

	/**
	 * Sets that the element needs to be redrawn
	 */
	void redraw();

	/**
	 * Closes the current view. The view must have been added
	 * via addView, so there's a remaining view afterwards
	 */
	virtual void close();

	/**
	 * Sets the focus to a new view
	 */
	virtual void replaceView(UIElement *ui, bool replaceAllViews = false);
	virtual void replaceView(const Common::String &name, bool replaceAllViews = false);

	/**
	 * Adds a focused view to the view stack without replacing current one
	 */
	virtual void addView(UIElement *ui);
	virtual void addView(const Common::String &name);
	void addView();
	void open() {
		addView();
	}

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
	 * Gets a view's name
	 */
	const Common::String &getName() const {
		return _name;
	}

	/**
	 * Returns a surface for drawing the element
	 */
	GfxSurface getSurface(bool innerBounds = false) const;

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
	 // Mouse move only has a minimal implementation for performance reasons
protected:
	virtual bool msgMouseMove(const MouseMoveMessage &msg) {
		return false;
	}

#ifdef USE_TTS
	/**
	 * Stops TTS voicing and clears the previously spoken text
	 */
	void stopTextToSpeech();
#endif

public:
	bool send(const MouseMoveMessage &msg) {
		return msgMouseMove(msg);
	}

#define MESSAGE(NAME)                                                     \
protected:                                                                \
	virtual bool msg##NAME(const NAME##Message &e) {                      \
		for (Common::Array<UIElement *>::iterator it = _children.begin(); \
			 it != _children.end(); ++it) {                               \
			if ((*it)->msg##NAME(e))                                      \
				return true;                                              \
		}                                                                 \
		return false;                                                     \
	}                                                                     \
                                                                          \
public:                                                                   \
	bool send(const Common::String &viewName, const NAME##Message &msg) { \
		UIElement *view = UIElement::findViewGlobally(viewName);          \
		assert(view);                                                     \
		return view->msg##NAME(msg);                                      \
	}                                                                     \
	bool send(const NAME##Message &msg) {                                 \
		return msg##NAME(msg);                                            \
	}

	MESSAGE(Focus);
	MESSAGE(Unfocus);
	MESSAGE(MouseEnter);
	MESSAGE(MouseLeave);
	MESSAGE(Keypress);
	MESSAGE(MouseDown);
	MESSAGE(MouseUp);
	MESSAGE(Action);
	MESSAGE(Game);
	MESSAGE(Value);
#undef MESSAGE

protected:
	/**
	 * Handles messages of types that the library has no dedicated msgXXX
	 * method for - i.e. message structs an engine defines for itself, derived
	 * from Message. Overrides should identify the types they care about
	 * (typically with dynamic_cast) and return the base class's result for
	 * everything else. The default passes the message to each child in turn
	 */
	virtual bool msgCustom(const Message &msg) {
		for (Common::Array<UIElement *>::iterator it = _children.begin();
			 it != _children.end(); ++it) {
			if ((*it)->msgCustom(msg))
				return true;
		}
		return false;
	}

public:
	/**
	 * Sends an engine-defined message, either to this element or the named
	 * view. Only used for types without one of the dedicated overloads
	 * above, which are preferred when they match exactly. Note that a type
	 * derived from one of the built-in messages will therefore come through
	 * here rather than being converted to its base
	 */
	template<class T>
	bool send(const T &msg) {
		return msgCustom(msg);
	}
	template<class T>
	bool send(const Common::String &viewName, const T &msg) {
		UIElement *view = UIElement::findViewGlobally(viewName);
		assert(view);
		return view->msgCustom(msg);
	}
};

/**
 * Main events and view manager. This is kept separate from the engine
 * class because the engine may add a lot of globals and bring in other
 * classes. So to save on compilation time, classes that only need to
 * access basic view management methods like addView or replaceView
 * only need to include events.h rather than the whole engine.
 */
class Events : public UIElement {
private:
	Graphics::Screen *_screen = nullptr;
	Common::RandomSource _randomSource;
	Common::Stack<UIElement *> _views;

	void nextFrame();

	/**
	 * Process an event
	 */
	void processEvent(Common::Event &ev);

protected:
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
	MESSAGE(MouseEnter);
	MESSAGE(MouseLeave);
	MESSAGE(Keypress);
	MESSAGE(MouseDown);
	MESSAGE(MouseUp);
	MESSAGE(MouseMove);
#undef MESSAGE

	/**
	 * Fallback for custom messages, to route them to the currently focused view
	 */
	bool msgCustom(const Message &msg) override {
		return !_views.empty() ? focusedView()->msgCustom(msg) : false;
	}

public:
	Events(const Common::String &engineName);
	virtual ~Events();

	/**
	 * Main game loop
	 */
	void runGame();

	/**
	 * Returns a random number
	 */
	int getRandomNumber(int minNumber, int maxNumber);
	int getRandomNumber(int maxNumber);

	/**
	 * Sets the focus to a new view
	 */
	void replaceView(UIElement *ui, bool replaceAllViews = false) override;
	void replaceView(const Common::String &name, bool replaceAllViews = false) override;

	/**
	 * Adds a focused view to the view stack without replacing current one
	 */
	void addView(UIElement *ui) override;
	void addView(const Common::String &name) override;

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
		return _views.size() < 2 ? nullptr : _views[_views.size() - 2];
	}

	/**
	 * Returns the earliest view on the view stack, if any
	 */
	UIElement *baseView() const {
		return _views.empty() ? nullptr : _views[0];
	}

	/**
	 * Returns true if a view of a given name is present
	 * at all in the visible view stack
	 */
	bool isPresent(const Common::String &name) const;

	/**
	 * Returns the underlying screen
	 */
	Graphics::Screen *getScreen() const {
		return _screen;
	}

	/**
	 * Draws the focused view
	 */
	void drawElements() override;

	/**
	 * Events manager doesn't have any intrinsic drawing
	 */
	void draw() override {
	}

	/**
	 * Called once every game frame
	 */
	bool tick() override {
		return !_views.empty() ? focusedView()->tick() : false;
	}

	/**
	 * Calling the close method for g_events closes the active view
	 */
	void close() override {
		focusedView()->close();
	}

	/**
	 * Updates the underlying screen
	 */
	void updateScreen();

	/**
	 * Returns a font object that GfxSurface instances can use
	 */
	virtual Graphics::Font *createFont() const;
};

extern Events *g_events;

} // namespace Views
} // namespace Graphics

#endif
