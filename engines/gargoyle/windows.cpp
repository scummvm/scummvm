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

Windows::Windows(Graphics::Screen *screen) : _screen(screen), _forceRedraw(true), _moreFocus(false),
		_windowList(nullptr), _rootWin(nullptr), _focusWin(nullptr) {
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
	_imageW = _imageH = 0;
	_cellW = _cellH = 0;
	_baseLine = 15;
	_leading = 20;
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
		pairwin = new PairWindow(method, newwin, size);
		pairwin->child1 = splitwin;
		pairwin->child2 = newwin;

		splitwin->parent = pairwin;
		newwin->parent = pairwin;
		pairwin->parent = oldparent;

		if (oldparent) {
			PairWindow *parentWin = dynamic_cast<PairWindow *>(oldparent);
			assert(parentWin);
			if (parentWin->child1 == splitwin)
				parentWin->child1 = pairwin;
			else
				parentWin->child2 = pairwin;
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
		win = new BlankWindow(rock);
		break;
	case wintype_TextGrid:
		win = new TextGridWindow(rock);
		break;
	case wintype_TextBuffer:
		win = new TextBufferWindow(rock);
		break;
	case wintype_Graphics:
		win = new GraphicsWindow(rock);
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
	PairWindow *pwin = new PairWindow(method, key, size);
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

/*--------------------------------------------------------------------------*/

Window::Window(glui32 rock) : _magicnum(MAGIC_WINDOW_NUM), _rock(rock), _type(0),
		parent(nullptr), next(nullptr), prev(nullptr),
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

BlankWindow::BlankWindow(uint32 rock) : Window(rock) {
	_type = wintype_Blank;
}

/*--------------------------------------------------------------------------*/

TextGridWindow::TextGridWindow(uint32 rock) : Window(rock) {
	_type = wintype_TextGrid;
	width = height = 0;
	curx = cury = 0;
	inbuf = nullptr;
	inorgx = inorgy = 0;
	inmax = 0;
	incurs = inlen = 0;
	inarrayrock.num = 0;
	line_terminators = nullptr;
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
//	int y = bbox.top + line * Windows::_leading;
	lines[line].dirty = true;
	// TODO
//	winrepaint(bbox.left, y, bbox.right, y + Windows::_leading);
}

void TextGridWindow::TextGridRow::resize(size_t newSize) {
	chars.clear();
	attr.clear();
	chars.resize(newSize);
	attr.resize(newSize);
	Common::fill(&chars[0], &chars[0] + newSize, ' ');
}

/*--------------------------------------------------------------------------*/

TextBufferWindow::TextBufferWindow(uint32 rock) : Window(rock) {
	_type = wintype_TextBuffer;
}

/*--------------------------------------------------------------------------*/

GraphicsWindow::GraphicsWindow(uint32 rock) : Window(rock) {
	_type = wintype_Graphics;
}

/*--------------------------------------------------------------------------*/

PairWindow::PairWindow(glui32 method, Window *_key, glui32 _size) : Window(0),
		dir(method & winmethod_DirMask),
		division(method & winmethod_DivisionMask),
		wborder((method & winmethod_BorderMask) == winmethod_Border),
		vertical(dir == winmethod_Left || dir == winmethod_Right),
		backward(dir == winmethod_Left || dir == winmethod_Above),
		key(key), size(size), keydamage(0), child1(nullptr), child2(nullptr) {
	_type = wintype_Pair;
}

/*--------------------------------------------------------------------------*/

WindowStyle::WindowStyle() : font(0), reverse(0) {
	Common::fill(&bg[0], &bg[3], 0);
	Common::fill(&fg[0], &fg[3], 0);
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
