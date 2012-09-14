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
	: Entity(vm, objectPriority), _x(0), _y(0), _spriteUpdateCb(NULL), _filterXCb(NULL), _filterYCb(NULL),   
	_dataResource(vm), _doDeltaX(false), _doDeltaY(false), _needRefresh(false), _flags(0), _surface(NULL) {

	_name = "Sprite"; 
	SetMessageHandler(&Sprite::handleMessage);
	
}

Sprite::~Sprite() {
	delete _surface;
}

void Sprite::processDelta() {
	if (_doDeltaX) {
		_rect.x1 = _x - _deltaRect.x - _deltaRect.width + 1;
		_rect.x2 = _x - _deltaRect.x;
	} else {
		_rect.x1 = _x + _deltaRect.x;
		_rect.x2 = _x + _deltaRect.x + _deltaRect.width - 1;
	}
	if (_doDeltaY) {
		_rect.y1 = _y - _deltaRect.y - _deltaRect.height + 1;
		_rect.y2 = _y - _deltaRect.y;
	} else {
		_rect.y1 = _y + _deltaRect.y;
		_rect.y2 = _y + _deltaRect.y + _deltaRect.height - 1;
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

bool Sprite::checkCollision(NRect &rect) {
	return (_rect.x1 < rect.x2) && (rect.x1 < _rect.x2) && (_rect.y1 < rect.y2) && (rect.y1 < _rect.y2);	
}

uint32 Sprite::handleMessage(int messageNum, const MessageParam &param, Entity *sender) {
	switch (messageNum) {
	case 0x0005:
		// TODO: Draw debug marker (?)
		// TODO g_Screen->drawLine(_x - 5, _y, _x + 6, _y);
		// TODO g_Screen->drawLine(_x, _y - 5, _x, _y + 6);
		break;
	}
	return 0;
}

void Sprite::loadDataResource(uint32 fileHash) {
	_dataResource.load(fileHash);
}

void Sprite::createSurface(int surfacePriority, int16 width, int16 height) {
	_surface = new BaseSurface(_vm, surfacePriority, width, height);
}

int16 Sprite::defFilterY(int16 y) {
	// TODO return y - g_screen->field_26;
	return y;
}

void Sprite::setClipRect(int16 x1, int16 y1, int16 x2, int16 y2) {
	NRect &clipRect = _surface->getClipRect();
	clipRect.x1 = x1;
	clipRect.y1 = y1;
	clipRect.x2 = x2;
	clipRect.y2 = y2;
}

void Sprite::setClipRect(NRect& clipRect) {
	_surface->getClipRect() = clipRect;
}

void Sprite::setClipRect(NDrawRect& drawRect) {
	setClipRect(drawRect.x, drawRect.y, drawRect.x2(), drawRect.y2());
}

// StaticSprite

StaticSprite::StaticSprite(NeverhoodEngine *vm, int objectPriority)
	: Sprite(vm, objectPriority), _spriteResource(vm) {

	_name = "StaticSprite"; 

}

StaticSprite::StaticSprite(NeverhoodEngine *vm, const char *filename, int surfacePriority, int16 x, int16 y, int16 width, int16 height)
	: Sprite(vm, 0), _spriteResource(vm) {

	_name = "StaticSprite"; 
	init(calcHash(filename), surfacePriority, x, y, width, height);

}

StaticSprite::StaticSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x, int16 y, int16 width, int16 height)
	: Sprite(vm, 0), _spriteResource(vm) {
	_name = "StaticSprite"; 
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
	
	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = width;
	_drawRect.height = height; 

	_needRefresh = true;

	update();
	
}

void StaticSprite::update() {

	if (!_surface)
		return;
		
	if (_doDeltaX) {
		_surface->getDrawRect().x = filterX(_x - _drawRect.x - _drawRect.width + 1);
	} else {
		_surface->getDrawRect().x = filterX(_x + _drawRect.x);
	}
		
	if (_doDeltaY) {
		_surface->getDrawRect().y = filterY(_y - _drawRect.y - _drawRect.height + 1);
	} else {
		_surface->getDrawRect().y = filterY(_y + _drawRect.y);
	}

	if (_needRefresh) {
		_surface->drawSpriteResourceEx(_spriteResource, _doDeltaX, _doDeltaY, _drawRect.width, _drawRect.height);
		_needRefresh = false;
	}

}

void StaticSprite::load(uint32 fileHash, bool dimensions, bool position) {

	_spriteResource.load2(fileHash);

	if (dimensions) {
		_drawRect.x = 0;
		_drawRect.y = 0;
		_drawRect.width = _spriteResource.getDimensions().width;
		_drawRect.height = _spriteResource.getDimensions().height;
	}

	if (position) {
		_x = _spriteResource.getPosition().x;
		_y = _spriteResource.getPosition().y;
	}

	_needRefresh = true;

}

// AnimatedSprite

AnimatedSprite::AnimatedSprite(NeverhoodEngine *vm, int objectPriority)
	: Sprite(vm, objectPriority), _animResource(vm) {

	init();
}

AnimatedSprite::AnimatedSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x, int16 y)
	: Sprite(vm, 1100), _animResource(vm) {

	init();
	SetUpdateHandler(&AnimatedSprite::update);
	createSurface1(fileHash, surfacePriority);
	_x = x;
	_y = y;
	startAnimation(fileHash, 0, -1);
}

