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

#ifndef ACCESS_ANIMATION_H
#define ACCESS_ANIMATION_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/memstream.h"
#include "access/data.h"
#include "access/files.h"

namespace Access {

class AnimationResource;
class Animation;
class AnimationFrame;
class AnimationFramePart;

class AnimationManager : public Manager {
private:
	Common::Array<Animation *> _animationTimers;
	AnimationResource *_animation;
public:
	Animation *_animStart;
	Common::Point _base;
	int _frameScale;
public:
	AnimationManager(AccessEngine *vm);
	~AnimationManager();
	void freeAnimationData();
	void loadAnimations(Resource *res);

	Animation *findAnimation(int animId);
	Animation *setAnimation(int animId);

	void animate(int animId);

	/**
	 * Clear the list of currently active animations
	 */
	void clearTimers();

	/**
	 * Add an animation to the list of currently animating ones
	 */
	void setAnimTimer(Animation *anim);

	/**
	 * Update the timing of all currently active animation
	 */
	void updateTimers();
};

class AnimationResource {
private:
	Common::Array<Animation *> _animations;
public:
	AnimationResource(AccessEngine *vm, Resource *res);
	~AnimationResource();

	int getCount() { return _animations.size(); }
	Animation *getAnimation(int idx) { return _animations[idx]; }
};

class Animation : public Manager {
private:
	Common::Array<AnimationFrame *> _frames;

	void anim0();
	void anim1();
	void anim2();
	void anim3();
	void anim4();
	void animNone();
	void anim7();

	AnimationFrame *calcFrame();
	AnimationFrame *calcFrame1();
	void setFrame(AnimationFrame *frame);
	void setFrame1(AnimationFrame *frame);
public:
	int _type;
	int _scaling;
	int _frameNumber;
	int _initialTicks;
	int _loopCount;
	int _countdownTicks;
	int _currentLoopCount;
public:
	Animation(AccessEngine *vm, Common::SeekableReadStream *stream);
	~Animation();

	void animate();
};

class AnimationFrame {
public:
	int _baseX, _baseY;
	int _frameDelay;
	Common::Array<AnimationFramePart *> _parts;
public:
	AnimationFrame(Common::SeekableReadStream *stream, int startOffset);
	~AnimationFrame();
};

class AnimationFramePart {
public:
	byte _flags;
	int _spritesIndex;
	int _frameIndex;
	Common::Point _position;
	int _offsetY;
public:
	AnimationFramePart(Common::SeekableReadStream *stream);
};

} // End of namespace Access

#endif /* ACCESS_ANIMATION_H */
