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
#include "gargoyle/glk_types.h"
#include "gargoyle/picture.h"
#include "gargoyle/stream.h"

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
private:
	Graphics::Screen *_screen;
	Window * _windowList;      ///< List of all windows
	Window *_rootWin;          ///< The topmost window
	Window *_focusWin;         ///< The window selected by the player
	bool _forceRedraw;
	bool _moreFocus;
	bool _claimSelect;
	WindowMask *_mask;
	Stream *_currentStr;
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
	static bool _confLockCols, _confLockRows;
	static int _wMarginx;
	static int _wMarginy;
	static int _wPaddingx;
	static int _wPaddingy;
	static int _wBorderx;
	static int _wBordery;
	static int _tMarginx;
	static int _tMarginy;
	static int _wMarginXsave;
	static int _wMarginYsave;
	static int _cols;
	static int _rows;
	static int _imageW, _imageH;
	static int _cellW, _cellH;
	static int _baseLine;
	static int _leading;
	static int _scrollWidth;
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
	 * Set the current output stream
	 */
	void setCurrent(Stream *stream) { _currentStr = stream; }

	/**
	 * Gets the current output stream
	 */
	Stream *getCurrent() const { return _currentStr; }

	/**
	 * Repaint an area of the windows
	 */
	void repaint(const Common::Rect &box);
};

/**
 * Window styles
 */
struct WindowStyle {
	int font;
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

	Window *parent;            ///< pair window which contains this one
	Common::Rect bbox;
	int yadj;

	Stream *_stream;      ///< the window stream.
	Stream *_echoStream;  ///< the window's echo stream, if any.

	int line_request;
	int line_request_uni;
	int char_request;
	int char_request_uni;
	int mouse_request;
	int hyper_request;
	int more_request;
	int scroll_request;
	int image_loaded;

	glui32 echo_line_input;
	glui32 *line_terminators;
	glui32 termct;

	Attributes attr;
	byte bgcolor[3];
	byte fgcolor[3];

	gidispatch_rock_t disprock;
	Window *next, *prev;       ///< in the big linked list of windows
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
	int width, height;
	TextGridRows lines;

	int curx, cury;     ///< the window cursor position

                        ///< for line input
	void *inbuf;        ///< unsigned char* for latin1, glui32* for unicode
	int inorgx, inorgy;
	int inmax;
	int incurs, inlen;
	Attributes origattr;
	gidispatch_rock_t inarrayrock;
	glui32 *line_terminators;

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
	virtual glui32 getSplit(glui32 size, bool vertical) const override {
		return vertical ? size * Windows::_cellW + Windows::_tMarginx * 2 :
			size * Windows::_cellH + Windows::_tMarginy * 2;
	}
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
	int width, height;
	int spaced;
	int dashed;

	TextBufferRows lines;
	int scrollback;

	int numchars;       ///< number of chars in last line: lines[0]
	glui32 *chars;      ///< alias to lines[0].chars
	Attributes *attrs;  ///< alias to lines[0].attrs

						///< adjust margins temporarily for images
	int ladjw;
	int ladjn;
	int radjw;
	int radjn;

	/* Command history. */
	glui32 *history[HISTORYLEN];
	int historypos;
	int historyfirst, historypresent;

	/* for paging */
	int lastseen;
	int scrollpos;
	int scrollmax;

	/* for line input */
	void *inbuf;        ///< unsigned char* for latin1, glui32* for unicode
	int inmax;
	long infence;
	long incurs;
	Attributes origattr;
	gidispatch_rock_t inarrayrock;

	glui32 echo_line_input;
	glui32 *line_terminators;

	/* style hints and settings */
	WindowStyle styles[style_NUMSTYLES];

	/* for copy selection */
	glui32 *copybuf;
	int copypos;
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
	virtual glui32 getSplit(glui32 size, bool vertical) const override {
		return (vertical) ? size * Windows::_cellW : size * Windows::_cellH;
	}

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
	unsigned char bgnd[3];
	bool dirty;
	int w, h;
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
	int _keydamage;            ///< used as scratch space in window closing
	glui32 _size;              ///< size value
	glui32 _wborder;           ///< winMethod_Border, NoBorder
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
