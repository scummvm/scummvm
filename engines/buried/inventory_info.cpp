/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "buried/avi_frames.h"
#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/inventory_info.h"
#include "buried/resources.h"
#include "buried/video_window.h"

#include "common/str-array.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

InventoryInfoWindow::InventoryInfoWindow(BuriedEngine *vm, Window *parent, int currentItemID) : Window(vm, parent) {
	_currentItemID = 0;
	_spinStart = 0;
	_spinLength = 70;

	_textFont = _vm->_gfx->createFont(14);
	_rect = Common::Rect(0, 0, 432, 189);
	_videoWindow = new VideoWindow(_vm, this);

	if (!_videoWindow->openVideo(_vm->getFilePath(IDS_INVENTORY_SPIN_FILENAME)))
		error("Failed to load inventory info file");

	_videoWindow->setWindowPos(0, 268, 17, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosShowWindow);
	_videoWindow->enableWindow(false);

	_timer = setTimer(100);
	changeCurrentItem(currentItemID);
}

InventoryInfoWindow::~InventoryInfoWindow() {
	_vm->killTimer(_timer);
	delete _videoWindow;
	delete _textFont;
}

bool InventoryInfoWindow::changeCurrentItem(int newItemID) {
	_currentItemID = newItemID;

	_spinStart = newItemID * 71;
	_spinLength = 70;
	_videoWindow->stopVideo();

	_videoWindow->seekToFrame(_spinStart);
	invalidateWindow(false);

	_videoWindow->playToFrame(_spinStart + _spinLength);

	if (_currentItemID == kItemLensFilter) {
		// TODO: Set scoring flag
	}

	return true;
}

void InventoryInfoWindow::onPaint() {
	Graphics::Surface *background = _vm->_gfx->getBitmap(IDB_INVENTORY_INFO_BACKGROUND);

	// Draw the title
	uint32 textColor = _vm->_gfx->getColor(212, 109, 0);
	Common::Rect titleRect(10, 56, 263, 71);
	Common::String title = _vm->getString(IDES_ITEM_TITLE_BASE + _currentItemID);
	assert(!title.empty());

	Common::StringArray lines;
	_textFont->wordWrapText(title, titleRect.width(), lines);

	uint32 y = titleRect.top;
	for (uint32 i = 0; i < lines.size(); i++) {
		_textFont->drawString(background, lines[i], titleRect.left, titleRect.top, titleRect.width(), textColor);
		y += 14;
	}

	// Draw the description
	Common::Rect descRect(10, 89, 263, 186);
	Common::String desc = _vm->getString(IDES_ITEM_DESC_BASE + _currentItemID * 5);
	assert(!desc.empty());

	lines.clear();
	_textFont->wordWrapText(desc, descRect.width(), lines);

	y = descRect.top;
	for (uint32 i = 0; i < lines.size(); i++) {
		_textFont->drawString(background, lines[i], descRect.left, descRect.top, descRect.width(), textColor);
		y += 14;
	}

	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(background, absoluteRect.left, absoluteRect.top);

	background->free();
	delete background;
}

bool InventoryInfoWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void InventoryInfoWindow::onLButtonUp(const Common::Point &point, uint flags) {
	// TODO: Destroy window
}

void InventoryInfoWindow::onTimer(uint timer) {
	if (_videoWindow->getMode() == VideoWindow::kModeStopped) {
		_videoWindow->seekToFrame(_spinStart);
		_videoWindow->playToFrame(_spinStart + _spinLength);
	}
}

BurnedLetterViewWindow::BurnedLetterViewWindow(BuriedEngine *vm, Window *parent, const LocationStaticData &curSceneStaticData) : Window(vm, parent), _curSceneStaticData(curSceneStaticData) {
	_curView = 0;
	_translatedTextResourceID = IDBD_BLETTER_TRANS_TEXT_BASE;
	_curLineIndex = -1;
	_preBuffer = 0;

	_rect = Common::Rect(0, 0, 432, 189);

	_viewLineCount[0] = 8;
	_viewLineCount[1] = 8;
	_viewLineCount[2] = 7;

	_stillFrames = new AVIFrames(_vm->getFilePath(IDS_INVITEM_LETTER_FILENAME));

	_viewCount = 3;

	_top = Common::Rect(0, 0, 432, 60);
	_bottom = Common::Rect(0, 129, 432, 189);
	_left = Common::Rect(0, 60, 60, 129);
	_right = Common::Rect(372, 60, 432, 129);
	_putDown = Common::Rect(60, 60, 372, 129);

	_rebuildPage = true;

	// TODO: Scoring for reading the letter
}