void AnimatedSprite::init() {
	_name = "AnimatedSprite"; 
	_currFrameTicks = 0;
	_newAnimFileHash = 0;
	_deltaX = 0;
	_deltaY = 0;
	_nextAnimFileHash = 0;
	// TODO _callbackList = 0;
	_plFirstFrameIndex = 0;
	_currFrameIndex = 0;
	_currStickFrameIndex = -1;
	_finalizeStateCb = NULL;
	_currStateCb = NULL;
	_nextStateCb = NULL;
	_newStickFrameIndex = -1;
	_newStickFrameHash = 0;
	_frameChanged = false;
	_replOldColor = 0;
	_replNewColor = 0;
	_animResource.setReplEnabled(false);
	_playBackwards = false;
}

void AnimatedSprite::update() {
	updateAnim();
	handleSpriteUpdate();
	updatePosition();
}

void AnimatedSprite::updateDeltaXY() {
	if (_doDeltaX) {
		_x -= _deltaX;
	} else {
		_x += _deltaX;
	}
	if (_doDeltaY) {
		_y -= _deltaY;
	} else {
		_y += _deltaY;
	}
	_deltaX = 0;
	_deltaY = 0;
	processDelta();
}

void AnimatedSprite::setRepl(byte oldColor, byte newColor) {
	_replOldColor = oldColor;
	_replNewColor = newColor;
	_animResource.setReplEnabled(true);
}

void AnimatedSprite::clearRepl() {
	_replOldColor = 0;
	_replNewColor = 0;
	_animResource.setReplEnabled(false);
}

