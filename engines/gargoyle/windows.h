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
#include "gargoyle/picture.h"
#include "gargoyle/streams.h"
#include "gargoyle/window_mask.h"

namespace Gargoyle {

class Window;
class PairWindow;

#define HISTORYLEN 100
#define SCROLLBACK 512
#define TBLINELEN 300

/**
 * Main windows manager
 */
class Windows {
	friend class Window;
public:
	class iterator {
	private:
		Window *_current;
	public:
		/**
		 * Constructor
		 */
		iterator(Window *start) : _current(start) {}

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
private:
	Graphics::Screen *_screen;
	Window * _windowList;      ///< List of all windows
	Window *_rootWin;          ///< The topmost window
	Window *_focusWin;         ///< The window selected by the player
	bool _moreFocus;
	bool _claimSelect;
	WindowMask *_mask;
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
public:
	static bool _overrideReverse;
	static bool _overrideFgSet;
	static bool _overrideBgSet;
	static bool _forceRedraw;
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
	 * Destructor
	 */
	~Windows();

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

	void clearSelection();

	void selectionChanged();

	void clearClaimSelect() { _claimSelect = false; }

	void redraw();

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
	iterator begin() { return iterator(_windowList); }

	/**
	 * Returns the end point of window iteration
	 */
	iterator end() { return iterator(nullptr); }

	/**
	 * Gets a hyperlink
	 */
	glui32 getHyperlink(const Common::Point &pos) { return _mask->getHyperlink(pos); }

	/**
	 * Sets a hyperlink
	 */
	void setHyperlink(glui32 linkval, uint x0, uint y0, uint x1, uint y1) {
		return _mask->putHyperlink(linkval, x0, y0, x1, y1);
	}
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
	glui32 _magicnum;
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

	glui32 _echoLineInput;
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
	virtual void putChar(unsigned char ch) {}

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) {}

	/**
	 * Unput a unicode character
	 */
	virtual bool unputCharUni(uint32 ch) { return false; }

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const unsigned char *buf, size_t len) {}

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) {}

	/**
	 * Move the cursor
	 */
	virtual void moveCursor(const Common::Point &newPos) {}

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

/**
 * Text Grid window
 */
class TextGridWindow : public Window {
	/**
	 * Structure for a row within the grid window
	 */
	struct TextGridRow {
		Common::Array<uint32> _chars;
		Common::Array<Attributes> _attrs;
		bool dirty;

		/**
		 * Constructor
		 */
		TextGridRow() : dirty(false) {}

		/**
		 * Resize the row
		 */
		void resize(size_t newSize);
	};
	typedef Common::Array<TextGridRow> TextGridRows;
private:
	/**
	 * Mark a given text row as modified
	 */
	void touch(int line);

	void acceptReadChar(glui32 arg);

	/**
	 * Return or enter, during line input. Ends line input. 
	 */
	void acceptLine(glui32 keycode);

	/**
	 * Any regular key, during line input.
	 */
	void acceptReadLine(glui32 arg);
public:
	int _width, _height;
	TextGridRows _lines;

	int _curX, _curY;    ///< the window cursor position

						 ///< for line input
	void *_inBuf;        ///< unsigned char* for latin1, glui32* for unicode
	int _inOrgX, _inOrgY;
	int _inMax;
	int _inCurs, _inLen;
	Attributes _origAttr;
	gidispatch_rock_t _inArrayRock;
	glui32 *_lineTerminators;

	WindowStyle styles[style_NUMSTYLES]; ///< style hints and settings
public:
	/**
	 * Constructor
	 */
	TextGridWindow(Windows *windows, uint32 rock);

	/**
	 * Destructor
	 */
	virtual ~TextGridWindow();

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const override;

	/**
	 * Write a character
	 */
	virtual void putChar(unsigned char ch) override;

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) override;

	/**
	 * Unput a unicode character
	 */
	virtual bool unputCharUni(uint32 ch) override;

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const unsigned char *buf, size_t len) override;

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) override;

	/**
	 * Move the cursor
	 */
	virtual void moveCursor(const Common::Point &newPos) override;

	/**
	 * Clear the window
	 */
	virtual void clear() override;

	/**
	 * Click the window
	 */
	virtual void click(const Common::Point &newPos) override;

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEvent(char *buf, glui32 maxlen, glui32 initlen) override;

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEventUni(glui32 *buf, glui32 maxlen, glui32 initlen) override;

	/**
	 * Cancel an input line event
	 */
	virtual void cancelLineEvent(Event *ev) override;

	/**
	 * Cancel a mouse event
	 */
	virtual void cancelMouseEvent() override { _mouseRequest = false; }

	/**
	 * Cancel a hyperlink event
	 */
	virtual void cancelHyperlinkEvent() override { _hyperRequest = false; }

	/**
	 * Redraw the window
	 */
	virtual void redraw() override;
};

/**
 * Text Buffer window
 */
