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

#include "buried/biochip_view.h"
#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/resources.h"

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

	// TODO
	if (true)
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
		// TODO
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
		// TODO
		break;
	}

	// No entry for this BioChip
	return 0;
}

} // End of namespace Buried
