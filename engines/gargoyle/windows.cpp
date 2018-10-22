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
#include "gargoyle/conf.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/streams.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Gargoyle {

#define MAGIC_WINDOW_NUM (9876)
#define GLI_SUBPIX 8

bool Windows::_overrideReverse;
bool Windows::_overrideFgSet;
bool Windows::_overrideBgSet;
bool Windows::_forceRedraw;
int Windows::_overrideFgVal;
int Windows::_overrideBgVal;
int Windows::_zcolor_fg;
int Windows::_zcolor_bg;
byte Windows::_zcolor_LightGrey[3];
byte Windows::_zcolor_Foreground[3];
byte Windows::_zcolor_Background[3];
byte Windows::_zcolor_Bright[3];

/*--------------------------------------------------------------------------*/

Windows::iterator &Windows::iterator::operator++() {
	if (!_current)
		return *this;

	PairWindow *pairWin = dynamic_cast<PairWindow *>(_current);

	if (pairWin) {
		_current = !pairWin->_backward ? pairWin->_child1 : pairWin->_child2;
	} else {
		while (_current->_parent) {
			pairWin = dynamic_cast<PairWindow *>(_current->_parent);

			if (!pairWin->_backward) {
				if (_current == pairWin->_child1) {
					_current = pairWin->_child2;
					return *this;
				}
			} else {
				if (_current == pairWin->_child2) {
					_current = pairWin->_child1;
					return *this;
				}
			}

			_current = pairWin;
		}

		_current = nullptr;
	}

	return *this;
}

/*--------------------------------------------------------------------------*/

Windows::Windows(Graphics::Screen *screen) :
		_screen(screen), _moreFocus(false), _windowList(nullptr),
		_rootWin(nullptr), _focusWin(nullptr), _mask(nullptr), _claimSelect(0) {
	_mask = new WindowMask();
	_overrideReverse = false;
	_overrideFgSet = false;
	_overrideBgSet = false;
	_forceRedraw = true;
	_overrideFgVal = 0;
	_overrideBgVal = 0;
	_zcolor_fg = _zcolor_bg = 0;

	_zcolor_LightGrey[0] = _zcolor_LightGrey[1] = _zcolor_LightGrey[2] = 181;
	_zcolor_Foreground[0] = _zcolor_Foreground[1] = _zcolor_Foreground[2] = 0;
	_zcolor_Background[0] = _zcolor_Background[1] = _zcolor_Background[2] = 0;
	_zcolor_Bright[0] = _zcolor_Bright[1] = _zcolor_Bright[2] = 0;
}

Windows::~Windows() {
	delete _mask;
}

Window *Windows::windowOpen(Window *splitwin, glui32 method, glui32 size,
		glui32 wintype, glui32 rock) {
	Window *newwin, *oldparent;
	PairWindow *pairwin;
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
		// create pairwin, with newwin as the key
		pairwin = newPairWindow(method, newwin, size);
		pairwin->_child1 = splitwin;
		pairwin->_child2 = newwin;

		splitwin->_parent = pairwin;
		newwin->_parent = pairwin;
		pairwin->_parent = oldparent;

		if (oldparent) {
			PairWindow *parentWin = dynamic_cast<PairWindow *>(oldparent);
			assert(parentWin);
			if (parentWin->_child1 == splitwin)
				parentWin->_child1 = pairwin;
			else
				parentWin->_child2 = pairwin;
		} else {
			_rootWin = pairwin;
		}
	}

	rearrange();

	return newwin;
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

void Windows::clearSelection() {
	_mask->clearSelection();
}

void Windows::redraw() {
	// TODO: gli_windows_redraw
}

void Windows::repaint(const Common::Rect &box) {
	// TODO
}

void Windows::drawRect(int x0, int y0, int w, int h, const byte *rgb) {
	// TODO
}

byte *Windows::rgbShift(byte *rgb) {
	_zcolor_Bright[0] = (rgb[0] + 0x30) < 0xff ? (rgb[0] + 0x30) : 0xff;
	_zcolor_Bright[1] = (rgb[1] + 0x30) < 0xff ? (rgb[1] + 0x30) : 0xff;
	_zcolor_Bright[2] = (rgb[2] + 0x30) < 0xff ? (rgb[2] + 0x30) : 0xff;

	return _zcolor_Bright;
}

/*--------------------------------------------------------------------------*/

Window::Window(Windows *windows, glui32 rock) : _magicnum(MAGIC_WINDOW_NUM),
		_windows(windows), _rock(rock), _type(0), _parent(nullptr), _next(nullptr), _prev(nullptr),
		_yAdj(0), _lineRequest(0), _lineRequestUni(0), _charRequest(0), _charRequestUni(0),
		_mouseRequest(0), _hyperRequest(0), _moreRequest(0), _scrollRequest(0), _imageLoaded(0),
		_echoLineInput(true), _lineTerminatorsBase(nullptr), _termCt(0), _echoStream(nullptr) {
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

void Window::cancelLineEvent(Event *ev) {
	Event dummyEv;
	if (!ev)
		ev = &dummyEv;

	g_vm->_events->clearEvent(ev);
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
		_windows->drawRect(_bbox.left, y0, _bbox.width(), _bbox.bottom - y0, color);
	}
}

bool Window::checkTerminator(glui32 ch) {
	if (ch == keycode_Escape)
		return true;
	else if (ch >= keycode_Func12 && ch <= keycode_Func1)
		return true;
	else
		return false;
}

/*--------------------------------------------------------------------------*/

BlankWindow::BlankWindow(Windows *windows, uint32 rock) : Window(windows, rock) {
	_type = wintype_Blank;
}

/*--------------------------------------------------------------------------*/

