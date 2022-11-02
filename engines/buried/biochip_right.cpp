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
#include "buried/biochip_right.h"
#include "buried/biochip_view.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/livetext.h"
#include "buried/navarrow.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/video_window.h"

#include "graphics/surface.h"

namespace Buried {

BioChipRightWindow::BioChipRightWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_curBioChip = kItemBioChipInterface;
	_status = 0;
	_bioChipViewWindow = nullptr;
	_forceHelp = false;
	_forceComment = false;
	_jumpInProgress = false;

	_rect = Common::Rect(520, 102, 638, 281);
}

BioChipRightWindow::~BioChipRightWindow() {
	delete _bioChipViewWindow;
}

bool BioChipRightWindow::changeCurrentBioChip(int bioChipID) {
	if (_curBioChip == bioChipID)
		return true;

	// Signal live text to reset if this isn't the translate BioChip
	if (bioChipID != kItemBioChipTranslate)
		((GameUIWindow *)_parent)->_liveTextWindow->translateBiochipClosing();

	if (bioChipID != kItemBioChipEvidence)
		((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcLocateEnabled = 0;

	if (_bioChipViewWindow)
		destroyBioChipViewWindow();

	_curBioChip = bioChipID;
	_status = 0;

	((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcTranslateEnabled = 0;

	invalidateWindow(false);
	return true;
}

bool BioChipRightWindow::showBioChipMainView() {
	if (_bioChipViewWindow)
		return false;

	((GameUIWindow *)_parent)->_sceneViewWindow->bioChipWindowDisplayed(true);
	_vm->_sound->timerCallback();

	((GameUIWindow *)_parent)->_inventoryWindow->destroyInfoWindow();
	((GameUIWindow *)_parent)->_inventoryWindow->destroyBurnedLetterWindow();
	_vm->_sound->timerCallback();

	_bioChipViewWindow = new BioChipMainViewWindow(_vm, ((GameUIWindow *)_parent)->_sceneViewWindow, _curBioChip);
	_bioChipViewWindow->showWindow(kWindowShow);
	_vm->_sound->timerCallback();

	return true;
}

bool BioChipRightWindow::destroyBioChipViewWindow() {
	if (!_bioChipViewWindow)
		return false;

	_vm->_sound->timerCallback();
	delete _bioChipViewWindow;
	_bioChipViewWindow = nullptr;
	_vm->_sound->timerCallback();

	((GameUIWindow *)_parent)->_sceneViewWindow->bioChipWindowDisplayed(false);

	if (_status == 1) {
		_status = 0;
		invalidateWindow(false);
	}

	return true;
}

void BioChipRightWindow::swapAIBioChipIfActive() {
	if (_curBioChip == kItemBioChipAI) {
		_curBioChip = kItemBioChipBlank;
		invalidateWindow(false);
	}
}

void BioChipRightWindow::sceneChanged() {
	if (_curBioChip == kItemBioChipAI)
		invalidateWindow(false);
}

void BioChipRightWindow::disableEvidenceCapture() {
	if (_curBioChip == kItemBioChipEvidence) {
		_status = 0;
		((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcLocateEnabled = 0;
		invalidateWindow(false);
	}
}

void BioChipRightWindow::jumpInitiated(bool redraw) {
	_jumpInProgress = true;

	if (redraw)
		invalidateWindow(false);
}

void BioChipRightWindow::jumpEnded(bool redraw) {
	_jumpInProgress = false;

	if (redraw)
		invalidateWindow(false);
}

void BioChipRightWindow::onPaint() {
	int bitmapResID = -1;

	switch (_curBioChip) {
	case kItemBioChipAI: {
		bool helpComment = _forceHelp || ((GameUIWindow *)_parent)->_sceneViewWindow->checkForAIComment(AI_COMMENT_TYPE_HELP);
		bool information = _forceComment || ((GameUIWindow *)_parent)->_sceneViewWindow->checkForAIComment(AI_COMMENT_TYPE_INFORMATION);

		if (helpComment) {
			if (information)
				bitmapResID = 0;
			else
				bitmapResID = 1;
		} else {
			if (information)
				bitmapResID = 2;
			else
				bitmapResID = 3;
		}
		break;
	}
	case kItemBioChipBlank:
		bitmapResID = 4;
		break;
	case kItemBioChipCloak:
		bitmapResID = (_status == 0) ? 5 : 6;
		break;
	case kItemBioChipEvidence:
		switch (_status) {
		case 0:
			bitmapResID = 7;
			break;
		case 1:
			bitmapResID = 8;
			break;
		case 2:
			bitmapResID = 9;
			break;
		}
		break;
	case kItemBioChipFiles:
		bitmapResID = (_status == 0) ? 10 : 11;
		break;
	case kItemBioChipInterface:
		if (_vm->isDemo())
			bitmapResID = (_status == 0) ? IDB_BCR_INTERFACE_MENU : IDB_BCR_INTERFACE_EXIT;
		else
			bitmapResID = (_status == 0) ? 12 : 13;
		break;
	case kItemBioChipJump: {
		Location currentLocation;
		bitmapResID = 14;

		if (_status != 0)
			bitmapResID += 2;

		if (((GameUIWindow *)_parent)->_sceneViewWindow->getCurrentSceneLocation(currentLocation))
			if (currentLocation.timeZone == 4)
				bitmapResID++;

		if (_jumpInProgress)
			bitmapResID += 6;
		break;
	}
	case kItemBioChipTranslate:
		bitmapResID = (_status == 0) ? 18 : 19;
		break;
	}

	if (bitmapResID >= 0) {
		if (!_vm->isDemo())
			bitmapResID += IDB_BCR_BITMAP_BASE;

		Graphics::Surface *bitmap = _vm->_gfx->getBitmap(bitmapResID);
		Common::Rect absoluteRect = getAbsoluteRect();
		_vm->_gfx->blit(bitmap, absoluteRect.left, absoluteRect.top);
		bitmap->free();
		delete bitmap;
	}
}

void BioChipRightWindow::toggleBioChip() {
	if (_status == 0) {
		_status = 1;
		showBioChipMainView();
		invalidateWindow(false);
	} else {
		_status = 0;
		destroyBioChipViewWindow();
		invalidateWindow(false);
	}
}

int BioChipRightWindow::getCurrentBioChip() const {
	return _bioChipViewWindow ? _curBioChip : -1;
}

void BioChipRightWindow::onEnable(bool enable) {
	if (enable)
		_vm->removeMouseMessages(this);
}

void BioChipRightWindow::onLButtonUp(const Common::Point &point, uint flags) {
	Common::Rect upperButton(12, 120, 102, 148);
	Common::Rect lowerButton(12, 149, 102, 177);

	switch (_curBioChip) {
	case kItemBioChipAI:
		if (upperButton.contains(point) && ((GameUIWindow *)_parent)->_sceneViewWindow->playAIComment(AI_COMMENT_TYPE_HELP))
			invalidateWindow(false);

		if (lowerButton.contains(point) && ((GameUIWindow *)_parent)->_sceneViewWindow->playAIComment(AI_COMMENT_TYPE_INFORMATION))
			invalidateWindow(false);
		break;
	case kItemBioChipCloak:
		if (upperButton.contains(point)) {
			if (_status == 0) {
				_status = 1;

				// Disable navigation
				((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(0, 0, 0, 0, 0);

				VideoWindow *video = new VideoWindow(_vm, this);
				video->setWindowPos(nullptr, 2, 22, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosHideWindow);
				if (!video->openVideo(_vm->getFilePath(IDS_BC_CLOAKING_MOVIE_FILENAME))) {
					error("Failed to load cloaking video");
				}
				video->enableWindow(false);
				video->showWindow(kWindowShow);

				_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_BC_CLOAKING_SOUND_FILENAME));
				video->playToFrame(23);

				while (!_vm->shouldQuit() && video->getMode() != VideoWindow::kModeStopped) {
					_vm->yield(video, -1);
					_vm->_sound->timerCallback();
				}

				_vm->_sound->timerCallback();
				delete video;

				invalidateWindow(false);

				((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcCloakingEnabled = 1;
				((GameUIWindow *)_parent)->_inventoryWindow->enableWindow(false);
				((GameUIWindow *)_parent)->_sceneViewWindow->enableWindow(false);
				((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(false);

				Location currentLocation;
				((GameUIWindow *)_parent)->_sceneViewWindow->getCurrentSceneLocation(currentLocation);

				if (currentLocation.timeZone == 10)
					((GameUIWindow *)_parent)->_sceneViewWindow->displayLiveText(_vm->getString(IDS_CLOAK_BIOCHIP_AUTO_ACTIVATE));
				else
					((GameUIWindow *)_parent)->_sceneViewWindow->displayLiveText(_vm->getString(IDS_CLOAK_BIOCHIP_ACTIVATE));
			} else {
				_status = 0;

				VideoWindow *video = new VideoWindow(_vm, this);
				video->setWindowPos(nullptr, 2, 22, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosHideWindow);
				if (!video->openVideo(_vm->getFilePath(IDS_BC_CLOAKING_MOVIE_FILENAME))) {
					error("Failed to load cloaking video");
				}
				video->seekToFrame(24);
				video->enableWindow(false);
				video->showWindow(kWindowShow);

				_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_BC_CLOAKING_SOUND_FILENAME));
				video->playToFrame(47);

				while (!_vm->shouldQuit() && video->getMode() != VideoWindow::kModeStopped) {
					_vm->yield(video, -1);
					_vm->_sound->timerCallback();
				}

				_vm->_sound->timerCallback();
				delete video;

				invalidateWindow(false);

				((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcCloakingEnabled = 0;
				((GameUIWindow *)_parent)->_sceneViewWindow->resetNavigationArrows();
				((GameUIWindow *)_parent)->_inventoryWindow->enableWindow(true);
				((GameUIWindow *)_parent)->_sceneViewWindow->enableWindow(true);
				((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(true);
				((GameUIWindow *)_parent)->_sceneViewWindow->displayLiveText(_vm->getString(IDS_CLOAK_BIOCHIP_DEACTIVATE));
			}
		}
		break;
	case kItemBioChipEvidence:
		if (upperButton.contains(point)) {
			if (_status == 1) {
				_status = 0;
				((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcLocateEnabled = 0;
				invalidateWindow(false);
			} else {
				destroyBioChipViewWindow();
				_status = 1;
				((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcLocateEnabled = 1;
				invalidateWindow(false);
			}
		} else if (lowerButton.contains(point)) {
			if (_status == 2) {
				destroyBioChipViewWindow();
				_status = 0;
				invalidateWindow(false);
			} else {
				showBioChipMainView();
				_status = 2;
				((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcLocateEnabled = 0;
				invalidateWindow(false);
			}
		}
		break;
	case kItemBioChipFiles:
	case kItemBioChipInterface:
		if (upperButton.contains(point)) {
			toggleBioChip();
		}
		break;
	case kItemBioChipJump:
		if (upperButton.contains(point)) {
			toggleBioChip();
		} else if (lowerButton.contains(point)) {
			Location currentLocation;
			if (((GameUIWindow *)_parent)->_sceneViewWindow->getCurrentSceneLocation(currentLocation)) {
				if (currentLocation.timeZone != 4) {
					_status = 0;
					destroyBioChipViewWindow();
					((GameUIWindow *)_parent)->_inventoryWindow->destroyInfoWindow();
					((GameUIWindow *)_parent)->_inventoryWindow->destroyBurnedLetterWindow();
					invalidateWindow(false);
					((GameUIWindow *)_parent)->_sceneViewWindow->timeSuitJump(4);
				}
			}
		}
		break;
	case kItemBioChipTranslate:
		if (upperButton.contains(point)) {
			if (_status == 0) {
				_status = 1;
				invalidateWindow(false);

				((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcTranslateEnabled = 1;
				((GameUIWindow *)_parent)->_sceneViewWindow->invalidateWindow(false);
			} else {
				_status = 0;
				invalidateWindow(false);

				((GameUIWindow *)_parent)->_sceneViewWindow->getGlobalFlags().bcTranslateEnabled = 0;
				((GameUIWindow *)_parent)->_liveTextWindow->translateBiochipClosing();
				((GameUIWindow *)_parent)->_sceneViewWindow->invalidateWindow(false);
			}
		}
		break;
	}
}

} // End of namespace Buried
