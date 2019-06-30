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

#include "glk/frotz/windows.h"
#include "glk/frotz/frotz.h"
#include "glk/window_pair.h"
#include "glk/window_text_grid.h"
#include "glk/window_text_buffer.h"
#include "glk/conf.h"

namespace Glk {
namespace Frotz {

Windows::Windows() : _lower(_windows[0]), _upper(_windows[1]), _background(nullptr), _cwin(0) {
}

size_t Windows::size() const {
	return (g_vm->h_version < 6) ? 2 : 8;
}

Window &Windows::operator[](uint idx) {
	assert(idx < size());
	return _windows[idx];
}

void Windows::setup(bool isVersion6) {
	if (isVersion6) {
		// For graphic games we have a background window covering the entire screen for greater
		// flexibility of wher we draw pictures, and the lower and upper areas sit on top of them
		_background = g_vm->glk_window_open(0, 0, 0, wintype_Graphics, 0);
		_background->setBackgroundColor(0xffffff);

	} else {
		_lower = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
		_upper = g_vm->glk_window_open(_lower, winmethod_Above | winmethod_Fixed, 0, wintype_TextGrid, 0);

		_lower.update();
		_upper.update();
		g_vm->glk_set_window(_lower);
	}

	MonoFontInfo &mi = g_vm->_conf->_monoInfo;
	for (size_t idx = 0; idx < 8; ++idx) {
		Window &w = _windows[idx];
		w._windows = this;
		w._index = idx;
		w[FONT_NUMBER] = TEXT_FONT;
		w[FONT_SIZE] = (mi._cellH << 8) | mi._cellW;


		PropFontInfo &pi = g_conf->_propInfo;
		w._quotes = pi._quotes;
		w._dashes = pi._quotes;
		w._spaces = pi._spaces;
	}
}

void Windows::setWindow(int win) {
	_cwin = win;

	if (_windows[_cwin]._win)
		g_vm->glk_set_window(_windows[_cwin]._win);
}

/*--------------------------------------------------------------------------*/

Window::Window() : _windows(nullptr), _win(nullptr), _quotes(0), _dashes(0), _spaces(0),
		_currFont(TEXT_FONT), _prevFont(TEXT_FONT), _tempFont(TEXT_FONT), _currStyle(0), _oldStyle(0) {
	Common::fill(_properties, _properties + TRUE_BG_COLOR + 1, 0);
	_properties[Y_POS] = _properties[X_POS] = 1;
	_properties[Y_CURSOR] = _properties[X_CURSOR] = 1;
	_properties[FONT_NUMBER] = TEXT_FONT;
	_properties[FONT_SIZE] = (8 << 8) | 8;
}

void Window::update() {
	assert(_win);

	_properties[X_POS] = _win->_bbox.left / g_conf->_monoInfo._cellW + 1;
	_properties[Y_POS] = _win->_bbox.top / g_conf->_monoInfo._cellH + 1;
	_properties[X_SIZE] = _win->_bbox.width() / g_conf->_monoInfo._cellW;
	_properties[Y_SIZE] = _win->_bbox.height() / g_conf->_monoInfo._cellH;

	Point pt = _win->getCursor();
	_properties[X_CURSOR] = (g_vm->h_version != V6) ? pt.x + 1 : pt.x / g_conf->_monoInfo._cellW + 1;
	_properties[Y_CURSOR] = (g_vm->h_version != V6) ? pt.y + 1 : pt.y / g_conf->_monoInfo._cellH + 1;

	TextBufferWindow *win = dynamic_cast<TextBufferWindow *>(_win);
	_properties[LEFT_MARGIN] = (win ? win->_ladjw : 0) / g_conf->_monoInfo._cellW;
	_properties[RIGHT_MARGIN] = (win ? win->_radjw : 0) / g_conf->_monoInfo._cellW;
	_properties[FONT_SIZE] = (g_conf->_monoInfo._cellH << 8) | g_conf->_monoInfo._cellW;
}

Window &Window::operator=(winid_t win) {
	_win = win;

	// Set the screen colors
	if (win)
		win->_stream->setZColors(g_vm->_defaultForeground, g_vm->_defaultBackground);

	return *this;
}


void Window::setSize(const Point &newSize) {
	checkRepositionLower();

	_properties[X_SIZE] = newSize.x;
	_properties[Y_SIZE] = newSize.y;

	if (_win)
		_win->setSize(Point(newSize.x * g_conf->_monoInfo._cellW, newSize.y * g_conf->_monoInfo._cellH));
}

void Window::setPosition(const Point &newPos) {
	checkRepositionLower();

	_properties[X_POS] = newPos.x;
	_properties[Y_POS] = newPos.y;

	if (_win)
		_win->setPosition(Point((newPos.x - 1) * g_conf->_monoInfo._cellW, (newPos.y - 1) * g_conf->_monoInfo._cellH));
}

void Window::setCursor(const Point &newPos) {
	int x = newPos.x, y = newPos.y;

	if (!(_currStyle & FIXED_WIDTH_STYLE)) {
		_currStyle |= FIXED_WIDTH_STYLE;
		ensureGlkWindow();
	}

	if (y < 0) {
		// Cursor on/off
		if (y == -2)
			g_vm->_events->showMouseCursor(true);
		else if (y == -1)
			g_vm->_events->showMouseCursor(false);
		return;
	}

	if (!x || !y) {
		update();

		if (!x)
			x = _properties[X_CURSOR];
		if (!y)
			y = _properties[Y_CURSOR];
	}

	g_vm->glk_window_move_cursor(_win, x - 1, y - 1);
}

void Window::clear() {
	if (_win)
		g_vm->glk_window_clear(_win);
}

void Window::updateColors() {
	if (_win)
		_win->_stream->setZColors(_properties[TRUE_FG_COLOR], _properties[TRUE_BG_COLOR]);
}

void Window::updateColors(uint fore, uint back) {
	_properties[TRUE_FG_COLOR] = fore;
	_properties[TRUE_BG_COLOR] = back;
	updateColors();
}

uint Window::setFont(uint font) {
	int result = 0;

	switch (font) {
	case PREVIOUS_FONT:
		// previous font
		_tempFont = _currFont;
		_currFont = _prevFont;
		_prevFont = _tempFont;
		setStyle();
		result = _currFont;
		break;

	case TEXT_FONT:
	case GRAPHICS_FONT:
	case FIXED_WIDTH_FONT:
		_prevFont = _currFont;
		_currFont = font;
		setStyle();
		result = _prevFont;
		break;

	case PICTURE_FONT: // picture font, undefined per 1.1
	default:           // unavailable
		result = 0;
		break;
	}

	PropFontInfo &pi = g_conf->_propInfo;
	if (_currFont == GRAPHICS_FONT) {
		_quotes = pi._quotes;
		_dashes = pi._dashes;
		_spaces = pi._spaces;
		pi._quotes = 0;
		pi._dashes = 0;
		pi._spaces = 0;
	} else {
		pi._quotes = _quotes;
		pi._dashes = _dashes;
		pi._spaces = _spaces;
	}

	_properties[FONT_NUMBER] = font;
	return result;
}

void Window::setStyle(uint style) {
	if (style == 0)
		_currStyle = 0;
	else if (style != 0xf000)
		// not tickle time
		_currStyle |= style;

	if (g_vm->h_flags & FIXED_FONT_FLAG || _currFont == FIXED_WIDTH_FONT || _currFont == GRAPHICS_FONT)
		style = _currStyle | FIXED_WIDTH_STYLE;
	else
		style = _currStyle;

	if (g_vm->gos_linepending && _windows->currWin() == g_vm->gos_linewin)
		return;

	updateStyle();
}

void Window::updateStyle() {
	uint style = _currStyle;

	ensureGlkWindow();

	if (style & REVERSE_STYLE)
		setReverseVideo(true);

	if (style & FIXED_WIDTH_STYLE) {
		if (_currFont == GRAPHICS_FONT)
			_win->_stream->setStyle(style_User1);			// character graphics
		else if (style & BOLDFACE_STYLE && style & EMPHASIS_STYLE)
			_win->_stream->setStyle(style_BlockQuote);	// monoz
		else if (style & EMPHASIS_STYLE)
			_win->_stream->setStyle(style_Alert);			// monoi
		else if (style & BOLDFACE_STYLE)
			_win->_stream->setStyle(style_Subheader);		// monob
		else
			_win->_stream->setStyle(style_Preformatted);	// monor

		MonoFontInfo &fi = g_vm->_conf->_monoInfo;
		_properties[FONT_SIZE] = (fi._cellH << 8) | fi._cellW;
	} else {
		if (style & BOLDFACE_STYLE && style & EMPHASIS_STYLE)
			_win->_stream->setStyle(style_Note);			// propz
		else if (style & EMPHASIS_STYLE)
			_win->_stream->setStyle(style_Emphasized);	// propi
		else if (style & BOLDFACE_STYLE)
			_win->_stream->setStyle(style_Header);		// propb
		else
			_win->_stream->setStyle(style_Normal);		// propr

		PropFontInfo &fi = g_vm->_conf->_propInfo;
		_properties[FONT_SIZE] = (fi._cellH << 8) | fi._cellW;
	}

	if (_currStyle == 0)
		setReverseVideo(false);
}

void Window::setReverseVideo(bool reverse) {
	_win->_stream->setReverseVideo(reverse);
}

void Window::ensureGlkWindow() {
	// Create a new window	
	if (_win && (dynamic_cast<TextGridWindow *>(_win) != nullptr) != ((_currStyle & FIXED_WIDTH_STYLE) != 0)) {
		g_vm->glk_window_close(_win);
		_win = nullptr;
	}

	if (_currStyle & FIXED_WIDTH_STYLE) {
		// Text grid window
		_win = g_vm->glk_window_open(g_vm->glk_window_get_root(),
			winmethod_Arbitrary | winmethod_Fixed, 0, wintype_TextGrid, 0);
	} else {
		// text buffer window
		_win = g_vm->glk_window_open(g_vm->glk_window_get_root(),
			winmethod_Arbitrary | winmethod_Fixed, 0, wintype_TextBuffer, 0);
	}

	setSize(Point(_properties[X_SIZE], _properties[Y_SIZE]));
	setPosition(Point(_properties[X_POS], _properties[Y_POS]));

	g_vm->glk_set_window(_win);
}

const uint &Window::getProperty(WindowProperty propType) {
	if (_win)
		update();

	return _properties[propType];
}

void Window::setProperty(WindowProperty propType, uint value) {
	switch (propType) {
	case TRUE_FG_COLOR:
	case TRUE_BG_COLOR:
		_properties[propType] = value;
		updateColors();
		break;

	default:
		_properties[propType] = value;
	}
}

void Window::checkRepositionLower() {
	if (&_windows->_lower == this) {
		PairWindow *parent = dynamic_cast<PairWindow *>(_win->_parent);
		if (!parent)
			error("Parent was not a pair window");

		// Ensure the parent pair window is flagged as having children at arbitrary positions,
		// just in case it isn't already
		parent->_dir = winmethod_Arbitrary;
	}
}

} // End of namespace Frotz
} // End of namespace Glk
