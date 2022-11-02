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
#include "buried/graphics.h"
#include "buried/message.h"
#include "buried/demo/features.h"

#include "graphics/surface.h"

namespace Buried {

FeaturesDisplayWindow::FeaturesDisplayWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	Common::Rect parentRect = _parent->getClientRect();
	_rect.left = (parentRect.right - 640) / 2;
	_rect.top = (parentRect.bottom - 480) / 2;
	_rect.right = parentRect.left + 640;
	_rect.bottom = parentRect.top + 480;

	_curBackground = 0;
	_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "MISC/24BPP/FEATURE1.BMP" : "MISC/8BPP/FEATURE1.BMP");
}

FeaturesDisplayWindow::~FeaturesDisplayWindow() {
	if (_background) {
		_background->free();
		delete _background;
	}
}

void FeaturesDisplayWindow::onPaint() {
	_vm->_gfx->blit(_background, 0, 0);
}

bool FeaturesDisplayWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void FeaturesDisplayWindow::onLButtonUp(const Common::Point &point, uint flags) {
	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_parent);

	_curBackground++;
	if (_background) {
		_background->free();
		delete _background;
		_background = nullptr;
	}

	switch (_curBackground) {
	case 1:
		_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "MISC/24BPP/FEATURE2.BMP" : "MISC/8BPP/FEATURE2.BMP");
		break;
	case 2:
		_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "MISC/24BPP/FEATURE3.BMP" : "MISC/8BPP/FEATURE3.BMP");
		break;
	case 3:
		_background = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "MISC/24BPP/CLOSING.BMP" : "MISC/8BPP/CLOSING.BMP");
		break;
	case 4:
		_vm->quitGame();
		return;
	}

	invalidateWindow(false);
	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_parent);
}

} // End of namespace Buried
