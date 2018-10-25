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

#include "gargoyle/windows.h"
#include "gargoyle/window_graphics.h"
#include "gargoyle/window_pair.h"
#include "gargoyle/window_text_buffer.h"
#include "gargoyle/window_text_grid.h"
#include "gargoyle/conf.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/screen.h"
#include "gargoyle/streams.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Gargoyle {

bool Windows::_overrideReverse;
bool Windows::_overrideFgSet;
bool Windows::_overrideBgSet;
bool Windows::_forceRedraw;
bool Windows::_claimSelect;
bool Windows::_moreFocus;
int Windows::_overrideFgVal;
int Windows::_overrideBgVal;
int Windows::_zcolor_fg;
int Windows::_zcolor_bg;
byte Windows::_zcolor_LightGrey[3];
byte Windows::_zcolor_Foreground[3];
byte Windows::_zcolor_Background[3];
byte Windows::_zcolor_Bright[3];

/*--------------------------------------------------------------------------*/

Windows::Windows(Graphics::Screen *screen) : _screen(screen), _windowList(nullptr),
		_rootWin(nullptr), _focusWin(nullptr) {
	_overrideReverse = false;
	_overrideFgSet = false;
	_overrideBgSet = false;
	_forceRedraw = true;
	_claimSelect = false;
	_moreFocus = false;
	_overrideFgVal = 0;
	_overrideBgVal = 0;
	_zcolor_fg = _zcolor_bg = 0;
	_drawSelect = false;

	_zcolor_LightGrey[0] = _zcolor_LightGrey[1] = _zcolor_LightGrey[2] = 181;
	_zcolor_Foreground[0] = _zcolor_Foreground[1] = _zcolor_Foreground[2] = 0;
	_zcolor_Background[0] = _zcolor_Background[1] = _zcolor_Background[2] = 0;
	_zcolor_Bright[0] = _zcolor_Bright[1] = _zcolor_Bright[2] = 0;
}

Window *Windows::windowOpen(Window *splitwin, glui32 method, glui32 size,
		glui32 wintype, glui32 rock) {
	Window *newwin, *oldparent;
	PairWindow *pairWin;
	glui32 val;

	_forceRedraw = true;

	if (!_rootWin) {
		if (splitwin) {
			warning("window_open: ref must be NULL");
			return nullptr;
		}

		/* ignore method and size now */
		oldparent = NULL;
	} else {
		if (!splitwin) {
			warning("window_open: ref must not be NULL");
			return nullptr;
		}

		val = (method & winmethod_DivisionMask);
		if (val != winmethod_Fixed && val != winmethod_Proportional)
		{
			warning("window_open: invalid method (not fixed or proportional)");
			return nullptr;
		}

		val = (method & winmethod_DirMask);
		if (val != winmethod_Above && val != winmethod_Below
			&& val != winmethod_Left && val != winmethod_Right)
		{
			warning("window_open: invalid method (bad direction)");
			return nullptr;
		}

		oldparent = splitwin->_parent;
		if (oldparent && oldparent->_type != wintype_Pair)
		{
			warning("window_open: parent window is not Pair");
			return nullptr;
		}
	}

	assert(wintype != wintype_Pair);
	newwin = newWindow(wintype, rock);
	if (!newwin) {
		warning("window_open: unable to create window");
		return nullptr;
	}

	if (!splitwin) {
		_rootWin = newwin;
	} else {
		// create pairWin, with newwin as the key
		pairWin = newPairWindow(method, newwin, size);
		pairWin->_child1 = splitwin;
		pairWin->_child2 = newwin;

		splitwin->_parent = pairWin;
		newwin->_parent = pairWin;
		pairWin->_parent = oldparent;

		if (oldparent) {
			PairWindow *parentWin = dynamic_cast<PairWindow *>(oldparent);
			assert(parentWin);
			if (parentWin->_child1 == splitwin)
				parentWin->_child1 = pairWin;
			else
				parentWin->_child2 = pairWin;
		} else {
			_rootWin = pairWin;
		}
	}

	rearrange();

	return newwin;
}

