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

#include "glk/windows.h"
#include "glk/window_graphics.h"
#include "glk/window_pair.h"
#include "glk/window_text_buffer.h"
#include "glk/window_text_grid.h"
#include "glk/conf.h"
#include "glk/glk.h"
#include "glk/screen.h"
#include "glk/streams.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Glk {

bool Windows::_overrideReverse;
bool Windows::_overrideFgSet;
bool Windows::_overrideBgSet;
bool Windows::_forceRedraw;
bool Windows::_claimSelect;
bool Windows::_moreFocus;
uint Windows::_overrideFgVal;
uint Windows::_overrideBgVal;
uint Windows::_zcolor_fg;
uint Windows::_zcolor_bg;
uint Windows::_zcolor_LightGrey;
uint Windows::_zcolor_Foreground;
uint Windows::_zcolor_Background;
uint Windows::_zcolor_Bright;

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

	_zcolor_LightGrey = g_system->getScreenFormat().RGBToColor(181, 181, 181);
	_zcolor_Foreground = _zcolor_Background = 0;
	_zcolor_Bright = 0;
}

Windows::~Windows() {
	delete _rootWin;
}

Window *Windows::windowOpen(Window *splitwin, uint method, uint size,
							uint wintype, uint rock) {
	Window *newwin, *oldparent = nullptr;
	PairWindow *pairWin;
	uint val;

	_forceRedraw = true;

	if (!_rootWin) {
		if (splitwin) {
			warning("window_open: ref must be nullptr");
			return nullptr;
		}

		// ignore method and size now
		oldparent = nullptr;
	} else {
		if (!splitwin) {
			warning("window_open: ref must not be nullptr");
			return nullptr;
		}

		val = (method & winmethod_DivisionMask);
		if (val != winmethod_Fixed && val != winmethod_Proportional) {
			warning("window_open: invalid method (not fixed or proportional)");
			return nullptr;
		}

		val = (method & winmethod_DirMask);
		if (val != winmethod_Above && val != winmethod_Below && val != winmethod_Left
				&& val != winmethod_Right && val != winmethod_Arbitrary) {
			warning("window_open: invalid method (bad direction)");
			return nullptr;
		}

		oldparent = splitwin->_parent;
		if (oldparent && oldparent->_type != wintype_Pair) {
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
	} else if (splitwin->_type == wintype_Pair) {
		pairWin = static_cast<PairWindow *>(splitwin);
		pairWin->_dir = winmethod_Arbitrary;
		pairWin->_children.push_back(newwin);
		newwin->_parent = pairWin;
	} else {
		// create pairWin, with newwin as the key
		pairWin = newPairWindow(method, newwin, size);
		pairWin->_children.push_back(splitwin);
		pairWin->_children.push_back(newwin);

		splitwin->_parent = pairWin;
		newwin->_parent = pairWin;
		pairWin->_parent = oldparent;

		if (oldparent) {
			PairWindow *parentWin = dynamic_cast<PairWindow *>(oldparent);
			assert(parentWin);

			for (uint idx = 0; idx < parentWin->_children.size(); ++idx) {
				if (parentWin->_children[idx] == splitwin)
					parentWin->_children[idx] = pairWin;
			}
		} else {
			_rootWin = pairWin;
		}
	}

	rearrange();

	return newwin;
}

void Windows::windowClose(Window *win, StreamResult *result) {
	_forceRedraw = true;

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

		if (pairWin) {
			int index = pairWin->_children.indexOf(win);
			if (index == -1) {
				warning("windowClose: window tree is corrupted");
				return;
			}

			// Detach window being closed from parent pair window
			pairWin->_children.remove_at(index);
			win->_parent = nullptr;

			if (!(pairWin->_dir & winmethod_Arbitrary)) {
				// Get the remaining child window
				assert(pairWin->_children.size() == 1);
				sibWin = pairWin->_children.front();

				// Detach it from the pair window
				index = pairWin->_children.indexOf(sibWin);
				assert(index >= 0);
				pairWin->_children.remove_at(index);

				// Set up window as either the singular root, or grandparent pair window if one exists
				grandparWin = dynamic_cast<PairWindow *>(pairWin->_parent);
				if (!grandparWin) {
					_rootWin = sibWin;
					sibWin->_parent = nullptr;
				} else {
					index = grandparWin->_children.indexOf(pairWin);
					grandparWin->_children[index] = sibWin;
					sibWin->_parent = grandparWin;
				}
			}
		}

		// Begin closation
		win->_stream->fillResult(result);

		// Close the child window (and descendants), so that key-deletion can
		// crawl up the tree to the root window.
		win->close(true);

		if (pairWin && !(pairWin->_dir & winmethod_Arbitrary))
			// Now we can delete the parent pair.
			pairWin->close(false);

		// Sort out the arrangements
		rearrange();
	}
}