TextGridWindow::TextGridWindow(Windows *windows, uint32 rock) : Window(windows, rock) {
	_type = wintype_TextGrid;
	_width = _height = 0;
	_curX = _curY = 0;
	_inBuf = nullptr;
	_inOrgX = _inOrgY = 0;
	_inMax = 0;
	_inCurs = _inLen = 0;
	_inArrayRock.num = 0;
	_lineTerminators = nullptr;

	Common::copy(&g_conf->_gStyles[0], &g_conf->_gStyles[style_NUMSTYLES], styles);
}

TextGridWindow::~TextGridWindow() {
	if (_inBuf) {
		if (g_vm->gli_unregister_arr)
			(*g_vm->gli_unregister_arr)(_inBuf, _inMax, "&+#!Cn", _inArrayRock);
		_inBuf = nullptr;
	}

	delete[] _lineTerminators;
}

void TextGridWindow::rearrange(const Common::Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;

	newwid = box.width() / g_conf->_cellW;
	newhgt = box.height() / g_conf->_cellH;

	if (newwid == _width && newhgt == _height)
		return;

	_lines.resize(newhgt);
	for (int y = 0; y < newhgt; ++y) {
		_lines[y].resize(newwid);
		touch(y);
	}

	_attr.clear();
	_width = newwid;
	_height = newhgt;
}

void TextGridWindow::touch(int line) {
	int y = _bbox.top + line * g_conf->_leading;
	_lines[line].dirty = true;
	_windows->repaint(Common::Rect(_bbox.left, y, _bbox.right, y + g_conf->_leading));
}

glui32 TextGridWindow::getSplit(glui32 size, bool vertical) const {
	return vertical ? size * g_conf->_cellW + g_conf->_tMarginX * 2 :
		size * g_conf->_cellH + g_conf->_tMarginY * 2;
}

void TextGridWindow::putChar(unsigned char ch) {

}

void TextGridWindow::putCharUni(uint32 ch) {
	TextGridRow *ln;

	// Canonicalize the cursor position. That is, the cursor may have been
	// left outside the window area; wrap it if necessary.
	if (_curX < 0) {
		_curX = 0;
	} else if (_curX >= _width) {
		_curX = 0;
		_curY++;
	}
	if (_curY < 0)
		_curY = 0;
	else if (_curY >= _height)
		return; /* outside the window */

	if (ch == '\n') {
		/* a newline just moves the cursor. */
		_curY++;
		_curX = 0;
		return;
	}

	touch(_curY);

	ln = &(_lines[_curY]);
	ln->_chars[_curX] = ch;
	ln->_attrs[_curX] = _attr;

	_curX++;
	// We can leave the cursor outside the window, since it will be
	// canonicalized next time a character is printed.
}

bool TextGridWindow::unputCharUni(uint32 ch) {
	TextGridRow *ln;
	int oldx = _curX, oldy = _curY;

	/* Move the cursor back. */
	if (_curX >= _width)
		_curX = _width - 1;
	else
		_curX--;

	/* Canonicalize the cursor position. That is, the cursor may have been
	left outside the window area; wrap it if necessary. */
	if (_curX < 0) {
		_curX = _width - 1;
		_curY--;
	}
	if (_curY < 0)
		_curY = 0;
	else if (_curY >= _height)
		return false; // outside the window

	if (ch == '\n') {
		// a newline just moves the cursor.
		if (_curX == _width - 1)
			return 1; // deleted a newline
		_curX = oldx;
		_curY = oldy;
		return 0;    // it wasn't there */
	}

	ln = &(_lines[_curY]);
	if (ln->_chars[_curX] == ch) {
		ln->_chars[_curX] = ' ';
		ln->_attrs[_curX].clear();
		touch(_curY);
		return true; // deleted the char
	} else {
		_curX = oldx;
		_curY = oldy;
		return false; // it wasn't there
	}
}

void TextGridWindow::putBuffer(const unsigned char *buf, size_t len) {
	// TODO
}

void TextGridWindow::putBufferUni(const uint32 *buf, size_t len) {
	// TODO
}

void TextGridWindow::moveCursor(const Common::Point &pos) {
	// If the values are negative, they're really huge positive numbers --
	// remember that they were cast from glui32. So set them huge and
	// let canonicalization take its course.
	_curX = (pos.x < 0) ? 32767 : pos.x;
	_curY = (pos.y < 0) ? 32767 : pos.y;
}

void TextGridWindow::clear() {
	_attr.fgset = Windows::_overrideFgSet;
	_attr.bgset = Windows::_overrideBgSet;
	_attr.fgcolor = Windows::_overrideFgSet ? Windows::_overrideFgVal : 0;
	_attr.bgcolor = Windows::_overrideBgSet ? Windows::_overrideBgVal : 0;
	_attr.reverse = false;

	for (int k = 0; k < _height; k++) {
		TextGridRow &ln = _lines[k];
		touch(k);
		for (uint j = 0; j < ln._attrs.size(); ++j) {
			ln._chars[j] = ' ';
			ln._attrs[j].clear();
		}
	}

	_curX = 0;
	_curY = 0;
}

void TextGridWindow::click(const Common::Point &newPos) {
	int x = newPos.x - _bbox.left;
	int y = newPos.y - _bbox.top;

	if (_lineRequest || _charRequest || _lineRequestUni || _charRequestUni
			|| _moreRequest || _scrollRequest)
		_windows->setFocus(this);

	if (_mouseRequest) {
		g_vm->_events->eventStore(evtype_MouseInput, this, x / g_conf->_cellW, y / g_conf->_leading);
		_mouseRequest = false;
		if (g_conf->_safeClicks)
			g_vm->_events->_forceClick = true;
	}

	if (_hyperRequest) {
		glui32 linkval = _windows->getHyperlink(newPos);
		if (linkval)
		{
			g_vm->_events->eventStore(evtype_Hyperlink, this, linkval, 0);
			_hyperRequest = false;
			if (g_conf->_safeClicks)
				g_vm->_events->_forceClick = true;
		}
	}
}

