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

#include "buried/buried.h"
#include "buried/biochip_right.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/livetext.h"
#include "buried/navarrow.h"
#include "buried/resources.h"
#include "buried/sound.h"
#include "buried/video_window.h"

#include "graphics/surface.h"

namespace Buried {

BioChipRightWindow::BioChipRightWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_curBioChip = kItemBioChipInterface;
	_status = 0;
	_bioChipViewWindow = 0;
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

	// TODO
	//if (bioChipID != kItemBioChipEvidence)
	//	;

	if (_bioChipViewWindow)
		destroyBioChipViewWindow();

	// TODO: Set the translate enabled flag to false

	invalidateWindow(false);
	return true;
}

bool BioChipRightWindow::showBioChipMainView() {
	if (_bioChipViewWindow)
		return false;

	// TODO: Notify the view of the change
	_vm->_sound->timerCallback();

	// TODO: Destroy info window
	// TODO: Destroy burned letter window
	_vm->_sound->timerCallback();

	// TODO: BioChip main view window (child to scene view)
	_vm->_sound->timerCallback();

	return true;
}

bool BioChipRightWindow::destroyBioChipViewWindow() {
	if (_bioChipViewWindow)
		return false;

	_vm->_sound->timerCallback();
	delete _bioChipViewWindow;
	_bioChipViewWindow = 0;
	_vm->_sound->timerCallback();

	// TODO: Signal the change to the scene view window

	if (_status == 1) {
		_status = 0;
		invalidateWindow(false);
	}

	return true;
}

void BioChipRightWindow::sceneChanged() {
	if (_curBioChip == kItemBioChipAI)
		invalidateWindow(false);
}

void BioChipRightWindow::disableEvidenceCapture() {
	if (_curBioChip == kItemBioChipEvidence) {
		_status = 0;
		// TODO: Disable locate flag
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
		// TODO: Check scene view for help/information comment
		bool helpComment = _forceHelp || false;
		bool information = _forceComment || false;

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
		bitmapResID = (_status == 0) ? 12 : 13;
		break;
	case kItemBioChipJump:
		// TODO
		bitmapResID = 14;
		break;
	case kItemBioChipTranslate:
		bitmapResID = (_status == 0) ? 18 : 19;
		break;
	}

	if (bitmapResID >= 0) {
		Graphics::Surface *bitmap = _vm->_gfx->getBitmap(IDB_BCR_BITMAP_BASE + bitmapResID);
		Common::Rect absoluteRect = getAbsoluteRect();
		_vm->_gfx->blit(bitmap, absoluteRect.left, absoluteRect.top);
		bitmap->free();
		delete bitmap;
	}
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
		// TODO
		break;
	case kItemBioChipCloak:
		if (upperButton.contains(point)) {
			if (_status == 0) {
				_status = 1;

				// Disable navigation
				((GameUIWindow *)_parent)->_navArrowWindow->updateAllArrows(0, 0, 0, 0, 0);

				VideoWindow *video = new VideoWindow(_vm, this);
				video->setWindowPos(0, 2, 22, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosHideWindow);
				video->openVideo(_vm->getFilePath(IDS_BC_CLOAKING_MOVIE_FILENAME));
				video->enableWindow(false);
				video->showWindow(kWindowShow);

				_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_BC_CLOAKING_SOUND_FILENAME));
				video->playToFrame(23);

				while (!_vm->shouldQuit() && video->getMode() != VideoWindow::kModeStopped) {
					_vm->yield();
					_vm->_sound->timerCallback();
				}

				_vm->_sound->timerCallback();
				delete video;

				invalidateWindow(false);

				// TODO: Set cloaking flag
				// TODO: Disable some controls
				((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(false);
				// TODO: Change live text
			} else {
				_status = 0;

				VideoWindow *video = new VideoWindow(_vm, this);
				video->setWindowPos(0, 2, 22, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosHideWindow);
				video->openVideo(_vm->getFilePath(IDS_BC_CLOAKING_MOVIE_FILENAME));
				video->seekToFrame(24);
				video->enableWindow(false);
				video->showWindow(kWindowShow);

				_vm->_sound->playInterfaceSound(_vm->getFilePath(IDS_BC_CLOAKING_SOUND_FILENAME));
				video->playToFrame(47);

				while (!_vm->shouldQuit() && video->getMode() != VideoWindow::kModeStopped) {
					_vm->yield();
					_vm->_sound->timerCallback();
				}

				_vm->_sound->timerCallback();
				delete video;

				invalidateWindow(false);

				// TODO: Set cloaking flag
				// TODO: Enable navigation
				// TODO: Enable inventory controls
				((GameUIWindow *)_parent)->_navArrowWindow->enableWindow(true);
				// TODO: Change live text
			}
		}
		break;
	case kItemBioChipEvidence:
		if (upperButton.contains(point)) {
			if (_status == 1) {
				_status = 0;
				// TODO
				invalidateWindow(false);
			} else {
				destroyBioChipViewWindow();
				_status = 1;
				// TODO
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
				// TODO
				invalidateWindow(false);
			}
		}
		break;
	case kItemBioChipFiles:
		if (upperButton.contains(point)) {
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
		break;
	case kItemBioChipInterface:
		if (upperButton.contains(point)) {
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
		break;
	case kItemBioChipJump:
		if (upperButton.contains(point)) {
			if (_status == 0) {
				_status = 1;
				showBioChipMainView();
				invalidateWindow(false);
			} else {
				_status = 0;
				destroyBioChipViewWindow();
				invalidateWindow(false);
			}
		} else if (lowerButton.contains(point)) {
			// TODO
		}
		break;
	case kItemBioChipTranslate:
		if (upperButton.contains(point)) {
			if (_status == 0) {
				_status = 1;
				invalidateWindow(false);

				// TODO: Reset global flag
				// TODO: Redraw the scene window
			} else {
				_status = 0;

				// TODO: Reset global flag
				// TODO: Redraw the scene window
			}
		}
		break;
	}
}

} // End of namespace Buried
