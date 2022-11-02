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
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/navarrow.h"
#include "buried/navdata.h"
#include "buried/resources.h"
#include "buried/scene_view.h"

#include "common/keyboard.h"
#include "graphics/surface.h"

namespace Buried {

NavArrowWindow::NavArrowWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_background = _vm->_gfx->getBitmap(IDB_ARROW_BACKGROUND);

	_arrowBitmaps[0][0] = IDB_ARROW_UP_CLEAR;
	_arrowBitmaps[0][1] = IDB_ARROW_UP_LIT;
	_arrowBitmaps[0][2] = IDB_ARROW_UP_HIGHLIGHTED;
	_arrowBitmaps[1][0] = IDB_ARROW_LEFT_CLEAR;
	_arrowBitmaps[1][1] = IDB_ARROW_LEFT_LIT;
	_arrowBitmaps[1][2] = IDB_ARROW_LEFT_HIGHLIGHTED;
	_arrowBitmaps[2][0] = IDB_ARROW_RIGHT_CLEAR;
	_arrowBitmaps[2][1] = IDB_ARROW_RIGHT_LIT;
	_arrowBitmaps[2][2] = IDB_ARROW_RIGHT_HIGHLIGHTED;
	_arrowBitmaps[3][0] = IDB_ARROW_DOWN_CLEAR;
	_arrowBitmaps[3][1] = IDB_ARROW_DOWN_LIT;
	_arrowBitmaps[3][2] = IDB_ARROW_DOWN_HIGHLIGHTED;
	_arrowBitmaps[4][0] = IDB_ARROW_FORWARD_CLEAR;
	_arrowBitmaps[4][1] = IDB_ARROW_FORWARD_LIT;
	_arrowBitmaps[4][2] = IDB_ARROW_FORWARD_HIGHLIGHTED;

	for (int i = 0; i < NUM_ARROWS; i++)
		_arrowStatus[i] = BUTTON_DISABLED;

	rebuildArrows();

	_rect = Common::Rect(510, 292, 640, 418);
}

NavArrowWindow::~NavArrowWindow() {
	_background->free();
	delete _background;
}

bool NavArrowWindow::updateArrow(int button, int newStatus) {
	_arrowStatus[button] = newStatus;

	rebuildArrows();
	invalidateWindow(false);
	return true;
}

bool NavArrowWindow::updateAllArrows(int left, int up, int right, int down, int forward) {
	// clone2727 says: This is wrong. Left and up are swapped.
	// I can only imagine what bugs this causes.
	_arrowStatus[0] = left;
	_arrowStatus[1] = up;
	_arrowStatus[2] = right;
	_arrowStatus[3] = down;
	_arrowStatus[4] = forward;

	rebuildArrows();
	invalidateWindow(false);
	return true;
}

bool NavArrowWindow::updateAllArrows(const LocationStaticData &locationStaticData) {
	_arrowStatus[0] = (locationStaticData.destUp.destinationScene.timeZone >= 0) ? BUTTON_ENABLED : BUTTON_DISABLED;
	_arrowStatus[1] = (locationStaticData.destLeft.destinationScene.timeZone >= 0) ? BUTTON_ENABLED : BUTTON_DISABLED;
	_arrowStatus[2] = (locationStaticData.destRight.destinationScene.timeZone >= 0) ? BUTTON_ENABLED : BUTTON_DISABLED;
	_arrowStatus[3] = (locationStaticData.destDown.destinationScene.timeZone >= 0) ? BUTTON_ENABLED : BUTTON_DISABLED;
	_arrowStatus[4] = (locationStaticData.destForward.destinationScene.timeZone >= 0) ? BUTTON_ENABLED : BUTTON_DISABLED;

	rebuildArrows();
	invalidateWindow(false);
	return true;
}

bool NavArrowWindow::drawArrow(int xDst, int yDst, int arrow) {
	Graphics::Surface *arrowBitmap = _vm->_gfx->getBitmap(_arrowBitmaps[arrow][_arrowStatus[arrow]]);

	for (int ySrc = 0; ySrc < arrowBitmap->h; ySrc++)
		memcpy(_background->getBasePtr(xDst, yDst + ySrc), arrowBitmap->getBasePtr(0, ySrc), arrowBitmap->w * arrowBitmap->format.bytesPerPixel);

	arrowBitmap->free();
	delete arrowBitmap;
	return true;
}