void AnimatedSprite::updateAnim() {

	_frameChanged = false;

	if (_newAnimFileHash == 0) {
		if (_newStickFrameIndex != -1) {
			_currStickFrameIndex = _newStickFrameIndex == -2 ? _animResource.getFrameCount() - 1 : _newStickFrameIndex;
			_newStickFrameIndex = -1;
		} else if (_newStickFrameHash != 0) {
			_currStickFrameIndex = MAX<int16>(0, _animResource.getFrameIndex(_newStickFrameHash));
			_newStickFrameHash = 0;
		}
		if (_newAnimFileHash == 0 && _currFrameIndex != _currStickFrameIndex) {
			if (_currFrameTicks != 0 && (--_currFrameTicks == 0) && _animResource.getFrameCount() != 0) {
				
				if (_nextAnimFileHash != 0) {
					if (_animResource.loadInternal(_nextAnimFileHash)) {
						_currAnimFileHash = _nextAnimFileHash;
					} else {
						_animResource.loadInternal(calcHash("sqDefault"));
						_currAnimFileHash = 0;
					}
					if (_replOldColor != _replNewColor) {
						_animResource.setRepl(_replOldColor, _replNewColor);	
					}
					_nextAnimFileHash = 0;
					if (_animStatus != 0) {
						_currFrameIndex = _plFirstFrameHash != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_plFirstFrameHash)) : 0;
						_lastFrameIndex = _plLastFrameHash != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_plLastFrameHash)) : _animResource.getFrameCount() - 1;
					} else {
						_currFrameIndex = _plFirstFrameIndex != -1 ? _plFirstFrameIndex : _animResource.getFrameCount() - 1;
						_lastFrameIndex = _plLastFrameIndex != -1 ? _plLastFrameIndex : _animResource.getFrameCount() - 1; 
					}
				} else {
					updateFrameIndex();
				}
				if (_newAnimFileHash == 0)
					updateFrameInfo();
			}				
		}
	}
	
	if (_newAnimFileHash != 0) {
		if (_animStatus == 2) {
			_currStickFrameIndex = _currFrameIndex;
		} else {
			if (_animStatus == 1) {
				if (_animResource.loadInternal(_newAnimFileHash)) {
					_currAnimFileHash = _newAnimFileHash;
				} else {
					_animResource.loadInternal(calcHash("sqDefault"));
					_currAnimFileHash = 0;
				}
				if (_replOldColor != _replNewColor) {
					_animResource.setRepl(_replOldColor, _replNewColor);	
				}
				_newAnimFileHash = 0;
				_currFrameIndex = _plFirstFrameHash != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_plFirstFrameHash)) : 0;
				_lastFrameIndex = _plLastFrameHash != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_plLastFrameHash)) : _animResource.getFrameCount() - 1;
			} else {
				if (_animResource.loadInternal(_newAnimFileHash)) {
					_currAnimFileHash = _newAnimFileHash;
				} else {
					_animResource.loadInternal(calcHash("sqDefault"));
					_currAnimFileHash = 0;
				}
				if (_replOldColor != _replNewColor) {
					_animResource.setRepl(_replOldColor, _replNewColor);	
				}
				_newAnimFileHash = 0;
				_currFrameIndex = _plFirstFrameIndex != -1 ? _plFirstFrameIndex : _animResource.getFrameCount() - 1;
				_lastFrameIndex = _plLastFrameIndex != -1 ? _plLastFrameIndex : _animResource.getFrameCount() - 1;
			}
			updateFrameInfo();
		}

		if (_newStickFrameIndex != -1) {
			_currStickFrameIndex = _newStickFrameIndex == -2 ? _animResource.getFrameCount() - 1 : _newStickFrameIndex;
			_newStickFrameIndex = -1;
		} else if (_newStickFrameHash != 0) {
			_currStickFrameIndex = MAX<int16>(0, _animResource.getFrameIndex(_newStickFrameHash));
			_newStickFrameHash = 0;
		}

	}

}

void AnimatedSprite::updatePosition() {

	if (!_surface)
		return;

	if (_doDeltaX) {
		_surface->getDrawRect().x = filterX(_x - _drawRect.x - _drawRect.width + 1);
	} else {
		_surface->getDrawRect().x = filterX(_x + _drawRect.x);
	}

	if (_doDeltaY) {
		_surface->getDrawRect().y = filterY(_y - _drawRect.y - _drawRect.height + 1);
	} else {
		_surface->getDrawRect().y = filterY(_y + _drawRect.y);
	}

	if (_needRefresh) {
		_surface->drawAnimResource(_animResource, _currFrameIndex, _doDeltaX, _doDeltaY, _drawRect.width, _drawRect.height);
		_needRefresh = false;
	}

}

void AnimatedSprite::updateFrameIndex() {
	if (!_playBackwards) {
		if (_currFrameIndex < _lastFrameIndex) {
			_currFrameIndex++;
		} else {
			// Inform self about end of current animation
			// The caller can then e.g. set a new animation fileHash
			sendMessage(this, 0x3002, 0);
			if (_newAnimFileHash == 0)
				_currFrameIndex = 0;
		}
	} else {
		if (_currFrameIndex > 0) {
			_currFrameIndex--;
		} else {
			sendMessage(this, 0x3002, 0);
			if (_newAnimFileHash == 0)
				_currFrameIndex = _lastFrameIndex;
		}
	}
}

void AnimatedSprite::updateFrameInfo() {
	debug(8, "AnimatedSprite::updateFrameInfo()");
	const AnimFrameInfo &frameInfo = _animResource.getFrameInfo(_currFrameIndex);
	_frameChanged = true;
	_drawRect = frameInfo.rect;
	_deltaX = frameInfo.deltaX;
	_deltaY = frameInfo.deltaY;
	_deltaRect = frameInfo.deltaRect;
	_currFrameTicks = frameInfo.counter;
	processDelta();
	_needRefresh = true;
	if (frameInfo.frameHash != 0) {
		sendMessage(this, 0x100D, frameInfo.frameHash);
	}
}

