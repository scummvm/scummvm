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

#include "common/scummsys.h"
#include "mads/nebular/nebular.h"
#include "mads/nebular/core/scene.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes3.h"

namespace MADS {

namespace Nebular {

void Scene3xx::setAAName() {
	_game._aaName = Resources::formatAAName(4);
}

void Scene3xx::setPlayerSpritesPrefix() {
	_vm->_sound->command(5);

	Common::String oldName = _game._player._spritesPrefix;

	if (_globals[kSexOfRex] == REX_MALE)
		_game._player._spritesPrefix = "RXM";
	else
		_game._player._spritesPrefix = "ROX";

	if ((_scene->_nextSceneId == 313) || (_scene->_nextSceneId == 366)
	|| ((_scene->_nextSceneId >= 301) && (_scene->_nextSceneId <= 303))
	|| ((_scene->_nextSceneId == 304) && (_scene->_currentSceneId == 303))
	|| ((_scene->_nextSceneId == 311) && (_scene->_currentSceneId == 304))
	|| ((_scene->_nextSceneId >= 308) && (_scene->_nextSceneId <= 310))
	|| ((_scene->_nextSceneId >= 319) && (_scene->_nextSceneId <= 322))
	|| ((_scene->_nextSceneId >= 387) && (_scene->_nextSceneId <= 391))) {
		_game._player._spritesPrefix = "";
		_game._player._spritesChanged = true;
	}

	_game._player._scalingVelocity = true;
	if (oldName != _game._player._spritesPrefix)
		_game._player._spritesChanged = true;

	_vm->_palette->setEntry(16, 10, 63, 63);
	_vm->_palette->setEntry(17, 10, 45, 45);
}

void Scene3xx::sceneEntrySound() {
	if (!_vm->_musicFlag) {
		_vm->_sound->command(2);
		return;
	}

	switch (_scene->_nextSceneId) {
	case 301:
	case 302:
	case 303:
	case 304:
	case 308:
	case 309:
	case 310:
		_vm->_sound->command(11);
		break;

	case 311:
		if (_scene->_priorSceneId == 304)
			_vm->_sound->command(11);
		else
			_vm->_sound->command(10);
		break;

	case 313:
	case 316:
	case 320:
	case 322:
	case 357:
	case 358:
	case 359:
	case 360:
	case 361:
	case 387:
	case 388:
	case 389:
	case 390:
	case 391:
	case 399:
		_vm->_sound->command(10);
		break;

	case 318:
		if ((_scene->_priorSceneId == 357) || (_scene->_priorSceneId == 407))
			_vm->_sound->command(10);
		else if (_scene->_priorSceneId == 319)
			_vm->_sound->command(16);
		else
			_vm->_sound->command(3);

		_vm->_sound->command(50);
		break;

	case 319:
		_vm->_sound->command(16);
		break;

	case 321:
		_vm->_sound->command(18);
		break;

	default:
		break;
	}
}

void Scene3xx::initForceField(ForceField *force, bool flag) {
	force->_flag = flag;
	force->_vertical = 0;
	force->_horizontal = 0;
	force->_timer = 0;

	for (int count = 0; count < 40; count++)
		force->_seqId[count] = -1;

	if (force->_flag)
		_vm->_sound->command(24);
}

int Scene3xx::computeScale(int low, int high, int id) {
	int diff = high - (low + 2);
	int quotient = diff / 20;
	int remainder = diff % 20;
	int value = low + 2 + (quotient * id) + (remainder / (id + 1));

	return (value);
}

void Scene3xx::handleForceField(ForceField *force, int *sprites) {
	if (_game._trigger >= 150) {
		int id = _game._trigger - 150;
		if (id < 40) {
			if (id < 20)
				force->_vertical--;
			else
				force->_horizontal--;

			force->_seqId[id] = -1;
		}
		return;
	}

	if (!force->_flag || (_scene->_frameStartTime < force->_timer) || (force->_vertical + force->_horizontal >= 5))
		return;

	if (_vm->getRandomNumber(1, 1000) <= (200 + ((40 - (force->_vertical + force->_horizontal)) << 5))) {
		int id = -1;
		for (int i = 0; i < 100; i++) {
			int randIdx = _vm->getRandomNumber(0, 39);
			if (force->_seqId[randIdx] < 0) {
				id = randIdx;
				break;
			}
		}

		if (id < 0) {
			for (int i = 0; i < 40; i++) {
				if (force->_seqId[i] < 0) {
					id = i;
					break;
				}
			}
		}

		int speedX, speedY;
		int posX, posY;
		int randVal = _vm->getRandomNumber(1, 100);
		int spriteId;
		bool mirror;

		if (id >= 20) {
			spriteId = 2;
			mirror = (randVal <= 50);
			posX = mirror ? 315 : 5;
			posY = computeScale(15, 119, id - 20);
			speedX = 1000 * (mirror ? -1 : 1);
			speedY = 0;
		} else if (randVal <= 50) {
			spriteId = 1;
			mirror = false;
			posX = computeScale(21, 258, id);
			posY = 0;
			speedX = 0;
			speedY = 600;
		} else {
			spriteId = 0;
			mirror = false;
			posX = computeScale(21, 258, id);
			posY = 155;
			speedX = 0;
			speedY = -600;
		}

		if (id >= 0) {
			force->_seqId[id] = _scene->_sequences.addSpriteCycle(sprites[spriteId], mirror, 2, 0, 0, 0);
			_scene->_sequences.setDepth(force->_seqId[id], 8);
			_scene->_sequences.setPosition(force->_seqId[id], Common::Point(posX, posY));
			_scene->_sequences.setMotion(force->_seqId[id], 2, speedX, speedY);
			_scene->_sequences.addSubEntry(force->_seqId[id], SEQUENCE_TRIGGER_EXPIRE, 0, 150 + id);
			if (spriteId == 2)
				force->_horizontal++;
			else
				force->_vertical++;
		}
	}

	force->_timer = _scene->_frameStartTime + 4;
}

/*------------------------------------------------------------------------*/

void Scene300s::preActions() {
	_game._player._needToWalk = false;
}

} // namespace Nebular
} // namespace MADS