class TextBufferWindow : public Window {
	/**
	 * Structure for a row within the window
	 */
	struct TextBufferRow {
		Common::Array<uint32> chars;
		Common::Array<Attributes> attr;
		int len, newline;
		bool dirty, repaint;
		Picture *lpic, *rpic;
		glui32 lhyper, rhyper;
		int lm, rm;

		/**
		 * Constructor
		 */
		TextBufferRow();

		/**
		 * Resize the row
		 */
		void resize(size_t newSize);
	};
	typedef Common::Array<TextBufferRow> TextBufferRows;
private:
	void reflow();
	void touchScroll();
	bool putPicture(Picture *pic, glui32 align, glui32 linkval);
	void putTextUni(const glui32 *buf, int len, int pos, int oldlen);
	void flowBreak();

	/**
	 * Mark a given text row as modified
	 */
	void touch(int line);
public:
	int _width, _height;
	int _spaced;
	int _dashed;

	TextBufferRows _lines;
	int _scrollBack;

	int _numChars;        ///< number of chars in last line: lines[0]
	glui32 *_chars;       ///< alias to lines[0].chars
	Attributes *_attrs;  ///< alias to lines[0].attrs

    ///< adjust margins temporarily for images
	int _ladjw;
	int _ladjn;
	int _radjw;
	int _radjn;

	/* Command history. */
	glui32 *_history[HISTORYLEN];
	int _historyPos;
	int _historyFirst, _historyPresent;

	/* for paging */
	int _lastSeen;
	int _scrollPos;
	int _scrollMax;

	/* for line input */
	void *_inBuf;        ///< unsigned char* for latin1, glui32* for unicode
	int _inMax;
	long _inFence;
	long _inCurs;
	Attributes _origAttr;
	gidispatch_rock_t _inArrayRock;

	glui32 _echoLineInput;
	glui32 *_lineTerminators;

	/* style hints and settings */
	WindowStyle styles[style_NUMSTYLES];

	/* for copy selection */
	glui32 *_copyBuf;
	int _copyPos;
public:
	/**
	 * Constructor
	 */
	TextBufferWindow(Windows *windows, uint32 rock);

	/**
	 * Destructor
	 */
	virtual ~TextBufferWindow();

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const override;

	/**
	 * Write a character
	 */
	virtual void putChar(unsigned char ch) override;

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) override;

	/**
	 * Unput a unicode character
	 */
	virtual bool unputCharUni(uint32 ch) override;

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const unsigned char *buf, size_t len) override;

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) override;

	/**
	 * Move the cursor
	 */
	virtual void moveCursor(const Common::Point &newPos) override;

	/**
	 * Clear the window
	 */
	virtual void clear() override;

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEvent(char *buf, glui32 maxlen, glui32 initlen) override;

	/**
	 * Prepare for inputing a line
	 */
	virtual void requestLineEventUni(glui32 *buf, glui32 maxlen, glui32 initlen) override;

	/**
	 * Cancel an input line event
	 */
	virtual void cancelLineEvent(Event *ev) override;

	/**
	 * Cancel a hyperlink event
	 */
	virtual void cancelHyperlinkEvent() override { _hyperRequest = false; }

	/**
	 * Redraw the window
	 */
	virtual void redraw() override;
};

/**
 * Graphics window
 */
class GraphicsWindow : public Window {
private:
	void touch();
public:
	unsigned char _bgnd[3];
	bool _dirty;
	glui32 _w, _h;
	Graphics::ManagedSurface *_surface;
public:
	/**
	 * Constructor
	 */
	GraphicsWindow(Windows *windows, uint32 rock);

	/**
	 * Destructor
	 */
	virtual ~GraphicsWindow();

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const override {
		return size;
	}

	/**
	 * Cancel a mouse event
	 */
	virtual void cancelMouseEvent() override { _mouseRequest = false; }

	/**
	 * Cancel a hyperlink event
	 */
	virtual void cancelHyperlinkEvent() override { _hyperRequest = false; }

	/**
	 * Redraw the window
	 */
	virtual void redraw() override;

	/**
	 * Get the window dimensions
	 */
	void getSize(glui32 *w, glui32 *h) {
		*w = _w;
		*h = _h;
	}
};

/**
 * Pair window
 */
class PairWindow : public Window {
public:
	Window *_child1, *_child2;

	/* split info... */
	glui32 _dir;               ///< winmethod_Left, Right, Above, or Below
	bool _vertical, _backward; ///< flags
	glui32 _division;          ///< winmethod_Fixed or winmethod_Proportional
	Window *_key;              ///< NULL or a leaf-descendant (not a Pair)
	int _keyDamage;            ///< used as scratch space in window closing
	glui32 _size;              ///< size value
	glui32 _wBorder;           ///< winMethod_Border, NoBorder
public:
	/**
	 * Constructor
	 */
	PairWindow(Windows *windows, glui32 method, Window *key, glui32 size);

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) override;

	/**
	 * Redraw the window
	 */
	virtual void redraw() override;
};

} // End of namespace Gargoyle

#endif
