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
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/inventory_info.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/video_window.h"

#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

InventoryInfoWindow::InventoryInfoWindow(BuriedEngine *vm, Window *parent, int currentItemID) : Window(vm, parent) {
	_currentItemID = 0;
	_spinStart = 0;
	_spinLength = 70;

	_fontHeight = (_vm->getLanguage() == Common::JA_JPN) ? 11 : 14;
	_textFont = _vm->_gfx->createFont(_fontHeight);

	_rect = Common::Rect(0, 0, 432, 189);
	_videoWindow = new VideoWindow(_vm, this);

	if (!_videoWindow->openVideo(_vm->getFilePath(_vm->isDemo() ? IDS_INVENTORY_SPIN_FILENAME_DEMO : IDS_INVENTORY_SPIN_FILENAME)))
		error("Failed to load inventory info file");

	_videoWindow->setWindowPos(nullptr, 268, 17, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosShowWindow);
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

	if (_vm->isDemo()) {
		_spinStart = newItemID * 72;
		_spinLength = 71;
	} else {
		_spinStart = newItemID * 71;
		_spinLength = 70;
	}

	_videoWindow->stopVideo();

	_videoWindow->seekToFrame(_spinStart);
	invalidateWindow(false);

	_videoWindow->playToFrame(_spinStart + _spinLength);

	if (_currentItemID == kItemLensFilter)
		((GameUIWindow *)(_parent->getParent()))->_sceneViewWindow->getGlobalFlags().scoreResearchLensFilter = 1;

	return true;
}

void InventoryInfoWindow::onPaint() {
	Graphics::Surface *background = _vm->_gfx->getBitmap(IDB_INVENTORY_INFO_BACKGROUND);

	// Draw the title
	uint32 textColor = _vm->_gfx->getColor(212, 109, 0);
	Common::Rect titleRect(10, 56, 263, 71);
	Common::String title = _vm->getString(IDES_ITEM_TITLE_BASE + _currentItemID);
	assert(!title.empty());
	_vm->_gfx->renderText(background, _textFont, title, titleRect.left, titleRect.top, titleRect.width(), titleRect.height(), textColor, _fontHeight);

	// Draw the description
	Common::Rect descRect(10, 89, 263, 186);
	Common::String desc = _vm->getString(IDES_ITEM_DESC_BASE + _currentItemID * 5);
	assert(!desc.empty());
	_vm->_gfx->renderText(background, _textFont, desc, descRect.left, descRect.top, descRect.width(), descRect.height(), textColor, _fontHeight);

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
	((GameUIWindow *)(_parent->getParent()))->_inventoryWindow->destroyInfoWindow();
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
	_preBuffer = nullptr;

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

	((GameUIWindow *)(_parent->getParent()))->_sceneViewWindow->getGlobalFlags().readBurnedLetter = 1;
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
		if (!_preBuffer)
			error("Failed to get burned letter section");

		_rebuildPage = false;
	}

	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->opaqueTransparentBlit(_vm->_gfx->getScreen(), absoluteRect.left, absoluteRect.top, absoluteRect.width(), absoluteRect.height(), _preBuffer, 0, 0, 0, 0, 0, 0);

	if (_curLineIndex >= 0 && ((SceneViewWindow *)_parent)->getGlobalFlags().bcTranslateEnabled == 1) {
		int numLines = _viewLineCount[_curView];
		uint32 boxColor = _vm->_gfx->getColor(255, 0, 0);
		Common::Rect box(1, (187 / numLines) * _curLineIndex, 430, (187 / numLines) * (_curLineIndex + 1) - 1);
		box.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(box, boxColor);
	}
}

void BurnedLetterViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_top.contains(point) && _curView > 0) {
		_curView--;
		_curLineIndex = -1;

		int offset = _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed());

		// Only draw if transitions are enabled
		if (offset != 189) {
			TempCursorChange cursorChange(kCursorWait);

			Graphics::Surface *newFrame = _stillFrames->getFrameCopy(_curView);

			for (int i = 0; i < 189; i += offset) {
				_preBuffer->move(0, offset, _preBuffer->h);

				for (int j = 0; j < offset; j++)
					memcpy(_preBuffer->getBasePtr(0, j), newFrame->getBasePtr(0, _preBuffer->h - (i + offset) + j), newFrame->w * newFrame->format.bytesPerPixel);

				invalidateWindow(false);
				_vm->yield(nullptr, -1);
			}

			newFrame->free();
			delete newFrame;
		}

		_rebuildPage = true;
		invalidateWindow(false);
	}

	if (_bottom.contains(point) && _curView < _viewCount - 1) {
		_curView++;
		_curLineIndex = -1;

		int offset = _vm->_gfx->computeVPushOffset(_vm->getTransitionSpeed());

		// Only draw if transitions are enabled
		if (offset != 189) {
			TempCursorChange cursorChange(kCursorWait);

			Graphics::Surface *newFrame = _stillFrames->getFrameCopy(_curView);

			for (int i = 0; i < 189; i += offset) {
				_preBuffer->move(0, -offset, _preBuffer->h);

				for (int j = 0; j < offset; j++)
					memcpy(_preBuffer->getBasePtr(0, newFrame->h - offset + j), newFrame->getBasePtr(0, i + j), newFrame->w * newFrame->format.bytesPerPixel);

				invalidateWindow(false);
				_vm->yield(nullptr, -1);
			}

			newFrame->free();
			delete newFrame;
		}

		_rebuildPage = true;
		invalidateWindow(false);
	}

	if (_putDown.contains(point))
		((GameUIWindow *)(_parent->getParent()))->_inventoryWindow->destroyBurnedLetterWindow();
}

void BurnedLetterViewWindow::onMouseMove(const Common::Point &point, uint flags) {
	_curMousePos = point;

	if (((SceneViewWindow *)_parent)->getGlobalFlags().bcTranslateEnabled == 1) {
		int lineCount = _viewLineCount[_curView];
		int textLineNumber = 0;
		for (int i = 0; i < _curView; i++)
			textLineNumber += _viewLineCount[i];

		int lineIndex = ((point.y - 2) / (187 / lineCount));
		if (lineIndex > (lineCount - 1))
			lineIndex = lineCount - 1;

		if (_curLineIndex != lineIndex) {
			_curLineIndex = lineIndex;
			invalidateWindow(false);

			Common::String translatedText = _vm->getString(_translatedTextResourceID + textLineNumber + _curLineIndex);
			((SceneViewWindow *)_parent)->displayTranslationText(translatedText);
		}

		return;
	}

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