void AnimatedSprite::createSurface1(uint32 fileHash, int surfacePriority) {
	NDimensions dimensions = _animResource.loadSpriteDimensions(fileHash);
	_surface = new BaseSurface(_vm, surfacePriority, dimensions.width, dimensions.height);
}

void AnimatedSprite::createShadowSurface1(BaseSurface *shadowSurface, uint32 fileHash, int surfacePriority) {
	NDimensions dimensions = _animResource.loadSpriteDimensions(fileHash);
	_surface = new ShadowSurface(_vm, surfacePriority, dimensions.width, dimensions.height, shadowSurface);
}

void AnimatedSprite::createShadowSurface(BaseSurface *shadowSurface, int16 width, int16 height, int surfacePriority) {
	_surface = new ShadowSurface(_vm, surfacePriority, width, height, shadowSurface);
}

void AnimatedSprite::startAnimation(uint32 fileHash, int16 plFirstFrameIndex, int16 plLastFrameIndex) {
	debug(2, "AnimatedSprite::startAnimation(%08X, %d, %d)", fileHash, plFirstFrameIndex, plLastFrameIndex);
	_newAnimFileHash = fileHash;
	_plFirstFrameIndex = plFirstFrameIndex;
	_plLastFrameIndex = plLastFrameIndex;
	_newStickFrameHash = 0;
	_animStatus = 0;
	_playBackwards = false;
	_newStickFrameIndex = -1;
	_currStickFrameIndex = -1;
}

void AnimatedSprite::stopAnimation() {
	_newAnimFileHash = 1;
	_animStatus = 2;
}

void AnimatedSprite::startAnimationByHash(uint32 fileHash, uint32 plFirstFrameHash, uint32 plLastFrameHash) {
	debug(2, "AnimatedSprite::startAnimationByHash(%08X, %08X, %08X)", fileHash, plFirstFrameHash, plLastFrameHash);
	_newAnimFileHash = fileHash;
	_plFirstFrameHash = plFirstFrameHash;
	_plLastFrameHash = plLastFrameHash;
	_newStickFrameHash = 0;
	_animStatus = 1;
	_playBackwards = false;
	_newStickFrameIndex = -1;
	_currStickFrameIndex = -1;
}

void AnimatedSprite::nextAnimationByHash(uint32 fileHash2, uint32 plFirstFrameHash, uint32 plLastFrameHash) {
	_nextAnimFileHash = fileHash2;
	_plFirstFrameHash = plFirstFrameHash;
	_plLastFrameHash = plLastFrameHash;
	_newStickFrameHash = 0;
	_animStatus = 1;
	_playBackwards = false;
	_newStickFrameIndex = -1;
	_currStickFrameIndex = -1;
}

void AnimatedSprite::setFinalizeState(AnimationCb finalizeStateCb) {
	if (_finalizeStateCb)
		(this->*_finalizeStateCb)();
	_finalizeStateCb = finalizeStateCb;
}

void AnimatedSprite::gotoState(AnimationCb currStateCb) {
	if (_finalizeStateCb) {
		AnimationCb cb = _finalizeStateCb;
		_finalizeStateCb = NULL;
		(this->*cb)();
	}
	// TODO _callbackList = NULL;
	_nextStateCb = NULL;
	_currStateCb = currStateCb;
	if (_currStateCb)
		(this->*_currStateCb)();
}

void AnimatedSprite::gotoNextState() {
	if (_finalizeStateCb) {
		AnimationCb cb = _finalizeStateCb;
		_finalizeStateCb = NULL;
		(this->*cb)();
	}
	if (_nextStateCb) {
		_currStateCb = _nextStateCb;
		_nextStateCb = NULL;
		//debug("Fire _nextStateCb '%s'", _nextStateCbName.c_str());
		(this->*_currStateCb)();
#if 0 // TODO		
	} else if (_callbackList) {
		removeCallbackList();
#endif		
	} else {
		_currStateCb = NULL;
	}
}

} // End of namespace Neverhood