Window *Windows::newWindow(uint type, uint rock) {
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

PairWindow *Windows::newPairWindow(uint method, Window *key, uint size) {
	PairWindow *pwin = new PairWindow(this, method, key, size);
	pwin->_next = _windowList;
	_windowList = pwin;
	if (pwin->_next)
		pwin->_next->_prev = pwin;

	return pwin;
}

void Windows::rearrange() {
	if (_rootWin) {
		Rect box;
		Point cell(g_conf->_monoInfo._cellW, g_conf->_monoInfo._cellH);

		if (g_conf->_lockCols) {
			int desired_width = g_conf->_wMarginSaveX * 2 + cell.x * g_conf->_cols;
			if (desired_width > g_conf->_imageW)
				g_conf->_wMarginX = g_conf->_wMarginSaveX;
			else
				g_conf->_wMarginX = (g_conf->_imageW - cell.x * g_conf->_cols) / 2;
		}

		if (g_conf->_lockRows) {
			int desired_height = g_conf->_wMarginSaveY * 2 + cell.y * g_conf->_rows;
			if (desired_height > g_conf->_imageH)
				g_conf->_wMarginY = g_conf->_wMarginSaveY;
			else
				g_conf->_wMarginY = (g_conf->_imageH - cell.y * g_conf->_rows) / 2;
		}

		box.left = g_conf->_wMarginX;
		box.top = g_conf->_wMarginY;
		box.right = g_conf->_imageW - g_conf->_wMarginX;
		box.bottom = g_conf->_imageH - g_conf->_wMarginY;

		_rootWin->rearrange(box);
	}
}

void Windows::inputGuessFocus() {
	Window *altWin = _focusWin;

	do {
		if (altWin
				&& (altWin->_lineRequest || altWin->_charRequest ||
					altWin->_lineRequestUni || altWin->_charRequestUni))
			break;
		altWin = iterateTreeOrder(altWin);
	} while (altWin != _focusWin);

	if (_focusWin != altWin) {
		_focusWin = altWin;
		_forceRedraw = true;
		redraw();
	}
}

void Windows::inputMoreFocus() {
	Window *altWin = _focusWin;

	do {
		if (altWin && altWin->_moreRequest)
			break;
		altWin = iterateTreeOrder(altWin);
	} while (altWin != _focusWin);

	_focusWin = altWin;
}

void Windows::inputNextFocus() {
	Window *altWin = _focusWin;

	do {
		altWin = iterateTreeOrder(altWin);
		if (altWin
				&& (altWin->_lineRequest || altWin->_charRequest ||
					altWin->_lineRequestUni || altWin->_charRequestUni))
			break;
	} while (altWin != _focusWin);

	if (_focusWin  != altWin) {
		_focusWin = altWin;
		_forceRedraw = true;
		redraw();
	}
}

void Windows::inputScrollFocus() {
	Window *altWin = _focusWin;

	do {
		if (altWin && altWin->_scrollRequest)
			break;
		altWin = iterateTreeOrder(altWin);
	} while (altWin != _focusWin);

	_focusWin = altWin;
}

void Windows::inputHandleKey(uint key) {
	if (_moreFocus) {
		inputMoreFocus();
	} else if (_focusWin && (_focusWin->_lineRequest || _focusWin->_lineRequestUni) &&
			_focusWin->checkTerminators(key)) {
		// WORKAROUND: Do line terminators checking first. This was first needed for Beyond Zork,
		// since it needs the Page Up/Down keys to scroll the description area rathern than the buffer area
	} else {
		switch (key) {
		case keycode_Tab:
			inputNextFocus();
			return;
		case keycode_PageUp:
		case keycode_PageDown:
		case keycode_MouseWheelUp:
		case keycode_MouseWheelDown:
			inputScrollFocus();
			break;
		default:
			inputGuessFocus();
			break;
		}
	}

	Window *win = _focusWin;
	if (!win)
		return;

	bool deferExit = false;

	TextGridWindow *gridWindow = dynamic_cast<TextGridWindow *>(win);
	TextBufferWindow *bufWindow = dynamic_cast<TextBufferWindow *>(win);

	if (gridWindow) {
		if (gridWindow->_charRequest || gridWindow->_charRequestUni)
			gridWindow->acceptReadChar(key);
		else if (gridWindow->_lineRequest || gridWindow->_lineRequestUni)
			gridWindow->acceptReadLine(key);
	} else if (bufWindow) {
		if (bufWindow->_charRequest || bufWindow->_charRequestUni)
			bufWindow->acceptReadChar(key);
		else if (bufWindow->_lineRequest || bufWindow->_lineRequestUni)
			bufWindow->acceptReadLine(key);
		else if (bufWindow->_moreRequest || bufWindow->_scrollRequest)
			deferExit = bufWindow->acceptScroll(key);
	}

	if (!deferExit && g_vm->_terminated)
		g_vm->quitGame();
}

void Windows::inputHandleClick(const Point &pos) {
	if (_rootWin)
		_rootWin->click(pos);
}

void Windows::selectionChanged() {
	_claimSelect = false;
	_forceRedraw = true;
	redraw();
}

void Windows::redraw() {
	_claimSelect = false;

	if (_forceRedraw) {
		repaint(Rect(0, 0, g_conf->_imageW, g_conf->_imageH));
		g_vm->_screen->fill(g_conf->_windowColor);
	}

	if (_rootWin)
		_rootWin->redraw();

	if (_moreFocus)
		refocus(_focusWin);

	_forceRedraw = false;
}

void Windows::redrawRect(const Rect &r) {
	_drawSelect = true;
	repaint(r);
}

void Windows::repaint(const Rect &box) {
	g_vm->_events->redraw();
}

uint Windows::rgbShift(uint color) {
	uint8 r, g, b;
	Graphics::PixelFormat pf = g_system->getScreenFormat();
	pf.colorToRGB(color, r, g, b);

	r = ((uint)r + 0x30) < 0xff ? ((uint)r + 0x30) : 0xff;
	g = ((uint)g + 0x30) < 0xff ? ((uint)g + 0x30) : 0xff;
	b = ((uint)b + 0x30) < 0xff ? ((uint)b + 0x30) : 0xff;

	_zcolor_Bright = pf.RGBToColor(r, g, b);
	return _zcolor_Bright;
}

/*--------------------------------------------------------------------------*/

Windows::iterator &Windows::iterator::operator++() {
	_current = _current->_next;
	return *this;
}

Windows::iterator &Windows::iterator::operator--() {
	_current = _current->_prev;
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
		return pairWin->_backward ? pairWin->_children.back() : pairWin->_children.front();
	} else {
		while (win->_parent) {
			pairWin = dynamic_cast<PairWindow *>(win->_parent);
			assert(pairWin);
			int index = pairWin->_children.indexOf(win);
			assert(index != -1);

			if (!pairWin->_backward) {
				if (index < ((int)pairWin->_children.size() - 1))
					return pairWin->_children[index + 1];
			} else {
				if (index > 0)
					return pairWin->_children[index - 1];
			}

			win = pairWin;
		}

		return nullptr;
	}
}

