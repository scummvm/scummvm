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
#include "buried/biochip_view.h"
#include "buried/buried.h"
#include "buried/fbcdata.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/resources.h"
#include "buried/scene_view.h"

#include "common/stream.h"
#include "graphics/surface.h"

namespace Buried {

BioChipMainViewWindow::BioChipMainViewWindow(BuriedEngine *vm, Window *parent, int currentBioChipID) : Window(vm, parent) {
	_currentBioChipID = -1;
	_rect = Common::Rect(0, 0, 432, 189);
	_bioChipDisplayWindow = createBioChipSpecificViewWindow(currentBioChipID);
	_currentBioChipID = currentBioChipID;

	if (_bioChipDisplayWindow)
		_bioChipDisplayWindow->showWindow(kWindowShow);

	_oldCursor = _vm->_gfx->setCursor(kCursorArrow);
}

BioChipMainViewWindow::~BioChipMainViewWindow() {
	delete _bioChipDisplayWindow;
}

bool BioChipMainViewWindow::onSetCursor(uint message) {
	_vm->_gfx->setCursor((Cursor)_oldCursor);
	return true;
}

bool BioChipMainViewWindow::changeCurrentBioChip(int newBioChipID) {
	delete _bioChipDisplayWindow;
	_currentBioChipID = newBioChipID;
	_bioChipDisplayWindow = createBioChipSpecificViewWindow(newBioChipID);

	if (_bioChipDisplayWindow)
		_bioChipDisplayWindow->showWindow(kWindowShow);

	return true;
}

enum {
	REGION_NONE = 0
};

enum {
	REGION_SAVE = 1,
	REGION_RESTORE = 2,
	REGION_PAUSE = 3,
	REGION_QUIT = 4,
	REGION_FLICKER = 5,
	REGION_TRANSITION_SPEED = 6
};

class InterfaceBioChipViewWindow : public Window {
public:
	InterfaceBioChipViewWindow(BuriedEngine *vm, Window *parent);
	~InterfaceBioChipViewWindow();

	void onPaint();
	void onLButtonDown(const Common::Point &point, uint flags);
	void onLButtonUp(const Common::Point &point, uint flags);
	void onMouseMove(const Common::Point &point, uint flags);

private:
	Common::Rect _save;
	Common::Rect _pause;
	Common::Rect _restore;
	Common::Rect _quit;
	Common::Rect _flicker;
	Common::Rect _transitionSpeed;

	int _curRegion;
	int _transLocation;
	int _soundLocation;

	Graphics::Surface *_background;
	Graphics::Surface *_cycleCheck;
	Graphics::Surface *_caret;
};

InterfaceBioChipViewWindow::InterfaceBioChipViewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_save = Common::Rect(192, 37, 300, 74);
	_pause = Common::Rect(192, 84, 300, 121);
	_restore = Common::Rect(313, 37, 421, 74);
	_quit = Common::Rect(313, 84, 421, 121);
	_flicker = Common::Rect(14, 146, 164, 166);
	_transitionSpeed = Common::Rect(14, 100, 125, 140);

	_curRegion = REGION_NONE;
	_soundLocation = 0;
	_transLocation = _vm->getTransitionSpeed() * 50;

	_rect = Common::Rect(0, 0, 432, 189);

	_background = _vm->_gfx->getBitmap(IDB_BCV_INTERFACE_MAIN);
	_cycleCheck = _vm->_gfx->getBitmap(IDB_BCV_INTERFACE_CHECK);
	_caret = _vm->_gfx->getBitmap(IDB_BCV_INTERFACE_HANDLE);
}

InterfaceBioChipViewWindow::~InterfaceBioChipViewWindow() {
	_background->free();
	delete _background;

	_cycleCheck->free();
	delete _cycleCheck;

	_caret->free();
	delete _caret;
}

void InterfaceBioChipViewWindow::onPaint() {
	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(_background, absoluteRect.left, absoluteRect.top);

	if (((SceneViewWindow *)_parent)->getCyclingStatus())
		_vm->_gfx->blit(_cycleCheck, absoluteRect.left + 13, absoluteRect.top + 144);

	if (_caret)
		_vm->_gfx->opaqueTransparentBlit(_vm->_gfx->getScreen(), absoluteRect.left + _transLocation + 14, absoluteRect.top + 97, 15, 30, _caret, 0, 0, 0, 248, _vm->isTrueColor() ? 252 : 248, 248);
}

void InterfaceBioChipViewWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_save.contains(point))
		_curRegion = REGION_SAVE;
	else if (_restore.contains(point))
		_curRegion = REGION_RESTORE;
	else if (_pause.contains(point))
		_curRegion = REGION_PAUSE;
	else if (_quit.contains(point))
		_curRegion = REGION_QUIT;
	else if (_flicker.contains(point))
		_curRegion = REGION_FLICKER;
	else if (_transitionSpeed.contains(point))
		_curRegion = REGION_TRANSITION_SPEED;
}

void InterfaceBioChipViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	switch (_curRegion) {
	case REGION_SAVE:
		// TODO
		break;
	case REGION_RESTORE:
		// TODO
		break;
	case REGION_QUIT:
		// TODO
		break;
	case REGION_PAUSE:
		// TODO
		break;
	case REGION_FLICKER:
		if (_flicker.contains(point)) {
			if (((SceneViewWindow *)(_parent->getParent()))->getCyclingStatus()) {
				((SceneViewWindow *)(_parent->getParent()))->enableCycling(false);
				invalidateRect(_flicker, false);
			} else {
				((SceneViewWindow *)(_parent->getParent()))->enableCycling(true);
				invalidateRect(_flicker, false);
			}
		}
		break;
	case REGION_TRANSITION_SPEED:
		_transLocation = CLIP<int>(point.x - 14, 0, 150);

		if ((_transLocation % 50) > 25)
			_transLocation = (_transLocation / 50 + 1) * 50;
		else
			_transLocation = _transLocation / 50 * 50;

		_vm->setTransitionSpeed(_transLocation / 50);

		invalidateWindow(false);
		break;
	}

	_curRegion = REGION_NONE;
}

void InterfaceBioChipViewWindow::onMouseMove(const Common::Point &point, uint flags) {
	if (_curRegion == REGION_TRANSITION_SPEED) {
		int newPos = CLIP<int>(point.x - 14, 0, 150);

		if ((newPos % 50) > 25)
			newPos = (newPos / 50 + 1) * 50;
		else
			newPos = newPos / 50 * 50;

		if (_transLocation != newPos) {
			_transLocation = newPos;
			invalidateWindow(false);
		}
	}
}

class FilesBioChipViewWindow : public Window {
public:
	FilesBioChipViewWindow(BuriedEngine *vm, Window *parent);
	
	void onPaint();
	void onLButtonUp(const Common::Point &point, uint flags);

private:
	int _curPage;
	AVIFrames _stillFrames;
	Common::Array<FilesPage> _navData;
};

FilesBioChipViewWindow::FilesBioChipViewWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_curPage = 0;
	_rect = Common::Rect(0, 0, 432, 189);

	Common::SeekableReadStream *fbcStream = _vm->getFileBCData(IDBD_BC_VIEW_DATA);
	assert(fbcStream);

	fbcStream->skip(2); // Page count

	while (fbcStream->pos() < fbcStream->size()) {
		FilesPage page;
		page.pageID = fbcStream->readSint16LE();
		page.returnPageIndex = fbcStream->readSint16LE();
		page.nextButtonPageIndex = fbcStream->readSint16LE();
		page.prevButtonPageIndex = fbcStream->readSint16LE();

		for (int i = 0; i < 6; i++) {
			page.hotspots[i].left = fbcStream->readSint16LE();
			page.hotspots[i].top = fbcStream->readSint16LE();
			page.hotspots[i].right = fbcStream->readSint16LE();
			page.hotspots[i].bottom = fbcStream->readSint16LE();
			page.hotspots[i].pageIndex = fbcStream->readSint16LE();
		}

		_navData.push_back(page);
	}

	delete fbcStream;

	if (!_stillFrames.open(_vm->getFilePath(IDS_BC_FILES_VIEW_FILENAME)))
		error("Failed to open files biochip video");
}

void FilesBioChipViewWindow::onPaint() {
	const Graphics::Surface *frame = _stillFrames.getFrame(_curPage);
	assert(frame);

	Common::Rect absoluteRect = getAbsoluteRect();
	_vm->_gfx->blit(frame, absoluteRect.left, absoluteRect.top);
}

void FilesBioChipViewWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_curPage < 0 || _curPage >= (int)_navData.size())
		return;

	const FilesPage &page = _navData[_curPage];

	Common::Rect returnButton(343, 157, 427, 185);
	Common::Rect next(193, 25, 241, 43);
	Common::Rect previous(253, 25, 301, 43);

	if (page.returnPageIndex >= 0 && returnButton.contains(point)) {
		_curPage = page.returnPageIndex;
		invalidateWindow(false);
		return;
	}

	if (page.nextButtonPageIndex >= 0 && next.contains(point)) {
		_curPage = page.nextButtonPageIndex;
		invalidateWindow(false);

		if (_curPage == 6)
			((SceneViewWindow *)(_parent->getParent()))->getGlobalFlags().scoreResearchBCJumpsuit = 1;
		else if (_curPage == 21)
			((SceneViewWindow *)(_parent->getParent()))->getGlobalFlags().scoreResearchMichelle = 1;
		else if (_curPage == 31)
			((SceneViewWindow *)(_parent->getParent()))->getGlobalFlags().scoreResearchMichelleBkg = 1;

		return;
	}

	if (page.prevButtonPageIndex >= 0 && previous.contains(point)) {
		_curPage = page.prevButtonPageIndex;
		invalidateWindow(false);
		return;
	}

	for (int i = 0; i < 6; i++) {
		if (page.hotspots[i].pageIndex >= 0 && Common::Rect(page.hotspots[i].left, page.hotspots[i].top, page.hotspots[i].right, page.hotspots[i].bottom).contains(point)) {
			_curPage = page.hotspots[i].pageIndex;
			invalidateWindow(false);
			return;
		}
	}
}

Window *BioChipMainViewWindow::createBioChipSpecificViewWindow(int bioChipID) {
	switch (bioChipID) {
	case kItemBioChipInterface:
		return new InterfaceBioChipViewWindow(_vm, this);
	case kItemBioChipJump:
		// TODO
		break;
	case kItemBioChipEvidence:
		// TODO
		break;
	case kItemBioChipFiles:
		return new FilesBioChipViewWindow(_vm, this);
	}

	// No entry for this BioChip
	return 0;
}

} // End of namespace Buried