BurnedLetterViewWindow::~BurnedLetterViewWindow() {
	if (_preBuffer) {
		_preBuffer->free();
		delete _preBuffer;
	}

	delete _stillFrames;
}

void BurnedLetterViewWindow::onPaint() {
	if (_rebuildPage) {
		if (_preBuffer) {
			_preBuffer->free();
			delete _preBuffer;
		}

		_preBuffer = _stillFrames->getFrameCopy(_curView);
		_rebuildPage = false;
	}

	Common::Rect absoluteRect = getAbsoluteRect();
	byte transValue = _vm->isDemo() ? 2 : 0;
	_vm->_gfx->opaqueTransparentBlit(_vm->_gfx->getScreen(), absoluteRect.left, absoluteRect.top, absoluteRect.width(), absoluteRect.height(), _preBuffer, 0, 0, 0, transValue, transValue, transValue);

	if (_curLineIndex >= 0 && false) { // TODO: Translation
		int numLines = _viewLineCount[_curView];
		uint32 boxColor = _vm->_gfx->getColor(255, 0, 0);
		Common::Rect box(1, (187 / numLines) * _curLineIndex, 430, (187 / numLines) * (_curLineIndex + 1) - 1);
		_vm->_gfx->getScreen()->frameRect(box, boxColor);
	}
}

void BurnedLetterViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_top.contains(point) && _curView > 0) {
		_curView--;

		Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);

		Graphics::Surface *newFrame = _stillFrames->getFrameCopy(_curView);

		int offset = _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed());
		for (int i = 0; i < 189; i += offset) {
			_preBuffer->move(0, offset, _preBuffer->h - offset);

			for (int j = 0; j < offset; j++)
				memcpy(_preBuffer->getBasePtr(0, j), newFrame->getBasePtr(0, i + j), newFrame->w * newFrame->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}

		_curLineIndex = -1;
		_rebuildPage = true;
		invalidateWindow(false);

		_vm->_gfx->setCursor(oldCursor);
	}

	if (_bottom.contains(point) && _curView < _viewCount - 1) {
		_curView++;

		Cursor oldCursor = _vm->_gfx->setCursor(kCursorWait);

		Graphics::Surface *newFrame = _stillFrames->getFrameCopy(_curView);

		int offset = _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed());
		for (int i = 189 - offset; i >= 0; i -= offset) {
			_preBuffer->move(0, -offset, _preBuffer->h - offset);

			for (int j = 0; j < offset; j++)
				memcpy(_preBuffer->getBasePtr(0, j), newFrame->getBasePtr(0, i + j), newFrame->w * newFrame->format.bytesPerPixel);

			invalidateWindow(false);
			_vm->yield();
		}

		_curLineIndex = -1;
		_rebuildPage = true;
		invalidateWindow(false);

		_vm->_gfx->setCursor(oldCursor);
	}

	if (_putDown.contains(point)) {
		// TODO: Destroy the window
	}
}

void BurnedLetterViewWindow::onMouseMove(const Common::Point &point, uint flags) {
	_curMousePos = point;

	// TODO: Translation

	// Since translation was not enabled, check the current line flag
	if (_curLineIndex != -1) {
		// If the flag was not set to -1, reset it and invalidate the window
		_curLineIndex = -1;
		invalidateWindow(false);
	}
}

bool BurnedLetterViewWindow::onSetCursor(uint message) {
	Cursor cursorID = kCursorArrow;

	if (_top.contains(_curMousePos) && _curView > 0) 
		cursorID = kCursorMoveUp;

	if (_bottom.contains(_curMousePos) && _curView < (_viewCount - 1))
		cursorID = kCursorMoveDown;

	if (_putDown.contains(_curMousePos))
		cursorID = kCursorPutDown;

	_vm->_gfx->setCursor(cursorID);

	return true;
}

} // End of namespace Buried
