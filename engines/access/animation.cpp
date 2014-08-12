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

#include "common/endian.h"
#include "common/memstream.h"
#include "access/access.h"
#include "access/animation.h"

namespace Access {

AnimationResource::AnimationResource(AccessEngine *vm, const byte *data, int size) {
	Common::MemoryReadStream stream(data, size);
	int count = stream.readUint16LE();

	Common::Array<int> offsets;
	for (int i = 0; i < count; ++i)
		offsets.push_back(stream.readUint32LE());

	_animations.reserve(count);
	for (int i = 0; i < count; ++i) {
		stream.seek(offsets[i]);
		Animation *anim = new Animation(vm, stream);
		_animations.push_back(anim);
	}
}

AnimationResource::~AnimationResource() {
	for (int i = 0; i < (int)_animations.size(); ++i)
		delete _animations[i];
}

/*------------------------------------------------------------------------*/

Animation::Animation(AccessEngine *vm, Common::MemoryReadStream &stream):
		Manager(vm) {
	uint32 startOfs = stream.pos();

	_type = stream.readByte();
	_scaling = stream.readByte();
	stream.readByte(); // unk
	_frameNumber = stream.readByte();
	_initialTicks = stream.readUint16LE();
	stream.readUint16LE(); // unk
	stream.readUint16LE(); // unk
	_loopCount = stream.readUint16LE();
	_countdownTicks = stream.readUint16LE();
	_currentLoopCount = stream.readUint16LE();
	stream.readUint16LE(); // unk

	Common::Array<uint16> frameOffsets;
	uint16 ofs;
	while ((ofs = stream.readUint16LE()) != 0)
		frameOffsets.push_back(ofs);

	for (int i = 0; i < (int)frameOffsets.size(); i++) {
		stream.seek(startOfs + frameOffsets[i]);

		AnimationFrame *frame = new AnimationFrame(stream, startOfs);
		_frames.push_back(frame);
	}
}

Animation::~Animation() {
	for (int i = 0; i < (int)_frames.size(); ++i)
		delete _frames[i];
}

typedef void(Animation::*AnimationMethodPtr)();

void Animation::animate() {
	static const AnimationMethodPtr METHODS[8] =
	{ &Animation::anim0, &Animation::anim1, &Animation::anim2, &Animation::anim3, 
	&Animation::anim4, &Animation::animNone, &Animation::animNone, &Animation::anim7 };

	(this->*METHODS[_type])();
}

void Animation::anim0() {
	if (_currentLoopCount != -1) {
		if (_countdownTicks != 0) {
			setFrame1(calcFrame());
		} else {
			_countdownTicks = _initialTicks;
			++_frameNumber;
			AnimationFrame *frame = calcFrame();

			if (frame == nullptr) {
				_frameNumber = 0;
				_currentLoopCount = -1;
			}

			setFrame(frame);
		}
	}
}

void Animation::anim1() {
	if (_currentLoopCount == -1 || _countdownTicks != 0) {
		setFrame1(calcFrame());
	} else {
		_countdownTicks = _initialTicks;
		++_frameNumber;
		AnimationFrame *frame = calcFrame();

		if (frame == nullptr) {
			--_frameNumber;
			_currentLoopCount = -1;
		}

		setFrame(frame);
	}
}

void Animation::anim2() {
	if (_countdownTicks != 0) {
		setFrame1(calcFrame());
	} else {
		_countdownTicks = _initialTicks;
		++_frameNumber;
		AnimationFrame *frame = calcFrame();

		if (frame == nullptr) {
			_frameNumber = 0;
			frame = calcFrame();
		}

		setFrame(frame);
	}
}

void Animation::anim3() {
	if (_currentLoopCount != -1) {
		if (_countdownTicks != 0) {
			setFrame1(calcFrame());
		} else {
			_countdownTicks = _initialTicks;
			++_frameNumber;
			AnimationFrame *frame = calcFrame();

			if (frame == nullptr) {
				--_currentLoopCount;
				_frameNumber = 0;
				frame = calcFrame();
			}

			setFrame(frame);
		}
	}
}

void Animation::anim4() {
	if (_currentLoopCount == -1 || _countdownTicks != 0) {
		setFrame1(calcFrame());
	} else {
		_countdownTicks = _initialTicks;
		++_frameNumber;
		AnimationFrame *frame = calcFrame();

		if (frame == nullptr) {
			if (--_currentLoopCount == -1) {
				setFrame1(calcFrame());
				return;
			} else {
				_frameNumber = 0;
				frame = calcFrame();
			}
		}

		setFrame(frame);
	}
}

void Animation::animNone() {
	// No implementation
}

void Animation::anim7() {
	setFrame(calcFrame1());
}

AnimationFrame *Animation::calcFrame() {
	return _frames[_frameNumber];
}

AnimationFrame *Animation::calcFrame1() {
	return _frames[0];
}

void Animation::setFrame(AnimationFrame *frame) {
	error("TODO");
}

void Animation::setFrame1(AnimationFrame *frame) {
	error("TODO");
}


/*------------------------------------------------------------------------*/

AnimationFrame::AnimationFrame(Common::MemoryReadStream &stream, int startOffset) {
	uint16 nextOffset;

	stream.readByte(); // unk
	_baseX = stream.readUint16LE();
	_baseY = stream.readUint16LE();
	_frameDelay = stream.readUint16LE();
	nextOffset = stream.readUint16LE();

	while (nextOffset != 0) {
		stream.seek(startOffset + nextOffset);

		AnimationFramePart *framePart = new AnimationFramePart(stream);
		_parts.push_back(framePart);

		nextOffset = stream.readUint16LE();
	}
}

AnimationFrame::~AnimationFrame() {
	for (int i = 0; i < (int)_parts.size(); ++i)
		delete _parts[i];
}

/*------------------------------------------------------------------------*/

AnimationFramePart::AnimationFramePart(Common::MemoryReadStream &stream) {
	_flags = stream.readByte();
	_slotIndex = stream.readByte();
	_spriteIndex = stream.readByte();
	_position.x = stream.readUint16LE();
	_position.y = stream.readUint16LE();
	_priority = stream.readUint16LE();
}

/*------------------------------------------------------------------------*/

AnimationManager::AnimationManager(AccessEngine *vm) : Manager(vm) {
	_animation = nullptr;
	_animStart = nullptr;
}

AnimationManager::~AnimationManager() {
	delete _animation;
}

void AnimationManager::freeAnimationData() {
	delete _animation;
	_animation = nullptr;
	_animStart = nullptr;
}

void AnimationManager::clearTimers() {
	for (uint i = 0; i < _animationTimers.size(); ++i)
		delete _animationTimers[i];

	_animationTimers.clear();
}

void AnimationManager::loadAnimations(const byte *data, int size) {
	_animationTimers.clear();
	delete _animation;
	_animation = new AnimationResource(_vm, data,  size);
}


Animation *AnimationManager::setAnimation(int animId) {
	Animation *anim = findAnimation(animId);

	anim->_countdownTicks = anim->_initialTicks;
	anim->_frameNumber = 0;

	anim->_currentLoopCount = (anim->_type != 3 && anim->_type != 4) ? 0 : 
		anim->_loopCount;
	anim->_field10 = 0;
		
	return anim;
}

void AnimationManager::setAnimTimer(Animation *anim) {
	_animationTimers.push_back(anim);
}

Animation *AnimationManager::findAnimation(int animId) {
	_animStart = _animation->getAnimation(animId);
	return _animStart;
}

void AnimationManager::animate(int animId) {
	Animation *anim = findAnimation(animId);
	anim->animate();
}

} // End of namespace Access
