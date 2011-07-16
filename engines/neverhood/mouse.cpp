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
	
	debug(7, "Mouse433::Mouse433(%08X)", fileHash);
	
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
	debug(7, "Mouse433::update()");
	updateCursor();
	_frameNum++;
	if (_frameNum >= 6)
		_frameNum = 0;
	_needRefresh = _frameNum % 2 == 0;		
}

uint32 Mouse433::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug(7, "Mouse433::handleMessage(%04X)", messageNum);
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
	debug(7, "Mouse433::updateCursor()");
	
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
	
	debug(7, "Mouse435::Mouse435(%08X)", fileHash);
	
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
	debug(7, "Mouse435::update()");
	updateCursor();
	_frameNum++;
	if (_frameNum >= 6)
		_frameNum = 0;
	_needRefresh = _frameNum % 2 == 0;		
}

uint32 Mouse435::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug(7, "Mouse435::handleMessage(%04X)", messageNum);
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
	debug(7, "Mouse435::updateCursor()");
	
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

// NavigationMouse

NavigationMouse::NavigationMouse(NeverhoodEngine *vm, uint32 fileHash, int type)
	: StaticSprite(vm, 2000), _mouseCursorResource(vm), _type(type), _frameNum(0) {

	_mouseCursorResource.load(fileHash);
	_mouseCursorResource.setCursorNum(0);
	_x = _vm->getMouseX();	
	_y = _vm->getMouseY();
	createSurface(2000, 32, 32);
	SetUpdateHandler(&NavigationMouse::update);
	SetMessageHandler(&NavigationMouse::handleMessage);
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

void NavigationMouse::update() {
	updateCursor();
	_frameNum++;
	if (_frameNum >= 6)
		_frameNum = 0;
	_needRefresh = _frameNum % 2 == 0;		
}

uint32 NavigationMouse::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug("NavigationMouse: _type = %d", _type);
	uint32 messageResult = 0;
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
		_needRefresh = true;
		processDelta();
		break;
	}
	return messageResult;
}

void NavigationMouse::updateCursor() {

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