void Windows::windowClose(Window *win, StreamResult *result) {
	if (win == _rootWin || win->_parent == nullptr) {
		// Close the root window, which means all windows.
		_rootWin = nullptr;

		// Begin (simpler) closation
		win->_stream->fillResult(result);
		win->close(true);
	} else {
		// Have to jigger parent
		Window *sibWin;
		PairWindow *pairWin = dynamic_cast<PairWindow *>(win->_parent);
		PairWindow *grandparWin;

		if (win == pairWin->_child1) {
			sibWin = pairWin->_child2;
		} else if (win == pairWin->_child2) {
			sibWin = pairWin->_child1;
		} else {
			warning("windowClose: window tree is corrupted");
			return;
		}

		grandparWin = dynamic_cast<PairWindow *>(pairWin->_parent);
		if (!grandparWin) {
			_rootWin = sibWin;
			sibWin->_parent = nullptr;
		} else {
			if (grandparWin->_child1 == pairWin)
				grandparWin->_child1 = sibWin;
			else
				grandparWin->_child2 = sibWin;
			sibWin->_parent = grandparWin;
		}

		// Begin closation
		win->_stream->fillResult(result);

		// Close the child window (and descendants), so that key-deletion can
		// crawl up the tree to the root window.
		win->close(true);

		// This probably isn't necessary, but the child *is* gone, so just in case.
		if (win == pairWin->_child1)
			pairWin->_child1 = nullptr;
		else if (win == pairWin->_child2)
			pairWin->_child2 = nullptr;

		// Now we can delete the parent pair.
		pairWin->close(false);

		// Sort out the arrangements
		rearrange();
	}
}

Window *Windows::newWindow(glui32 type, glui32 rock) {
	Window *win;

	switch (type) {
	case wintype_Blank:
		win = new BlankWindow(this, rock);
		break;
	case wintype_TextGrid:
		win = new TextGridWindow(this, rock);
		break;
	case wintype_TextBuffer:
		win = new TextBufferWindow(this, rock);
		break;
	case wintype_Graphics:
		win = new GraphicsWindow(this, rock);
		break;
	case wintype_Pair:
		error("Pair windows cannot be created directly");
	default:
		error("Unknown window type");
	}

	win->_next = _windowList;
	_windowList = win;
	if (win->_next)
		win->_next->_prev = win;

	return win;
}

PairWindow *Windows::newPairWindow(glui32 method, Window *key, glui32 size) {
	PairWindow *pwin = new PairWindow(this, method, key, size);
	pwin->_next = _windowList;
	_windowList = pwin;
	if (pwin->_next)
		pwin->_next->_prev = pwin;

	return pwin;
}

void Windows::rearrange() {
	if (_rootWin) {
		Common::Rect box;

		if (g_conf->_lockCols) {
			int desired_width = g_conf->_wMarginSaveX * 2 + g_conf->_cellW * g_conf->_cols;
			if (desired_width > g_conf->_imageW)
				g_conf->_wMarginX = g_conf->_wMarginSaveX;
			else
				g_conf->_wMarginX = (g_conf->_imageW - g_conf->_cellW * g_conf->_cols) / 2;
		}

		if (g_conf->_lockRows) {
			int desired_height = g_conf->_wMarginSaveY * 2 + g_conf->_cellH * g_conf->_rows;
			if (desired_height > g_conf->_imageH)
				g_conf->_wMarginY = g_conf->_wMarginSaveY;
			else
				g_conf->_wMarginY = (g_conf->_imageH - g_conf->_cellH * g_conf->_rows) / 2;
		}

		box.left = g_conf->_wMarginX;
		box.top = g_conf->_wMarginY;
		box.right = g_conf->_imageW - g_conf->_wMarginX;
		box.bottom = g_conf->_imageH - g_conf->_wMarginY;

		_rootWin->rearrange(box);
	}
}

void Windows::selectionChanged() {
	_claimSelect = false;
	_forceRedraw = true;
	redraw();
}

void Windows::redraw() {
	_claimSelect = false;

	if (_forceRedraw) {
		repaint(Common::Rect(0, 0, g_conf->_imageW, g_conf->_imageH));
		g_vm->_screen->fill(g_conf->_windowColor);
	}

	if (_rootWin)
		_rootWin->redraw();

	if (_moreFocus)
		refocus(_focusWin);

	_forceRedraw = 0;
}

void Windows::redrawRect(const Common::Rect &r) {
	_drawSelect = true;
	repaint(r);
}

void Windows::repaint(const Common::Rect &box) {
	// No implementation
}

byte *Windows::rgbShift(byte *rgb) {
	_zcolor_Bright[0] = (rgb[0] + 0x30) < 0xff ? (rgb[0] + 0x30) : 0xff;
	_zcolor_Bright[1] = (rgb[1] + 0x30) < 0xff ? (rgb[1] + 0x30) : 0xff;
	_zcolor_Bright[2] = (rgb[2] + 0x30) < 0xff ? (rgb[2] + 0x30) : 0xff;

	return _zcolor_Bright;
}

/*--------------------------------------------------------------------------*/

