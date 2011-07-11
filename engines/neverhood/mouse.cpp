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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "neverhood/mouse.h"

namespace Neverhood {

// TODO: Use CursorMan

Mouse433::Mouse433(NeverhoodEngine *vm, uint32 fileHash, NRect *mouseRect)
	: StaticSprite(vm, 2000), _mouseCursorResource(vm), _frameNum(0) {
	
	debug("Mouse433::Mouse433(%08X)", fileHash);
	
	if (mouseRect) {
		_mouseRect = *mouseRect;
	} else {
		_mouseRect.x1 = -1;
		_mouseRect.y1 = -1;
		_mouseRect.x2 = -1;
		_mouseRect.y2 = -1;
	}
	_mouseCursorResource.load(fileHash);
	_x = _vm->getMouseX();	
	_y = _vm->getMouseY();	
	if (_x >= _mouseRect.x1 && _x <= _mouseRect.x2 &&
		_y >= _mouseRect.y1 && _y <= _mouseRect.y2) {
		_mouseCursorResource.setCursorNum(1);
	} else {
		_mouseCursorResource.setCursorNum(4);
	}
	createSurface(2000, 32, 32);
	SetUpdateHandler(&Mouse433::update);
	SetMessageHandler(&Mouse433::handleMessage);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = 32;
	_drawRect.height = 32;
	_deltaRect.x = 0;
	_deltaRect.y = 0;
	_deltaRect.width = 32;
	_deltaRect.height = 32;
	processDelta();
	_needRefresh = true;
	updateCursor();
}

void Mouse433::load(uint32 fileHash) {
	_mouseCursorResource.load(fileHash);
	_needRefresh = true;
}

void Mouse433::update() {
	debug("Mouse433::update()");
	updateCursor();
	_frameNum++;
	if (_frameNum >= 6)
		_frameNum = 0;
	_needRefresh = _frameNum % 2 == 0;		
}

uint32 Mouse433::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Mouse433::handleMessage(%04X)", messageNum);
	uint32 messageResult = 0;
	if (messageNum != 5) {
		messageResult = Sprite::handleMessage(messageNum, param, sender);
		switch (messageNum) {
		case 0x4002:
			_x = param.asPoint().x;
			_y = param.asPoint().y;
			if (_x >= _mouseRect.x1 && _x <= _mouseRect.x2 &&
				_y >= _mouseRect.y1 && _y <= _mouseRect.y2) {
				_mouseCursorResource.setCursorNum(1);
			} else {
				_mouseCursorResource.setCursorNum(4);
			}
			processDelta();
			break;
		}
	} else {
		// TODO: Debug stuff
	}
	return messageResult;
}

void Mouse433::updateCursor() {
	debug("Mouse433::updateCursor()");
	
	if (!_surface)
		return;

	if (_doDeltaX) {
		_surface->getDrawRect().x = filterX(_x - _drawRect.width - _drawRect.x + 1);
	} else {
		_surface->getDrawRect().x = filterX(_x + _drawRect.x);
	}

	if (_doDeltaY) {
		_surface->getDrawRect().y = filterY(_y - _drawRect.height - _drawRect.y + 1);
	} else {
		_surface->getDrawRect().y = filterY(_y + _drawRect.y);
	}

	if (_needRefresh) {
		_needRefresh = false;
		_drawRect = _mouseCursorResource.getRect();
		_surface->drawMouseCursorResource(_mouseCursorResource, _frameNum / 2);
	}

}

Mouse435::Mouse435(NeverhoodEngine *vm, uint32 fileHash, int16 x1, int16 x2)
	: StaticSprite(vm, 2000), _mouseCursorResource(vm), _frameNum(0), _x1(x1), _x2(x2) {
	
	debug("Mouse435::Mouse435(%08X)", fileHash);
	
	_mouseCursorResource.load(fileHash);
	_x = _vm->getMouseX();	
	_y = _vm->getMouseY();
	if (_x <= _x1) {
		_mouseCursorResource.setCursorNum(6);
	} else if (_x >= _x2) {
		_mouseCursorResource.setCursorNum(5);
	} else {
		_mouseCursorResource.setCursorNum(4);
	}
	createSurface(2000, 32, 32);
	SetUpdateHandler(&Mouse435::update);
	SetMessageHandler(&Mouse435::handleMessage);
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = 32;
	_drawRect.height = 32;
	_deltaRect.x = 0;
	_deltaRect.y = 0;
	_deltaRect.width = 32;
	_deltaRect.height = 32;
	processDelta();
	_needRefresh = true;
	updateCursor();
}

void Mouse435::load(uint32 fileHash) {
	_mouseCursorResource.load(fileHash);
	_needRefresh = true;
}

void Mouse435::update() {
	debug("Mouse435::update()");
	updateCursor();
	_frameNum++;
	if (_frameNum >= 6)
		_frameNum = 0;
	_needRefresh = _frameNum % 2 == 0;		
}

uint32 Mouse435::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("Mouse435::handleMessage(%04X)", messageNum);
	uint32 messageResult = 0;
	if (messageNum != 5) {
		messageResult = Sprite::handleMessage(messageNum, param, sender);
		switch (messageNum) {
		case 0x4002:
			_x = param.asPoint().x;
			_y = param.asPoint().y;
			if (_x <= _x1) {
				_mouseCursorResource.setCursorNum(6);
			} else if (_x >= _x2) {
				_mouseCursorResource.setCursorNum(5);
			} else {
				_mouseCursorResource.setCursorNum(4);
			}
			processDelta();
			break;
		}
	} else {
		// TODO: Debug stuff
	}
	return messageResult;
}

void Mouse435::updateCursor() {
	debug("Mouse435::updateCursor()");
	
	if (!_surface)
		return;

	if (_doDeltaX) {
		_surface->getDrawRect().x = filterX(_x - _drawRect.width - _drawRect.x + 1);
	} else {
		_surface->getDrawRect().x = filterX(_x + _drawRect.x);
	}

	if (_doDeltaY) {
		_surface->getDrawRect().y = filterY(_y - _drawRect.height - _drawRect.y + 1);
	} else {
		_surface->getDrawRect().y = filterY(_y + _drawRect.y);
	}

	if (_needRefresh) {
		_needRefresh = false;
		_drawRect = _mouseCursorResource.getRect();
		_surface->drawMouseCursorResource(_mouseCursorResource, _frameNum / 2);
	}

}

} // End of namespace Neverhood
