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

#include "gargoyle/window_text_grid.h"
#include "gargoyle/conf.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/screen.h"
#include "gargoyle/window_mask.h"

namespace Gargoyle {

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
		glui32 linkval = g_vm->_windowMask->getHyperlink(newPos);
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

	ev->type = evtype_LineInput;
	ev->window = this;
	ev->val1 = _inLen;
	ev->val2 = 0;

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
	Screen &screen = *g_vm->_screen;

	Window::redraw();

	x0 = _bbox.left;
	y0 = _bbox.top;

	for (i = 0; i < _height; i++) {
		ln = &_lines[i];
		if (ln->dirty || Windows::_forceRedraw) {
			ln->dirty = 0;

			x = x0;
			y = y0 + i * g_conf->_leading;

			/* clear any stored hyperlink coordinates */
			g_vm->_windowMask->putHyperlink(0, x0, y, x0 + g_conf->_cellW * _width, y + g_conf->_leading);

			a = 0;
			for (b = 0; b < _width; b++) {
				if (ln->_attrs[a] == ln->_attrs[b]) {
					link = ln->_attrs[a].hyper;
					font = ln->_attrs[a].attrFont(styles);
					fgcolor = link ? g_conf->_linkColor : ln->_attrs[a].attrFg(styles);
					bgcolor = ln->_attrs[a].attrBg(styles);
					w = (b - a) * g_conf->_cellW;
					screen.fillRect(x, y, w, g_conf->_leading, bgcolor);
					o = x;

					for (k = a; k < b; k++) {
						screen.drawStringUni(o * GLI_SUBPIX,
							y + g_conf->_baseLine, font, fgcolor,
							&ln->_chars[k], 1, -1);
						o += g_conf->_cellW;
					}
					if (link) {
						screen.fillRect(x, y + g_conf->_baseLine + 1, w,
							g_conf->_linkStyle, g_conf->_linkColor);
						g_vm->_windowMask->putHyperlink(link, x, y, x + w, y + g_conf->_leading);
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
			screen.fillRect(x, y, w, g_conf->_leading, bgcolor);

			o = x;
			for (k = a; k < b; k++) {
				screen.drawStringUni(o * GLI_SUBPIX,
					y + g_conf->_baseLine, font, fgcolor,
					&ln->_chars[k], 1, -1);
				o += g_conf->_cellW;
			}
			if (link) {
				screen.fillRect(x, y + g_conf->_baseLine + 1, w,
					g_conf->_linkStyle, g_conf->_linkColor);
				g_vm->_windowMask->putHyperlink(link, x, y, x + w, y + g_conf->_leading);
			}
		}
	}
}

void TextGridWindow::getSize(glui32 *width, glui32 *height) const {
	if (width)
		*width = _bbox.width() / g_conf->_cellW;
	if (height)
		*height = _bbox.height() / g_conf->_cellH;
}

/*--------------------------------------------------------------------------*/

void TextGridWindow::TextGridRow::resize(size_t newSize) {
	_chars.clear();
	_attrs.clear();
	_chars.resize(newSize);
	_attrs.resize(newSize);
	Common::fill(&_chars[0], &_chars[0] + newSize, ' ');
}

} // End of namespace Gargoyle
