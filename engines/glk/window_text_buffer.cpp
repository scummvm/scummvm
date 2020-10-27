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

#include "glk/window_text_buffer.h"
#include "glk/conf.h"
#include "glk/glk.h"
#include "glk/screen.h"
#include "glk/selection.h"
#include "glk/unicode.h"

namespace Glk {

/**
 *
 * How many pixels we add to left/right margins
 */
#define SLOP (2 * GLI_SUBPIX)


TextBufferWindow::TextBufferWindow(Windows *windows, uint rock) : TextWindow(windows, rock),
		_font(g_conf->_propInfo), _historyPos(0), _historyFirst(0), _historyPresent(0),
		_lastSeen(0), _scrollPos(0), _scrollMax(0), _scrollBack(SCROLLBACK), _width(-1), _height(-1),
		_inBuf(nullptr), _lineTerminators(nullptr), _echoLineInput(true), _ladjw(0), _radjw(0),
		_ladjn(0), _radjn(0), _numChars(0), _chars(nullptr), _attrs(nullptr), _spaced(0), _dashed(0),
		_copyBuf(0), _copyPos(0) {
	_type = wintype_TextBuffer;
	_history.resize(HISTORYLEN);

	_lines.resize(SCROLLBACK);
	_chars = _lines[0]._chars;
	_attrs = _lines[0]._attrs;

	Common::copy(&g_conf->_tStyles[0], &g_conf->_tStyles[style_NUMSTYLES], _styles);

	if (g_conf->_speak)
		gli_initialize_tts();
}

TextBufferWindow::~TextBufferWindow() {
	if (g_conf->_speak)
		gli_free_tts();

	if (_inBuf) {
		if (g_vm->gli_unregister_arr)
			(*g_vm->gli_unregister_arr)(_inBuf, _inMax, "&+#!Cn", _inArrayRock);
		_inBuf = nullptr;
	}

	delete[] _copyBuf;
	delete[] _lineTerminators;

	for (int i = 0; i < _scrollBack; i++) {
		if (_lines[i]._lPic)
			_lines[i]._lPic->decrement();
		if (_lines[i]._rPic)
			_lines[i]._rPic->decrement();
	}
}

void TextBufferWindow::rearrange(const Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;
	int rnd;

	newwid = MAX((box.width() - g_conf->_tMarginX * 2 - g_conf->_scrollWidth) / _font._cellW, 0);
	newhgt = MAX((box.height() - g_conf->_tMarginY * 2) / _font._cellH, 0);

	// align text with bottom
	rnd = newhgt * _font._cellH + g_conf->_tMarginY * 2;
	_yAdj = (box.height() - rnd);
	_bbox.top += (box.height() - rnd);

	if (newwid != _width) {
		_width = newwid;
		reflow();
	}

	if (newhgt != _height) {
		// scroll up if we obscure new lines
		if (_lastSeen >= newhgt - 1)
			_scrollPos += (_height - newhgt);

		_height = newhgt;

		// keep window within 'valid' lines
		if (_scrollPos > _scrollMax - _height + 1)
			_scrollPos = _scrollMax - _height + 1;
		if (_scrollPos < 0)
			_scrollPos = 0;
		touchScroll();

		// allocate copy buffer
		if (_copyBuf)
			delete[] _copyBuf;
		_copyBuf = new uint32[_height * TBLINELEN];

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

	_lines[0]._len = _numChars;

	// allocate temp buffers
	Attributes *attrbuf = new Attributes[SCROLLBACK * TBLINELEN];
	uint32 *charbuf = new uint32[SCROLLBACK * TBLINELEN];
	int *alignbuf = new int[SCROLLBACK];
	Picture **pictbuf = new Picture *[SCROLLBACK];
	uint *hyperbuf = new uint[SCROLLBACK];
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

	// copy text to temp buffers

	oldattr = _attr;
	curattr.clear();

	x = 0;
	p = 0;
	s = _scrollMax < SCROLLBACK ? _scrollMax : SCROLLBACK - 1;

	for (k = s; k >= 0; k--) {
		if (k == 0 && _lineRequest)
			inputbyte = p + _inFence;

		if (_lines[k]._lPic) {
			offsetbuf[x] = p;
			alignbuf[x] = imagealign_MarginLeft;
			pictbuf[x] = _lines[k]._lPic;

			if (pictbuf[x]) pictbuf[x]->increment();
			hyperbuf[x] = _lines[k]._lHyper;
			x++;
		}

		if (_lines[k]._rPic) {
			offsetbuf[x] = p;
			alignbuf[x] = imagealign_MarginRight;
			pictbuf[x] = _lines[k]._rPic;
			if (pictbuf[x]) pictbuf[x]->increment();
			hyperbuf[x] = _lines[k]._rHyper;
			x++;
		}

		for (i = 0; i < _lines[k]._len; i++) {
			attrbuf[p] = curattr = _lines[k]._attrs[i];
			charbuf[p] = _lines[k]._chars[i];
			p++;
		}

		if (_lines[k]._newLine) {
			attrbuf[p] = curattr;
			charbuf[p] = '\n';
			p++;
		}
	}

	offsetbuf[x] = -1;

	// clear window
	clear();

	// and dump text back
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

	// terribly sorry about this...
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
	g_vm->_selection->clearSelection();
	_windows->repaint(_bbox);

	for (int i = 0; i < _scrollMax; i++)
		_lines[i]._dirty = true;
}

bool TextBufferWindow::putPicture(Picture *pic, uint align, uint linkval) {
	if (align == imagealign_MarginRight) {
		if (_lines[0]._rPic || _numChars)
			return false;

		_radjw = (pic->w + g_conf->_tMarginX) * GLI_SUBPIX;
		_radjn = (pic->h + _font._cellH - 1) / _font._cellH;
		_lines[0]._rPic = pic;
		_lines[0]._rm = _radjw;
		_lines[0]._rHyper = linkval;
	} else {
		if (align != imagealign_MarginLeft && _numChars)
			putCharUni('\n');

		if (_lines[0]._lPic || _numChars)
			return false;

		_ladjw = (pic->w + g_conf->_tMarginX) * GLI_SUBPIX;
		_ladjn = (pic->h + _font._cellH - 1) / _font._cellH;
		_lines[0]._lPic = pic;
		_lines[0]._lm = _ladjw;
		_lines[0]._lHyper = linkval;

		if (align != imagealign_MarginLeft)
			flowBreak();
	}

	return true;
}

uint TextBufferWindow::drawPicture(const Common::String &name, uint align, uint scaled, uint width, uint height) {
	Picture *pic;
	uint hyperlink;
	int error;

	pic = g_vm->_pictures->load(name);

	if (!pic)
		return false;

	if (!_imageLoaded) {
		g_vm->_pictures->increment();
		_imageLoaded = true;
	}

	if (scaled) {
		Picture *tmp;
		tmp = g_vm->_pictures->scale(pic, width, height);
		pic = tmp;
	}

	hyperlink = _attr.hyper;

	pic->increment();
	error = putPicture(pic, align, hyperlink);

	return error;
}

void TextBufferWindow::putText(const char *buf, int len, int pos, int oldlen) {
	int diff = len - oldlen;

	if (_numChars + diff >= TBLINELEN)
		return;

	if (diff != 0 && pos + oldlen < _numChars) {
		memmove(_chars + pos + len,
				_chars + pos + oldlen,
				(_numChars - (pos + oldlen)) * 4);
		memmove(_attrs + pos + len,
				_attrs + pos + oldlen,
				(_numChars - (pos + oldlen)) * sizeof(Attributes));
	}
	if (len > 0) {
		for (int i = 0; i < len; i++) {
			_chars[pos + i] = buf[i];
			_attrs[pos + i].set(style_Input);
		}
	}
	_numChars += diff;

	if (_inBuf) {
		if (_inCurs >= pos + oldlen)
			_inCurs += diff;
		else if (_inCurs >= pos)
			_inCurs = pos + len;
	}

	touch(0);
}

void TextBufferWindow::putTextUni(const uint32 *buf, int len, int pos, int oldlen) {
	int diff = len - oldlen;

	if (_numChars + diff >= TBLINELEN)
		return;

	if (diff != 0 && pos + oldlen < _numChars) {
		memmove(_chars + pos + len,
				_chars + pos + oldlen,
				(_numChars - (pos + oldlen)) * 4);
		memmove(_attrs + pos + len,
				_attrs + pos + oldlen,
				(_numChars - (pos + oldlen)) * sizeof(Attributes));
	}
	if (len > 0) {
		int i;
		memmove(_chars + pos, buf, len * 4);
		for (i = 0; i < len; i++)
			_attrs[pos + i].set(style_Input);
	}
	_numChars += diff;

	if (_inBuf) {
		if (_inCurs >= pos + oldlen)
			_inCurs += diff;
		else if (_inCurs >= pos)
			_inCurs = pos + len;
	}

	touch(0);
}

void TextBufferWindow::touch(int line) {
	_lines[line]._dirty = true;
	g_vm->_selection->clearSelection();

	int y = _bbox.top + g_conf->_tMarginY + (_height - line - 1) * _font._leading;
	_windows->repaint(Rect(_bbox.left, y - 2, _bbox.right, y + _font._leading + 2));
}

uint TextBufferWindow::getSplit(uint size, bool vertical) const {
	return (vertical) ? size * _font._cellW : size * _font._cellH;
}

void TextBufferWindow::putCharUni(uint32 ch) {
	uint bchars[TBLINELEN];
	Attributes battrs[TBLINELEN];
	int pw;
	int bpoint;
	int saved;
	int i;
	int linelen;
	uint color;

	gli_tts_speak(&ch, 1);

	pw = (_bbox.right - _bbox.left - g_conf->_tMarginX * 2 - g_conf->_scrollWidth) * GLI_SUBPIX;
	pw = pw - 2 * SLOP - _radjw - _ladjw;

	color = Windows::_overrideBgSet ? g_conf->_windowColor : _bgColor;

	// oops ... overflow
	if (_numChars + 1 >= TBLINELEN)
		scrollOneLine(0);

	if (ch == '\n') {
		scrollOneLine(1);
		return;
	}

	if (_font._quotes) {
		// fails for 'tis a wonderful day in the '80s
		if (_font._quotes > 1 && ch == '\'') {
			if (_numChars == 0 || leftquote(_chars[_numChars - 1]))
				ch = UNI_LSQUO;
		}

		if (ch == '`')
			ch = UNI_LSQUO;

		if (ch == '\'')
			ch = UNI_RSQUO;

		if (ch == '"') {
			if (_numChars == 0 || leftquote(_chars[_numChars - 1]))
				ch = UNI_LDQUO;
			else
				ch = UNI_RDQUO;
		}
	}

	if (_font._dashes && _attr.style != style_Preformatted) {
		if (ch == '-') {
			_dashed++;
			if (_dashed == 2) {
				_numChars--;
				if (_font._dashes == 2)
					ch = UNI_NDASH;
				else
					ch = UNI_MDASH;
			}
			if (_dashed == 3) {
				_numChars--;
				ch = UNI_MDASH;
				_dashed = 0;
			}
		} else {
			_dashed = 0;
		}
	}

	if (_font._spaces && _attr.style != style_Preformatted
			&& _styles[_attr.style].bg == color
			&& !_styles[_attr.style].reverse) {
		// turn (period space space) into (period space)
		if (_font._spaces == 1) {
			if (ch == '.')
				_spaced = 1;
			else if (ch == ' ' && _spaced == 1)
				_spaced = 2;
			else if (ch == ' ' && _spaced == 2) {
				_spaced = 0;
				return;
			} else {
				_spaced = 0;
			}
		}

		// Turn (per sp x) into (per sp sp x)
		if (_font._spaces == 2) {
			if (ch == '.')
				_spaced = 1;
			else if (ch == ' ' && _spaced == 1)
				_spaced = 2;
			else if (ch != ' ' && _spaced == 2) {
				_spaced = 0;
				putCharUni(' ');
			} else {
				_spaced = 0;
			}
		}
	}

	_chars[_numChars] = ch;
	_attrs[_numChars] = _attr;
	_numChars++;

	// kill spaces at the end for line width calculation
	linelen = _numChars;
	while (linelen > 1 && _chars[linelen - 1] == ' '
			&& _styles[_attrs[linelen - 1].style].bg == color
			&& !_styles[_attrs[linelen - 1].style].reverse)
		linelen--;

	if (calcWidth(_chars, _attrs, 0, linelen, -1) >= pw) {
		bpoint = _numChars;

		for (i = _numChars - 1; i > 0; i--) {
			if (_chars[i] == ' ') {
				bpoint = i + 1; // skip space
				break;
			}
		}

		saved = _numChars - bpoint;

		memcpy(bchars, _chars + bpoint, saved * 4);
		memcpy(battrs, _attrs + bpoint, saved * sizeof(Attributes));
		_numChars = bpoint;

		scrollOneLine(0);

		memcpy(_chars, bchars, saved * 4);
		memcpy(_attrs, battrs, saved * sizeof(Attributes));
		_numChars = saved;
	}

	touch(0);
}

bool TextBufferWindow::unputCharUni(uint32 ch) {
	if (_numChars > 0 && _chars[_numChars - 1] == ch) {
		_numChars--;
		touch(0);
		return true;
	}

	return false;
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
		_lines[i]._len = 0;

		if (_lines[i]._lPic) _lines[i]._lPic->decrement();
		_lines[i]._lPic = nullptr;
		if (_lines[i]._rPic) _lines[i]._rPic->decrement();
		_lines[i]._rPic = nullptr;

		_lines[i]._lHyper = 0;
		_lines[i]._rHyper = 0;
		_lines[i]._lm = 0;
		_lines[i]._rm = 0;
		_lines[i]._newLine = 0;
		_lines[i]._dirty = true;
		_lines[i]._repaint = false;
	}

	_lastSeen = 0;
	_scrollPos = 0;
	_scrollMax = 0;

	for (i = 0; i < _height; i++)
		touch(i);
}

void TextBufferWindow::click(const Point &newPos) {
	int gh = false;
	int gs = false;

	if (_lineRequest || _charRequest
			|| _lineRequestUni || _charRequestUni
			|| _moreRequest || _scrollRequest)
		_windows->setFocus(this);

	if (_hyperRequest) {
		uint linkval = g_vm->_selection->getHyperlink(newPos);
		if (linkval) {
			g_vm->_events->store(evtype_Hyperlink, this, linkval, 0);
			_hyperRequest = false;
			if (g_conf->_safeClicks)
				g_vm->_events->_forceClick = 1;
			gh = true;
		}
	}

	if (newPos.x > _bbox.right - g_conf->_scrollWidth) {
		if (newPos.y < _bbox.top + g_conf->_tMarginY + g_conf->_scrollWidth)
			acceptScroll(keycode_Up);
		else if (newPos.y > _bbox.bottom - g_conf->_tMarginY - g_conf->_scrollWidth)
			acceptScroll(keycode_Down);
		else if (newPos.y < (_bbox.top + _bbox.bottom) / 2)
			acceptScroll(keycode_PageUp);
		else
			acceptScroll(keycode_PageDown);
		gs = true;
	}

	if (!gh && !gs) {
		g_vm->_copySelect = true;
		g_vm->_selection->startSelection(newPos);
	}
}

void TextBufferWindow::requestLineEvent(char *buf, uint maxlen, uint initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni) {
		warning("request_line_event: window already has keyboard request");
		return;
	}