void TextGridWindow::requestLineEvent(char *buf, glui32 maxlen, glui32 initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni)
	{
		warning("request_line_event: window already has keyboard request");
		return;
	}

	if ((int)maxlen > (_width - _curX))
		maxlen = (_width - _curX);

	_inBuf = buf;
	_inMax = maxlen;
	_inLen = 0;
	_inCurs = 0;
	_inOrgX = _curX;
	_inOrgY = _curY;
	_origAttr = _attr;
	_attr.set(style_Input);

	if (initlen > maxlen)
		initlen = maxlen;

	if (initlen) {
		TextGridRow *ln = &_lines[_inOrgY];

		for (glui32 ix = 0; ix < initlen; ix++) {
			ln->_attrs[_inOrgX + ix].set(style_Input);
			ln->_chars[_inOrgX + ix] = buf[ix];
		}

		_inCurs += initlen;
		_inLen += initlen;
		_curX = _inOrgX + _inCurs;
		_curY = _inOrgY;

		touch(_inOrgY);
	}

	if (_lineTerminatorsBase && _termCt) {
		_lineTerminators = new glui32[_termCt + 1];

		if (_lineTerminators) {
			memcpy(_lineTerminators, _lineTerminatorsBase, _termCt * sizeof(glui32));
			_lineTerminators[_termCt] = 0;
		}
	}

	if (g_vm->gli_register_arr)
		_inArrayRock = (*g_vm->gli_register_arr)(buf, maxlen, "&+#!Cn");
}

void TextGridWindow::requestLineEventUni(glui32 *buf, glui32 maxlen, glui32 initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni) {
		warning("requestLineEventUni: window already has keyboard request");
		return;
	}

	if ((int)maxlen > (_width - _curX))
		maxlen = (_width - _curX);

	_inBuf = buf;
	_inMax = maxlen;
	_inLen = 0;
	_inCurs = 0;
	_inOrgX = _curX;
	_inOrgY = _curY;
	_origAttr = _attr;
	_attr.set(style_Input);

	if (initlen > maxlen)
		initlen = maxlen;

	if (initlen) {
		TextGridRow *ln = &(_lines[_inOrgY]);

		for (glui32 ix = 0; ix<initlen; ix++) {
			ln->_attrs[_inOrgX + ix].set(style_Input);
			ln->_chars[_inOrgX + ix] = buf[ix];
		}

		_inCurs += initlen;
		_inLen += initlen;
		_curX = _inOrgX + _inCurs;
		_curY = _inOrgY;

		touch(_inOrgY);
	}
	
	if (_lineTerminatorsBase && _termCt) {
		_lineTerminators = new glui32[_termCt + 1];

		if (_lineTerminators) {
			memcpy(_lineTerminators, _lineTerminatorsBase, _termCt * sizeof(glui32));
			_lineTerminators[_termCt] = 0;
		}
	}

	if (g_vm->gli_register_arr)
		_inArrayRock = (*g_vm->gli_register_arr)(buf, maxlen, "&+#!Iu");
}

void TextGridWindow::cancelLineEvent(Event *ev) {
	int ix;
	void *inbuf;
	int inmax;
	int unicode = _lineRequestUni;
	gidispatch_rock_t inarrayrock;
	TextGridRow *ln = &_lines[_inOrgY];
	Event dummyEv;

	if (!ev)
		ev = &dummyEv;

	g_vm->_events->clearEvent(ev);

	if (!_lineRequest && !_lineRequestUni)
		return;


	inbuf = _inBuf;
	inmax = _inMax;
	inarrayrock = _inArrayRock;

	if (!unicode) {
		for (ix = 0; ix<_inLen; ix++)
		{
			glui32 ch = ln->_chars[_inOrgX + ix];
			if (ch > 0xff)
				ch = '?';
			((char *)inbuf)[ix] = (char)ch;
		}
		if (_echoStream)
			_echoStream->echoLine((char *)_inBuf, _inLen);
	} else {
		for (ix = 0; ix<_inLen; ix++)
			((glui32 *)inbuf)[ix] = ln->_chars[_inOrgX + ix];
		if (_echoStream)
			_echoStream->echoLineUni((glui32 *)inbuf, _inLen);
	}

	_curY = _inOrgY + 1;
	_curX = 0;
	_attr = _origAttr;

	ev->_type = evtype_LineInput;
	ev->_window = this;
	ev->_val1 = _inLen;
	ev->_val2 = 0;

	_lineRequest = false;
	_lineRequestUni = false;

	if (_lineTerminators) {
		free(_lineTerminators);
		_lineTerminators = nullptr;
	}

	_inBuf = nullptr;
	_inMax = 0;
	_inOrgX = 0;
	_inOrgY = 0;

	if (g_vm->gli_unregister_arr)
		(*g_vm->gli_unregister_arr)(inbuf, inmax, unicode ? "&+#!Iu" : "&+#!Cn", inarrayrock);
}

void TextGridWindow::acceptReadChar(glui32 arg) {
	glui32 key;

	switch (arg)
	{
	case keycode_Erase:
		key = keycode_Delete;
		break;
	case keycode_MouseWheelUp:
	case keycode_MouseWheelDown:
		return;
	default:
		key = arg;
	}

	if (key > 0xff && key < (0xffffffff - keycode_MAXVAL + 1))
	{
		if (!(_charRequestUni) || key > 0x10ffff)
			key = keycode_Unknown;
	}

	_charRequest = false;
	_charRequestUni = false;
	g_vm->_events->eventStore(evtype_CharInput, this, key, 0);
}

