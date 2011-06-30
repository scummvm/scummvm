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

#include "neverhood/sprite.h"

namespace Neverhood {

// Sprite

Sprite::Sprite(NeverhoodEngine *vm, int objectPriority)
	: Entity(vm, objectPriority), _x(0), _y(0),
	_spriteUpdateCb(NULL), _filterXCb(NULL), _filterYCb(NULL),   
	_doDeltaX(false), _doDeltaY(false), _needRedraw(false),
	_deltaX1(0), _deltaY1(0), _deltaX2(0), _deltaY2(0),
	_flags(0) {

	SetMessageHandler(&Sprite::handleMessage);
	
}

Sprite::~Sprite() {
	delete _surface;
}

void Sprite::processDelta() {
	if (_doDeltaX) {
		_rect.x1 = _x - _deltaX1 - _deltaX2 + 1;
		_rect.x2 = _x - _deltaX1;
	} else {
		_rect.x1 = _x + _deltaX1;
		_rect.x2 = _x + _deltaX1 + _deltaX2 - 1;
	}
	if (_doDeltaY) {
		_rect.y1 = _y - _deltaY1 - _deltaY2 + 1;
		_rect.y2 = _y - _deltaY1;
	} else {
		_rect.y1 = _y + _deltaY1;
		_rect.y2 = _y + _deltaY1 + _deltaY2 - 1;
	}
}

void Sprite::setDoDeltaX(int type) {
	// Clear, set or toggle
	_doDeltaX = type == 2 ? !_doDeltaX : type == 1;
}

void Sprite::setDoDeltaY(int type) {
	// Clear, set or toggle
	_doDeltaY = type == 2 ? !_doDeltaY : type == 1;
}

bool Sprite::isPointInside(int16 x, int16 y) {
	return x >= _rect.x1 && x <= _rect.x2 && y >= _rect.y1 && y <= _rect.y2;
}

uint32 Sprite::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	if (messageNum == 5) {
		// TODO: Draw debug marker (?)
		// TODO g_Screen->drawLine(_x - 5, _y, _x + 6, _y);
		// TODO g_Screen->drawLine(_x, _y - 5, _x, _y + 6);
	}
	return 0;
}

void Sprite::createSurface(int surfacePriority, int16 width, int16 height) {
	_surface = new BaseSurface(_vm, surfacePriority, width, height);
}

// StaticSprite

StaticSprite::StaticSprite(NeverhoodEngine *vm, int objectPriority)
	: Sprite(vm, objectPriority), _spriteResource(vm) {
	
}

StaticSprite::StaticSprite(NeverhoodEngine *vm, const char *filename, int surfacePriority, int16 x, int16 y, int16 width, int16 height)
	: Sprite(vm, 0), _spriteResource(vm) {

	// TODO init(calcHash(filename), surfacePriority, x, y, width, height);

}

StaticSprite::StaticSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x, int16 y, int16 width, int16 height)
	: Sprite(vm, 0), _spriteResource(vm) {

	init(fileHash, surfacePriority, x, y, width, height);
	
}

void StaticSprite::init(uint32 fileHash, int surfacePriority, int16 x, int16 y, int16 width, int16 height) {

	_spriteResource.load2(fileHash);

	if (width == 0)
		width = _spriteResource.getDimensions().width;

	if (height == 0)
		height = _spriteResource.getDimensions().height;

	createSurface(surfacePriority, width, height);

	_x = x == kDefPosition ? _spriteResource.getPosition().x : x;
	_y = y == kDefPosition ? _spriteResource.getPosition().y : y;

	_rect1.x1 = 0;
	_rect1.y1 = 0;
	_rect1.x2 = width;
	_rect1.y2 = height; 

	_needRedraw = true;

	update();
	
}

void StaticSprite::update() {

	if (!_surface)
		return;
		
	if (_doDeltaX) {
		_x = filterX(_x - _rect1.x1 - _rect1.x2 + 1);
	} else {
		_x = filterX(_x + _rect1.x1);
	}
		
	if (_doDeltaY) {
		_y = filterY(_y - _rect1.y1 - _rect1.y2 + 1);
	} else {
		_y = filterY(_y + _rect1.y1);
	}

	if (_needRedraw) {
		// TODO _surface->drawSpriteResourceEx(_spriteResource, _doDeltaX, _doDeltaY, _rect1.x2, _rect1.y2);
		_needRedraw = false;
	}

}

void StaticSprite::load(uint32 fileHash, bool dimensions, bool position) {

	_spriteResource.load2(fileHash);

	if (dimensions) {
		_rect1.x1 = 0;
		_rect1.y1 = 0;
		_rect1.x2 = _spriteResource.getDimensions().width;
		_rect1.y2 = _spriteResource.getDimensions().height;
	}

	if (position) {
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
	}

	_needRedraw = true;

}

} // End of namespace Neverhood
