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

#include "common/endian.h"
#include "common/memstream.h"
#include "access/access.h"
#include "access/animation.h"

namespace Access {

AnimationResource::AnimationResource(AccessEngine *vm, Resource *res) {
	int count = res->_stream->readUint16LE();

	Common::Array<int> offsets;
	for (int i = 0; i < count; ++i)
		offsets.push_back(res->_stream->readUint32LE());

	_animations.reserve(count);
	for (int i = 0; i < count; ++i) {
		res->_stream->seek(offsets[i]);
		Animation *anim = new Animation(vm, res->_stream);
		_animations.push_back(anim);
	}
}

AnimationResource::~AnimationResource() {
	for (int i = 0; i < (int)_animations.size(); ++i)
		delete _animations[i];
}

/*------------------------------------------------------------------------*/

Animation::Animation(AccessEngine *vm, Common::SeekableReadStream *stream) : Manager(vm) {
	uint32 startOfs = stream->pos();

	_type = stream->readByte();

	// WORKAROUND: In Amazon floppy English, there's an animation associated with
	// the librarian that isn't used, and has junk data. Luckily, it's animation
	// type is also invalid, so if the _type isn't in range, exit immediately
	if (_type < 0 || _type > 7) {
		_scaling = -1;
		_frameNumber = -1;
		_initialTicks = _countdownTicks = 0;
		_loopCount = _currentLoopCount = 0;
		return;
	}

	_scaling = stream->readSByte();
	stream->readByte(); // unk
	_frameNumber = stream->readByte();
	_initialTicks = stream->readUint16LE();
	stream->readUint16LE(); // unk
	stream->readUint16LE(); // unk
	_loopCount = stream->readSint16LE();
	_countdownTicks = stream->readUint16LE();
	_currentLoopCount = stream->readSint16LE();
	stream->readUint16LE(); // unk

	Common::Array<uint16> frameOffsets;
	uint16 ofs;
	while ((ofs = stream->readUint16LE()) != 0)
		frameOffsets.push_back(ofs);

	for (int i = 0; i < (int)frameOffsets.size(); i++) {
		stream->seek(startOfs + frameOffsets[i]);

		AnimationFrame *frame = new AnimationFrame(stream, startOfs);
		_frames.push_back(frame);
	}
}

Animation::~Animation() {
	for (uint i = 0; i < _frames.size(); ++i)
		delete _frames[i];
}

typedef void(Animation::*AnimationMethodPtr)();

void Animation::animate() {
	static const AnimationMethodPtr METHODS[8] = {
	   &Animation::anim0, &Animation::anim1, &Animation::anim2, &Animation::anim3,
	   &Animation::anim4, &Animation::animNone, &Animation::animNone, &Animation::anim7
	};

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
				frame = calcFrame();
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
			frame = calcFrame();
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
	// Empty implementation
}

void Animation::anim7() {
	setFrame(calcFrame1());
}

AnimationFrame *Animation::calcFrame() {
	return (_frameNumber < (int)_frames.size()) ? _frames[_frameNumber] : nullptr;
}

AnimationFrame *Animation::calcFrame1() {
	return _frames[0];
}

void Animation::setFrame(AnimationFrame *frame) {
	assert(frame);
	_countdownTicks += frame->_frameDelay;
	setFrame1(frame);
}

void Animation::setFrame1(AnimationFrame *frame) {
	_vm->_animation->_base.x = frame->_baseX;
	_vm->_animation->_base.y = frame->_baseY;

	// Loop to add image draw requests for the parts of the frame
	for (uint i = 0; i < frame->_parts.size(); ++i) {
		AnimationFramePart *part = frame->_parts[i];
		ImageEntry ie;

		// Set the flags
		ie._flags = part->_flags & ~IMGFLAG_UNSCALED;
		if (_vm->_animation->_frameScale == -1)
			ie._flags |= IMGFLAG_UNSCALED;

		// Set the other fields
		ie._spritesPtr = _vm->_objectsTable[part->_spritesIndex];
		ie._frameNumber = part->_frameIndex;
		ie._position = part->_position + _vm->_animation->_base;
		ie._offsetY = part->_offsetY - ie._position.y;

		_vm->_images.addToList(ie);
	}
}

/*------------------------------------------------------------------------*/

AnimationFrame::AnimationFrame(Common::SeekableReadStream *stream, int startOffset) {
	uint16 nextOffset;

	stream->readByte(); // unk
	_baseX = stream->readUint16LE();
	_baseY = stream->readUint16LE();
	_frameDelay = stream->readUint16LE();
	nextOffset = stream->readUint16LE();

	while (nextOffset != 0) {
		stream->seek(startOffset + nextOffset);

		AnimationFramePart *framePart = new AnimationFramePart(stream);
		_parts.push_back(framePart);

		nextOffset = stream->readUint16LE();
	}
}

AnimationFrame::~AnimationFrame() {
	for (int i = 0; i < (int)_parts.size(); ++i)
		delete _parts[i];
}

/*------------------------------------------------------------------------*/

AnimationFramePart::AnimationFramePart(Common::SeekableReadStream *stream) {
	_flags = stream->readByte();
	_spritesIndex = stream->readByte();
	_frameIndex = stream->readByte();
	_position.x = stream->readUint16LE();
	_position.y = stream->readUint16LE();
	_offsetY = stream->readUint16LE();
}

/*------------------------------------------------------------------------*/

AnimationManager::AnimationManager(AccessEngine *vm) : Manager(vm) {
	_animation = nullptr;
	_animStart = nullptr;
	_frameScale = 0;
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
	_animationTimers.clear();
}

void AnimationManager::loadAnimations(Resource *res) {
	_animationTimers.clear();
	delete _animation;
	_animation = new AnimationResource(_vm, res);
}


Animation *AnimationManager::setAnimation(int animId) {
	Animation *anim = findAnimation(animId);
	if (!anim)
		return nullptr;

	anim->_countdownTicks = anim->_initialTicks;
	anim->_frameNumber = 0;

	anim->_currentLoopCount = (anim->_type != 3 && anim->_type != 4) ? 0 : anim->_loopCount;

	return anim;
}

void AnimationManager::setAnimTimer(Animation *anim) {
	_animationTimers.push_back(anim);
}

Animation *AnimationManager::findAnimation(int animId) {
	_animStart = (_animation == nullptr) ? nullptr : _animation->getAnimation(animId);
	return _animStart;
}

void AnimationManager::animate(int animId) {
	Animation *anim = findAnimation(animId);
	_frameScale = anim->_scaling;
	anim->animate();
}

void AnimationManager::updateTimers() {
	for (uint idx = 0; idx < _animationTimers.size(); ++idx) {
		if (_animationTimers[idx]->_countdownTicks > 0)
			_animationTimers[idx]->_countdownTicks--;
	}
}

} // End of namespace Access
