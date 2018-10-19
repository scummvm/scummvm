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
#include "common/stream.h"
#include "graphics/screen.h"
#include "gargoyle/glk_types.h"

namespace Gargoyle {

class Window;
class PairWindow;

class Windows {
private:
	Graphics::Screen *_screen;
	Window * _windowList;      ///< List of all windows
	Window *_rootWin;          ///< The topmost window
	Window *_focusWin;         ///< The window selected by the player
	bool _forceRedraw;
	bool _moreFocus;
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
};

/**
 * Window styles
 */
struct WindowStyle {
	int font;
	byte bg[3];
	byte fg[3];
	int reverse;

	WindowStyle();
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

/**
 * Window definition
 */
class Window {
public:
	glui32 _magicnum;
	glui32 _rock;
	glui32 _type;

	Window *parent;               ///< pair window which contains this one
	Common::Rect bbox;
	int yadj;

	Common::WriteStream *str;     ///< the window stream.
	Common::WriteStream *echostr; ///< the window's echo stream, if any.

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
	Window *next, *prev;          ///< in the big linked list of windows
public:
	/**
	 * Constructor
	 */
	Window(uint32 rock);

	/**
	 * Destructor
	 */
	virtual ~Window() {}

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box) { bbox = box; }
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
	BlankWindow(uint32 rock);
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
	TextGridWindow(uint32 rock);

	/**
	 * Rearranges the window
	 */
	virtual void rearrange(const Common::Rect &box);
};

/**
 * Text Buffer window
 */
class TextBufferWindow : public Window {
public:
	/**
	 * Constructor
	 */
	TextBufferWindow(uint32 rock);
};

/**
 * Graphics window
 */
class GraphicsWindow : public Window {
public:
	/**
	 * Constructor
	 */
	GraphicsWindow(uint32 rock);
};

/**
 * Pair window
 */
class PairWindow : public Window {
public:
	Window *child1, *child2;

	/* split info... */
	glui32 dir;             ///< winmethod_Left, Right, Above, or Below
	int vertical, backward; ///< flags
	glui32 division;        ///< winmethod_Fixed or winmethod_Proportional
	Window *key;            ///< NULL or a leaf-descendant (not a Pair)
	int keydamage;          ///< used as scratch space in window closing
	glui32 size;            ///< size value
	glui32 wborder;         ///< winMethod_Border, NoBorder
public:
	/**
	 * Constructor
	 */
	PairWindow(glui32 method, Window *_key, glui32 _size);
};

} // End of namespace Gargoyle

#endif
