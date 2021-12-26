/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "graphics/surface.h"
#include "graphics/font.h"

#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/livetext.h"
#include "buried/resources.h"

namespace Buried {

LiveTextWindow::LiveTextWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	// Initialize member variables to empty
	_textTranslation = false;

	// Create font
	_fontHeight = (_vm->getLanguage() == Common::JA_JPN) ? 12 : 14;
	_font = _vm->_gfx->createFont(_fontHeight);

	// Create window
	_rect = Common::Rect(137, 21, 447, 87);

	// Update the text in the window
	updateLiveText(_vm->getString(IDS_SAVE_GAME_MESSAGE), false);
}

LiveTextWindow::~LiveTextWindow() {
	delete _font;
}

bool LiveTextWindow::updateLiveText(const Common::String &text, bool notifyUser) {
	// Set translated text flag
	_textTranslation = false;

	if (text.empty()) {
		_text.clear();

		invalidateWindow(false);

		((GameUIWindow *)_parent)->setWarningState(false);
		return true;
	}

	_text = text;

	// Redraw the window
	invalidateWindow(false);

	if (notifyUser)
		((GameUIWindow *)_parent)->flashWarningLight();

	return true;
}

bool LiveTextWindow::updateTranslationText(const Common::String &text, bool notifyUser) {
	if (text.empty()) {
		_text.clear();

		invalidateWindow(false);

		((GameUIWindow *)_parent)->setWarningState(false);
		return true;
	}

	_text = text;

	// Set translated text flag
	_textTranslation = true;

	// Redraw the window
	invalidateWindow(false);

	((GameUIWindow *)_parent)->setWarningState(false);

	return true;
}

void LiveTextWindow::translateBiochipClosing() {
	// If the current text is translated text, then kill it now
	if (_textTranslation)
		updateLiveText();
}

void LiveTextWindow::onPaint() {
	// Draw the background bitmap
	Graphics::Surface *surface = _vm->_gfx->getBitmap(IDB_LIVE_TEXT_BACKGROUND);

	// Draw the text on top of that
	if (!_text.empty())
		_vm->_gfx->renderText(surface, _font, _text, 30, 4, 270, 50, _vm->_gfx->getColor(212, 109, 0), _fontHeight);

	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(surface, absoluteRect.left, absoluteRect.top);

	surface->free();
	delete surface;
}

void LiveTextWindow::onEnable(bool enable) {
	if (enable)
		_vm->removeMouseMessages(this);
}

} // End of namespace Buried