	_lineRequest = true;
	int pw;

	gli_tts_flush();

	// because '>' prompt is ugly without extra space
	if (_numChars && _chars[_numChars - 1] == '>')
		putCharUni(' ');
	if (_numChars && _chars[_numChars - 1] == '?')
		putCharUni(' ');

	// make sure we have some space left for typing...
	pw = (_bbox.right - _bbox.left - g_conf->_tMarginX * 2) * GLI_SUBPIX;
	pw = pw - 2 * SLOP - _radjw + _ladjw;
	if (calcWidth(_chars, _attrs, 0, _numChars, -1) >= pw * 3 / 4)
		putCharUni('\n');

	_inBuf = buf;
	_inMax = maxlen;
	_inFence = _numChars;
	_inCurs = _numChars;
	_origAttr = _attr;
	_attr.set(style_Input);

	_historyPos = _historyPresent;

	if (initlen) {
		touch(0);
		putText(buf, initlen, _inCurs, 0);
	}

	// WORKAROUND: Mark bottom line as dirty so caret will be drawn
	_lines[0]._dirty = true;

	_echoLineInput = _echoLineInputBase;

	if (_lineTerminatorsBase && _termCt) {
		_lineTerminators = new uint[_termCt + 1];

		if (_lineTerminators) {
			memcpy(_lineTerminators, _lineTerminatorsBase, _termCt * sizeof(uint));
			_lineTerminators[_termCt] = 0;
		}
	}

