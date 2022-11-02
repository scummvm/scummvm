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

#include "buried/avi_frames.h"
#include "buried/buried.h"
#include "buried/credits.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/resources.h"
#include "buried/sound.h"

#include "graphics/surface.h"

namespace Buried {

CreditsWindow::CreditsWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_curButton = 1;
	_curPage = 0;

	_buttons[0] = Common::Rect(18, 92, 294, 152);
	_buttons[1] = Common::Rect(18, 164, 294, 224);
	_buttons[2] = Common::Rect(18, 236, 294, 296);
	_buttons[3] = Common::Rect(18, 308, 294, 368);
	_buttons[4] = Common::Rect(18, 380, 294, 440);
	_returnButton = Common::Rect(572, 459, 639, 479);

	Common::Rect parentRect = _parent->getClientRect();
	_rect.left = (parentRect.right - 640) / 2;
	_rect.top = (parentRect.bottom - 480) / 2;
	_rect.right = parentRect.left + 640;
	_rect.bottom = parentRect.top + 480;

	_normalImage = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_CREDITS_MAIN_FILENAME));
	_highlightedImage = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_CREDITS_HIGHLIGHT_FILENAME));

	_stillFrames = new AVIFrames(_vm->getFilePath(IDS_CREDITS_MOVIE_FILENAME));
}

CreditsWindow::~CreditsWindow() {
	delete _stillFrames;

	_normalImage->free();
	delete _normalImage;

	_highlightedImage->free();
	delete _highlightedImage;
}

bool CreditsWindow::showCredits() {
	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_parent);

	showWindow(kWindowShow);
	invalidateWindow(false);

	return true;
}

void CreditsWindow::onPaint() {
	_vm->_gfx->blit(_normalImage, 0, 0);
	_vm->_gfx->blit(_highlightedImage, Common::Rect(0, (_curButton - 1) * 60, 276, _curButton * 60), Common::Rect(18, (_curButton - 1) * 72 + 92, 294, (_curButton - 1) * 72 + 152));

	const Graphics::Surface *frame = _stillFrames->getFrame(_curPage);
	if (frame)
		_vm->_gfx->blit(frame, 304, 0);
}

bool CreditsWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void CreditsWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_returnButton.contains(point)) {
		((FrameWindow *)_parent)->showMainMenu();
		return;
	}

	if (_curButton == 1) {
		// Displaying the main menu for the Presto people
		if (_curPage == 0) {
			// Check against the list of people
			for (int i = 0; i < 17; i++) {
				if (Common::Rect(319, i * 16 + 181, 619, i * 16 + 193).contains(point)) {
					_curPage = i + 1;
					invalidateRect(Common::Rect(304, 0, 640, 480), false);
					return;
				}
			}
		} else {
			// Check for a return click
			if (Common::Rect(312, 24, 625, 454).contains(point)) {
				_curButton = 1;
				_curPage = 0;
				invalidateRect(Common::Rect(304, 0, 640, 480), false);
			}
		}
	}

	// Check normal buttons
	for (int i = 0; i < 5; i++) {
		if (_buttons[i].contains(point) && _curButton != i + 1) {
			_curButton = i + 1;

			switch (_curButton) {
			case 1:
				_curPage = 0;
				break;
			case 2:
				_curPage = 18;
				break;
			case 3:
				_curPage = 19;
				break;
			case 4:
				_curPage = 20;
				break;
			case 5:
				_curPage = 21;
				break;
			}

			invalidateWindow(false);
		}
	}
}

} // End of namespace Buried
