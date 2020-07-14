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

#include "glk/window_text_grid.h"
#include "glk/conf.h"
#include "glk/glk.h"
#include "glk/selection.h"
#include "glk/screen.h"

namespace Glk {

TextGridWindow::TextGridWindow(Windows *windows, uint rock) : TextWindow(windows, rock),
		_font(g_conf->_monoInfo) {
	_type = wintype_TextGrid;
	_width = _height = 0;
	_curX = _curY = 0;
	_inBuf = nullptr;
	_inOrgX = _inOrgY = 0;
	_inMax = 0;
	_inCurs = _inLen = 0;
	_inArrayRock.num = 0;
	_lineTerminators = nullptr;

	Common::copy(&g_conf->_gStyles[0], &g_conf->_gStyles[style_NUMSTYLES], _styles);

	if (g_conf->_speak)
		gli_initialize_tts();
}

TextGridWindow::~TextGridWindow() {
	if (g_conf->_speak)
		gli_free_tts();

	if (_inBuf) {
		if (g_vm->gli_unregister_arr)
			(*g_vm->gli_unregister_arr)(_inBuf, _inMax, "&+#!Cn", _inArrayRock);
		_inBuf = nullptr;
	}

	delete[] _lineTerminators;
}

void TextGridWindow::rearrange(const Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;

	newwid = MAX(box.width() / _font._cellW, 0);
	newhgt = MAX(box.height() / _font._cellH, 0);

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
	int y = _bbox.top + line * _font._leading;
	_lines[line].dirty = true;
	_windows->repaint(Rect(_bbox.left, y, _bbox.right, y + _font._leading));
}

uint TextGridWindow::getSplit(uint size, bool vertical) const {
	return vertical ? size * _font._cellW : size * _font._cellH;
}

void TextGridWindow::putCharUni(uint32 ch) {
	TextGridRow *ln;

	// This may not be the best way to do this, but some games use user styles to
	// display some gliphs from ASCII characters. Those should not be spoken as
	// they make no sense.
	if (_attr.style < style_User1)
		gli_tts_speak(&ch, 1);

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
		return; // outside the window

	if (ch == '\n') {
		// a newline just moves the cursor.
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

	// Move the cursor back.
	if (_curX >= _width)
		_curX = _width - 1;
	else
		_curX--;

	// Canonicalize the cursor position. That is, the cursor may have been
	// left outside the window area; wrap it if necessary.
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
		return 0;    // it wasn't there
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

void TextGridWindow::moveCursor(const Point &pos) {
	// If the values are negative, they're really huge positive numbers --
	// remember that they were cast from uint. So set them huge and
	// let canonicalization take its course.
	if (_curY >= 0 && _curY < _height && _lines[_curY].dirty) {
		const uint32 NEWLINE = '\n';
		gli_tts_speak((const uint32 *)&NEWLINE, 1);
	}

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

void TextGridWindow::click(const Point &newPos) {
	int x = newPos.x - _bbox.left;
	int y = newPos.y - _bbox.top;

	if (_lineRequest || _charRequest || _lineRequestUni || _charRequestUni
			|| _moreRequest || _scrollRequest)
		_windows->setFocus(this);

	if (_mouseRequest) {
		g_vm->_events->store(evtype_MouseInput, this, x / _font._cellW, y / _font._leading);
		_mouseRequest = false;
		if (g_conf->_safeClicks)
			g_vm->_events->_forceClick = true;
	}

	if (_hyperRequest) {
		uint linkval = g_vm->_selection->getHyperlink(newPos);
		if (linkval) {
			g_vm->_events->store(evtype_Hyperlink, this, linkval, 0);
			_hyperRequest = false;
			if (g_conf->_safeClicks)
				g_vm->_events->_forceClick = true;
		}
	}
}

void TextGridWindow::requestLineEvent(char *buf, uint maxlen, uint initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni) {
		warning("request_line_event: window already has keyboard request");
		return;
	}

	_lineRequest = true;
	gli_tts_flush();

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

		for (uint ix = 0; ix < initlen; ix++) {
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
		_lineTerminators = new uint32[_termCt + 1];

		if (_lineTerminators) {
			memcpy(_lineTerminators, _lineTerminatorsBase, _termCt * sizeof(uint32));
			_lineTerminators[_termCt] = 0;
		}
	}

	if (g_vm->gli_register_arr)
		_inArrayRock = (*g_vm->gli_register_arr)(buf, maxlen, "&+#!Cn");

	// Switch focus to the new window
	_windows->inputGuessFocus();
}

void TextGridWindow::requestLineEventUni(uint32 *buf, uint maxlen, uint initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni) {
		warning("requestLineEventUni: window already has keyboard request");
		return;
	}

	_lineRequestUni = true;
	gli_tts_flush();

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

		for (uint ix = 0; ix < initlen; ix++) {
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
		_lineTerminators = new uint32[_termCt + 1];

		if (_lineTerminators) {
			memcpy(_lineTerminators, _lineTerminatorsBase, _termCt * sizeof(uint));
			_lineTerminators[_termCt] = 0;
		}
	}

	if (g_vm->gli_register_arr)
		_inArrayRock = (*g_vm->gli_register_arr)(buf, maxlen, "&+#!Iu");

	// Switch focus to the new window
	_windows->inputGuessFocus();
}

void TextGridWindow::requestCharEvent() {
	_charRequest = true;

	// Switch focus to the new window
	_windows->inputGuessFocus();
}

void TextGridWindow::requestCharEventUni() {
	_charRequestUni = true;

	// Switch focus to the new window
	_windows->inputGuessFocus();
}

void TextGridWindow::cancelLineEvent(Event *ev) {
	int ix;
	void *inbuf;
	int inmax;
	bool unicode = _lineRequestUni;
	gidispatch_rock_t inarrayrock;
	TextGridRow *ln = &_lines[_inOrgY];
	Event dummyEv;

	if (!ev)
		ev = &dummyEv;

	ev->clear();

	if (!_lineRequest && !_lineRequestUni)
		return;


	inbuf = _inBuf;
	inmax = _inMax;
	inarrayrock = _inArrayRock;

	if (!unicode) {
		for (ix = 0; ix < _inLen; ix++) {
			uint32 ch = ln->_chars[_inOrgX + ix];
			if (ch > 0xff)
				ch = '?';
			((char *)inbuf)[ix] = (char)ch;
		}
		if (_echoStream)
			_echoStream->echoLine((char *)_inBuf, _inLen);
	} else {
		for (ix = 0; ix < _inLen; ix++)
			((uint *)inbuf)[ix] = ln->_chars[_inOrgX + ix];
		if (_echoStream)
			_echoStream->echoLineUni((uint32 *)inbuf, _inLen);
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
		delete[] _lineTerminators;
		_lineTerminators = nullptr;
	}

	_inBuf = nullptr;
	_inMax = 0;
	_inOrgX = 0;
	_inOrgY = 0;

	if (g_vm->gli_unregister_arr)
		(*g_vm->gli_unregister_arr)(inbuf, inmax, unicode ? "&+#!Iu" : "&+#!Cn", inarrayrock);
}

void TextGridWindow::acceptReadChar(uint arg) {
	uint key;

	switch (arg) {
	case keycode_Erase:
		key = keycode_Delete;
		break;
	case keycode_MouseWheelUp:
	case keycode_MouseWheelDown:
		return;
	default:
		key = arg;
	}

	gli_tts_purge();

	if (key > 0xff && key < (0xffffffff - keycode_MAXVAL + 1)) {
		if (!(_charRequestUni) || key > 0x10ffff)
			key = keycode_Unknown;
	}

	_charRequest = false;
	_charRequestUni = false;
	g_vm->_events->store(evtype_CharInput, this, key, 0);
}

void TextGridWindow::acceptLine(uint32 keycode) {
	int ix;
	void *inbuf;
	int inmax;
	gidispatch_rock_t inarrayrock;
	TextGridRow *ln = &(_lines[_inOrgY]);
	bool unicode = _lineRequestUni;

	if (!_inBuf)
		return;

	inbuf = _inBuf;
	inmax = _inMax;
	inarrayrock = _inArrayRock;

	gli_tts_purge();

	if (!unicode) {
		for (ix = 0; ix < _inLen; ix++)
			((char *)inbuf)[ix] = (char)ln->_chars[_inOrgX + ix];
		if (_echoStream)
			_echoStream->echoLine((char *)inbuf, _inLen);
		if (g_conf->_speakInput) {
			const char NEWLINE = '\n';
			gli_tts_speak((const char *)inbuf, _inLen);
			gli_tts_speak((const char *)&NEWLINE, 1);
		}
	} else {
		for (ix = 0; ix < _inLen; ix++)
			((uint *)inbuf)[ix] = ln->_chars[_inOrgX + ix];
		if (_echoStream)
			_echoStream->echoLineUni((const uint32 *)inbuf, _inLen);
		if (g_conf->_speakInput) {
			const uint32 NEWLINE = '\n';
			gli_tts_speak((const uint32 *)inbuf, _inLen);
			gli_tts_speak((const uint32 *)&NEWLINE, 1);
		}
	}

	_curY = _inOrgY + 1;
	_curX = 0;
	_attr = _origAttr;

	if (_lineTerminators) {
		uint val2 = keycode;
		if (val2 == keycode_Return)
			val2 = 0;
		g_vm->_events->store(evtype_LineInput, this, _inLen, val2);
		delete[] _lineTerminators;
		_lineTerminators = nullptr;
	} else {
		g_vm->_events->store(evtype_LineInput, this, _inLen, 0);
	}
	_lineRequest = false;
	_lineRequestUni = false;
	_inBuf = nullptr;
	_inMax = 0;
	_inOrgX = 0;
	_inOrgY = 0;

	if (g_vm->gli_unregister_arr)
		(*g_vm->gli_unregister_arr)(inbuf, inmax, unicode ? "&+#!Iu" : "&+#!Cn", inarrayrock);
}

void TextGridWindow::acceptReadLine(uint32 arg) {
	int ix;
	TextGridRow *ln = &(_lines[_inOrgY]);

	if (!_inBuf)
		return;

	if (_lineTerminators && checkTerminators(arg)) {
		const uint32 *cx;
		for (cx = _lineTerminators; *cx; cx++) {
			if (*cx == arg) {
				acceptLine(arg);
				return;
			}
		}
	}

	switch (arg) {

	// Delete keys, during line input.
	case keycode_Delete:
		if (_inLen <= 0)
			return;
		if (_inCurs <= 0)
			return;
		for (ix = _inCurs; ix < _inLen; ix++)
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
		for (ix = _inCurs; ix < _inLen - 1; ix++)
			ln->_chars[_inOrgX + ix] = ln->_chars[_inOrgX + ix + 1];
		ln->_chars[_inOrgX + _inLen - 1] = ' ';
		_inLen--;
		break;

	case keycode_Escape:
		if (_inLen <= 0)
			return;
		for (ix = 0; ix < _inLen; ix++)
			ln->_chars[_inOrgX + ix] = ' ';
		_inLen = 0;
		_inCurs = 0;
		break;

	// Cursor movement keys, during line input.
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

		if (_font._caps && (arg > 0x60 && arg < 0x7b))
			arg -= 0x20;

		for (ix = _inLen; ix > _inCurs; ix--)
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
	uint link;
	int font;
	uint fgcolor, bgcolor;
	Screen &screen = *g_vm->_screen;

	gli_tts_flush();

	Window::redraw();

	x0 = _bbox.left;
	y0 = _bbox.top;

	for (i = 0; i < _height; i++) {
		ln = &_lines[i];
		if (ln->dirty || Windows::_forceRedraw) {
			ln->dirty = false;

			x = x0;
			y = y0 + i * _font._leading;

			// clear any stored hyperlink coordinates
			g_vm->_selection->putHyperlink(0, x0, y, x0 + _font._cellW * _width, y + _font._leading);

			a = 0;
			for (b = 0; b < _width; b++) {
				if (ln->_attrs[a] != ln->_attrs[b]) {
					link = ln->_attrs[a].hyper;
					font = ln->_attrs[a].attrFont(_styles);
					fgcolor = link ? _font._linkColor : ln->_attrs[a].attrFg(_styles);
					bgcolor = ln->_attrs[a].attrBg(_styles);
					w = (b - a) * _font._cellW;
					screen.fillRect(Rect::fromXYWH(x, y, w, _font._leading), bgcolor);
					o = x;

					for (k = a, o = x; k < b; k++, o += _font._cellW) {
						screen.drawStringUni(Point(o * GLI_SUBPIX, y + _font._baseLine), font,
											 fgcolor, Common::U32String(&ln->_chars[k], 1), -1);
					}
					if (link) {
						screen.fillRect(Rect::fromXYWH(x, y + _font._baseLine + 1, w,
													   _font._linkStyle), _font._linkColor);
						g_vm->_selection->putHyperlink(link, x, y, x + w, y + _font._leading);
					}

					x += w;
					a = b;
				}
			}
			link = ln->_attrs[a].hyper;
			font = ln->_attrs[a].attrFont(_styles);
			fgcolor = link ? _font._linkColor : ln->_attrs[a].attrFg(_styles);
			bgcolor = ln->_attrs[a].attrBg(_styles);
			w = (b - a) * _font._cellW;
			w += _bbox.right - (x + w);
			screen.fillRect(Rect::fromXYWH(x, y, w, _font._leading), bgcolor);

			// Draw the caret if necessary
			if (_windows->getFocusWindow() == this && i == _curY &&
					(_lineRequest || _lineRequestUni || _charRequest || _charRequestUni)) {
				_font.drawCaret(Point((x0 + _curX * _font._cellW) * GLI_SUBPIX, y + _font._baseLine));
			}

			// Write out the text
			for (k = a, o = x; k < b; k++, o += _font._cellW) {
				screen.drawStringUni(Point(o * GLI_SUBPIX, y + _font._baseLine), font,
									 fgcolor, Common::U32String(&ln->_chars[k], 1));
			}
			if (link) {
				screen.fillRect(Rect::fromXYWH(x, y + _font._baseLine + 1, w, _font._linkStyle), _font._linkColor);
				g_vm->_selection->putHyperlink(link, x, y, x + w, y + _font._leading);
			}
		}
	}
}

void TextGridWindow::getSize(uint *width, uint *height) const {
	if (width)
		*width = _bbox.width() / _font._cellW;
	if (height)
		*height = _bbox.height() / _font._cellH;
}

/*--------------------------------------------------------------------------*/

void TextGridWindow::TextGridRow::resize(size_t newSize) {
	size_t oldSize = _chars.size();
	if (newSize != oldSize) {
		_chars.resize(newSize);
		_attrs.resize(newSize);

		if (newSize > oldSize)
			Common::fill(&_chars[0] + oldSize, &_chars[0] + newSize, ' ');
	}
}

} // End of namespace Glk