Windows::iterator &Windows::iterator::operator++() {
	_current = _windows->iterateTreeOrder(_current);
	return *this;
}

void Windows::refocus(Window *win) {
	Window *focus = win;
	do {
		if (focus && focus->_moreRequest) {
			_focusWin = focus;
			return;
		}

		focus = iterateTreeOrder(focus);
	} while (focus != win);

	_moreFocus = false;
}

Window *Windows::iterateTreeOrder(Window *win) {
	if (!win)
		return _rootWin;

	PairWindow *pairWin = dynamic_cast<PairWindow *>(win);
	if (pairWin) {
		if (!pairWin->_backward)
			return pairWin->_child1;
		else
			return pairWin->_child2;
	} else {
		while (win->_parent) {
			pairWin = dynamic_cast<PairWindow *>(win->_parent);
			assert(pairWin);

			if (!pairWin->_backward) {
				if (win == pairWin->_child1)
					return pairWin->_child2;
			} else {
				if (win == pairWin->_child2)
					return pairWin->_child1;
			}

			win = pairWin;
		}

		return nullptr;
	}
}

/*--------------------------------------------------------------------------*/

Window::Window(Windows *windows, glui32 rock) : _windows(windows), _rock(rock),
		_type(0), _parent(nullptr), _next(nullptr), _prev(nullptr), _yAdj(0),
		_lineRequest(0), _lineRequestUni(0), _charRequest(0), _charRequestUni(0),
		_mouseRequest(0), _hyperRequest(0), _moreRequest(0), _scrollRequest(0), _imageLoaded(0),
		_echoLineInputBase(true), _lineTerminatorsBase(nullptr), _termCt(0), _echoStream(nullptr) {
	_attr.fgset = 0;
	_attr.bgset = 0;
	_attr.reverse = 0;
	_attr.style = 0;
	_attr.fgcolor = 0;
	_attr.bgcolor = 0;
	_attr.hyper = 0;

	Common::fill(&_bgColor[0], &_bgColor[3], 3);
	Common::fill(&_fgColor[0], &_fgColor[3], 3);
	_dispRock.num = 0;

	Streams &streams = *g_vm->_streams;
	_stream = streams.addWindowStream(this);
}

Window::~Window() {
	if (g_vm->gli_unregister_obj)
		(*g_vm->gli_unregister_obj)(this, gidisp_Class_Window, _dispRock);

	
	_echoStream = nullptr;
	delete _stream;

	delete[] _lineTerminatorsBase;

	Window *prev = _prev;
	Window *next = _next;

	if (prev)
		prev->_next = next;
	else
		_windows->_windowList = next;
	if (next)
		next->_prev = prev;
}

void Window::close(bool recurse) {
	if (_windows->getFocusWindow() == this)
		// Focused window is being removed
		_windows->setFocus(nullptr);

	for (Window *wx = _parent; wx; wx = wx->_parent) {
		PairWindow *pairWin = dynamic_cast<PairWindow *>(wx);

		if (pairWin && pairWin->_key == this) {
			pairWin->_key = nullptr;
			pairWin->_keyDamage = true;
		}
	}

	PairWindow *pairWin = dynamic_cast<PairWindow *>(this);
	if (pairWin) {
		pairWin->_child1->close(recurse);
		pairWin->_child2->close(recurse);
	}

	// Finally, delete the window
	delete this;
}

void Window::cancelLineEvent(Event *ev) {
	Event dummyEv;
	if (!ev)
		ev = &dummyEv;

	g_vm->_events->clearEvent(ev);
}

void Window::moveCursor(const Common::Point &newPos) {
	warning("moveCursor: not a TextGrid window");
}

void Window::requestLineEvent(char *buf, glui32 maxlen, glui32 initlen) {
	warning("requestLineEvent: window does not support keyboard input");
}

void Window::requestLineEventUni(glui32 *buf, glui32 maxlen, glui32 initlen) {
	warning("requestLineEventUni: window does not support keyboard input");
}

void Window::redraw() {
	if (Windows::_forceRedraw) {
		unsigned char *color = Windows::_overrideBgSet ? g_conf->_windowColor : _bgColor;
		int y0 = _yAdj ? _bbox.top - _yAdj : _bbox.top;
		g_vm->_screen->fillRect(_bbox.left, y0, _bbox.width(), _bbox.bottom - y0, color);
	}
}

void Window::acceptReadLine(glui32 arg) {
	warning("acceptReadLine:: window does not support keyboard input");
}

void Window::acceptReadChar(glui32 arg) {
	warning("acceptReadChar:: window does not support keyboard input");
}