void TextGridWindow::acceptLine(glui32 keycode) {
	int ix;
	void *inbuf;
	int inmax;
	gidispatch_rock_t inarrayrock;
	TextGridRow *ln = &(_lines[_inOrgY]);
	int unicode = _lineRequestUni;

	if (!_inBuf)
		return;

	inbuf = _inBuf;
	inmax = _inMax;
	inarrayrock = _inArrayRock;

	if (!unicode) {
		for (ix = 0; ix<_inLen; ix++)
			((char *)inbuf)[ix] = (char)ln->_chars[_inOrgX + ix];
		if (_echoStream)
			_echoStream->echoLine((char *)inbuf, _inLen);
	} else {
		for (ix = 0; ix<_inLen; ix++)
			((glui32 *)inbuf)[ix] = ln->_chars[_inOrgX + ix];
		if (_echoStream)
			_echoStream->echoLineUni((glui32 *)inbuf, _inLen);
	}

	_curY = _inOrgY + 1;
	_curX = 0;
	_attr = _origAttr;

	if (_lineTerminators)
	{
		glui32 val2 = keycode;
		if (val2 == keycode_Return)
			val2 = 0;
		g_vm->_events->eventStore(evtype_LineInput, this, _inLen, val2);
		free(_lineTerminators);
		_lineTerminators = NULL;
	} else {
		g_vm->_events->eventStore(evtype_LineInput, this, _inLen, 0);
	}
	_lineRequest = false;
	_lineRequestUni = false;
	_inBuf = NULL;
	_inMax = 0;
	_inOrgX = 0;
	_inOrgY = 0;

	if (g_vm->gli_unregister_arr)
		(*g_vm->gli_unregister_arr)(inbuf, inmax, unicode ? "&+#!Iu" : "&+#!Cn", inarrayrock);
}

void TextGridWindow::acceptReadLine(glui32 arg) {
	int ix;
	TextGridRow *ln = &(_lines[_inOrgY]);

	if (!_inBuf)
		return;

	if (_lineTerminators && checkTerminator(arg)) {
		glui32 *cx;
		for (cx = _lineTerminators; *cx; cx++) {
			if (*cx == arg) {
				acceptLine(arg);
				return;
			}
		}
	}

	switch (arg) {

		/* Delete keys, during line input. */

	case keycode_Delete:
		if (_inLen <= 0)
			return;
		if (_inCurs <= 0)
			return;
		for (ix = _inCurs; ix<_inLen; ix++)
			ln->_chars[_inOrgX + ix - 1] = ln->_chars[_inOrgX + ix];
		ln->_chars[_inOrgX + _inLen - 1] = ' ';
		_inCurs--;
		_inLen--;
		break;

	case keycode_Erase:
		if (_inLen <= 0)
			return;
		if (_inCurs >= _inLen)
			return;
		for (ix = _inCurs; ix<_inLen - 1; ix++)
			ln->_chars[_inOrgX + ix] = ln->_chars[_inOrgX + ix + 1];
		ln->_chars[_inOrgX + _inLen - 1] = ' ';
		_inLen--;
		break;

	case keycode_Escape:
		if (_inLen <= 0)
			return;
		for (ix = 0; ix<_inLen; ix++)
			ln->_chars[_inOrgX + ix] = ' ';
		_inLen = 0;
		_inCurs = 0;
		break;

		/* Cursor movement keys, during line input. */

	case keycode_Left:
		if (_inCurs <= 0)
			return;
		_inCurs--;
		break;

	case keycode_Right:
		if (_inCurs >= _inLen)
			return;
		_inCurs++;
		break;

	case keycode_Home:
		if (_inCurs <= 0)
			return;
		_inCurs = 0;
		break;

	case keycode_End:
		if (_inCurs >= _inLen)
			return;
		_inCurs = _inLen;
		break;

	case keycode_Return:
		acceptLine(arg);
		break;

	default:
		if (_inLen >= _inMax)
			return;

		if (arg < 32 || arg > 0xff)
			return;

		if (g_conf->_caps && (arg > 0x60 && arg < 0x7b))
			arg -= 0x20;

		for (ix = _inLen; ix>_inCurs; ix--)
			ln->_chars[_inOrgX + ix] = ln->_chars[_inOrgX + ix - 1];
		ln->_attrs[_inOrgX + _inLen].set(style_Input);
		ln->_chars[_inOrgX + _inCurs] = arg;

		_inCurs++;
		_inLen++;
	}

	_curX = _inOrgX + _inCurs;
	_curY = _inOrgY;

	touch(_inOrgY);
}

