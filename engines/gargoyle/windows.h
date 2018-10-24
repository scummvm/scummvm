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

#ifndef GARGOYLE_WINDOWS_H
#define GARGOYLE_WINDOWS_H

#include "common/array.h"
#include "common/list.h"
#include "common/rect.h"
#include "graphics/screen.h"
#include "gargoyle/draw.h"
#include "gargoyle/events.h"
#include "gargoyle/glk_types.h"
#include "gargoyle/fonts.h"
#include "gargoyle/streams.h"
#include "gargoyle/window_mask.h"

namespace Gargoyle {

class Window;
class PairWindow;

#define HISTORYLEN 100
#define SCROLLBACK 512
#define TBLINELEN 300
#define GLI_SUBPIX 8


/**
 * Main windows manager
 */
class Windows : public Draw {
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
		iterator(Windows *windows, Window *start) : _windows(windows), _current(start) {}

		/**
		 * Dereference
		 */
		Window *operator*() const { return _current; }

		/**
		 * Move to next
		 */
		iterator &operator++();

		/**
		 * Equality test
		 */
		bool operator==(const iterator &i) { return _current == i._current; }

		/**
		 * Inequality test
		 */
		bool operator!=(const iterator &i) { return _current != i._current; }
	};
	friend class iterator;
private:
	Graphics::Screen *_screen;
	Window * _windowList;      ///< List of all windows
	Window *_rootWin;          ///< The topmost window
	Window *_focusWin;         ///< The window selected by the player
	bool _drawSelect;
private:
	/**
	 * Create a new window
	 */
	Window *newWindow(glui32 type, glui32 rock);

	/**
	 * Create a new pair window
	 */
	PairWindow *newPairWindow(glui32 method, Window *key, glui32 size);

	/**
	 * Rearrange windows
	 */
	void rearrange();

	void refocus(Window *win);

	Window *iterateTreeOrder(Window *win);
public:
	static bool _overrideReverse;
	static bool _overrideFgSet;
	static bool _overrideBgSet;
	static bool _forceRedraw;
	static bool _claimSelect;
	static bool _moreFocus;
	static int _overrideFgVal;
	static int _overrideBgVal;
	static int _zcolor_fg, _zcolor_bg;
	static byte _zcolor_LightGrey[3];
	static byte _zcolor_Foreground[3];
	static byte _zcolor_Background[3];
	static byte _zcolor_Bright[3];

	static byte *rgbShift(byte *rgb);
public:
	/**
	 * Constructor
	 */
	Windows(Graphics::Screen *screen);

	/**
	 * Open a new window
	 */
	Window *windowOpen(Window *splitwin, glui32 method, glui32 size,
		glui32 wintype, glui32 rock);

	/**
	 * Return the root window
	 */
	Window *getRoot() const { return _rootWin; }

	/**
	 * Gets the focused window
	 */
	Window *getFocusWindow() const { return _focusWin; }

	/**
	 * Setst the focused window
	 */
	void setFocus(Window *win) { _focusWin = win; }

	void selectionChanged();

	void clearClaimSelect() { _claimSelect = false; }

	void redraw();

	void redrawRect(const Common::Rect &r);

	/**
	 * Repaint an area of the windows
	 */
	void repaint(const Common::Rect &box);

	/**
	 * Draw an area of the windows
	 */
	void drawRect(int x0, int y0, int w, int h, const byte *rgb);

	/**
	 * Get an iterator that will move over the tree
	 */
	iterator begin() { return iterator(this, _windowList); }

	/**
	 * Returns the end point of window iteration
	 */
	iterator end() { return iterator(this, nullptr); }
};

/**
 * Window styles
 */
struct WindowStyle {
	FACES font;
	byte bg[3];
	byte fg[3];
	int reverse;
};

/**
 * Window attributes
 */
struct Attributes {
    unsigned fgset   : 1;
    unsigned bgset   : 1;
    unsigned reverse : 1;
    unsigned         : 1;
    unsigned style   : 4;
    unsigned fgcolor : 24;
    unsigned bgcolor : 24;
    unsigned hyper   : 32;

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
	void set(glui32 s) {
		clear();
		style = s;
	}

	/**
	 * Equality comparison
	 */
	bool operator==(const Attributes &src) {
		return fgset == src.fgset && bgset == src.bgset && reverse == src.reverse
			&& style == src.style && fgcolor == src.fgcolor && bgcolor == src.bgcolor
			&& hyper == src.hyper;
	}

	byte *attrBg(WindowStyle *styles);
	byte *attrFg(WindowStyle *styles);

	/**
	 * Get the font from the attribute's style
	 */
	FACES attrFont(WindowStyle *styles) const { return styles[style].font; }
};

/**
 * Window definition
 */
class Window : public Draw {
public:
	Windows *_windows;
	glui32 _rock;
	glui32 _type;

	Window *_parent;       ///< pair window which contains this one
	Window *_next, *_prev; ///< in the big linked list of windows
	Common::Rect _bbox;
	int _yAdj;

	Stream *_stream;       ///< the window stream.
	Stream *_echoStream;   ///< the window's echo stream, if any.

	int _lineRequest;
	int _lineRequestUni;
	int _charRequest;
	int _charRequestUni;
	int _mouseRequest;
	int _hyperRequest;
	int _moreRequest;
	int _scrollRequest;
	int _imageLoaded;

	glui32 _echoLineInputBase;
	glui32 *_lineTerminatorsBase;
	glui32 _termCt;

	Attributes _attr;
	byte _bgColor[3];
	byte _fgColor[3];

	gidispatch_rock_t _dispRock;
protected:
	bool checkTerminator(glui32 ch);
public:
	/**
	 * Constructor
	 */
	Window(Windows *windows, uint32 rock);

	/**
	 * Destructor
	 */
	virtual ~Window();

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) { _bbox = box; }

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const { return 0; }

	/**
	 * Write a character
	 */
	virtual void putCharUni(uint32 ch) {}

	/**
	 * Unput a unicode character
	 */
	virtual bool unputCharUni(uint32 ch) { return false; }

	/**
	 * Move the cursor
	 */
	virtual void moveCursor(const Common::Point &newPos);

	/**
	 * Clear the window
	 */
	virtual void clear() {}

	/**
	 * Click the window
	 */
	virtual void click(const Common::Point &newPos) {}

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEvent(char *buf, glui32 maxlen, glui32 initlen);

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEventUni(glui32 *buf, glui32 maxlen, glui32 initlen);

	/**
	 * Cancel an input line event
	 */
	virtual void cancelLineEvent(Event *ev);

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

	virtual glui32 imageDraw(glui32 image, glui32 align, bool scaled, glui32 width = 0,
		glui32 height = 0) { return false; }

	virtual void acceptReadLine(glui32 arg);

	virtual void acceptReadChar(glui32 arg);

	int acceptScroll(glui32 arg);
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
	BlankWindow(Windows *windows, uint32 rock);
};

} // End of namespace Gargoyle

#endif
