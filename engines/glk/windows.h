/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef GLK_WINDOWS_H
#define GLK_WINDOWS_H

#include "common/array.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/screen.h"
#include "glk/events.h"
#include "glk/fonts.h"
#include "glk/glk_types.h"
#include "glk/screen.h"
#include "glk/selection.h"
#include "glk/streams.h"

namespace Glk {

class Window;
class PairWindow;

#define HISTORYLEN 100
#define SCROLLBACK 512
#define TBLINELEN 300
#define GLI_SUBPIX 8


/**
 * Main windows manager
 */
class Windows {
	friend class Window;
public:
	class iterator {
	private:
		Windows *_windows;
		Window *_current;
	public:
		/**
		 * Constructor
		 */
		iterator(Windows *windows, Window *start) : _current(start) { _windows = windows; }

		/**
		 * Dereference
		 */
		Window *operator*() const {
			return _current;
		}

		/**
		 * Move to next
		 */
		iterator &operator++();

		/**
		 * Move to previous
		 */
		iterator &operator--();

		/**
		 * Equality test
		 */
		bool operator==(const iterator &i) {
			return _current == i._current;
		}

		/**
		 * Inequality test
		 */
		bool operator!=(const iterator &i) {
			return _current != i._current;
		}
	};
	friend class iterator;
private:
	Graphics::Screen *_screen;
	Window *_windowList;       ///< List of all windows
	Window *_rootWin;          ///< The topmost window
	Window *_focusWin;         ///< The window selected by the player
	bool _drawSelect;
private:
	/**
	 * Create a new window
	 */
	Window *newWindow(uint type, uint rock);

	/**
	 * Create a new pair window
	 */
	PairWindow *newPairWindow(uint method, Window *key, uint size);

	/**
	 * Set the window focus
	 */
	void refocus(Window *win);

	/**
	 * Used to loop over windows in tree order
	 */
	Window *iterateTreeOrder(Window *win);

	/**
	 * Pick first window which has a more request
	 */
	void inputMoreFocus();

	/**
	 *
	 */
	void inputNextFocus();

	/**
	 * Pick first window which might want scrolling.
	 * This is called after pressing page keys.
	 */
	void inputScrollFocus();
public:
	static bool _overrideReverse;
	static bool _overrideFgSet;
	static bool _overrideBgSet;
	static bool _forceRedraw;
	static bool _claimSelect;
	static bool _moreFocus;
	static uint _overrideFgVal;
	static uint _overrideBgVal;
	static uint _zcolor_fg, _zcolor_bg;
	static uint _zcolor_LightGrey;
	static uint _zcolor_Foreground;
	static uint _zcolor_Background;
	static uint _zcolor_Bright;

	static uint rgbShift(uint color);
public:
	/**
	 * Constructor
	 */
	Windows(Graphics::Screen *screen);

	/**
	 * Destructor
	 */
	~Windows();

	/**
	 * Open a new window
	 */
	Window *windowOpen(Window *splitwin, uint method, uint size,
					   uint wintype, uint rock);

	/**
	 * Close an existing window
	 */
	void windowClose(Window *win, StreamResult *result = nullptr);

	/**
	 * Return the root window
	 */
	Window *getRoot() const {
		return _rootWin;
	}

	/**
	 * Gets the focused window
	 */
	Window *getFocusWindow() const {
		return _focusWin;
	}

	/**
	 * Setst the focused window
	 */
	void setFocus(Window *win) {
		_focusWin = win;
	}

	/**
	 * Pick first window which might want input. This is called after every keystroke.
	 */
	void inputGuessFocus();

	/**
	 * Handle input keypress
	 */
	void inputHandleKey(uint key);

	/**
	 * Handle mouse clicks
	 */
	void inputHandleClick(const Point &pos);

	void selectionChanged();

	void clearClaimSelect() {
		_claimSelect = false;
	}

	/**
	 * Rearrange windows
	 */
	void rearrange();

	void redraw();

	void redrawRect(const Rect &r);

	/**
	 * Repaint an area of the windows
	 */
	void repaint(const Rect &box);

	/**
	 * Get an iterator that will move over the tree
	 */
	iterator begin() {
		return iterator(this, _windowList);
	}

