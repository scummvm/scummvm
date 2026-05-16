/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	for (auto *animation : _animations)
		delete animation;
}

/*------------------------------------------------------------------------*/

Animation::Animation(AccessEngine *vm, Common::SeekableReadStream *stream) : Manager(vm) {
	uint32 startOfs = stream->pos();

	_type = stream->readByte();

	// WORKAROUND: In Amazon floppy English, there's an animation associated with
	// the librarian that isn't used, and has junk data. Luckily, it's animation
	// type is also invalid, so if the _type isn't in range, exit immediately
	if (_type > 7 && _vm->getGameID() == kGameAmazon) {
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

	for (uint16 frameOffset : frameOffsets) {
		stream->seek(startOfs + frameOffset);

		AnimationFrame *frame = new AnimationFrame(stream, startOfs);
		_frames.push_back(frame);
	}
}

Animation::~Animation() {
	for (auto *frame : _frames)
		delete frame;
}

typedef void(Animation::*AnimationMethodPtr)();

void Animation::animate() {
	static const AnimationMethodPtr METHODS[13] = {
	   &Animation::anim0, &Animation::anim1, &Animation::anim2, &Animation::anim3,
	   &Animation::anim4, &Animation::animNone, &Animation::animNone, &Animation::anim7,
	   &Animation::anim8, &Animation::anim9, &Animation::anim10, &Animation::anim11,
	   &Animation::anim12,
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
			const AnimationFrame *frame = calcFrame();

			if (frame == nullptr) {
				_frameNumber = 0;
				_currentLoopCount = -1;
				frame = calcFrame();
			}

			setFrame(frame);
		}
	}
}

// Loop and then leave the last frame
void Animation::anim1() {
	if (_currentLoopCount == -1 || _countdownTicks != 0) {
		setFrame1(calcFrame());
	} else {
		_countdownTicks = _initialTicks;
		++_frameNumber;
		const AnimationFrame *frame = calcFrame();

		if (frame == nullptr) {
			--_frameNumber;
			_currentLoopCount = -1;
			frame = calcFrame();
		}

		setFrame(frame);
	}
}

// Loop forever
void Animation::anim2() {
	if (_countdownTicks != 0) {
		setFrame1(calcFrame());
	} else {
		_countdownTicks = _initialTicks;
		++_frameNumber;
		const AnimationFrame *frame = calcFrame();

		if (frame == nullptr) {
			_frameNumber = 0;
			frame = calcFrame();
		}

		setFrame(frame);
	}
}

// Loop and stop.
void Animation::anim3() {
	if (_currentLoopCount == -1)
		return;

	if (_countdownTicks != 0) {
		setFrame1(calcFrame());
	} else {
		_countdownTicks = _initialTicks;
		++_frameNumber;
		const AnimationFrame *frame = calcFrame();

		if (frame == nullptr) {
			--_currentLoopCount;
			_frameNumber = 0;
			frame = calcFrame();
		}

		setFrame(frame);
	}
}