/*--------------------------------------------------------------------------*/

Window::Window(Windows *windows, uint rock) : _windows(windows), _rock(rock),
	_type(0), _parent(nullptr), _next(nullptr), _prev(nullptr), _yAdj(0),
	_lineRequest(false), _lineRequestUni(false), _charRequest(false),
	_charRequestUni(false), _mouseRequest(false), _hyperRequest(false),
	_moreRequest(false), _scrollRequest(false), _imageLoaded(false),
	_echoLineInputBase(true), _lineTerminatorsBase(nullptr), _termCt(0), _echoStream(nullptr) {
	_attr.fgset = false;
	_attr.bgset = false;
	_attr.reverse = false;
	_attr.style = 0;
	_attr.fgcolor = 0;
	_attr.bgcolor = 0;
	_attr.hyper = 0;

	_bgColor = g_conf->_windowColor;
	_fgColor = g_conf->_propInfo._moreColor;

	Streams &streams = *g_vm->_streams;
	_stream = streams.openWindowStream(this);

	if (g_vm->gli_register_obj)
		_dispRock = (*g_vm->gli_register_obj)(this, gidisp_Class_Window);
}

Window::~Window() {
	if (g_vm->gli_unregister_obj)
		(*g_vm->gli_unregister_obj)(this, gidisp_Class_Window, _dispRock);

	// Remove the window from the parent's children list
	PairWindow *parent = dynamic_cast<PairWindow *>(_parent);
	if (parent) {
		int index = parent->_children.indexOf(this);
		if (index != -1)
			parent->_children.remove_at(index);
	}

	delete[] _lineTerminatorsBase;

	// Remove the window from the master list of windows
	Window *prev = _prev;
	Window *next = _next;

	if (prev)
		prev->_next = next;
	else
		_windows->_windowList = next;
	if (next)
		next->_prev = prev;

	// Delete any attached window stream
	_echoStream = nullptr;
	delete _stream;
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
		for (uint idx = 0; idx < pairWin->_children.size(); ++idx)
			pairWin->_children[idx]->close();
	}

	// Finally, delete the window
	delete this;
}