	/**
	 * Returns the end point of window iteration
	 */
	iterator end() {
		return iterator(this, nullptr);
	}
};

/**
 * Used for the static definition of default styles
 */
struct WindowStyleStatic {
	FACES font;
	byte bg[3];
	byte fg[3];
	bool reverse;
};

/**
 * Window styles
 */
struct WindowStyle {
	FACES font;
	uint bg;
	uint fg;
	bool reverse;

	/**
	 * Constructor
	 */
	WindowStyle() : font(MONOR), fg(0), bg(0), reverse(false) {}

	/**
	 * Constructor
	 */
	WindowStyle(const WindowStyleStatic &src);

	/**
	 * Equality comparison
	 */
	bool operator==(const WindowStyle &src) const {
		return !memcmp(this, &src, sizeof(WindowStyle));
	}

	/**
	 * Returns true if the font is proportinate
	 */
	bool isProp() const {
		return font == PROPR || font == PROPI || font == PROPB || font == PROPZ;
	}

	/**
	 * Returns true ifont the font is bold
	 */
	bool isBold() const {
		return font == PROPB || font == PROPZ || font == MONOB || font == MONOZ;
	}

	/**
	 * Returns true ifont the font is italic
	 */
	bool isItalic() const {
		return font == PROPI || font == PROPZ || font == MONOI || font == MONOZ;
	}

	/**
	 * Returns a font that has the following combination of proportinate, bold, and italic
	 */
	static FACES makeFont(bool p, bool b, bool i) {
		if (p && !b && !i) return PROPR;
		if (p && !b &&  i) return PROPI;
		if (p &&  b && !i) return PROPB;
		if (p &&  b &&  i) return PROPZ;
		if (!p && !b && !i) return MONOR;
		if (!p && !b &&  i) return MONOI;
		if (!p &&  b && !i) return MONOB;
		if (!p &&  b &&  i) return MONOZ;
		return PROPR;
	}
};

/**
 * Window attributes
 */
struct Attributes {
	bool fgset      : 1;
	bool bgset      : 1;
	bool reverse    : 1;
	unsigned        : 1;
	unsigned style  : 4;
	uint fgcolor;
	uint bgcolor;
	uint hyper;

	/**
	 * Constructor
	 */
	Attributes() {
		clear();
	}

	/**
	 * Clear
	 */
	void clear();

	/**
	 * Set the style
	 */
	void set(uint s) {
		clear();
		style = s;
	}

	/**
	 * Equality comparison
	 */
	bool operator==(const Attributes &src) const {
		return fgset == src.fgset && bgset == src.bgset && reverse == src.reverse
			   && style == src.style && fgcolor == src.fgcolor && bgcolor == src.bgcolor
			   && hyper == src.hyper;
	}
	/**
	 * Inequality comparison
	 */
	bool operator!=(const Attributes &src) const {
		return fgset != src.fgset || bgset != src.bgset || reverse != src.reverse
			   || style != src.style || fgcolor != src.fgcolor || bgcolor != src.bgcolor
			   || hyper != src.hyper;
	}

	/**
	 * Return the background color for the current font style
	 */
	uint attrBg(const WindowStyle *styles);

	/**
	 * Return the foreground color for the current font style
	 */
	uint attrFg(const WindowStyle *styles);

	/**
	 * Get the font for the current font style
	 */
	FACES attrFont(const WindowStyle *styles) const {
		return styles[style].font;
	}
};

/**
 * Window definition
 */
class Window {
public:
	Windows *_windows;
	uint _rock;
	uint _type;

	Window *_parent;       ///< pair window which contains this one
	Window *_next, *_prev; ///< in the big linked list of windows
	Rect _bbox;
	int _yAdj;

	Stream *_stream;       ///< the window stream.
	Stream *_echoStream;   ///< the window's echo stream, if any.

	bool _lineRequest;
	bool _lineRequestUni;
	bool _charRequest;
	bool _charRequestUni;
	bool _mouseRequest;
	bool _hyperRequest;
	bool _moreRequest;
	bool _scrollRequest;
	bool _imageLoaded;

