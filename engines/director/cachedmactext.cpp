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

#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macwindowmanager.h"

#include "director/cachedmactext.h"
#include "director/cast.h"

namespace Director {

void CachedMacText::makeMacText() {
	assert(_width != -1);
	assert(_wm != NULL);

	if ((int)_textCast->textAlign == -1)
		_align = (Graphics::TextAlign)3;
	else
		_align = (Graphics::TextAlign)((int)_textCast->textAlign + 1);

	_macText = new Graphics::MacText(_textCast->_ftext,
	                                 _wm,
	                                 _macFont,
	                                 0x00,
	                                 0xff,
	                                 _width,
	                                 _align,
	                                 1);
	// TODO destroy me
}

CachedMacText::CachedMacText(TextCast *const textCast,
                             int version,
                             int defaultWidth,
                             Graphics::MacWindowManager *const wm
                            )
	:
	_surface(NULL), _macFont(NULL),
	_macText(NULL), _width(defaultWidth), _dirty(true), _textCast(textCast),
	_wm(wm) {
	_macFont = new Graphics::MacFont(_textCast->fontId,
	                                 _textCast->fontSize,
	                                 _textCast->textSlant);
	// TODO destroy me

	if (_width == -1)  {
		if (version >= 4) {
			// This came from frame.cpp
			_width = _textCast->initialRect.right;
		} else {
			_width = _textCast->initialRect.width();
		}
	}

	if (_wm != NULL)
		makeMacText();
}

void CachedMacText::setWm(Graphics::MacWindowManager *wm) {
	if (wm != _wm) {
		_dirty = true;
		_wm = wm;
		makeMacText();
	}
}

void CachedMacText::clip(int width) {
	if (width != _width) {
		_dirty = true;
		_width = width;
		if (_wm != NULL)
			makeMacText();
	}
}

void CachedMacText::forceDirty() {
	// STUB
	assert(false);
}

const Graphics::ManagedSurface *CachedMacText::getSurface() {
	assert(_wm != NULL);
	if (_dirty) {
		_macText->render();
		_surface = _macText->getSurface();
		_dirty = false;
	}
	return _surface;
}

int CachedMacText::getLineCount() {
	assert(_macText != NULL);
	return _macText->getLineCount();
}

} // End of namespace Director
