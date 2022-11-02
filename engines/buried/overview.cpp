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
#include "buried/overview.h"
#include "buried/resources.h"
#include "buried/sound.h"

#include "graphics/surface.h"

namespace Buried {

OverviewWindow::OverviewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_currentImage = nullptr;
	_currentStatus = -1;
	_timer = 0xFFFFFFFF;

	Common::Rect parentRect = _parent->getClientRect();
	_rect.left = (parentRect.right - 640) / 2;
	_rect.top = (parentRect.bottom - 480) / 2;
	_rect.right = parentRect.left + 640;
	_rect.bottom = parentRect.top + 480;

	_background = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_IF_OV_FULL_SCREEN_DIB));
}

OverviewWindow::~OverviewWindow() {
	_vm->_sound->stopInterfaceSound();

	_background->free();
	delete _background;

	if (_currentImage) {
		_currentImage->free();
		delete _currentImage;
	}

	if (_timer != 0xFFFFFFFF)
		_vm->killTimer(_timer);
}

bool OverviewWindow::startOverview() {
	_vm->_sound->setAmbientSound("");

	showWindow(kWindowShow);
	invalidateWindow();
	setFocus();

	_timer = setTimer(1000);
	return true;
}

void OverviewWindow::onPaint() {
	_vm->_gfx->blit(_background, 0, 0);

	if (_currentImage) {
		switch (_currentStatus) {
		case 0: // Navigational buttons
			_vm->_gfx->blit(_currentImage, 498, 274);
			break;
		case 1: // Inventory buttons
			_vm->_gfx->blit(_currentImage, 163, 352);
			break;
		case 2: // BioChip buttons
			_vm->_gfx->blit(_currentImage, 509, 89);
			break;
		case 3: // Message buttons
			_vm->_gfx->blit(_currentImage, 93, 0);
			break;
		case 4: // Final info - no image to render to the screen
			break;
		}
	}
}

bool OverviewWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void OverviewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	_vm->_sound->stopInterfaceSound();
}

void OverviewWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	_vm->_sound->stopInterfaceSound();
}

void OverviewWindow::onTimer(uint timer) {
	_vm->_sound->timerCallback();

	if (_currentStatus >= 0 && _vm->_sound->isInterfaceSoundPlaying())
		return;

	if (_currentImage) {
		_currentImage->free();
		delete _currentImage;
		_currentImage = nullptr;
	}

	// Switch on the current status in order to determine which action to take next
	switch (_currentStatus) {
	case -1: // Starting value - kick things off
		_currentStatus = 0;
		_currentImage = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_IF_OV_NAV_ARROWS_DIB));
		invalidateRect(Common::Rect(498, 274, 640, 433), false);

		_vm->_sound->stopInterfaceSound();
		_vm->_sound->timerCallback();
		_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_IF_OV_NAV_ARROWS_AUDIO));
		break;
	case 0: // Played initial stuff
		_currentStatus = 1;
		_currentImage = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_IF_OV_INVENTORY_DIB));
		invalidateRect(Common::Rect(498, 274, 640, 433), false);
		invalidateRect(Common::Rect(163, 352, 472, 472), false);

		_vm->_sound->stopInterfaceSound();
		_vm->_sound->timerCallback();
		_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_IF_OV_INVENTORY_AUDIO));
		break;
	case 1:
		_currentStatus = 2;
		_currentImage = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_IF_OV_BIOCHIPS_DIB));
		invalidateRect(Common::Rect(163, 352, 472, 472), false);
		invalidateRect(Common::Rect(509, 89, 640, 275), false);

		_vm->_sound->stopInterfaceSound();
		_vm->_sound->timerCallback();
		_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_IF_OV_BIOCHIPS_AUDIO));
		break;
	case 2:
		_currentStatus = 3;
		_currentImage = _vm->_gfx->getBitmap(_vm->getFilePath(IDS_IF_OV_MESSAGE_BOX_DIB));
		invalidateRect(Common::Rect(509, 89, 640, 275), false);
		invalidateRect(Common::Rect(93, 0, 482, 108), false);

		_vm->_sound->stopInterfaceSound();
		_vm->_sound->timerCallback();
		_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_IF_OV_MESSAGE_BOX_AUDIO));
		break;
	case 3:
		_currentStatus = 4;
		invalidateRect(Common::Rect(93, 0, 482, 108), false);

		_vm->_sound->stopInterfaceSound();
		_vm->_sound->timerCallback();
		_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_IF_OV_FURTHER_INFO_AUDIO));
		break;
	case 4:
		_currentStatus = 5;
		((FrameWindow *)_parent)->returnToMainMenu();
		break;
	}
}

} // End of namespace Buried
