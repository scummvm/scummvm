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

Window *BioChipMainViewWindow::createBioChipSpecificViewWindow(int bioChipID) {
	switch (bioChipID) {
	case kItemBioChipInterface:
		// TODO
		break;
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