void TextGridWindow::redraw() {
	TextGridRow *ln;
	int x0, y0;
	int x, y, w;
	int i, a, b, k, o;
	glui32 link;
	int font;
	byte *fgcolor, *bgcolor;

	x0 = _bbox.left;
	y0 = _bbox.top;

	for (i = 0; i < _height; i++) {
		ln = &_lines[i];
		if (ln->dirty || Windows::_forceRedraw) {
			ln->dirty = 0;

			x = x0;
			y = y0 + i * g_conf->_leading;

			/* clear any stored hyperlink coordinates */
			_windows->setHyperlink(0, x0, y, x0 + g_conf->_cellW * _width, y + g_conf->_leading);

			a = 0;
			for (b = 0; b < _width; b++) {
				if (ln->_attrs[a] == ln->_attrs[b]) {
					link = ln->_attrs[a].hyper;
					font = ln->_attrs[a].attrFont(styles);
					fgcolor = link ? g_conf->_linkColor : ln->_attrs[a].attrFg(styles);
					bgcolor = ln->_attrs[a].attrBg(styles);
					w = (b - a) * g_conf->_cellW;
					_windows->drawRect(x, y, w, g_conf->_leading, bgcolor);
					o = x;

					for (k = a; k < b; k++) {
						drawStringUni(o * GLI_SUBPIX,
							y + g_conf->_baseLine, font, fgcolor,
							&ln->_chars[k], 1, -1);
						o += g_conf->_cellW;
					}
					if (link) {
						_windows->drawRect(x, y + g_conf->_baseLine + 1, w,
							g_conf->_linkStyle, g_conf->_linkColor);
						_windows->setHyperlink(link, x, y, x + w, y + g_conf->_leading);
					}
					x += w;
					a = b;
				}
			}
			link = ln->_attrs[a].hyper;
			font = ln->_attrs[a].attrFont(styles);
			fgcolor = link ? g_conf->_linkColor : ln->_attrs[a].attrFg(styles);
			bgcolor = ln->_attrs[a].attrBg(styles);
			w = (b - a) * g_conf->_cellW;
			w += _bbox.right - (x + w);
			_windows->drawRect(x, y, w, g_conf->_leading, bgcolor);

			o = x;
			for (k = a; k < b; k++) {
				drawStringUni(o * GLI_SUBPIX,
					y + g_conf->_baseLine, font, fgcolor,
					&ln->_chars[k], 1, -1);
				o += g_conf->_cellW;
			}
			if (link) {
				_windows->drawRect(x, y + g_conf->_baseLine + 1, w,
					g_conf->_linkStyle, g_conf->_linkColor);
				_windows->setHyperlink(link, x, y, x + w, y + g_conf->_leading);
			}
		}
	}
}

/*--------------------------------------------------------------------------*/

void TextGridWindow::TextGridRow::resize(size_t newSize) {
	_chars.clear();
	_attrs.clear();
	_chars.resize(newSize);
	_attrs.resize(newSize);
	Common::fill(&_chars[0], &_chars[0] + newSize, ' ');
}

/*--------------------------------------------------------------------------*/

TextBufferWindow::TextBufferWindow(Windows *windows, uint32 rock) : Window(windows, rock),
		_historyPos(0), _historyFirst(0), _historyPresent(0), _lastSeen(0), _scrollPos(0),
		_scrollMax(0), _scrollBack(SCROLLBACK), _width(-1), _height(-1), _inBuf(nullptr),
		_lineTerminators(nullptr), _echoLineInput(true), _ladjw(0), _radjw(0), _ladjn(0),
		_radjn(0), _numChars(0), _chars(nullptr), _attrs(nullptr),
		_spaced(0), _dashed(0), _copyBuf(0), _copyPos(0) {
	_type = wintype_TextBuffer;
	Common::fill(&_history[0], &_history[HISTORYLEN], nullptr);

	Common::copy(&g_conf->_tStyles[0], &g_conf->_tStyles[style_NUMSTYLES], styles);
}

TextBufferWindow::~TextBufferWindow() {
	if (_inBuf) {
		if (g_vm->gli_unregister_arr)
			(*g_vm->gli_unregister_arr)(_inBuf, _inMax, "&+#!Cn", _inArrayRock);
		_inBuf = nullptr;
	}

	delete[] _copyBuf;
	delete[] _lineTerminators;

	for (int i = 0; i < _scrollBack; i++) {
		if (_lines[i].lpic)
			_lines[i].lpic->decrement();
		if (_lines[i].rpic)
			_lines[i].rpic->decrement();
	}
}

void TextBufferWindow::rearrange(const Common::Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;
	int rnd;

	newwid = (box.width() - g_conf->_tMarginX * 2 - g_conf->_scrollWidth) / g_conf->_cellW;
	newhgt = (box.height() - g_conf->_tMarginY * 2) / g_conf->_cellH;

	/* align text with bottom */
	rnd = newhgt * g_conf->_cellH + g_conf->_tMarginY * 2;
	_yAdj = (box.height() - rnd);
	_bbox.top += (box.height() - rnd);

	if (newwid != _width) {
		_width = newwid;
		reflow();
	}

	if (newhgt != _height) {
		/* scroll up if we obscure new lines */
		if (_lastSeen >= newhgt - 1)
			_scrollPos += (_height - newhgt);

		_height = newhgt;

		/* keep window within 'valid' lines */
		if (_scrollPos > _scrollMax - _height + 1)
			_scrollPos = _scrollMax - _height + 1;
		if (_scrollPos < 0)
			_scrollPos = 0;
		touchScroll();

		/* allocate copy buffer */
		if (_copyBuf)
			delete[] _copyBuf;
		_copyBuf = new glui32[_height * TBLINELEN];

		for (int i = 0; i < (_height * TBLINELEN); i++)
			_copyBuf[i] = 0;

		_copyPos = 0;
	}
}