FontInfo *Window::getFontInfo() {
	error("Tried to get font info for a non-text window");
}

void Window::cancelLineEvent(Event *ev) {
	Event dummyEv;
	if (!ev)
		ev = &dummyEv;

	ev->clear();
}

void Window::moveCursor(const Point &newPos) {
	warning("moveCursor: not a TextGrid window");
}

void Window::requestLineEvent(char *buf, uint maxlen, uint initlen) {
	warning("requestLineEvent: window does not support keyboard input");
}

void Window::requestLineEventUni(uint32 *buf, uint maxlen, uint initlen) {
	warning("requestLineEventUni: window does not support keyboard input");
}

void Window::redraw() {
	if (Windows::_forceRedraw) {
		uint color = Windows::_overrideBgSet ? g_conf->_windowColor : _bgColor;
		int y0 = _yAdj ? _bbox.top - _yAdj : _bbox.top;
		g_vm->_screen->fillRect(Rect(_bbox.left, y0, _bbox.right, _bbox.bottom), color);
	}
}

void Window::acceptReadLine(uint32 arg) {
	warning("acceptReadLine:: window does not support keyboard input");
}

void Window::acceptReadChar(uint arg) {
	warning("acceptReadChar:: window does not support keyboard input");
}

void Window::getArrangement(uint *method, uint *size, Window **keyWin) {
	warning("getArrangement: not a Pair window");
}

void Window::setArrangement(uint method, uint size, Window *keyWin) {
	warning("setArrangement: not a Pair window");
}

void Window::requestCharEvent() {
	warning("requestCharEvent: window does not support keyboard input");
}

void Window::requestCharEventUni() {
	warning("requestCharEventUni: window does not support keyboard input");
}

void Window::flowBreak() {
	warning("flowBreak: not a text buffer window");
}

void Window::eraseRect(bool whole, const Rect &box) {
	warning("eraseRect: not a graphics window");
}

void Window::fillRect(uint color, const Rect &box) {
	warning("fillRect: not a graphics window");
}

void Window::setBackgroundColor(uint color) {
	warning("setBackgroundColor: not a graphics window");
}

const WindowStyle *Window::getStyles() const {
	warning("getStyles: not a text window");
	return nullptr;
}

void Window::setTerminatorsLineEvent(const uint32 *keycodes, uint count) {
	if (dynamic_cast<TextBufferWindow *>(this) || dynamic_cast<TextGridWindow *>(this)) {
		delete[] _lineTerminatorsBase;
		_lineTerminatorsBase = nullptr;

		if (!keycodes || count == 0) {
			_termCt = 0;
		} else {
			_lineTerminatorsBase = new uint[count + 1];
			if (_lineTerminatorsBase) {
				memcpy(_lineTerminatorsBase, keycodes, count * sizeof(uint));
				_lineTerminatorsBase[count] = 0;
				_termCt = count;
			}
		}
	} else {
		warning("setTerminatorsLineEvent: window does not support keyboard input");
	}
}

bool Window::checkBasicTerminators(uint32 ch) {
	if (ch == keycode_Escape)
		return true;
	else if (ch >= keycode_Func12 && ch <= keycode_Func1)
		return true;
	else
		return false;
}