bool NavArrowWindow::rebuildArrows() {
	_background->free();
	delete _background;
	_background = _vm->_gfx->getBitmap(IDB_ARROW_BACKGROUND);

	drawArrow(37, 2, 0);
	drawArrow(2, 39, 1);
	drawArrow(64, 38, 2);
	drawArrow(38, 68, 3);

	Graphics::Surface *centerArrow = _vm->_gfx->getBitmap(_arrowBitmaps[4][_arrowStatus[4]]);
	_vm->_gfx->opaqueTransparentBlit(_background, 39, 49, centerArrow->w, centerArrow->h, centerArrow, 0, 0, (_arrowStatus[4] == BUTTON_DISABLED) ? 50 : 85, 255, 255, 255);
	centerArrow->free();
	delete centerArrow;
	return true;
}

void NavArrowWindow::onLButtonDown(const Common::Point &point, uint flags) {
	Common::Rect leftButton(1, 43, 40, 78);
	Common::Rect upButton(40, 1, 76, 45);
	Common::Rect rightButton(63, 45, 130, 71);
	Common::Rect downButton(42, 71, 78, 124);
	Common::Rect forwardButton(39, 49, 101, 91);

	((GameUIWindow *)_parent)->_inventoryWindow->destroyInfoWindow();
	((GameUIWindow *)_parent)->_inventoryWindow->destroyBurnedLetterWindow();

	// This logic was broken in the original. retVal wasn't initialized.
	bool retVal = false;

	// Did we click anywhere near the forward button?
	if (forwardButton.contains(point)) {
		// If we only clicked on the forward arrow, then take care of it here
		if (!rightButton.contains(point) && !downButton.contains(point)) {
			if (_arrowStatus[4] == BUTTON_ENABLED)
				((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionForward);
		} else {
			if (rightButton.contains(point)) {
				Graphics::Surface *centerArrow = _vm->_gfx->getBitmap(_arrowBitmaps[4][_arrowStatus[4]]);

				if (_vm->_gfx->checkPointAgainstMaskedBitmap(centerArrow, 39, 49, point, 255, 255, 255)) {
					if (_arrowStatus[4] == BUTTON_ENABLED)
						retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionForward);
				} else {
					if (_arrowStatus[2] == BUTTON_ENABLED)
						retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionRight);
				}

				centerArrow->free();
				delete centerArrow;
			}

			if (downButton.contains(point)) {
				Graphics::Surface *centerArrow = _vm->_gfx->getBitmap(_arrowBitmaps[4][_arrowStatus[4]]);

				if (_vm->_gfx->checkPointAgainstMaskedBitmap(centerArrow, 39, 49, point, 255, 255, 255)) {
					if (_arrowStatus[4] == BUTTON_ENABLED)
						retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionForward);
				} else {
					if (_arrowStatus[3] == BUTTON_ENABLED)
						retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionDown);
				}

				centerArrow->free();
				delete centerArrow;
			}
		}
	} else {
		if (upButton.contains(point) && _arrowStatus[0] == BUTTON_ENABLED)
			retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionUp);

		if (leftButton.contains(point) && _arrowStatus[1] == BUTTON_ENABLED)
			retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionLeft);

		if (rightButton.contains(point) && _arrowStatus[2] == BUTTON_ENABLED)
			retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionRight);

		if (downButton.contains(point) && _arrowStatus[3] == BUTTON_ENABLED)
			retVal = ((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionDown);
	}

	if (retVal) {
		rebuildArrows();
		invalidateWindow(false);
	}
}

void NavArrowWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	switch (key.keycode) {
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_LEFT:
		if (_arrowStatus[1] == BUTTON_ENABLED)
			((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionLeft);
		break;
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_RIGHT:
		if (_arrowStatus[2] == BUTTON_ENABLED)
			((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionRight);
		break;
	case Common::KEYCODE_KP2:
	case Common::KEYCODE_DOWN:
		if (_arrowStatus[3] == BUTTON_ENABLED)
			((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionDown);
		break;
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_UP:
		if (_arrowStatus[0] == BUTTON_ENABLED)
			((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionUp);
		break;
	case Common::KEYCODE_KP5:
		if (_arrowStatus[4] == BUTTON_ENABLED)
			((GameUIWindow *)_parent)->_sceneViewWindow->moveInDirection(kDirectionForward);
		break;
	default:
		break;
	}
}

void NavArrowWindow::onPaint() {
	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(_background, absoluteRect.left, absoluteRect.top);
}

void NavArrowWindow::onEnable(bool enable) {
	if (enable)
		_vm->removeMouseMessages(this);
}

} // End of namespace Buried