void TextBufferWindow::reflow() {
	int inputbyte = -1;
	Attributes curattr, oldattr;
	int i, k, p, s;
	int x;

	if (_height < 4 || _width < 20)
		return;

	_lines[0].len = _numChars;

	/* allocate temp buffers */
	Attributes *attrbuf = new Attributes[SCROLLBACK * TBLINELEN];
	glui32 *charbuf = new glui32[SCROLLBACK * TBLINELEN];
	int *alignbuf = new int[SCROLLBACK];
	Picture **pictbuf = new Picture *[SCROLLBACK];
	glui32 *hyperbuf = new glui32[SCROLLBACK];
	int *offsetbuf = new int[SCROLLBACK];

	if (!attrbuf || !charbuf || !alignbuf || !pictbuf || !hyperbuf || !offsetbuf) {
		delete[] attrbuf;
		delete[] charbuf;
		delete[] alignbuf;
		delete[] pictbuf;
		delete[] hyperbuf;
		delete[] offsetbuf;
		return;
	}

	/* copy text to temp buffers */

	oldattr = _attr;
	curattr.clear();

	x = 0;
	p = 0;
	s = _scrollMax < SCROLLBACK ? _scrollMax : SCROLLBACK - 1;

	for (k = s; k >= 0; k--) {
		if (k == 0 && _lineRequest)
			inputbyte = p + _inFence;

		if (_lines[k].lpic) {
			offsetbuf[x] = p;
			alignbuf[x] = imagealign_MarginLeft;
			pictbuf[x] = _lines[k].lpic;

			if (pictbuf[x]) pictbuf[x]->increment();
			hyperbuf[x] = _lines[k].lhyper;
			x++;
		}

		if (_lines[k].rpic) {
			offsetbuf[x] = p;
			alignbuf[x] = imagealign_MarginRight;
			pictbuf[x] = _lines[k].rpic;
			if (pictbuf[x]) pictbuf[x]->increment();
			hyperbuf[x] = _lines[k].rhyper;
			x++;
		}

		for (i = 0; i < _lines[k].len; i++) {
			attrbuf[p] = curattr = _lines[k].attr[i];
			charbuf[p] = _lines[k].chars[i];
			p++;
		}

		if (_lines[k].newline) {
			attrbuf[p] = curattr;
			charbuf[p] = '\n';
			p++;
		}
	}

	offsetbuf[x] = -1;

	/* clear window */

	clear();

	/* and dump text back */

	x = 0;
	for (i = 0; i < p; i++) {
		if (i == inputbyte)
			break;
		_attr = attrbuf[i];

		if (offsetbuf[x] == i) {
			putPicture(pictbuf[x], alignbuf[x], hyperbuf[x]);
			x++;
		}

		putCharUni(charbuf[i]);
	}

	/* terribly sorry about this... */
	_lastSeen = 0;
	_scrollPos = 0;

	if (inputbyte != -1) {
		_inFence = _numChars;
		putTextUni(charbuf + inputbyte, p - inputbyte, _numChars, 0);
		_inCurs = _numChars;
	}

	// free temp buffers
	delete[] attrbuf;
	delete[] charbuf;
	delete[] alignbuf;
	delete[] pictbuf;
	delete[] hyperbuf;
	delete[] offsetbuf;

	_attr = oldattr;

	touchScroll();
}

void TextBufferWindow::touchScroll() {
	_windows->clearSelection();
	_windows->repaint(_bbox);

	for (int i = 0; i < _scrollMax; i++)
		_lines[i].dirty = true;
}

bool TextBufferWindow::putPicture(Picture *pic, glui32 align, glui32 linkval) {
	if (align == imagealign_MarginRight)
	{
		if (_lines[0].rpic || _numChars)
			return false;

		_radjw = (pic->w + g_conf->_tMarginX) * GLI_SUBPIX;
		_radjn = (pic->h + g_conf->_cellH - 1) / g_conf->_cellH;
		_lines[0].rpic = pic;
		_lines[0].rm = _radjw;
		_lines[0].rhyper = linkval;
	} else {
		if (align != imagealign_MarginLeft && _numChars)
			putCharUni('\n');

		if (_lines[0].lpic || _numChars)
			return false;

		_ladjw = (pic->w + g_conf->_tMarginX) * GLI_SUBPIX;
		_ladjn = (pic->h + g_conf->_cellH - 1) / g_conf->_cellH;
		_lines[0].lpic = pic;
		_lines[0].lm = _ladjw;
		_lines[0].lhyper = linkval;

		if (align != imagealign_MarginLeft)
			flowBreak();
	}

	return true;
}

void TextBufferWindow::flowBreak() {
	// TODO
}

void TextBufferWindow::putTextUni(const glui32 *buf, int len, int pos, int oldlen) {
	// TODO
}

void TextBufferWindow::touch(int line) {
	int y = _bbox.top + g_conf->_tMarginY + (_height - line - 1) * g_conf->_leading;
	_lines[line].dirty = 1;
	_windows->clearSelection();
	_windows->repaint(Common::Rect(_bbox.left, y - 2, _bbox.right, y + g_conf->_leading + 2));
}

glui32 TextBufferWindow::getSplit(glui32 size, bool vertical) const {
	return (vertical) ? size * g_conf->_cellW : size * g_conf->_cellH;
}

void TextBufferWindow::putChar(unsigned char ch) {
}

