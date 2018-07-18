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

void ObjectAnimationTask::updateObjects() {
	Scene *const scene = getTaskManager()->getGame().getGameData().getCurrentScene();
	if (!scene) {
		return;
	}

	for (uint8 i = 1; i <= scene->getNoObjects(); ++i) {
		Object *const object = scene->getObject(i);
		// Skip if object animation not active.
		if (!object->_AC)
			continue;

		// Number of framers must be higher than 1.
		if (object->_NA <= 1)
			continue;

		const uint8 currentAnimOffset = object->_CA - object->_FA;

		const bool randomized = object->_FR != 0;
		const bool belowRandomFrame = currentAnimOffset < (object->_FR - 1);

		uint8 maxAnimOffset = object->_NA - 1;
		if (randomized && belowRandomFrame) {
			maxAnimOffset = object->_FR - 2;
		}

		uint8 nextAnimationOffset = currentAnimOffset + 1;
		if (currentAnimOffset == maxAnimOffset) {
			if (randomized && object->_unknown != 0 && getTaskManager()->getGame().getRandomSource().getRandomNumber(object->_unknown) == 0)
				nextAnimationOffset = object->_FR - 1;
			else
				nextAnimationOffset = 0;
		}

		// TODO: Hardcoded animations.

		object->_CA = nextAnimationOffset + object->_FA;
		getTaskManager()->getGame().getRoom().drawObjectAnimation(i, nextAnimationOffset);
	}
}

}
