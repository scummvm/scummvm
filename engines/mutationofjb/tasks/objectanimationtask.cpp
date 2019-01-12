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

		object->_currentFrame = nextAnimationOffset + object->_firstFrame;

		const bool drawObject = handleHardcodedAnimation(object);
		if (drawObject) {
			getTaskManager()->getGame().getRoom().drawObject(i);
		}
	}
}

bool ObjectAnimationTask::handleHardcodedAnimation(Object *const object) {
	GameData &gameData = getTaskManager()->getGame().getGameData();
	Scene *const scene = gameData.getCurrentScene();

	const bool carnivalScene = gameData._currentScene == 30 && !gameData._partB;
	const bool tavernScene = gameData._currentScene == 8 && gameData._partB;

	if (carnivalScene) {
		// This alternates between the two burglars' talking animations.
		// Each burglar gets to talk for a varying amount of time since
		// the switch occurs when his random frame is reached.
		if (object->_WX == 1 && object->_currentFrame == 79) {
			object->_currentFrame = 68;
			object->_active = 0;
			scene->getObject(6)->_active = 1;
			scene->getObject(7)->_active = 0;
			scene->getObject(8)->_active = 1;
			return false;
		} else if (object->_WX == 2 && object->_currentFrame == 91) {
			object->_currentFrame = 80;
			object->_active = 0;
			scene->getObject(5)->_active = 1;
			scene->getObject(7)->_active = 1;
			scene->getObject(8)->_active = 0;
			return false;
		}

		// The following makes sure you can't interact with the glass
		// while the scientist is drinking from it.
		if (scene->getObject(4)->_currentFrame > 52 && scene->getObject(4)->_active) {
			scene->getStatic(9)->_active = 0; // disable scientist's glass
		} else {
			scene->getStatic(9)->_active = 1; // enable scientist's glass
		}

		if (!scene->getObject(4)->_active) {
			scene->getStatic(9)->_active = 0; // disable scientist's glass
		}
	} else if (tavernScene) {
		// Similarly to the carnival burglars, this alternates between
		// the talking animations of the two soldiers in the tavern.
		//
		// At some point the script disables their conversation
		// by nulling their _WX registers.
		if (object->_WX == 3 && object->_currentFrame == 46) {
			object->_currentFrame = 30;
			object->_active = 0;
			scene->getObject(3)->_active = 1;
			return false;
		} else if (object->_WX == 4 && object->_currentFrame == 63) {
			object->_currentFrame = 47;
			object->_active = 0;
			scene->getObject(2)->_active = 1;
			return false;
		}
	}

	return true;
}

}
