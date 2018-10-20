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
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Gargoyle {

#define MAGIC_WINDOW_NUM (9876)
#define GLI_SUBPIX 8

bool Windows::_confLockCols;
bool Windows::_confLockRows;
int Windows::_wMarginx;
int Windows::_wMarginy;
int Windows::_wPaddingx;
int Windows::_wPaddingy;
int Windows::_wBorderx;
int Windows::_wBordery;
int Windows::_tMarginx;
int Windows::_tMarginy;
int Windows::_wMarginXsave;
int Windows::_wMarginYsave;
int Windows::_cols;
int Windows::_rows;
int Windows::_imageW;
int Windows::_imageH;
int Windows::_cellW;
int Windows::_cellH;
int Windows::_baseLine;
int Windows::_leading;
int Windows::_scrollWidth;
bool Windows::_overrideReverse;
bool Windows::_overrideFgSet;
bool Windows::_overrideBgSet;
int Windows::_overrideFgVal;
int Windows::_overrideBgVal;


WindowStyle T_STYLES[style_NUMSTYLES] = {
	{ PROPR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Normal
	{ PROPI,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Emphasized
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Preformatted
	{ PROPB,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Header
	{ PROPB,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Subheader
	{ PROPZ,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Alert
	{ PROPR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Note
	{ PROPR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< BlockQuote
	{ PROPB,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Input
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< User1
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< User2
};

WindowStyle G_STYLES[style_NUMSTYLES] = {
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Normal
	{ MONOI,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Emphasized
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Preformatted
	{ MONOB,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Header
	{ MONOB,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Subheader
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Alert
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Note
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< BlockQuote
	{ MONOR,{ 0xff,0xff,0xff },{ 0x00,0x00,0x00 }, 0 }, ///< Input
	{ MONOR,{ 0x60,0x60,0x60 },{ 0xff,0xff,0xff }, 0 }, ///< User1
	{ MONOR,{ 0x60,0x60,0x60 },{ 0xff,0xff,0xff }, 0 }, ///< User2
};

/*--------------------------------------------------------------------------*/

Windows::Windows(Graphics::Screen *screen) : _screen(screen), _forceRedraw(true), _moreFocus(false),
		_windowList(nullptr), _rootWin(nullptr), _focusWin(nullptr), _mask(nullptr),
		_claimSelect(0), _currentStr(nullptr) {
	_imageW = _screen->w;
	_imageH = _screen->h;
	_cellW = _cellH = 8;
	_confLockCols = false;
	_confLockRows = false;
	_wMarginx = 15;
	_wMarginy = 15;
	_wPaddingx = 0;
	_wPaddingy = 0;
	_wBorderx = 1;
	_wBordery = 1;
	_tMarginx = 7;
	_tMarginy = 7;
	_wMarginXsave = 15;
	_wMarginYsave = 15;
	_cols = 60;
	_rows = 25;
	_baseLine = 15;
	_leading = 20;
	_scrollWidth = 0;
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

		oldparent = splitwin->parent;
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

		splitwin->parent = pairwin;
		newwin->parent = pairwin;
		pairwin->parent = oldparent;

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

	win->next = _windowList;
	_windowList = win;
	if (win->next)
		win->next->prev = win;

	return win;
}

PairWindow *Windows::newPairWindow(glui32 method, Window *key, glui32 size) {
	PairWindow *pwin = new PairWindow(this, method, key, size);
	pwin->next = _windowList;
	_windowList = pwin;
	if (pwin->next)
		pwin->next->prev = pwin;

	return pwin;
}

void Windows::rearrange() {
	if (_rootWin) {
		Common::Rect box;

		if (_confLockCols) {
			int desired_width = _wMarginXsave * 2 + _cellW * _cols;
			if (desired_width > _imageW)
				_wMarginx = _wMarginXsave;
			else
				_wMarginx = (_imageW - _cellW * _cols) / 2;
		}

		if (_confLockRows) {
			int desired_height = _wMarginYsave * 2 + _cellH * _rows;
			if (desired_height > _imageH)
				_wMarginy = _wMarginYsave;
			else
				_wMarginy = (_imageH - _cellH * _rows) / 2;
		}

		box.left = _wMarginx;
		box.top = _wMarginy;
		box.right = _imageW - _wMarginx;
		box.bottom = _imageH - _wMarginy;

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

void Windows::setCurrent(Common::WriteStream *stream) {
	_currentStr = stream;
}

void Windows::repaint(const Common::Rect &box) {
	// TODO
}

/*--------------------------------------------------------------------------*/

Window::Window(Windows *windows, glui32 rock) : _magicnum(MAGIC_WINDOW_NUM),
		_windows(windows), _rock(rock), _type(0), parent(nullptr), next(nullptr), prev(nullptr),
		yadj(0), line_request(0), line_request_uni(0), char_request(0), char_request_uni(0),
		mouse_request(0), hyper_request(0), more_request(0), scroll_request(0), image_loaded(0),
		echo_line_input(true),  line_terminators(nullptr), termct(0), str(nullptr), echostr(nullptr) {
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
}

/*--------------------------------------------------------------------------*/

BlankWindow::BlankWindow(Windows *windows, uint32 rock) : Window(windows, rock) {
	_type = wintype_Blank;
}

/*--------------------------------------------------------------------------*/

TextGridWindow::TextGridWindow(Windows *windows, uint32 rock) : Window(windows, rock) {
	_type = wintype_TextGrid;
	width = height = 0;
	curx = cury = 0;
	inbuf = nullptr;
	inorgx = inorgy = 0;
	inmax = 0;
	incurs = inlen = 0;
	inarrayrock.num = 0;
	line_terminators = nullptr;

	Common::copy(&G_STYLES[0], &G_STYLES[style_NUMSTYLES], styles);
}

void TextGridWindow::rearrange(const Common::Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;

	newwid = box.width() / Windows::_cellW;
	newhgt = box.height() / Windows::_cellH;

	if (newwid == width && newhgt == height)
		return;

	lines.resize(newhgt);
	for (int y = 0; y < newhgt; ++y) {
		lines[y].resize(newwid);
		touch(y);
	}

	attr.clear();
	width = newwid;
	height = newhgt;
}

void TextGridWindow::touch(int line) {
	int y = bbox.top + line * Windows::_leading;
	lines[line].dirty = true;
	_windows->repaint(Common::Rect(bbox.left, y, bbox.right, y + Windows::_leading));
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
		historypos(0), historyfirst(0), historypresent(0), lastseen(0), scrollpos(0),
		scrollmax(0), scrollback(SCROLLBACK), width(-1), height(-1), inbuf(nullptr),
		line_terminators(nullptr), echo_line_input(true), ladjw(0), radjw(0), ladjn(0),
		radjn(0), numchars(0), chars(nullptr), attrs(nullptr),
		spaced(0), dashed(0), copybuf(0), copypos(0) {
	_type = wintype_TextBuffer;
	Common::fill(&history[0], &history[HISTORYLEN], nullptr);

	Common::copy(&T_STYLES[0], &T_STYLES[style_NUMSTYLES], styles);
}

void TextBufferWindow::rearrange(const Common::Rect &box) {
	Window::rearrange(box);
	int newwid, newhgt;
	int rnd;

	newwid = (box.width() - Windows::_tMarginx * 2 - Windows::_scrollWidth) / Windows::_cellW;
	newhgt = (box.height() - Windows::_tMarginy * 2) / Windows::_cellH;

	/* align text with bottom */
	rnd = newhgt * Windows::_cellH + Windows::_tMarginy * 2;
	yadj = (box.height() - rnd);
	bbox.top += (box.height() - rnd);

	if (newwid != width) {
		width = newwid;
		reflow();
	}

	if (newhgt != height) {
		/* scroll up if we obscure new lines */
		if (lastseen >= newhgt - 1)
			scrollpos += (height - newhgt);

		height = newhgt;

		/* keep window within 'valid' lines */
		if (scrollpos > scrollmax - height + 1)
			scrollpos = scrollmax - height + 1;
		if (scrollpos < 0)
			scrollpos = 0;
		touchScroll();

		/* allocate copy buffer */
		if (copybuf)
			delete[] copybuf;
		copybuf = new glui32[height * TBLINELEN];

		for (int i = 0; i < (height * TBLINELEN); i++)
			copybuf[i] = 0;

		copypos = 0;
	}
}

void TextBufferWindow::reflow() {
	int inputbyte = -1;
	Attributes curattr, oldattr;
	int i, k, p, s;
	int x;

	if (height < 4 || width < 20)
		return;

	lines[0].len = numchars;

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
	s = scrollmax < SCROLLBACK ? scrollmax : SCROLLBACK - 1;

	for (k = s; k >= 0; k--) {
		if (k == 0 && line_request)
			inputbyte = p + infence;

		if (lines[k].lpic) {
			offsetbuf[x] = p;
			alignbuf[x] = imagealign_MarginLeft;
			pictbuf[x] = lines[k].lpic;

			if (pictbuf[x]) pictbuf[x]->increment();
			hyperbuf[x] = lines[k].lhyper;
			x++;
		}

		if (lines[k].rpic) {
			offsetbuf[x] = p;
			alignbuf[x] = imagealign_MarginRight;
			pictbuf[x] = lines[k].rpic;
			if (pictbuf[x]) pictbuf[x]->increment();
			hyperbuf[x] = lines[k].rhyper;
			x++;
		}

		for (i = 0; i < lines[k].len; i++) {
			attrbuf[p] = curattr = lines[k].attr[i];
			charbuf[p] = lines[k].chars[i];
			p++;
		}

		if (lines[k].newline) {
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
	lastseen = 0;
	scrollpos = 0;

	if (inputbyte != -1) {
		infence = numchars;
		putTextUnit(charbuf + inputbyte, p - inputbyte, numchars, 0);
		incurs = numchars;
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

	for (int i = 0; i < scrollmax; i++)
		lines[i].dirty = true;
}

void TextBufferWindow::clear() {
	int i;

	attr.fgset = Windows::_overrideFgSet;
	attr.bgset = Windows::_overrideBgSet;
	attr.fgcolor = Windows::_overrideFgSet ? Windows::_overrideFgVal : 0;
	attr.bgcolor = Windows::_overrideBgSet ? Windows::_overrideBgVal : 0;
	attr.reverse = false;

	ladjw = radjw = 0;
	ladjn = radjn = 0;

	spaced = 0;
	dashed = 0;

	numchars = 0;

	for (i = 0; i < scrollback; i++) {
		lines[i].len = 0;

		if (lines[i].lpic) lines[i].lpic->decrement();
		lines[i].lpic = nullptr;
		if (lines[i].rpic) lines[i].rpic->decrement();
		lines[i].rpic = nullptr;

		lines[i].lhyper = 0;
		lines[i].rhyper = 0;
		lines[i].lm = 0;
		lines[i].rm = 0;
		lines[i].newline = 0;
		lines[i].dirty = true;
		lines[i].repaint = false;
	}

	lastseen = 0;
	scrollpos = 0;
	scrollmax = 0;

	for (i = 0; i < height; i++)
		touch(i);
}

bool TextBufferWindow::putPicture(Picture *pic, glui32 align, glui32 linkval) {
	if (align == imagealign_MarginRight)
	{
		if (lines[0].rpic || numchars)
			return false;

		radjw = (pic->w + Windows::_tMarginx) * GLI_SUBPIX;
		radjn = (pic->h + Windows::_cellH - 1) / Windows::_cellH;
		lines[0].rpic = pic;
		lines[0].rm = radjw;
		lines[0].rhyper = linkval;
	} else {
		if (align != imagealign_MarginLeft && numchars)
			putCharUni('\n');

		if (lines[0].lpic || numchars)
			return false;

		ladjw = (pic->w + Windows::_tMarginx) * GLI_SUBPIX;
		ladjn = (pic->h + Windows::_cellH - 1) / Windows::_cellH;
		lines[0].lpic = pic;
		lines[0].lm = ladjw;
		lines[0].lhyper = linkval;

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

	pw = (bbox.right - bbox.left - Windows::_tMarginx * 2 - gli_scroll_width) * GLI_SUBPIX;
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
	int y = bbox.top + Windows::_tMarginy + (height - line - 1) * Windows::_leading;
	lines[line].dirty = 1;
	_windows->clearSelection();
	_windows->repaint(Common::Rect(bbox.left, y - 2, bbox.right, y + Windows::_leading + 2));
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
		splitwid = Windows::_wPaddingx; // want border?
	else
		splitwid = Windows::_wPaddingy; // want border?

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
