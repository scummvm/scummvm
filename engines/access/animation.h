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

#ifndef ACCESS_ANIMATION_H
#define ACCESS_ANIMATION_H

#include "common/scummsys.h"
#include "common/array.h"
#include "access/data.h"

namespace Access {

class Animation;

class AnimationManager : public Manager {
private:
	Animation *findAnimation(int animId);
public:
	const byte *_anim;
	Animation *_animation;
	Common::Array<Animation *> _animationTimers;
public:
	AnimationManager(AccessEngine *vm);
	~AnimationManager();
	void freeAnimationData();
	void clearTimers();

	Animation *setAnimation(int animId);

	void setAnimTimer(Animation *anim);
};

class Animation {
public:
	int _type;
	int _scaling;
	int _frameNumber;
	int _ticks;
	int _loopCount;
	int _countdownTicks;
	int _currentLoopCount;
	int _field10;
public:
	Animation(const byte *data);
};

} // End of namespace Access

#endif /* ACCESS_ANIMATION_H */