void TextBufferWindow::putCharUni(glui32 ch) {
	/*
	glui32 bchars[TBLINELEN];
	Attributes battrs[TBLINELEN];
	int pw;
	int bpoint;
	int saved;
	int i;
	int linelen;
	unsigned char *color;

	gli_tts_speak(&ch, 1);

	pw = (_bbox.right - _bbox.left - g_conf->_tMarginX * 2 - gli_scroll_width) * GLI_SUBPIX;
	pw = pw - 2 * SLOP - radjw - ladjw;

	color = Windows::_overrideBgSet ? gli_window_color : bgcolor;

	// oops ... overflow
	if (numchars + 1 >= TBLINELEN)
		scrolloneline(dwin, 0);

	if (ch == '\n') {
		scrolloneline(dwin, 1);
		return;
	}

	if (gli_conf_quotes) {
		// fails for 'tis a wonderful day in the '80s
		if (gli_conf_quotes > 1 && ch == '\'')
		{
			if (numchars == 0 || leftquote(_chars[numchars - 1]))
				ch = UNI_LSQUO;
		}

		if (ch == '`')
			ch = UNI_LSQUO;

		if (ch == '\'')
			ch = UNI_RSQUO;

		if (ch == '"')
		{
			if (numchars == 0 || leftquote(_chars[numchars - 1]))
				ch = UNI_LDQUO;
			else
				ch = UNI_RDQUO;
		}
	}

	if (gli_conf_dashes && attr.style != style_Preformatted)
	{
		if (ch == '-')
		{
			dashed++;
			if (dashed == 2)
			{
				numchars--;
				if (gli_conf_dashes == 2)
					ch = UNI_NDASH;
				else
					ch = UNI_MDASH;
			}
			if (dashed == 3)
			{
				numchars--;
				ch = UNI_MDASH;
				dashed = 0;
			}
		}
		else
			dashed = 0;
	}

	if (gli_conf_spaces && attr.style != style_Preformatted
		&& styles[attr.style].bg == color
		&& !styles[attr.style].reverse)
	{
		// turn (period space space) into (period space)
		if (gli_conf_spaces == 1)
		{
			if (ch == '.')
				spaced = 1;
			else if (ch == ' ' && spaced == 1)
				spaced = 2;
			else if (ch == ' ' && spaced == 2)
			{
				spaced = 0;
				return;
			}
			else
				spaced = 0;
		}

		// Turn (per sp x) into (per sp sp x)
		if (gli_conf_spaces == 2)
		{
			if (ch == '.')
				spaced = 1;
			else if (ch == ' ' && spaced == 1)
				spaced = 2;
			else if (ch != ' ' && spaced == 2)
			{
				spaced = 0;
				win_textbuffer_putchar_uni(win, ' ');
			}
			else
				spaced = 0;
		}
	}

	_chars[numchars] = ch;
	attrs[numchars] = attr;
	numchars++;

	// kill spaces at the end for line width calculation
	linelen = numchars;
	while (linelen > 1 && _chars[linelen - 1] == ' '
		&& styles[attrs[linelen - 1].style].bg == color
		&& !styles[attrs[linelen - 1].style].reverse)
		linelen--;

	if (calcwidth(dwin, _chars, attrs, 0, linelen, -1) >= pw)
	{
		bpoint = numchars;

		for (i = numchars - 1; i > 0; i--)
			if (_chars[i] == ' ')
			{
				bpoint = i + 1; // skip space
				break;
			}

		saved = numchars - bpoint;

		memcpy(bchars, _chars + bpoint, saved * 4);
		memcpy(battrs, attrs + bpoint, saved * sizeof(attr_t));
		numchars = bpoint;

		scrolloneline(dwin, 0);

		memcpy(_chars, bchars, saved * 4);
		memcpy(attrs, battrs, saved * sizeof(attr_t));
		numchars = saved;
	}

	touch(0);
	*/
}

bool TextBufferWindow::unputCharUni(uint32 ch) {
	// TODO
	return false;
}

void TextBufferWindow::putBuffer(const unsigned char *buf, size_t len) {
	// TODO
}

void TextBufferWindow::putBufferUni(const uint32 *buf, size_t len) {
	// TODO
}

void TextBufferWindow::moveCursor(const Common::Point &newPos) {
	// TODO
}

void TextBufferWindow::clear() {
	int i;

	_attr.fgset = Windows::_overrideFgSet;
	_attr.bgset = Windows::_overrideBgSet;
	_attr.fgcolor = Windows::_overrideFgSet ? Windows::_overrideFgVal : 0;
	_attr.bgcolor = Windows::_overrideBgSet ? Windows::_overrideBgVal : 0;
	_attr.reverse = false;

	_ladjw = _radjw = 0;
	_ladjn = _radjn = 0;

	_spaced = 0;
	_dashed = 0;

	_numChars = 0;

	for (i = 0; i < _scrollBack; i++) {
		_lines[i].len = 0;

		if (_lines[i].lpic) _lines[i].lpic->decrement();
		_lines[i].lpic = nullptr;
		if (_lines[i].rpic) _lines[i].rpic->decrement();
		_lines[i].rpic = nullptr;

		_lines[i].lhyper = 0;
		_lines[i].rhyper = 0;
		_lines[i].lm = 0;
		_lines[i].rm = 0;
		_lines[i].newline = 0;
		_lines[i].dirty = true;
		_lines[i].repaint = false;
	}

	_lastSeen = 0;
	_scrollPos = 0;
	_scrollMax = 0;

	for (i = 0; i < _height; i++)
		touch(i);
}

void TextBufferWindow::requestLineEvent(char *buf, glui32 maxlen, glui32 initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni)
	{
		warning("request_line_event: window already has keyboard request");
		return;
	}

	// TODO
}

void TextBufferWindow::requestLineEventUni(glui32 *buf, glui32 maxlen, glui32 initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni)
	{
		warning("request_line_event_uni: window already has keyboard request");
		return;
	}

	// TODO
}

