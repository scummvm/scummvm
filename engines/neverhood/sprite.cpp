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
	_doDeltaX(false), _doDeltaY(false), _needRefresh(false),
	_flags(0) {

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

void Sprite::createSurface(int surfacePriority, int16 width, int16 height) {
	_surface = new BaseSurface(_vm, surfacePriority, width, height);
}

// StaticSprite

StaticSprite::StaticSprite(NeverhoodEngine *vm, int objectPriority)
	: Sprite(vm, objectPriority), _spriteResource(vm) {

	_name = "StaticSprite"; 

}

StaticSprite::StaticSprite(NeverhoodEngine *vm, const char *filename, int surfacePriority, int16 x, int16 y, int16 width, int16 height)
	: Sprite(vm, 0), _spriteResource(vm) {

	_name = "StaticSprite"; 
	// TODO init(calcHash(filename), surfacePriority, x, y, width, height);

}

StaticSprite::StaticSprite(NeverhoodEngine *vm, uint32 fileHash, int surfacePriority, int16 x, int16 y, int16 width, int16 height)
	: Sprite(vm, 0), _spriteResource(vm) {
	
	debug("StaticSprite::StaticSprite(%08X)", fileHash);

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
	
	debug("StaticSprite::init() final: x = %d; y = %d", _x, _y);

	_drawRect.x = 0;
	_drawRect.y = 0;
	_drawRect.width = width;
	_drawRect.width = height; 

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
	setFileHash(fileHash, 0, -1);
	
}

void AnimatedSprite::init() {
	_name = "AnimatedSprite"; 
	_counter = 0;
	_fileHash1 = 0;
	_deltaX = 0;
	_deltaY = 0;
	_fileHash2 = 0;
	// TODO _callbackList = 0;
	_frameIndex3 = 0;
	_frameIndex = 0;
	_hashListIndex = -1;
	_callback1Cb = NULL;
	_callback2Cb = NULL;
	_callback3Cb = NULL;
	_newHashListIndex = -1;
	_fileHash4 = 0;
	_flag = false;
	_replOldByte = 0;
	_replNewByte = 0;
	// TODO _animResource.replEnabled = 0;
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

void AnimatedSprite::updateAnim() {

	_flag = false;

	if (_fileHash1 == 0) {
		if (_newHashListIndex != -1) {
			_hashListIndex = _newHashListIndex == -2 ? _animResource.getFrameCount() - 1 : _newHashListIndex;
			_newHashListIndex = -1;
		} else if (_fileHash4 != 0) {
			_hashListIndex = MAX<int16>(0, _animResource.getFrameIndex(_fileHash4));
			_fileHash4 = 0;
		}
		if (_fileHash1 == 0 && _frameIndex != _hashListIndex) {
			if (_counter != 0)
				_counter--;
			if (_counter == 0 && _animResource.getFrameCount() != 0) {
				
				if (_fileHash2 != 0) {
					if (_animResource.loadInternal(_fileHash2)) {
						_fileHash3 = _fileHash2;
					} else {
						debug("TODO");
						// TODO _animResource.loadInternal(calcHash("sqDefault"));
						_fileHash3 = 0;
					}
					if (_replNewByte != _replOldByte) {
						debug("TODO");
						// TODO _animResource.setRepl(_replOldByte, _replNewByte);
					}
					_fileHash2 = 0;
					if (_animStatus != 0) {
						_frameIndex = _fileHash6 != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_fileHash6)) : 0;
						_frameIndex2 = _fileHash5 != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_fileHash5)) : _animResource.getFrameCount() - 1;
					} else {
						_frameIndex = _frameIndex3 != -1 ? _frameIndex3 : _animResource.getFrameCount() - 1;
						_frameIndex2 = _frameIndex4 != -1 ? _frameIndex4 : _animResource.getFrameCount() - 1; 
					}
				} else {
					updateFrameIndex();
				}
				if (_fileHash1 == 0)
					updateFrameInfo();
			}				
		}
	}
	
	if (_fileHash1 != 0) {
		if (_animStatus == 2) {
			_hashListIndex = _frameIndex;
		} else {
			if (_animStatus == 1) {
				if (_animResource.loadInternal(_fileHash1)) {
					_fileHash3 = _fileHash1;
				} else {
					debug("TODO");
					// TODO _animResource.loadInternal(calcHash("sqDefault"));
					_fileHash3 = 0;
				}
				if (_replNewByte != _replOldByte) {
					debug("TODO");
					// TODO _animResource.setRepl(_replOldByte, _replNewByte);
				}
				_fileHash1 = 0;
				_frameIndex = _fileHash6 != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_fileHash6)) : 0;
				_frameIndex2 = _fileHash5 != 0 ? MAX<int16>(0, _animResource.getFrameIndex(_fileHash5)) : _animResource.getFrameCount() - 1;
			} else {
				if (_animResource.loadInternal(_fileHash1)) {
					_fileHash3 = _fileHash1;
				} else {
					debug("TODO");
					// TODO _animResource.loadInternal(calcHash("sqDefault"));
					_fileHash3 = 0;
				}
				if (_replNewByte != _replOldByte) {
					debug("TODO");
					// TODO _animResource.setRepl(_replOldByte, _replNewByte);
				}
				_fileHash1 = 0;
				_frameIndex = _frameIndex3 != -1 ? _frameIndex3 : _animResource.getFrameCount() - 1;
				_frameIndex2 = _frameIndex4 != -1 ? _frameIndex4 : _animResource.getFrameCount() - 1;
			}
			updateFrameInfo();
		}

		if (_newHashListIndex != -1) {
			_hashListIndex = _newHashListIndex == -2 ? _animResource.getFrameCount() - 1 : _newHashListIndex;
			_newHashListIndex = -1;
		} else if (_fileHash4 != 0) {
			_hashListIndex = MAX<int16>(0, _animResource.getFrameIndex(_fileHash4));
			_fileHash4 = 0;
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
		_surface->drawAnimResource(_animResource, _frameIndex, _doDeltaX, _doDeltaY, _drawRect.width, _drawRect.height);
		_needRefresh = false;
	}

}