	if (g_vm->gli_register_arr)
		_inArrayRock = (*g_vm->gli_register_arr)(buf, maxlen, "&+#!Cn");

	// Switch focus to the new window
	_windows->inputGuessFocus();
}

void TextBufferWindow::requestLineEventUni(uint32 *buf, uint maxlen, uint initlen) {
	if (_charRequest || _lineRequest || _charRequestUni || _lineRequestUni) {
		warning("request_line_event_uni: window already has keyboard request");
		return;
	}

	int pw;

	_lineRequestUni = true;
	gli_tts_flush();

	// because '>' prompt is ugly without extra space
	if (_numChars && _chars[_numChars - 1] == '>')
		putCharUni(' ');
	if (_numChars && _chars[_numChars - 1] == '?')
		putCharUni(' ');

	// make sure we have some space left for typing...
	pw = (_bbox.right - _bbox.left - g_conf->_tMarginX * 2) * GLI_SUBPIX;
	pw = pw - 2 * SLOP - _radjw + _ladjw;
	if (calcWidth(_chars, _attrs, 0, _numChars, -1) >= pw * 3 / 4)
		putCharUni('\n');

	//_lastSeen = 0;

	_inBuf = buf;
	_inMax = maxlen;
	_inFence = _numChars;
	_inCurs = _numChars;
	_origAttr = _attr;
	_attr.set(style_Input);

	_historyPos = _historyPresent;

	if (initlen) {
		touch(0);
		putTextUni(buf, initlen, _inCurs, 0);
	}

	_echoLineInput = _echoLineInputBase;

	if (_lineTerminatorsBase && _termCt) {
		_lineTerminators = new uint[_termCt + 1];

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

void TextBufferWindow::requestCharEvent() {
	_charRequest = true;

	// Switch focus to the new window
	_windows->inputGuessFocus();
}

void TextBufferWindow::requestCharEventUni() {
	_charRequestUni = true;

	// Switch focus to the new window
	_windows->inputGuessFocus();
}

void TextBufferWindow::cancelLineEvent(Event *ev) {
	gidispatch_rock_t inarrayrock;
	int ix;
	int len;
	void *inbuf;
	int inmax;
	bool unicode = _lineRequestUni;
	Event dummyEv;

	if (!ev)
		ev = &dummyEv;

	ev->clear();

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
		for (ix = 0; ix < len; ix++) {
			uint32 ch = _chars[_inFence + ix];
			if (ch > 0xff)
				ch = '?';
			((char *)inbuf)[ix] = (char)ch;
		}
	} else {
		for (ix = 0; ix < len; ix++)
			((uint *)inbuf)[ix] = _chars[_inFence + ix];
	}

	_attr = _origAttr;

	ev->type = evtype_LineInput;
	ev->window = this;
	ev->val1 = len;
	ev->val2 = 0;

	_lineRequest = false;
	_lineRequestUni = false;
	if (_lineTerminators) {
		delete[] _lineTerminators;
		_lineTerminators = nullptr;
	}

	_inBuf = nullptr;
	_inMax = 0;

	if (_echoLineInput) {
		putCharUni('\n');
	} else {
		_numChars = _inFence;
		touch(0);
	}

	if (g_vm->gli_unregister_arr)
		(*g_vm->gli_unregister_arr)(inbuf, inmax, unicode ? "&+#!Iu" : "&+#!Cn", inarrayrock);
}

void TextBufferWindow::redraw() {
	int linelen;
	int nsp, spw, pw;
	int x0, y0, x1, y1;
	int x, y, w;
	int a, b;
	uint link;
	int font;
	uint color;
	int i;
	int hx0, hx1, hy0, hy1;
	int selrow, selchar, sx0, sx1, selleft, selright;
	bool selBuf;
	int tx, tsc, tsw, lsc, rsc;
	Screen &screen = *g_vm->_screen;

	gli_tts_flush();

	Window::redraw();

	_lines[0]._len = _numChars;
	sx0 = sx1 = selleft = selright = 0;

	x0 = (_bbox.left + g_conf->_tMarginX) * GLI_SUBPIX;
	x1 = (_bbox.right - g_conf->_tMarginX - g_conf->_scrollWidth) * GLI_SUBPIX;
	y0 = _bbox.top + g_conf->_tMarginY;
	y1 = _bbox.bottom - g_conf->_tMarginY;

	pw = x1 - x0 - 2 * GLI_SUBPIX;

	// check if any part of buffer is selected
	selBuf = g_vm->_selection->checkSelection(Rect(x0 / GLI_SUBPIX, y0, x1 / GLI_SUBPIX, y1));

	for (i = _scrollPos + _height - 1; i >= _scrollPos; i--) {
		// top of line
		y = y0 + (_height - (i - _scrollPos) - 1) * _font._leading;

		// check if part of line is selected
		if (selBuf) {
			selrow = g_vm->_selection->getSelection(Rect(x0 / GLI_SUBPIX, y,
													x1 / GLI_SUBPIX, y + _font._leading), &sx0, &sx1);
			selleft = (sx0 == x0 / GLI_SUBPIX);
			selright = (sx1 == x1 / GLI_SUBPIX);
		} else {
			selrow = false;
		}

		// mark selected line dirty
		if (selrow)
			_lines[i]._dirty = true;

		TextBufferRow ln(_lines[i]);

		// skip if we can
		if (!ln._dirty && !ln._repaint && !Windows::_forceRedraw && _scrollPos == 0)
			continue;

		// repaint previously selected lines if needed
		if (ln._repaint && !Windows::_forceRedraw)
			_windows->redrawRect(Rect(x0 / GLI_SUBPIX, y,
									  x1 / GLI_SUBPIX, y + _font._leading));

		// keep selected line dirty and flag for repaint
		if (!selrow) {
			_lines[i]._dirty = false;
			_lines[i]._repaint = false;
		} else {
			_lines[i]._repaint = true;
		}

		// leave bottom line blank for [more] prompt
		if (i == _scrollPos && i > 0)
			continue;

		linelen = ln._len;

		// kill spaces at the end unless they're a different color
		color = Windows::_overrideBgSet ? g_conf->_windowColor : _bgColor;
		while (i > 0 && linelen > 1 && ln._chars[linelen - 1] == ' '
				&& _styles[ln._attrs[linelen - 1].style].bg == color
				&& !_styles[ln._attrs[linelen - 1].style].reverse)
			linelen --;

		// kill characters that would overwrite the scroll bar
		while (linelen > 1 && calcWidth(ln._chars, ln._attrs, 0, linelen, -1) >= pw)
			linelen --;

		/*
		 * count spaces and width for justification
		 */
		if (_font._justify && !ln._newLine && i > 0) {
			for (a = 0, nsp = 0; a < linelen; a++)
				if (ln._chars[a] == ' ')
					nsp ++;
			w = calcWidth(ln._chars, ln._attrs, 0, linelen, 0);
			if (nsp)
				spw = (x1 - x0 - ln._lm - ln._rm - 2 * SLOP - w) / nsp;
			else
				spw = 0;
		} else {
			spw = -1;
		}

		// find and highlight selected characters
		if (selrow && !Windows::_claimSelect) {
			lsc = 0;
			rsc = 0;
			selchar = false;
			// optimized case for all chars selected
			if (selleft && selright) {
				rsc = linelen > 0 ? linelen - 1 : 0;
				selchar = calcWidth(ln._chars, ln._attrs, lsc, rsc, spw) / GLI_SUBPIX;
			} else {
				// optimized case for leftmost char selected
				if (selleft) {
					tsc = linelen > 0 ? linelen - 1 : 0;
					selchar = calcWidth(ln._chars, ln._attrs, lsc, tsc, spw) / GLI_SUBPIX;
				} else {
					// find the substring contained by the selection
					tx = (x0 + SLOP + ln._lm) / GLI_SUBPIX;
					// measure string widths until we find left char
					for (tsc = 0; tsc < linelen; tsc++) {
						tsw = calcWidth(ln._chars, ln._attrs, 0, tsc, spw) / GLI_SUBPIX;
						if (tsw + tx >= sx0 ||
								((tsw + tx + GLI_SUBPIX) >= sx0 && ln._chars[tsc] != ' ')) {
							lsc = tsc;
							selchar = true;
							break;
						}
					}
				}
				if (selchar) {
					// optimized case for rightmost char selected
					if (selright) {
						rsc = linelen > 0 ? linelen - 1 : 0;
					} else {
						// measure string widths until we find right char
						for (tsc = lsc; tsc < linelen; tsc++) {
							tsw = calcWidth(ln._chars, ln._attrs, lsc, tsc, spw) / GLI_SUBPIX;
							if (tsw + sx0 < sx1)
								rsc = tsc;
						}
						if (lsc && !rsc)
							rsc = lsc;
					}
				}
			}
			// reverse colors for selected chars
			if (selchar) {
				for (tsc = lsc; tsc <= rsc; tsc++) {
					ln._attrs[tsc].reverse = !ln._attrs[tsc].reverse;
					_copyBuf[_copyPos] = ln._chars[tsc];
					_copyPos++;
				}
			}
			// add newline if we reach the end of the line
			if (ln._len == 0 || ln._len == (rsc + 1)) {
				_copyBuf[_copyPos] = '\n';
				_copyPos++;
			}
		}

		// clear any stored hyperlink coordinates
		g_vm->_selection->putHyperlink(0, x0 / GLI_SUBPIX, y,
									   x1 / GLI_SUBPIX, y + _font._leading);

		/*
		 * fill in background colors
		 */
		color = Windows::_overrideBgSet ? g_conf->_windowColor : _bgColor;
		screen.fillRect(Rect::fromXYWH(x0 / GLI_SUBPIX, y, (x1 - x0) / GLI_SUBPIX, _font._leading),
						color);

		x = x0 + SLOP + ln._lm;
		a = 0;
		for (b = 0; b < linelen; b++) {
			if (ln._attrs[a] != ln._attrs[b]) {
				link = ln._attrs[a].hyper;
				font = ln._attrs[a].attrFont(_styles);
				color = ln._attrs[a].attrBg(_styles);
				w = screen.stringWidthUni(font, Common::U32String(ln._chars + a, b - a), spw);
				screen.fillRect(Rect::fromXYWH(x / GLI_SUBPIX, y, w / GLI_SUBPIX, _font._leading),
								color);
				if (link) {
					screen.fillRect(Rect::fromXYWH(x / GLI_SUBPIX + 1, y + _font._baseLine + 1,
												   w / GLI_SUBPIX + 1, _font._linkStyle), _font._linkColor);
					g_vm->_selection->putHyperlink(link, x / GLI_SUBPIX, y,
												   x / GLI_SUBPIX + w / GLI_SUBPIX,
												   y + _font._leading);
				}
				x += w;
				a = b;
			}
		}
		link = ln._attrs[a].hyper;
		font = ln._attrs[a].attrFont(_styles);
		color = ln._attrs[a].attrBg(_styles);
		w = screen.stringWidthUni(font, Common::U32String(ln._chars + a, b - a), spw);
		screen.fillRect(Rect::fromXYWH(x / GLI_SUBPIX, y, w / GLI_SUBPIX, _font._leading), color);
		if (link) {
			screen.fillRect(Rect::fromXYWH(x / GLI_SUBPIX + 1, y + _font._baseLine + 1,
										   w / GLI_SUBPIX + 1, _font._linkStyle), _font._linkColor);
			g_vm->_selection->putHyperlink(link, x / GLI_SUBPIX, y,
										   x / GLI_SUBPIX + w / GLI_SUBPIX,
										   y + _font._leading);
		}
		x += w;

		color = Windows::_overrideBgSet ? g_conf->_windowColor : _bgColor;
		screen.fillRect(Rect::fromXYWH(x / GLI_SUBPIX, y, x1 / GLI_SUBPIX - x / GLI_SUBPIX, _font._leading), color);

		/*
		 * draw caret
		 */

		if (_windows->getFocusWindow() == this && i == 0 && (_lineRequest || _lineRequestUni)) {
			w = calcWidth(_chars, _attrs, 0, _inCurs, spw);
			if (w < pw - _font._caretShape * 2 * GLI_SUBPIX)
				_font.drawCaret(Point(x0 + SLOP + ln._lm + w, y + _font._baseLine));
		}

		/*
		 * draw text
		 */

		x = x0 + SLOP + ln._lm;
		a = 0;
		for (b = 0; b < linelen; b++) {
			if (ln._attrs[a] != ln._attrs[b]) {
				link = ln._attrs[a].hyper;
				font = ln._attrs[a].attrFont(_styles);
				color = link ? _font._linkColor : ln._attrs[a].attrFg(_styles);
				x = screen.drawStringUni(Point(x, y + _font._baseLine),
										 font, color, Common::U32String(ln._chars + a, b - a), spw);
				a = b;
			}
		}
		link = ln._attrs[a].hyper;
		font = ln._attrs[a].attrFont(_styles);
		color = link ? _font._linkColor : ln._attrs[a].attrFg(_styles);
		screen.drawStringUni(Point(x, y + _font._baseLine), font, color, Common::U32String(ln._chars + a, linelen - a), spw);
	}

	/*
	 * draw more prompt
	 */
	if (_scrollPos && _height > 1) {
		x = x0 + SLOP;
		y = y0 + (_height - 1) * _font._leading;

		g_vm->_selection->putHyperlink(0, x0 / GLI_SUBPIX, y,
									   x1 / GLI_SUBPIX, y + _font._leading);

		color = Windows::_overrideBgSet ? g_conf->_windowColor : _bgColor;
		screen.fillRect(Rect::fromXYWH(x / GLI_SUBPIX, y, x1 / GLI_SUBPIX - x / GLI_SUBPIX, _font._leading), color);

		w = screen.stringWidth(_font._moreFont, _font._morePrompt);

		if (_font._moreAlign == 1)
			// center
			x = x0 + SLOP + (x1 - x0 - w - SLOP * 2) / 2;
		if (_font._moreAlign == 2)
			// right
			x = x1 - SLOP - w;

		color = Windows::_overrideFgSet ? _font._moreColor : _fgColor;
		screen.drawString(Point(x, y + _font._baseLine),
						  _font._moreFont, color, _font._morePrompt);
		y1 = y; // don't want pictures overdrawing "[more]"

		// try to claim the focus
		_moreRequest = true;
		Windows::_moreFocus = true;
	} else {
		_moreRequest = false;
		y1 = y0 + _height * _font._leading;
	}

	/*
	 * draw the images
	 */
	for (i = 0; i < _scrollBack; i++) {
		TextBufferRow ln(_lines[i]);

		y = y0 + (_height - (i - _scrollPos) - 1) * _font._leading;

		if (ln._lPic) {
			if (y < y1 && y + ln._lPic->h > y0) {
				ln._lPic->drawPicture(Point(x0 / GLI_SUBPIX, y),
					Rect(x0 / GLI_SUBPIX, y0, x1 / GLI_SUBPIX, y1));
				link = ln._lHyper;
				hy0 = y > y0 ? y : y0;
				hy1 = y + ln._lPic->h < y1 ? y + ln._lPic->h : y1;
				hx0 = x0 / GLI_SUBPIX;
				hx1 = x0 / GLI_SUBPIX + ln._lPic->w < x1 / GLI_SUBPIX
					  ? x0 / GLI_SUBPIX + ln._lPic->w
					  : x1 / GLI_SUBPIX;
				g_vm->_selection->putHyperlink(link, hx0, hy0, hx1, hy1);
			}
		}

		if (ln._rPic) {
			if (y < y1 && y + ln._rPic->h > y0) {
				ln._rPic->drawPicture(Point(x1 / GLI_SUBPIX - ln._rPic->w, y),
					Rect(x0 / GLI_SUBPIX, y0, x1 / GLI_SUBPIX, y1));
				link = ln._rHyper;
				hy0 = y > y0 ? y : y0;
				hy1 = y + ln._rPic->h < y1 ? y + ln._rPic->h : y1;
				hx0 = x1 / GLI_SUBPIX - ln._rPic->w > x0 / GLI_SUBPIX
					  ? x1 / GLI_SUBPIX - ln._rPic->w
					  : x0 / GLI_SUBPIX;
				hx1 = x1 / GLI_SUBPIX;
				g_vm->_selection->putHyperlink(link, hx0, hy0, hx1, hy1);
			}
		}
	}

	/*
	 * Draw the scrollbar
	 */

	// try to claim scroll keys
	_scrollRequest = _scrollMax > _height;

	if (_scrollRequest && g_conf->_scrollWidth) {
		int t0, t1;
		x0 = _bbox.right - g_conf->_scrollWidth;
		x1 = _bbox.right;
		y0 = _bbox.top + g_conf->_tMarginY;
		y1 = _bbox.bottom - g_conf->_tMarginY;

		g_vm->_selection->putHyperlink(0, x0, y0, x1, y1);

		y0 += g_conf->_scrollWidth / 2;
		y1 -= g_conf->_scrollWidth / 2;

		// pos = thbot, pos - ht = thtop, max = wtop, 0 = wbot
		t0 = (_scrollMax - _scrollPos) - (_height - 1);
		t1 = (_scrollMax - _scrollPos);
		if (_scrollMax > _height) {
			t0 = t0 * (y1 - y0) / _scrollMax + y0;
			t1 = t1 * (y1 - y0) / _scrollMax + y0;
		} else {
			t0 = t1 = y0;
		}

		screen.fillRect(Rect::fromXYWH(x0 + 1, y0, x1 - x0 - 2, y1 - y0), g_conf->_scrollBg);
		screen.fillRect(Rect::fromXYWH(x0 + 1, t0, x1 - x0 - 2, t1 - t0), g_conf->_scrollFg);

		for (i = 0; i < g_conf->_scrollWidth / 2 + 1; i++) {
			screen.fillRect(Rect::fromXYWH(x0 + g_conf->_scrollWidth / 2 - i,
										   y0 - g_conf->_scrollWidth / 2 + i, i * 2, 1), g_conf->_scrollFg);
			screen.fillRect(Rect::fromXYWH(x0 + g_conf->_scrollWidth / 2 - i,
										   y1 + g_conf->_scrollWidth / 2 - i, i * 2, 1), g_conf->_scrollFg);
		}
	}

	// Keep track of selected text to be ready when user copies it to the clipboard
	if (selBuf && _copyPos) {
		Windows::_claimSelect = true;

		g_vm->_clipboard->clipboardStore(Common::U32String(_copyBuf, _copyPos));
		for (i = 0; i < _copyPos; i++)
			_copyBuf[i] = 0;
		_copyPos = 0;
	}

	// no more prompt means all text has been seen
	if (!_moreRequest)
		_lastSeen = 0;
}

int TextBufferWindow::acceptScroll(uint arg) {
	int pageht = _height - 2;        // 1 for prompt, 1 for overlap
	int startpos = _scrollPos;

	switch (arg) {
	case keycode_PageUp:
		_scrollPos += pageht;
		break;
	case keycode_End:
		_scrollPos = 0;
		break;
	case keycode_Up:
		_scrollPos++;
		break;
	case keycode_Down:
	case keycode_Return:
		_scrollPos--;
		break;
	case keycode_MouseWheelUp:
		_scrollPos += 3;
		startpos = true;
		break;
	case keycode_MouseWheelDown:
		_scrollPos -= 3;
		startpos = true;
		break;
	case ' ':
	case keycode_PageDown:
		//default:
		if (pageht)
			_scrollPos -= pageht;
		else
			_scrollPos = 0;
		break;
	default:
		break;
	}

	if (_scrollPos > _scrollMax - _height + 1)
		_scrollPos = _scrollMax - _height + 1;
	if (_scrollPos < 0)
		_scrollPos = 0;
	touchScroll();

	return (startpos || _scrollPos);
}

void TextBufferWindow::acceptReadChar(uint arg) {
	uint key;

	if (_height < 2)
		_scrollPos = 0;

	if (_scrollPos
			|| arg == keycode_PageUp
			|| arg == keycode_MouseWheelUp) {
		acceptScroll(arg);
		return;
	}

	switch (arg) {
	case keycode_Erase:
		key = keycode_Delete;
		break;
	case keycode_MouseWheelUp:
	case keycode_MouseWheelDown:
		return;
	default:
		key = arg;
		break;
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

void TextBufferWindow::acceptReadLine(uint32 arg) {
	uint *cx;
	Common::U32String s;
	int len;

	if (_height < 2)
		_scrollPos = 0;

	if (!_inBuf)
		return;

	if (_lineTerminators && checkTerminators(arg)) {
		for (cx = _lineTerminators; *cx; cx++) {
			if (*cx == arg) {
				acceptLine(arg);
				return;
			}
		}
	}

	if (_scrollPos || arg == keycode_PageUp || arg == keycode_MouseWheelUp) {
		acceptScroll(arg);
		return;
	}

	switch (arg) {
	// History keys (up and down)
	case keycode_Up:
		if (_historyPos == _historyFirst)
			return;
		if (_historyPos == _historyPresent) {
			len = _numChars - _inFence;

			if (len > 0)
				s = Common::U32String(&(_chars[_inFence]), len);
			_history[_historyPos] = s;
		}

		_historyPos--;
		if (_historyPos < 0)
			_historyPos += HISTORYLEN;
		s = _history[_historyPos];
		putTextUni(s.u32_str(), s.size(), _inFence, _numChars - _inFence);
		break;

	case keycode_Down:
		if (_historyPos == _historyPresent)
			return;
		_historyPos++;
		if (_historyPos >= HISTORYLEN)
			_historyPos -= HISTORYLEN;
		s = _history[_historyPos];
		putTextUni(s.u32_str(), s.size(), _inFence, _numChars - _inFence);
		break;

	// Cursor movement keys, during line input.
	case keycode_Left:
		if (_inCurs <= _inFence)
			return;
		_inCurs--;
		break;

	case keycode_Right:
		if (_inCurs >= _numChars)
			return;
		_inCurs++;
		break;

	case keycode_Home:
		if (_inCurs <= _inFence)
			return;
		_inCurs = _inFence;
		break;

	case keycode_End:
		if (_inCurs >= _numChars)
			return;
		_inCurs = _numChars;
		break;

	case keycode_SkipWordLeft:
		while (_inCurs > _inFence && _chars[_inCurs - 1] == ' ')
			_inCurs--;
		while (_inCurs > _inFence && _chars[_inCurs - 1] != ' ')
			_inCurs--;
		break;

	case keycode_SkipWordRight:
		while (_inCurs < _numChars && _chars[_inCurs] != ' ')
			_inCurs++;
		while (_inCurs < _numChars && _chars[_inCurs] == ' ')
			_inCurs++;
		break;

	// Delete keys, during line input.
	case keycode_Delete:
		if (_inCurs <= _inFence)
			return;
		putTextUni(nullptr, 0, _inCurs - 1, 1);
		break;

	case keycode_Erase:
		if (_inCurs >= _numChars)
			return;
		putTextUni(nullptr, 0, _inCurs, 1);
		break;

	case keycode_Escape:
		if (_inFence >= _numChars)
			return;
		putTextUni(nullptr, 0, _inFence, _numChars - _inFence);
		break;

	// Regular keys
	case keycode_Return:
		acceptLine(arg);
		break;

	default:
		if (arg >= 32 && arg <= 0x10FFFF) {
			if (_font._caps && (arg > 0x60 && arg < 0x7b))
				arg -= 0x20;
			putTextUni(&arg, 1, _inCurs, 0);
		}
		break;
	}

	touch(0);
}

void TextBufferWindow::acceptLine(uint32 keycode) {
	int ix;
	int len, olen;
	void *inbuf;
	Common::U32String s, o;
	int inmax;
	gidispatch_rock_t inarrayrock;
	bool unicode = _lineRequestUni;

	if (!_inBuf)
		return;

	inbuf = _inBuf;
	inmax = _inMax;
	inarrayrock = _inArrayRock;

	len = _numChars - _inFence;
	if (_echoStream)
		_echoStream->echoLineUni(_chars + _inFence, len);

	gli_tts_purge();
	if (g_conf->_speakInput) {
		const uint32 NEWLINE = '\n';
		gli_tts_speak(_chars + _inFence, len);
		gli_tts_speak((const uint32 *)&NEWLINE, 1);
	}

	/*
	* Store in history.
	* The history is a ring buffer, with historypresent being the index of the most recent
	* element and historyfirst the index of the oldest element.
	* A history entry should not repeat the string from the entry before it.
	*/
	if (len) {
		s = Common::U32String(_chars + _inFence, len);
		_history[_historyPresent].clear();

		o = _history[(_historyPresent == 0 ? HISTORYLEN : _historyPresent) - 1];
		olen = o.size();

		if (len != olen || !s.equals(o)) {
			_history[_historyPresent] = s;

			_historyPresent++;
			if (_historyPresent == HISTORYLEN)
				_historyPresent = 0;

			if (_historyPresent == _historyFirst) {
				_historyFirst++;
				if (_historyFirst == HISTORYLEN)
					_historyFirst = 0;
			}
		}
	}

	// Store in event buffer.
	if (len > inmax)
		len = inmax;

	if (!unicode) {
		for (ix = 0; ix < len; ix++) {
			uint32 ch = _chars[_inFence + ix];
			if (ch > 0xff)
				ch = '?';
			((char *)inbuf)[ix] = (char)ch;
		}
	} else {
		for (ix = 0; ix < len; ix++)
			((uint *)inbuf)[ix] = _chars[_inFence + ix];
	}

	_attr = _origAttr;

	if (_lineTerminators) {
		if (keycode == keycode_Return)
			keycode = 0;
		else
			// TODO: Currently particularly for Beyond Zork, we don't echo a newline
			// for line terminators, allowing description area scrolling to not keep adding
			// blank lines in the command area. In the future I may need to make it configurable
			// when I see if any other line terminators need to have a newline
			_echoLineInput = false;

		g_vm->_events->store(evtype_LineInput, this, len, keycode);
		delete[] _lineTerminators;
		_lineTerminators = nullptr;
	} else {
		g_vm->_events->store(evtype_LineInput, this, len, 0);
	}

	_lineRequest = false;
	_lineRequestUni = false;
	_inBuf = nullptr;
	_inMax = 0;

	if (_echoLineInput) {
		putCharUni('\n');
	} else {
		_numChars = _inFence;
		touch(0);
	}

	if (g_vm->gli_unregister_arr)
		(*g_vm->gli_unregister_arr)(inbuf, inmax, unicode ? "&+#!Iu" : "&+#!Cn", inarrayrock);
}

bool TextBufferWindow::leftquote(uint32 c) {
	switch (c) {
	case '(':
	case '[':

	// The following are Unicode characters in the "Separator, Space" category.
	case 0x0020:
	case 0x00a0:
	case 0x1680:
	case 0x2000:
	case 0x2001:
	case 0x2002:
	case 0x2003:
	case 0x2004:
	case 0x2005:
	case 0x2006:
	case 0x2007:
	case 0x2008:
	case 0x2009:
	case 0x200a:
	case 0x202f:
	case 0x205f:
	case 0x3000:
		return true;
	default:
		return false;
	}
}

void TextBufferWindow::scrollOneLine(bool forced) {
	_lastSeen++;
	_scrollMax++;

	if (_scrollMax > _scrollBack - 1
			|| _lastSeen > _scrollBack - 1)
		scrollResize();

	if (_lastSeen >= _height)
		_scrollPos++;

	if (_scrollPos > _scrollMax - _height + 1)
		_scrollPos = _scrollMax - _height + 1;
	if (_scrollPos < 0)
		_scrollPos = 0;

	if (forced)
		_dashed = 0;
	_spaced = 0;

	_lines[0]._len = _numChars;
	_lines[0]._newLine = forced;

	for (int i = _scrollBack - 1; i > 0; i--) {
		memcpy(&_lines[i], &_lines[i - 1], sizeof(TextBufferRow));
		if (i < _height)
			touch(i);
	}

	if (_radjn)
		_radjn--;
	if (_radjn == 0)
		_radjw = 0;
	if (_ladjn)
		_ladjn--;
	if (_ladjn == 0)
		_ladjw = 0;

	touch(0);
	_lines[0]._len = 0;
	_lines[0]._newLine = 0;
	_lines[0]._lm = _ladjw;
	_lines[0]._rm = _radjw;
	_lines[0]._lPic = nullptr;
	_lines[0]._rPic = nullptr;
	_lines[0]._lHyper = 0;
	_lines[0]._rHyper = 0;
	
	Common::fill(_chars, _chars + TBLINELEN, ' ');
	Attributes *a = _attrs;
	for (int i = 0; i < TBLINELEN; ++i, ++a)
		a->clear();

	_numChars = 0;

	touchScroll();

}

void TextBufferWindow::scrollResize() {
	int i;

	_lines.clear();
	_lines.resize(_scrollBack + SCROLLBACK);

	_chars = _lines[0]._chars;
	_attrs = _lines[0]._attrs;

	for (i = _scrollBack; i < (_scrollBack + SCROLLBACK); i++) {
		_lines[i]._dirty = false;
		_lines[i]._repaint = false;
		_lines[i]._lm = 0;
		_lines[i]._rm = 0;
		_lines[i]._lPic = 0;
		_lines[i]._rPic = 0;
		_lines[i]._lHyper = 0;
		_lines[i]._rHyper = 0;
		_lines[i]._len = 0;
		_lines[i]._newLine = 0;
		memset(_lines[i]._chars, ' ', sizeof _lines[i]._chars);
		memset(_lines[i]._attrs, 0, sizeof _lines[i]._attrs);
	}

	_scrollBack += SCROLLBACK;
}

int TextBufferWindow::calcWidth(const uint32 *chars, const Attributes *attrs, int startchar, int numChars, int spw) {
	Screen &screen = *g_vm->_screen;
	int w = 0;
	int a, b;

	a = startchar;
	for (b = startchar; b < numChars; b++) {
		if (attrs[a] != attrs[b]) {
			w += screen.stringWidthUni(attrs[a].attrFont(_styles),
									   Common::U32String(chars + a, b - a), spw);
			a = b;
		}
	}

	w += screen.stringWidthUni(attrs[a].attrFont(_styles), Common::U32String(chars + a, b - a), spw);

	return w;
}

void TextBufferWindow::getSize(uint *width, uint *height) const {
	if (width)
		*width = (_bbox.width() - g_conf->_tMarginX * 2) / _font._cellW;
	if (height)
		*height = (_bbox.height() - g_conf->_tMarginY * 2) / _font._cellH;
}

void TextBufferWindow::flowBreak() {
	while (_ladjn || _radjn)
		putCharUni('\n');
}

/*--------------------------------------------------------------------------*/

TextBufferWindow::TextBufferRow::TextBufferRow() : _len(0), _newLine(0), _dirty(false),
	_repaint(false), _lPic(nullptr), _rPic(nullptr), _lHyper(0), _rHyper(0),
	_lm(0), _rm(0) {
	Common::fill(&_chars[0], &_chars[TBLINELEN], 0);
}

} // End of namespace Glk