void TextBufferWindow::cancelLineEvent(Event *ev) {
	gidispatch_rock_t inarrayrock;
	int ix;
	int len;
	void *inbuf;
	int inmax;
	int unicode = _lineRequestUni;
	Event dummyEv;

	if (!ev)
		ev = &dummyEv;

	g_vm->_events->clearEvent(ev);

	if (!_lineRequest && !_lineRequestUni)
		return;

	if (!_inBuf)
		return;

	inbuf = _inBuf;
	inmax = _inMax;
	inarrayrock = _inArrayRock;

	len = _numChars - _inFence;
	if (_echoStream)
		_echoStream->echoLineUni(_chars + _inFence, len);

	if (len > inmax)
		len = inmax;

	if (!unicode) {
		for (ix = 0; ix<len; ix++) {
			glui32 ch = _chars[_inFence + ix];
			if (ch > 0xff)
				ch = '?';
			((char *)inbuf)[ix] = (char)ch;
		}
	}
	else {
		for (ix = 0; ix<len; ix++)
			((glui32 *)inbuf)[ix] = _chars[_inFence + ix];
	}

	_attr = _origAttr;

	ev->_type = evtype_LineInput;
	ev->_window = this;
	ev->_val1 = len;
	ev->_val2 = 0;

	_lineRequest = false;
	_lineRequestUni = false;
	if (_lineTerminators) {
		free(_lineTerminators);
		_lineTerminators = nullptr;
	}
	_inBuf = nullptr;
	_inMax = 0;

	if (_echoLineInput) {
		putCharUni('\n');
	}
	else {
		_numChars = _inFence;
		touch(0);
	}

	if (g_vm->gli_unregister_arr)
		(*g_vm->gli_unregister_arr)(inbuf, inmax, unicode ? "&+#!Iu" : "&+#!Cn", inarrayrock);
}

void TextBufferWindow::redraw() {
	// TODO
}

/*--------------------------------------------------------------------------*/

TextBufferWindow::TextBufferRow::TextBufferRow() : len(0), newline(0), dirty(false), repaint(false),
		lpic(nullptr), rpic(nullptr), lhyper(0), rhyper(0), lm(0), rm(0) {
}

void TextBufferWindow::TextBufferRow::resize(size_t newSize) {
	chars.clear();
	attr.clear();
	chars.resize(newSize);
	attr.resize(newSize);
	Common::fill(&chars[0], &chars[0] + newSize, ' ');
}

/*--------------------------------------------------------------------------*/

GraphicsWindow::GraphicsWindow(Windows *windows, uint32 rock) : Window(windows, rock),
_w(0), _h(0), _dirty(false), _surface(nullptr) {
	_type = wintype_Graphics;
	Common::copy(&_bgColor[0], &_bgColor[3], _bgnd);
}

GraphicsWindow::~GraphicsWindow() {
	delete _surface;
}

void GraphicsWindow::rearrange(const Common::Rect &box) {
	int newwid, newhgt;
	int bothwid, bothhgt;
	int oldw, oldh;
	Graphics::ManagedSurface *newSurface;

	_bbox = box;

	newwid = box.width();
	newhgt = box.height();
	oldw = _w;
	oldh = _h;

	if (newwid <= 0 || newhgt <= 0) {
		_w = 0;
		_h = 0;
		delete _surface;
		_surface = NULL;
		return;
	}

	bothwid = _w;
	if (newwid < bothwid)
		bothwid = newwid;
	bothhgt = _h;
	if (newhgt < bothhgt)
		bothhgt = newhgt;

	newSurface = new Graphics::ManagedSurface(newwid, newhgt,
		Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	// If the new surface is equal or bigger than the old one, copy it over
	if (_surface && bothwid && bothhgt)
		newSurface->blitFrom(*_surface);

	delete _surface;
	_surface = newSurface;
	_w = newwid;
	_h = newhgt;

	touch();
}

void GraphicsWindow::touch() {
	_dirty = true;
	_windows->repaint(_bbox);
}

void GraphicsWindow::redraw() {
	// TODO
}

/*--------------------------------------------------------------------------*/

PairWindow::PairWindow(Windows *windows, glui32 method, Window *key, glui32 size) :
		Window(windows, 0),
		_dir(method & winmethod_DirMask),
		_division(method & winmethod_DivisionMask),
		_wBorder((method & winmethod_BorderMask) == winmethod_Border),
		_vertical(_dir == winmethod_Left || _dir == winmethod_Right),
		_backward(_dir == winmethod_Left || _dir == winmethod_Above),
		_key(key), _size(size), _keyDamage(0), _child1(nullptr), _child2(nullptr) {
	_type = wintype_Pair;
}

void PairWindow::rearrange(const Common::Rect &box) {
	Common::Rect box1, box2;
	int min, diff, split, splitwid, max;
	Window *ch1, *ch2;

	_bbox = box;

	if (_vertical) {
		min = _bbox.left;
		max = _bbox.right;
	} else {
		min = _bbox.top;
		max = _bbox.bottom;
	}
	diff = max - min;

	// We now figure split.
	if (_vertical)
		splitwid = g_conf->_wPaddingX; // want border?
	else
		splitwid = g_conf->_wPaddingY; // want border?

	switch (_division) {
	case winmethod_Proportional:
		split = (diff * _size) / 100;
		break;

	case winmethod_Fixed:
		split = !_key ? 0 : _key->getSplit(_size, _vertical);
		break;

	default:
		split = diff / 2;
		break;
	}

	if (!_backward)
		split = max - split - splitwid;
	else
		split = min + split;

	if (min >= max) {
		split = min;
	} else {
		if (split < min)
			split = min;
		else if (split > max - splitwid)
			split = max - splitwid;
	}

	if (_vertical) {
		box1.left = _bbox.left;
		box1.right = split;
		box2.left = split + splitwid;
		box2.right = _bbox.right;
		box1.top = _bbox.top;
		box1.bottom = _bbox.bottom;
		box2.top = _bbox.top;
		box2.bottom = _bbox.bottom;
	} else {
		box1.top = _bbox.top;
		box1.bottom = split;
		box2.top = split + splitwid;
		box2.bottom = _bbox.bottom;
		box1.left = _bbox.left;
		box1.right = _bbox.right;
		box2.left = _bbox.left;
		box2.right = _bbox.right;
	}

	if (!_backward) {
		ch1 = _child1;
		ch2 = _child2;
	} else {
		ch1 = _child2;
		ch2 = _child1;
	}

	ch1->rearrange(box1);
	ch2->rearrange(box2);
}

void PairWindow::redraw() {
	// TODO
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
