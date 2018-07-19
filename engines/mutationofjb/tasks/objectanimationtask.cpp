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

#include "mutationofjb/tasks/objectanimationtask.h"

#include "mutationofjb/tasks/taskmanager.h"
#include "mutationofjb/game.h"
#include "mutationofjb/gamedata.h"
#include "mutationofjb/room.h"

namespace MutationOfJB {

static const int TICK_MILLIS = 100;

// TODO: Respect currentScene._delay.
ObjectAnimationTask::ObjectAnimationTask() : _timer(TICK_MILLIS) {
}

void ObjectAnimationTask::start() {
	setState(RUNNING);
	_timer.start();
}

void ObjectAnimationTask::update() {
	_timer.update();
	if (_timer.isFinished()) {
		_timer.start();
		updateObjects();
	}
}

/**
 * Advances every object animation in the current scene to the next frame.
 *
 * Normally the animation restarts after the last object frame. However, some animations have random
 * elements to them. If _randomFrame is set, the animation restarts when _randomFrame is reached.
 * Additionally, there is a chance with each frame until _randomFrame that the animation may jump
 * straight to _randomFrame and continue until the last frame, then wrap around to the first frame.
 *
 * Randomness is used to introduce variety - e.g. in the starting scene a perched bird occassionally
 * spreads its wings.
 */
void ObjectAnimationTask::updateObjects() {
	Scene *const scene = getTaskManager()->getGame().getGameData().getCurrentScene();
	if (!scene) {
		return;
	}

	for (uint8 i = 1; i <= scene->getNoObjects(); ++i) {
		Object *const object = scene->getObject(i);
		// Skip if object animation not active.
		if (!object->_active)
			continue;

		// Number of frames must be higher than 1.
		if (object->_numFrames <= 1)
			continue;

		const uint8 currentAnimOffset = object->_currentFrame - object->_firstFrame;

		const bool randomized = object->_randomFrame != 0;
		const bool belowRandomFrame = currentAnimOffset < (object->_randomFrame - 1);

		uint8 maxAnimOffset = object->_numFrames - 1;
		if (randomized && belowRandomFrame) {
			maxAnimOffset = object->_randomFrame - 2;
		}

		uint8 nextAnimationOffset = currentAnimOffset + 1;
		if (currentAnimOffset == maxAnimOffset) {
			if (randomized && object->_jumpChance != 0 && getTaskManager()->getGame().getRandomSource().getRandomNumber(object->_jumpChance) == 0)
				nextAnimationOffset = object->_randomFrame - 1;
			else
				nextAnimationOffset = 0;
		}

		// TODO: Hardcoded animations.

		object->_currentFrame = nextAnimationOffset + object->_firstFrame;
		getTaskManager()->getGame().getRoom().drawObjectAnimation(i, nextAnimationOffset);
	}
}

}