void AnimatedSprite::updateFrameIndex() {
	if (!_playBackwards) {
		if (_frameIndex < _frameIndex2) {
			_frameIndex++;
		} else {
			// Inform self about end of current animation
			// The caller can then e.g. set a new animation fileHash
			sendMessage(0x3002, 0, this);
			if (_fileHash1 == 0)
				_frameIndex = 0;
		}
	} else {
		if (_frameIndex > 0) {
			_frameIndex--;
		} else {
			sendMessage(0x3002, 0, this);
			if (_fileHash1 == 0)
				_frameIndex = _frameIndex2;
		}
	}
}

void AnimatedSprite::updateFrameInfo() {
	debug(8, "AnimatedSprite::updateFrameInfo()");

	const AnimFrameInfo &frameInfo = _animResource.getFrameInfo(_frameIndex);
	
	_flag = true;
	_drawRect = frameInfo.rect;
	_deltaX = frameInfo.deltaX;
	_deltaY = frameInfo.deltaY;
	_deltaRect = frameInfo.deltaRect;
	_counter = frameInfo.counter;

	processDelta();

	_needRefresh = true;

	if (frameInfo.frameHash != 0) {
		sendMessage(0x100D, frameInfo.frameHash, this);
	}

}

void AnimatedSprite::createSurface1(uint32 fileHash, int surfacePriority) {
	NDimensions dimensions;
	// TODO dimensions = getAnimatedSpriteDimensions(fileHash);
	dimensions.width = 640;
	dimensions.height = 480;
	_surface = new BaseSurface(_vm, surfacePriority, dimensions.width, dimensions.height);
}

void AnimatedSprite::setFileHash(uint32 fileHash, int16 frameIndex3, int16 frameIndex4) {
	debug("AnimatedSprite::setFileHash(%08X, %d, %d)", fileHash, frameIndex3, frameIndex4);
	_fileHash1 = fileHash;
	_frameIndex3 = frameIndex3;
	_frameIndex4 = frameIndex4;
	_fileHash4 = 0;
	_animStatus = 0;
	_playBackwards = false;
	_newHashListIndex = -1;
	_hashListIndex = -1;
}

void AnimatedSprite::setFileHash1() {
	_fileHash1 = 1;
	_animStatus = 2;
}

void AnimatedSprite::setFileHash2(uint32 fileHash, uint32 fileHash6, uint32 fileHash5) {
	_fileHash1 = fileHash;
	_fileHash6 = fileHash6;
	_fileHash5 = fileHash5;
	_fileHash4 = 0;
	_animStatus = 1;
	_playBackwards = false;
	_newHashListIndex = -1;
	_hashListIndex = -1;
}

void AnimatedSprite::setFileHash3(uint32 fileHash2, uint32 fileHash6, uint32 fileHash5) {
	_fileHash2 = fileHash2;
	_fileHash6 = fileHash6;
	_fileHash5 = fileHash5;
	_fileHash4 = 0;
	_animStatus = 1;
	_playBackwards = false;
	_newHashListIndex = -1;
	_hashListIndex = -1;
}

void AnimatedSprite::setCallback1(AnimationCb callback1) {
	if (_callback1Cb) {
		(this->*_callback1Cb)();
	}
	_callback1Cb = callback1;
}

void AnimatedSprite::setCallback2(AnimationCb callback2) {

	if (_callback1Cb) {
		AnimationCb cb = _callback1Cb;
		_callback1Cb = NULL;
		(this->*cb)();
	}

	// TODO _callbackList = NULL;
	_callback3Cb = NULL;
	_callback2Cb = callback2;
	
	if (_callback2Cb) {
		(this->*_callback2Cb)();
	}

}

void AnimatedSprite::removeCallbacks() {

	if (_callback1Cb) {
		AnimationCb cb = _callback1Cb;
		_callback1Cb = NULL;
		(this->*cb)();
	}

	if (_callback3Cb) {
		_callback2Cb = _callback3Cb;
		_callback3Cb = NULL;
		(this->*_callback2Cb)();
#if 0 // TODO		
	} else if (_callbackList) {
		removeCallbackList();
#endif		
	} else {
		_callback2Cb = NULL;
	}

}

} // End of namespace Neverhood
