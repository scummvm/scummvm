/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/mouse.h"
#include "graphics/cursorman.h"

namespace Neverhood {

Mouse::Mouse(NeverhoodEngine *vm, uint32 fileHash, const NRect &mouseRect)
	: StaticSprite(vm, 2000), _mouseType(kMouseType433),
	_mouseCursorResource(vm), _frameNum(0) {

	_mouseRect = mouseRect;
	init(fileHash);
	if (_x >= _mouseRect.x1 && _x <= _mouseRect.x2 &&
		_y >= _mouseRect.y1 && _y <= _mouseRect.y2) {
		_mouseCursorResource.setCursorNum(1);
	} else {
		_mouseCursorResource.setCursorNum(4);
	}
	updateCursor();
}

Mouse::Mouse(NeverhoodEngine *vm, uint32 fileHash, int16 x1, int16 x2)
	: StaticSprite(vm, 2000), _mouseType(kMouseType435),
	_mouseCursorResource(vm), _frameNum(0), _x1(x1), _x2(x2) {

	init(fileHash);
	if (_x <= _x1) {
		_mouseCursorResource.setCursorNum(6);
	} else if (_x >= _x2) {
		_mouseCursorResource.setCursorNum(5);
	} else {
		_mouseCursorResource.setCursorNum(4);
	}
	updateCursor();
}

Mouse::Mouse(NeverhoodEngine *vm, uint32 fileHash, int type)
	: StaticSprite(vm, 2000), _mouseType(kMouseTypeNavigation),
	_mouseCursorResource(vm), _type(type), _frameNum(0) {

	init(fileHash);
	_mouseCursorResource.setCursorNum(0);
}

Mouse::~Mouse() {
	CursorMan.showMouse(false);
}

void Mouse::init(uint32 fileHash) {
	_mouseCursorResource.load(fileHash);
	_x = _vm->getMouseX();
	_y = _vm->getMouseY();
	createSurface(2000, 32, 32);
	SetUpdateHandler(&Mouse::update);
	SetMessageHandler(&Mouse::handleMessage);
	_drawOffset.x = 0;
	_drawOffset.y = 0;
	_drawOffset.width = 32;
	_drawOffset.height = 32;
	_collisionBoundsOffset = _drawOffset;
	updateBounds();
	_needRefresh = true;
	CursorMan.showMouse(false);
}

void Mouse::load(uint32 fileHash) {
	_mouseCursorResource.load(fileHash);
	_needRefresh = true;
}

void Mouse::update() {
	if (CursorMan.isVisible() && !_surface->getVisible()) {
		CursorMan.showMouse(false);
	} else if (!CursorMan.isVisible() && _surface->getVisible()) {
		CursorMan.showMouse(true);
	}
	updateCursor();
	_frameNum++;
	if (_frameNum >= 6)
		_frameNum = 0;
	_needRefresh = _frameNum % 2 == 0;
}

uint32 Mouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug(7, "Mouse::handleMessage(%04X)", messageNum);
	uint32 messageResult = Sprite::handleMessage(messageNum, param, sender);
	switch (messageNum) {
	case 0x2064:
		_x = param.asPoint().x;
		_y = param.asPoint().y;
		switch (_type) {
		case 1:
			if (_x >= 320)
				messageResult = 1;
			else
				messageResult = 0;
			break;
		case 2:
		default:
			if (_x < 100)
				messageResult = 0;
			else if (_x > 540)
				messageResult = 1;
			else
				messageResult = 2;
			break;
		case 3:
			if (_x < 100)
				messageResult = 0;
			else if (_x > 540)
				messageResult = 1;
			else
				messageResult = 4;
			break;
		case 4:
			if (_x < 100)
				messageResult = 0;
			else if (_x > 540)
				messageResult = 1;
			else if (_y >= 150)
				messageResult = 2;
			else
				messageResult = 3;
			break;
		case 5:
			if (_y >= 240)
				messageResult = 4;
			else
				messageResult = 3;
			break;
		}
		break;
	case 0x4002:
		_x = param.asPoint().x;
		_y = param.asPoint().y;
		updateCursorNum();
		updateBounds();
		break;
	default:
		break;
	}
	return messageResult;
}

void Mouse::updateCursor() {

	if (!_surface)
		return;

	if (_doDeltaX) {
		_surface->getDrawRect().x = filterX(_x - _drawOffset.width - _drawOffset.x + 1);
	} else {
		_surface->getDrawRect().x = filterX(_x + _drawOffset.x);
	}

	if (_doDeltaY) {
		_surface->getDrawRect().y = filterY(_y - _drawOffset.height - _drawOffset.y + 1);
	} else {
		_surface->getDrawRect().y = filterY(_y + _drawOffset.y);
	}

	if (_needRefresh) {
		_needRefresh = false;
		_drawOffset = _mouseCursorResource.getRect();
		_surface->drawMouseCursorResource(_mouseCursorResource, _frameNum / 2);
		Graphics::Surface *cursorSurface = _surface->getSurface();
		CursorMan.replaceCursor((const byte*)cursorSurface->getPixels(),
			cursorSurface->w, cursorSurface->h, -_drawOffset.x, -_drawOffset.y, 0);
	}

}

void Mouse::updateCursorNum() {
	switch (_mouseType) {
	case kMouseType433:
		if (_x >= _mouseRect.x1 && _x <= _mouseRect.x2 &&
			_y >= _mouseRect.y1 && _y <= _mouseRect.y2) {
			_mouseCursorResource.setCursorNum(1);
		} else {
			_mouseCursorResource.setCursorNum(4);
		}
		break;
	case kMouseType435:
		if (_x <= _x1) {
			_mouseCursorResource.setCursorNum(6);
		} else if (_x >= _x2) {
			_mouseCursorResource.setCursorNum(5);
		} else {
			_mouseCursorResource.setCursorNum(4);
		}
		break;
	case kMouseTypeNavigation:
		switch (_type) {
		case 1:
			if (_x >= 320)
				_mouseCursorResource.setCursorNum(5);
			else
				_mouseCursorResource.setCursorNum(6);
			break;
		case 2:
		default:
			if (_x < 100)
				_mouseCursorResource.setCursorNum(6);
			else if (_x > 540)
				_mouseCursorResource.setCursorNum(5);
			else
				_mouseCursorResource.setCursorNum(0);
			break;
		case 3:
			if (_x < 100)
				_mouseCursorResource.setCursorNum(1);
			else if (_x > 540)
				_mouseCursorResource.setCursorNum(1);
			break;
		case 4:
			if (_x < 100)
				_mouseCursorResource.setCursorNum(6);
			else if (_x > 540)
				_mouseCursorResource.setCursorNum(5);
			else if (_y >= 150)
				_mouseCursorResource.setCursorNum(0);
			else
				_mouseCursorResource.setCursorNum(3);
			break;
		case 5:
			if (_y >= 240)
				_mouseCursorResource.setCursorNum(2);
			else
				_mouseCursorResource.setCursorNum(3);
			break;
		}
		break;
	default:
		break;
	}

}

} // End of namespace Neverhood