bool Window::checkTerminators(uint32 ch) {
	if (checkBasicTerminators(ch))
		return true;

	for (uint idx = 0; idx < _termCt; ++idx) {
		if (_lineTerminatorsBase[idx] == ch)
			return true;
	}

	return false;
}

bool Window::imageDraw(uint image, uint align, int val1, int val2) {
	return imageDraw(Common::String::format("%u", image),
		align, val1, val2);
}

bool Window::imageDraw(const Common::String & image, uint align, int val1, int val2) {
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

void Window::getSize(uint *width, uint *height) const {
	if (width)
		*width = 0;
	if (height)
		*height = 0;
}

void Window::bringToFront() {
	PairWindow *pairWin = dynamic_cast<PairWindow *>(_parent);

	if (pairWin && pairWin->_dir == winmethod_Arbitrary && pairWin->_children.back() != this) {
		pairWin->_children.remove(this);
		pairWin->_children.push_back(this);
		Windows::_forceRedraw = true;
	}
}

void Window::sendToBack() {
	PairWindow *pairWin = dynamic_cast<PairWindow *>(_parent);

	if (pairWin && pairWin->_dir == winmethod_Arbitrary && pairWin->_children.front() != this) {
		pairWin->_children.remove(this);
		pairWin->_children.insert_at(0, this);
		Windows::_forceRedraw = true;
	}
}

/*--------------------------------------------------------------------------*/

BlankWindow::BlankWindow(Windows *windows, uint rock) : Window(windows, rock) {
	_type = wintype_Blank;
}

/*--------------------------------------------------------------------------*/

WindowStyle::WindowStyle(const WindowStyleStatic &src) : font(src.font), reverse(src.reverse) {
	Graphics::PixelFormat pf = g_system->getScreenFormat();
	fg = pf.RGBToColor(src.fg[0], src.fg[1], src.fg[2]);
	bg = pf.RGBToColor(src.bg[0], src.bg[1], src.bg[1]);
}

/*--------------------------------------------------------------------------*/

void Attributes::clear() {
	fgset = false;
	bgset = false;
	fgcolor = 0;
	bgcolor = 0;
	reverse = false;
	hyper = 0;
	style = 0;
}

uint Attributes::attrBg(const WindowStyle *styles) {
	int revset = reverse || (styles[style].reverse && !Windows::_overrideReverse);

	bool zfset = fgset ? fgset : Windows::_overrideFgSet;
	bool zbset = bgset ? bgset : Windows::_overrideBgSet;

	uint zfore = fgset ? fgcolor : Windows::_overrideFgVal;
	uint zback = bgset ? bgcolor : Windows::_overrideBgVal;

	if (zfset && zfore != Windows::_zcolor_fg) {
		Windows::_zcolor_Foreground = zfore;
		Windows::_zcolor_fg = zfore;
	}

	if (zbset && zback != Windows::_zcolor_bg) {
		Windows::_zcolor_Background = zback;
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
		else if (zbset && styles[style].fg == Windows::_zcolor_Background)
			return Windows::_zcolor_LightGrey;
		else
			return styles[style].fg;
	}
}

uint Attributes::attrFg(const WindowStyle *styles) {
	int revset = reverse || (styles[style].reverse && !Windows::_overrideReverse);

	bool zfset = fgset ? fgset : Windows::_overrideFgSet;
	bool zbset = bgset ? bgset : Windows::_overrideBgSet;

	uint zfore = fgset ? fgcolor : Windows::_overrideFgVal;
	uint zback = bgset ? bgcolor : Windows::_overrideBgVal;

	if (zfset && zfore != Windows::_zcolor_fg) {
		Windows::_zcolor_Foreground = zfore;
		Windows::_zcolor_fg = zfore;
	}

	if (zbset && zback != Windows::_zcolor_bg) {
		Windows::_zcolor_Background = zback;
		Windows::_zcolor_bg = zback;
	}

	if (!revset) {
		if (zfset)
			if (zfore == zback)
				return Windows::rgbShift(Windows::_zcolor_Foreground);
			else
				return Windows::_zcolor_Foreground;
		else if (zbset && styles[style].fg == Windows::_zcolor_Background)
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

} // End of namespace Glk
