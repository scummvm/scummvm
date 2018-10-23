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

#include "gargoyle/window_text_buffer.h"
#include "gargoyle/conf.h"
#include "gargoyle/gargoyle.h"

namespace Gargoyle {

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

} // End of namespace Gargoyle
