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
 * Window attributes
 */
struct attr_t {
    unsigned fgset   : 1;
    unsigned bgset   : 1;
    unsigned reverse : 1;
    unsigned         : 1;
    unsigned style   : 4;
    unsigned fgcolor : 24;
    unsigned bgcolor : 24;
    unsigned hyper   : 32;
};

struct WindowPair {
	Window *owner;
	Window *child1, *child2;

	// split info...
	glui32 dir;             ///< winmethod_Left, Right, Above, or Below
	int vertical, backward; ///< flags
	glui32 division;        ///< winmethod_Fixed or winmethod_Proportional
	Window *key;            ///< NULL or a leaf-descendant (not a Pair)
	int keydamage;          ///< used as scratch space in window closing
	glui32 size;            ///< size value
	glui32 wborder;         ///< winMethod_Border, NoBorder
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

	attr_t attr;
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
public:
	/**
	 * Constructor
	 */
	TextGridWindow(uint32 rock);
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