bool Window::checkTerminator(glui32 ch) {
	if (ch == keycode_Escape)
		return true;
	else if (ch >= keycode_Func12 && ch <= keycode_Func1)
		return true;
	else
		return false;
}

bool Window::imageDraw(glui32 image, glui32 align, glsi32 val1, glsi32 val2) {
	if (!g_conf->_graphics)
		return false;

	TextBufferWindow *bufWin = dynamic_cast<TextBufferWindow *>(this);
	GraphicsWindow *graWin = dynamic_cast<GraphicsWindow *>(this);

	if (bufWin)
		return bufWin->drawPicture(image, val1, false, 0, 0);
	if (graWin)
		return graWin->drawPicture(image, val1, val2, false, 0, 0);

	return false;
}

/*--------------------------------------------------------------------------*/

BlankWindow::BlankWindow(Windows *windows, uint32 rock) : Window(windows, rock) {
	_type = wintype_Blank;
}

/*--------------------------------------------------------------------------*/

void Attributes::clear() {
	fgset = 0;
	bgset = 0;
	fgcolor = 0;
	bgcolor = 0;
	reverse = false;
	hyper = 0;
	style = 0;
}

byte *Attributes::attrBg(WindowStyle *styles) {
	int revset = reverse || (styles[style].reverse && !Windows::_overrideReverse);

	int zfset = fgset ? fgset : Windows::_overrideFgSet;
	int zbset = bgset ? bgset : Windows::_overrideBgSet;

	int zfore = fgset ? fgcolor : Windows::_overrideFgVal;
	int zback = bgset ? bgcolor : Windows::_overrideBgVal;

	if (zfset && zfore != Windows::_zcolor_fg) {
		Windows::_zcolor_Foreground[0] = (zfore >> 16) & 0xff;
		Windows::_zcolor_Foreground[1] = (zfore >> 8) & 0xff;
		Windows::_zcolor_Foreground[2] = (zfore)& 0xff;
		Windows::_zcolor_fg = zfore;
	}

	if (zbset && zback != Windows::_zcolor_bg) {
		Windows::_zcolor_Background[0] = (zback >> 16) & 0xff;
		Windows::_zcolor_Background[1] = (zback >> 8) & 0xff;
		Windows::_zcolor_Background[2] = (zback)& 0xff;
		Windows::_zcolor_bg = zback;
	}

	if (!revset) {
		if (zbset)
			return Windows::_zcolor_Background;
		else
			return styles[style].bg;
	} else {
		if (zfset)
			if (zfore == zback)
				return Windows::rgbShift(Windows::_zcolor_Foreground);
			else
				return Windows::_zcolor_Foreground;
		else
			if (zbset && !memcmp(styles[style].fg, Windows::_zcolor_Background, 3))
				return Windows::_zcolor_LightGrey;
			else
				return styles[style].fg;
	}
}

byte *Attributes::attrFg(WindowStyle *styles) {
	int revset = reverse || (styles[style].reverse && !Windows::_overrideReverse);

	int zfset = fgset ? fgset : Windows::_overrideFgSet;
	int zbset = bgset ? bgset : Windows::_overrideBgSet;

	int zfore = fgset ? fgcolor : Windows::_overrideFgVal;
	int zback = bgset ? bgcolor : Windows::_overrideBgVal;

	if (zfset && zfore != Windows::_zcolor_fg) {
		Windows::_zcolor_Foreground[0] = (zfore >> 16) & 0xff;
		Windows::_zcolor_Foreground[1] = (zfore >> 8) & 0xff;
		Windows::_zcolor_Foreground[2] = (zfore)& 0xff;
		Windows::_zcolor_fg = zfore;
	}

	if (zbset && zback != Windows::_zcolor_bg) {
		Windows::_zcolor_Background[0] = (zback >> 16) & 0xff;
		Windows::_zcolor_Background[1] = (zback >> 8) & 0xff;
		Windows::_zcolor_Background[2] = (zback)& 0xff;
		Windows::_zcolor_bg = zback;
	}

	if (!revset) {
		if (zfset)
			if (zfore == zback)
				return Windows::rgbShift(Windows::_zcolor_Foreground);
			else
				return Windows::_zcolor_Foreground;
		else
			if (zbset && !memcmp(styles[style].fg, Windows::_zcolor_Background, 3))
				return Windows::_zcolor_LightGrey;
			else
				return styles[style].fg;
	} else {
		if (zbset)
			return Windows::_zcolor_Background;
		else
			return styles[style].bg;
	}
}

} // End of namespace Gargoyle