// Loop and stop?? How is this different from 3?
// It can't leave the last frame there, that breaks
// Stiletto's door opening in Noctropolis.
void Animation::anim4() {
	if (_currentLoopCount == -1)
		return;
	if (_countdownTicks != 0) {
		setFrame1(calcFrame());
	} else {
		_countdownTicks = _initialTicks;
		++_frameNumber;
		const AnimationFrame *frame = calcFrame();

		if (frame == nullptr) {
			if (--_currentLoopCount == -1) {
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

// Just a single frame.
void Animation::anim7() {
	setFrame(calcFrame1());
}

void Animation::anim8() {
	const AnimationFrame *frame = calcFrame();
	if (!_vm->_curPlayer->_playerOff) {
		assert(frame->_parts.size() > 0);
		const AnimationFramePart &part0 = frame->_parts[0];
		const SpriteFrame *spriteFrame0 = _vm->_objectsTable[part0._spritesIndex]->_frames[part0._frameIndex];
		_vm->_animation->_base.x = frame->_baseX;
		_vm->_animation->_base.y = frame->_baseY;
		int playerX = _vm->_curPlayer->_playerX;
		int playerY = _vm->_curPlayer->_playerY;
		int frameX = _vm->_animation->_base.x + part0._position.x;
		int frameY = _vm->_animation->_base.y + part0._position.y;
		int playerOffX = _vm->_curPlayer->_playerOffset.x;
		int playerOffY = _vm->_curPlayer->_playerOffset.y;

		if (playerX <= frameX + spriteFrame0->w &&
			playerY - playerOffY <= frameY + spriteFrame0->h &&
			playerY <= part0._offsetY && frameY <= playerY && frameX < playerOffX + playerX) {
			// Reset frame here?
			_frameNumber = 0;
			frame = calcFrame();
		}
	}
	setFrame(frame);
}

void Animation::anim9() {
	debug("TODO: Animation::anim9");
}

void Animation::anim10() {
	debug("TODO: Animation::anim10");
}

void Animation::anim11() {
	// Actor idle
	const AnimationFrame *frame = calcFrame();
	_countdownTicks += frame->_frameDelay;
	_scaling = _vm->_scale;
	debugC(kDebugGraphics, "anim11: idle %s scale %d (%d, %d) yoffset %d", (_vm->_curPlayer == _vm->_player ? "peter" : "stil"),
		_vm->_scale, _vm->_curPlayer->_playerX, _vm->_curPlayer->_playerY, _vm->_curPlayer->_playerOffset.y);
	setFrame1(frame, _vm->_curPlayer->_playerX, _vm->_curPlayer->_playerY - _vm->_curPlayer->_playerOffset.y);
}

void Animation::anim12() {
	// Actor walk
	const AnimationFrame *frame = calcFrame();
	if (_countdownTicks == 0) {
		const AnimationFrame *prevAnimationFrame = frame;
		_countdownTicks = _initialTicks;
		_frameNumber++;
		if (_frameNumber >= (int)_frames.size()) {
			_frameNumber = 1;
			prevAnimationFrame = calcFrame1();
		}
		frame = calcFrame();
		int16 deltaX = (frame->_baseX + frame->_parts[0]._position.x) -
			(prevAnimationFrame->_baseX + prevAnimationFrame->_parts[0]._position.x);
		int16 deltaY = (frame->_baseY + frame->_parts[0]._position.y) -
			(prevAnimationFrame->_baseY + prevAnimationFrame->_parts[0]._position.y);
		int16 xadd = ABS(deltaX) * _vm->_scale / 256;
		int16 xaddLow = (ABS(deltaX) * _vm->_scale) % 256;
		int16 yadd = ABS(deltaY) * _vm->_scale / 256;
		int16 yaddLow = (ABS(deltaY) * _vm->_scale) % 256;

		// Original uses "low" parts to do some fixed-point movement
		// to make scaling slightly smoother.
		Player *player = _vm->_curPlayer;

		if (deltaX < 0) {
			player->_playerX -= xadd;
			player->_playerXLow -= xaddLow;
			if (player->_playerXLow <= -256) {
				player->_playerX--;
				player->_playerXLow += 256;
			}
		} else {
			player->_playerX += xadd;
			player->_playerXLow += xaddLow;
			if (player->_playerXLow >= 256) {
				player->_playerX++;
				player->_playerXLow -= 256;
			}
		}

		if (deltaY < 0) {
			player->_playerY -= yadd;
			player->_playerYLow -= yaddLow;
			if (player->_playerYLow <= -256) {
				player->_playerY--;
				player->_playerYLow += 256;
			}
		} else {
			player->_playerY += yadd;
			player->_playerYLow += yaddLow;
			if (player->_playerYLow >= 256) {
				player->_playerY++;
				player->_playerYLow -= 256;
			}
		}

		_countdownTicks += frame->_frameDelay;

		debugC(kDebugGraphics, "anim12: %s pos %d, %d yoff %d (change %d %d -> %d %d) scale %d", (_vm->_curPlayer == _vm->_player ? "peter" : "stil"), _vm->_curPlayer->_playerX, _vm->_curPlayer->_playerY, _vm->_curPlayer->_playerOffset.y, deltaX, deltaY, xadd, yadd, _vm->_scale);
	}
	_scaling = _vm->_scale;
	setFrame1(frame, _vm->_curPlayer->_playerX, _vm->_curPlayer->_playerY - _vm->_curPlayer->_playerOffset.y);
}


const AnimationFrame *Animation::calcFrame() {
	return (_frameNumber < (int)_frames.size()) ? _frames[_frameNumber] : nullptr;
}

const AnimationFrame *Animation::calcFrame1() {
	return _frames[0];
}

void Animation::setFrame(const AnimationFrame *frame) {
	assert(frame);
	_countdownTicks += frame->_frameDelay;
	setFrame1(frame);
}

void Animation::setFrame1(const AnimationFrame *frame, int16 xoff, int16 yoff) {
	_vm->_animation->_base.x = frame->_baseX;
	_vm->_animation->_base.y = frame->_baseY;

	// Loop to add image draw requests for the parts of the frame
	for (const AnimationFramePart &part: frame->_parts) {
		ImageEntry ie;

		// Set the flags
		ie._flags = part._flags & ~IMGFLAG_UNSCALED;
		if (_scaling == -1)
			ie._flags |= IMGFLAG_UNSCALED;

		// Set the other fields
		ie._spritesPtr = _vm->_objectsTable[part._spritesIndex];
		ie._frameNumber = part._frameIndex;
		ie._position.x = xoff ? xoff : (part._position.x + _vm->_animation->_base.x);
		ie._position.y = yoff ? yoff : (part._position.y + _vm->_animation->_base.y);
		if (xoff && _scaling != -1) {
			// If xoff is set, the animation is for an actor so we need to apply scale
			// factor to frame offset for priority.  We also need to determine its size
			// now so Stiletto and Peter can have different scale factors in Noctropolis
			ie._offsetY = (part._offsetY - frame->_baseY - part._position.y) * _scaling / 256;
			ie._scaleOverride = _scaling;
			const SpriteFrame *frame = ie._spritesPtr->getFrame(ie._frameNumber);
			ie._sizeOverride = Common::Point(
				_vm->_screen->_scaleTable1[frame->w],
				_vm->_screen->_scaleTable1[frame->h]
			);
		} else {
			ie._offsetY = part._offsetY - ie._position.y;
		}

		/*
		Common::String dumpfn = Common::String::format("anim_%d_%d_%d_sprite_%d_frame_%d", ie._position.x, ie._position.y, ie._offsetY, part._spritesIndex, part._frameIndex);
		ie._spritesPtr->getFrame(ie._frameNumber)->dump(dumpfn.c_str());
		*/

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

		AnimationFramePart framePart(stream);
		_parts.push_back(framePart);

		nextOffset = stream->readUint16LE();
	}
}

AnimationFrame::~AnimationFrame() {
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
	anim->animate();
}

void AnimationManager::updateTimers() {
	for (auto *timer : _animationTimers) {
		if (timer->_countdownTicks > 0)
			timer->_countdownTicks--;
	}
}

} // End of namespace Access