	uint _echoLineInputBase;
	uint *_lineTerminatorsBase;
	uint _termCt;

	Attributes _attr;
	uint _bgColor, _fgColor;

	gidispatch_rock_t _dispRock;
public:
	static bool checkBasicTerminators(uint32 ch);
public:
	/**
	 * Constructor
	 */
	Window(Windows *windows, uint rock);

	/**
	 * Destructor
	 */
	virtual ~Window();

	/**
	 * Close and delete the window
	 */
	void close(bool recurse = true);

	/**
	 * Get the font info structure associated with the window
	 */
	virtual FontInfo *getFontInfo();

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Rect &box) {
		_bbox = box;
	}

	/**
	 * Set the size of a window
	 */
	virtual void setSize(const Point &newSize) {
		_bbox.setWidth(newSize.x);
		_bbox.setHeight(newSize.y);
		rearrange(_bbox);
	}

	/**
	 * Sets the position of a window
	 */
	virtual void setPosition(const Point &newPos) {
		_bbox.moveTo(newPos);
		rearrange(_bbox);
	}

	/**
	 * Get window split size within parent pair window
	 */
	virtual uint getSplit(uint size, bool vertical) const {
		return 0;
	}

	/**
	 * Write a character
	 */
	virtual void putCharUni(uint32 ch) {}

	/**
	 * Unput a unicode character
	 */
	virtual bool unputCharUni(uint32 ch) {
		return false;
	}

	/**
	 * Get the cursor position
	 */
	virtual Point getCursor() const { return Point(); }

	/**
	 * Move the cursor
	 */
	virtual void moveCursor(const Point &newPos);

	/**
	 * Clear the window
	 */
	virtual void clear() {}

	/**
	 * Click the window
	 */
	virtual void click(const Point &newPos) {}

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEvent(char *buf, uint maxlen, uint initlen);

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEventUni(uint32 *buf, uint maxlen, uint initlen);

	/**
	 * Cancel an input line event
	 */
	virtual void cancelLineEvent(Event *ev);

	/**
	 * Cancel a character event
	 */
	virtual void cancelCharEvent() {}

	/**
	 * Cancel a mouse event
	 */
	virtual void cancelMouseEvent() {}

	/**
	 * Cancel a hyperlink event
	 */
	virtual void cancelHyperlinkEvent() {}

	/**
	 * Redraw the window
	 */
	virtual void redraw();

	bool imageDraw(uint image, uint align, int val1, int val2);
	bool imageDraw(const Common::String &image, uint align, int val1, int val2);

	int acceptScroll(uint arg);

	bool checkTerminators(uint32 ch);

	void setTerminatorsLineEvent(const uint32 *keycodes, uint count);

	virtual void acceptReadLine(uint32 arg);

	virtual void acceptReadChar(uint arg);

	virtual void getArrangement(uint *method, uint *size, Window **keyWin);

	virtual void setArrangement(uint method, uint size, Window *keyWin);

	virtual void getSize(uint *width, uint *height) const;

	virtual void requestCharEvent();

	virtual void requestCharEventUni();

	virtual void setEchoLineEvent(uint val) {}

	virtual void requestMouseEvent() {}

	virtual void requestHyperlinkEvent() {}

	virtual void flowBreak();

	virtual void eraseRect(bool whole, const Rect &box);

	virtual void fillRect(uint color, const Rect &box);

	virtual void setBackgroundColor(uint color);

	/**
	 * Returns a pointer to the styles for the window
	 */
	virtual const WindowStyle *getStyles() const;

	/**
	 * In arbitrary window positioning mode, brings a window to the front of all other windows
	 */
	void bringToFront();

	/**
	 * In arbitrary window positioning mode, sends a window to the back of all other windows
	 */
	void sendToBack();
};
typedef Window *winid_t;

/**
 * Blank window
 */
class BlankWindow : public Window {
public:
	/**
	 * Constructor
	 */
	BlankWindow(Windows *windows, uint rock);
};

/**
 * Abstract common base for the text window classes
 */
class TextWindow : public Window {
public:
	/**
	 * Constructor
	 */
	TextWindow(Windows *windows, uint rock) : Window(windows, rock) {}
};

} // End of namespace Glk

#endif
