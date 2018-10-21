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
#include "gargoyle/events.h"
#include "gargoyle/glk_types.h"
#include "gargoyle/fonts.h"
#include "gargoyle/picture.h"
#include "gargoyle/streams.h"

namespace Gargoyle {

class Window;
class PairWindow;
struct WindowMask;

#define HISTORYLEN 100
#define SCROLLBACK 512
#define TBLINELEN 300

/**
 * Main windows manager
 */
class Windows {
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
	bool _forceRedraw;
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
	static int _overrideFgVal;
	static int _overrideBgVal;
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

	void clearSelection();

	/**
	 * Repaint an area of the windows
	 */
	void repaint(const Common::Rect &box);

	/**
	 * Get an iterator that will move over the tree
	 */
	iterator begin() { return iterator(_windowList); }

	/**
	 * Returns the end point of window iteration
	 */
	iterator end() { return iterator(nullptr); }
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
};

struct WindowMask {
	int hor;
	int ver;
	glui32 **links;
	Common::Rect select;

	WindowMask() : hor(0), ver(0), links(nullptr) {}
};

/**
 * Window definition
 */
class Window {
public:
	Windows *_windows;
	glui32 _magicnum;
	glui32 _rock;
	glui32 _type;

	Window *_parent;       ///< pair window which contains this one
	Window *_next, *_prev; ///< in the big linked list of windows
	Common::Rect bbox;
	int yadj;

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
	glui32 *_lineTerminators;
	glui32 _termCt;

	Attributes _attr;
	byte _bgColor[3];
	byte _fgColor[3];

	gidispatch_rock_t _dispRock;
public:
	/**
	 * Constructor
	 */
	Window(Windows *windows, uint32 rock);

	/**
	 * Destructor
	 */
	virtual ~Window() {}

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) { bbox = box; }

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const { return 0; }

	/**
	 * Cancel a line event
	 */
	virtual void cancelLineEvent(Event *ev);

	/**
	 * Write a character
	 */
	virtual void putChar(unsigned char ch) { /* TODO */ }

	/**
	 * Write a unicode character
	 */
	virtual void putCharUni(uint32 ch) { /* TODO */ }

	/**
	 * Write a buffer
	 */
	virtual void putBuffer(const unsigned char *buf, size_t len) { /* TODO */ }

	/**
	 * Write a unicode character
	 */
	virtual void putBufferUni(const uint32 *buf, size_t len) { /* TODO */ }
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
		Common::Array<uint32> chars;
		Common::Array<Attributes> attr;
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
public:
	int _width, _height;
	TextGridRows lines;

	int _curX, _curY;    ///< the window cursor position

                         ///< for line input
	void *_inBuf;        ///< unsigned char* for latin1, glui32* for unicode
	int _inorgX, _inorgY;
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
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const override;

	/**
	 * Cancel a line event
	 */
	virtual void cancelLineEvent(Event *ev) override;
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
	void putCharUni(glui32 ch);
	void putTextUnit(const glui32 *buf, int len, int pos, int oldlen);
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
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) override;

	/**
	 * Get window split size within parent pair window
	 */
	virtual glui32 getSplit(glui32 size, bool vertical) const override;

	/**
	 * Cancel a line event
	 */
	virtual void cancelLineEvent(Event *ev) override;

	/**
	 * Clear the window
	 */
	void clear();
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
	int _w, _h;
	Graphics::ManagedSurface *_surface;
public:
	/**
	 * Constructor
	 */
	GraphicsWindow(Windows *windows, uint32 rock);

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
};

} // End of namespace Gargoyle

#endif
