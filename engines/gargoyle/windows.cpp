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
int Windows::_overrideFgVal;
int Windows::_overrideBgVal;

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
		_screen(screen), _forceRedraw(true), _moreFocus(false), _windowList(nullptr),
		_rootWin(nullptr), _focusWin(nullptr), _mask(nullptr), _claimSelect(0) {
	_overrideReverse = false;
	_overrideFgSet = false;
	_overrideBgSet = false;
	_overrideFgVal = 0;
	_overrideBgVal = 0;
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

void Windows::clearSelection() {
	if (!_mask) {
		warning("clear_selection: mask not initialized");
		return;
	}

	if (_mask->select.left || _mask->select.right
		|| _mask->select.top || _mask->select.bottom)
		_forceRedraw = true;

	_mask->select = Common::Rect();
	_claimSelect = false;
}

void Windows::repaint(const Common::Rect &box) {
	// TODO
}

/*--------------------------------------------------------------------------*/

Window::Window(Windows *windows, glui32 rock) : _magicnum(MAGIC_WINDOW_NUM),
		_windows(windows), _rock(rock), _type(0), _parent(nullptr), _next(nullptr), _prev(nullptr),
		yadj(0), line_request(0), line_request_uni(0), char_request(0), char_request_uni(0),
		mouse_request(0), hyper_request(0), more_request(0), scroll_request(0), image_loaded(0),
		echo_line_input(true),  line_terminators(nullptr), termct(0), _echoStream(nullptr) {
	attr.fgset = 0;
	attr.bgset = 0;
	attr.reverse = 0;
	attr.style = 0;
	attr.fgcolor = 0;
	attr.bgcolor = 0;
	attr.hyper = 0;

	Common::fill(&bgcolor[0], &bgcolor[3], 3);
	Common::fill(&fgcolor[0], &fgcolor[3], 3);
	disprock.num = 0;

	Streams &streams = *g_vm->_streams;
	_stream = streams.addWindowStream(this);
}

void Window::cancelLineEvent(Event *ev) {
	Event dummyEv;
	if (!ev)
		ev = &dummyEv;

	g_vm->_events->clearEvent(ev);
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
	_inorgX = _inorgY = 0;
	_inMax = 0;
	_inCurs = _inLen = 0;
	_inArrayRock.num = 0;
	_lineTerminators = nullptr;

	Common::copy(&g_conf->_gStyles[0], &g_conf->_gStyles[style_NUMSTYLES], styles);
}

void TextGridWindow::rearrange(const Common::Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;

	newwid = box.width() / g_conf->_cellW;
	newhgt = box.height() / g_conf->_cellH;

	if (newwid == _width && newhgt == _height)
		return;

	lines.resize(newhgt);
	for (int y = 0; y < newhgt; ++y) {
		lines[y].resize(newwid);
		touch(y);
	}

	attr.clear();
	_width = newwid;
	_height = newhgt;
}

void TextGridWindow::touch(int line) {
	int y = bbox.top + line * g_conf->_leading;
	lines[line].dirty = true;
	_windows->repaint(Common::Rect(bbox.left, y, bbox.right, y + g_conf->_leading));
}

glui32 TextGridWindow::getSplit(glui32 size, bool vertical) const {
	return vertical ? size * g_conf->_cellW + g_conf->_tMarginX * 2 :
		size * g_conf->_cellH + g_conf->_tMarginY * 2;
}

void TextGridWindow::cancelLineEvent(Event *ev) {
	Event dummyEv;

	if (!ev)
		ev = &dummyEv;

	g_vm->_events->clearEvent(ev);

	if (!line_request && !line_request_uni)
		return;


	// TODO : textgrid_cancel_line
}

/*--------------------------------------------------------------------------*/

void TextGridWindow::TextGridRow::resize(size_t newSize) {
	chars.clear();
	attr.clear();
	chars.resize(newSize);
	attr.resize(newSize);
	Common::fill(&chars[0], &chars[0] + newSize, ' ');
}

/*--------------------------------------------------------------------------*/

TextBufferWindow::TextBufferWindow(Windows *windows, uint32 rock) : Window(windows, rock),
		_historyPos(0), _historyFirst(0), _historyPresent(0), _lastSeen(0), _scrollPos(0),
		_scrollMax(0), _scrollBack(SCROLLBACK), _width(-1), _height(-1), _inBuf(nullptr),
		_lineTerminators(nullptr), _echoLineInput(true), _ladjw(0), _radjw(0), _ladjn(0),
		_radjn(0), _numChars(0), _chars(nullptr), _attrs(nullptr),
		_spaced(0), _dashed(0), copybuf(0), copypos(0) {
	_type = wintype_TextBuffer;
	Common::fill(&_history[0], &_history[HISTORYLEN], nullptr);

	Common::copy(&g_conf->_tStyles[0], &g_conf->_tStyles[style_NUMSTYLES], styles);
}

void TextBufferWindow::rearrange(const Common::Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;
	int rnd;

	newwid = (box.width() - g_conf->_tMarginX * 2 - g_conf->_scrollWidth) / g_conf->_cellW;
	newhgt = (box.height() - g_conf->_tMarginY * 2) / g_conf->_cellH;

	/* align text with bottom */
	rnd = newhgt * g_conf->_cellH + g_conf->_tMarginY * 2;
	yadj = (box.height() - rnd);
	bbox.top += (box.height() - rnd);

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
		if (copybuf)
			delete[] copybuf;
		copybuf = new glui32[_height * TBLINELEN];

		for (int i = 0; i < (_height * TBLINELEN); i++)
			copybuf[i] = 0;

		copypos = 0;
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

	oldattr = attr;
	curattr.clear();

	x = 0;
	p = 0;
	s = _scrollMax < SCROLLBACK ? _scrollMax : SCROLLBACK - 1;

	for (k = s; k >= 0; k--) {
		if (k == 0 && line_request)
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
		attr = attrbuf[i];

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
		putTextUnit(charbuf + inputbyte, p - inputbyte, _numChars, 0);
		_inCurs = _numChars;
	}

	// free temp buffers
	delete[] attrbuf;
	delete[] charbuf;
	delete[] alignbuf;
	delete[] pictbuf;
	delete[] hyperbuf;
	delete[] offsetbuf;

	attr = oldattr;

	touchScroll();
}

void TextBufferWindow::touchScroll() {
	_windows->clearSelection();
	_windows->repaint(bbox);

	for (int i = 0; i < _scrollMax; i++)
		_lines[i].dirty = true;
}

void TextBufferWindow::clear() {
	int i;

	attr.fgset = Windows::_overrideFgSet;
	attr.bgset = Windows::_overrideBgSet;
	attr.fgcolor = Windows::_overrideFgSet ? Windows::_overrideFgVal : 0;
	attr.bgcolor = Windows::_overrideBgSet ? Windows::_overrideBgVal : 0;
	attr.reverse = false;

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

	return true ;
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

	pw = (bbox.right - bbox.left - g_conf->_tMarginX * 2 - gli_scroll_width) * GLI_SUBPIX;
	pw = pw - 2 * SLOP - radjw - ladjw;

	color = gli_override_bg_set ? gli_window_color : bgcolor;

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
			if (numchars == 0 || leftquote(chars[numchars - 1]))
				ch = UNI_LSQUO;
		}

		if (ch == '`')
			ch = UNI_LSQUO;

		if (ch == '\'')
			ch = UNI_RSQUO;

		if (ch == '"')
		{
			if (numchars == 0 || leftquote(chars[numchars - 1]))
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

	chars[numchars] = ch;
	attrs[numchars] = attr;
	numchars++;

	// kill spaces at the end for line width calculation
	linelen = numchars;
	while (linelen > 1 && chars[linelen - 1] == ' '
		&& styles[attrs[linelen - 1].style].bg == color
		&& !styles[attrs[linelen - 1].style].reverse)
		linelen--;

	if (calcwidth(dwin, chars, attrs, 0, linelen, -1) >= pw)
	{
		bpoint = numchars;

		for (i = numchars - 1; i > 0; i--)
			if (chars[i] == ' ')
			{
				bpoint = i + 1; // skip space
				break;
			}

		saved = numchars - bpoint;

		memcpy(bchars, chars + bpoint, saved * 4);
		memcpy(battrs, attrs + bpoint, saved * sizeof(attr_t));
		numchars = bpoint;

		scrolloneline(dwin, 0);

		memcpy(chars, bchars, saved * 4);
		memcpy(attrs, battrs, saved * sizeof(attr_t));
		numchars = saved;
	}

	touch(0);
	*/
}

void TextBufferWindow::putTextUnit(const glui32 *buf, int len, int pos, int oldlen) {
	// TODO
}

void TextBufferWindow::flowBreak() {
	// TODO
}

void TextBufferWindow::touch(int line) {
	int y = bbox.top + g_conf->_tMarginY + (_height - line - 1) * g_conf->_leading;
	_lines[line].dirty = 1;
	_windows->clearSelection();
	_windows->repaint(Common::Rect(bbox.left, y - 2, bbox.right, y + g_conf->_leading + 2));
}

glui32 TextBufferWindow::getSplit(glui32 size, bool vertical) const {
	return (vertical) ? size * g_conf->_cellW : size * g_conf->_cellH;
}

void TextBufferWindow::cancelLineEvent(Event *ev) {
	Event dummyEv;

	if (!ev)
		ev = &dummyEv;

	g_vm->_events->clearEvent(ev);

	if (!line_request && !line_request_uni)
		return;


	// TODO : textbuffer_cancel_line
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
		w(0), h(0), dirty(false), _surface(nullptr) {
	_type = wintype_Graphics;
	Common::copy(&bgcolor[0], &bgcolor[3], bgnd);
}

void GraphicsWindow::rearrange(const Common::Rect &box) {
	int newwid, newhgt;
	int bothwid, bothhgt;
	int oldw, oldh;
	Graphics::ManagedSurface *newSurface;

	bbox = box;

	newwid = box.width();
	newhgt = box.height();
	oldw = w;
	oldh = h;

	if (newwid <= 0 || newhgt <= 0) {
		w = 0;
		h = 0;
		delete _surface;
		_surface = NULL;
		return;
	}

	bothwid = w;
	if (newwid < bothwid)
		bothwid = newwid;
	bothhgt = h;
	if (newhgt < bothhgt)
		bothhgt = newhgt;

	newSurface = new Graphics::ManagedSurface(newwid, newhgt,
		Graphics::PixelFormat(3, 8, 8, 8, 0, 16, 8, 0, 0));

	// If the new surface is equal or bigger than the old one, copy it over
	if (_surface && bothwid && bothhgt)
		newSurface->blitFrom(*_surface);

	delete _surface;
	_surface = newSurface;
	w = newwid;
	h = newhgt;

	touch();
}

void GraphicsWindow::touch() {
	dirty = true;
	_windows->repaint(bbox);
}

/*--------------------------------------------------------------------------*/

PairWindow::PairWindow(Windows *windows, glui32 method, Window *key, glui32 size) :
		Window(windows, 0),
		_dir(method & winmethod_DirMask),
		_division(method & winmethod_DivisionMask),
		_wborder((method & winmethod_BorderMask) == winmethod_Border),
		_vertical(_dir == winmethod_Left || _dir == winmethod_Right),
		_backward(_dir == winmethod_Left || _dir == winmethod_Above),
		_key(key), _size(size), _keydamage(0), _child1(nullptr), _child2(nullptr) {
	_type = wintype_Pair;
}

void PairWindow::rearrange(const Common::Rect &box) {
	Common::Rect box1, box2;
	int min, diff, split, splitwid, max;
	Window *ch1, *ch2;

	bbox = box;

	if (_vertical) {
		min = bbox.left;
		max = bbox.right;
	} else {
		min = bbox.top;
		max = bbox.bottom;
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
		box1.left = bbox.left;
		box1.right = split;
		box2.left = split + splitwid;
		box2.right = bbox.right;
		box1.top = bbox.top;
		box1.bottom = bbox.bottom;
		box2.top = bbox.top;
		box2.bottom = bbox.bottom;
	} else {
		box1.top = bbox.top;
		box1.bottom = split;
		box2.top = split + splitwid;
		box2.bottom = bbox.bottom;
		box1.left = bbox.left;
		box1.right = bbox.right;
		box2.left = bbox.left;
		box2.right = bbox.right;
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

} // End of namespace Gargoyle
