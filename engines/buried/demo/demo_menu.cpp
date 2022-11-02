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

#include "buried/buried.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/message.h"
#include "buried/sound.h"
#include "buried/demo/demo_menu.h"

#include "common/system.h"
#include "graphics/surface.h"

namespace Buried {

enum {
	BUTTON_OVERVIEW = 1,
	BUTTON_TRAILER = 2,
	BUTTON_INTERACTIVE = 3,
	BUTTON_GALLERY = 4,
	BUTTON_QUIT = 5
};

DemoMainMenuWindow::DemoMainMenuWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_curButton = 0;

	Common::Rect parentRect = _parent->getClientRect();
	_rect.left = (parentRect.right - 640) / 2;
	_rect.top = (parentRect.bottom - 480) / 2;
	_rect.right = parentRect.left + 640;
	_rect.bottom = parentRect.top + 480;

	_overview = Common::Rect(29, 155, 168, 325);
	_trailer = Common::Rect(177, 155, 316, 325);
	_interactive = Common::Rect(324, 155, 463, 325);
	_gallery = Common::Rect(471, 155, 610, 325);
	_quit = Common::Rect(552, 439, 640, 480);

	if (_vm->isTrueColor())
		_background = _vm->_gfx->getBitmap("MISC/24BPP/MAINMENU.BMP");
	else
		_background = _vm->_gfx->getBitmap("MISC/8BPP/MAINMENU.BMP");

	_vm->_sound->setAmbientSound("MISC/MENULOOP.WAV");
}

DemoMainMenuWindow::~DemoMainMenuWindow() {
	_background->free();
	delete _background;
}

void DemoMainMenuWindow::showWithSplash() {
	Graphics::Surface *temp = _background;

	if (_vm->isTrueColor())
		_background = _vm->_gfx->getBitmap("MISC/24BPP/SPLASH.BMP");
	else
		_background = _vm->_gfx->getBitmap("MISC/8BPP/SPLASH.BMP");

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_parent);

	showWindow(kWindowShow);
	invalidateWindow(false);
	_vm->_gfx->updateScreen();

	uint32 startTime = g_system->getMillis();
	while (g_system->getMillis() < (startTime + 6000) && !_vm->hasMessage(this, kMessageTypeLButtonUp, kMessageTypeLButtonUp) && !_vm->shouldQuit())
		_vm->yield(nullptr, -1);

	_background->free();
	delete _background;
	_background = temp;
	invalidateWindow(false);

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_parent);
}

void DemoMainMenuWindow::onPaint() {
	_vm->_gfx->blit(_background, 0, 0);
}

bool DemoMainMenuWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void DemoMainMenuWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_overview.contains(point)) {
		_curButton = BUTTON_OVERVIEW;
		return;
	}

	if (_trailer.contains(point)) {
		_curButton = BUTTON_TRAILER;
		return;
	}

	if (_interactive.contains(point)) {
		_curButton = BUTTON_INTERACTIVE;
		return;
	}

	if (_gallery.contains(point)) {
		_curButton = BUTTON_GALLERY;
		return;
	}

	if (_quit.contains(point))
		_curButton = BUTTON_QUIT;
}

void DemoMainMenuWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_curButton == 0)
		return;

	switch (_curButton) {
	case BUTTON_OVERVIEW:
		if (_overview.contains(point)) {
			_vm->_sound->setAmbientSound();
			((FrameWindow *)_parent)->playMovie(_vm->isTrueColor() ? "MISC/24BPP/OVERVIEW.BMP" : "MISC/8BPP/OVERVIEW.BMP", "MISC/OVERVIEW.AVI", 160, 112);
		}
		return;
	case BUTTON_TRAILER:
		if (_trailer.contains(point)) {
			_vm->_sound->setAmbientSound();
			((FrameWindow *)_parent)->playMovie(_vm->isTrueColor() ? "MISC/24BPP/TRAILER.BMP" : "MISC/8BPP/TRAILER.BMP", "MISC/TRAILER.AVI", 104, 136);
		}
		return;
	case BUTTON_INTERACTIVE:
		if (_interactive.contains(point)) {
			_vm->_sound->setAmbientSound();

			// Reviewer mode check (control)
			if (_vm->isControlDown())
				((FrameWindow *)_parent)->_reviewerMode = true;

			((FrameWindow *)_parent)->startNewGame();
		}
		return;
	case BUTTON_GALLERY:
		if (_gallery.contains(point)) {
			_vm->_sound->setAmbientSound();
			((FrameWindow *)_parent)->playMovie(_vm->isTrueColor() ? "MISC/24BPP/GALLERY.BMP" : "MISC/8BPP/GALLERY.BMP", "MISC/GALLERY.AVI", 104, 136);
		}
		return;
	case BUTTON_QUIT:
		if (_quit.contains(point))
			((FrameWindow *)_parent)->showFeaturesScreen();
		return;
	}

	_curButton = 0;
	invalidateWindow(false);
}

} // End of namespace Buried
