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

#include "mads/madsv2/nebular/rooms/forcefield.h"
#include "mads/madsv2/nebular/rooms/thunks.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {
namespace RexNebular {
namespace Rooms {

static int compute_scale(int low, int high, int id) {
	int diff = high - (low + 2);
	int quotient = diff / 20;
	int remainder = diff % 20;
	int value = low + 2 + (quotient * id) + (remainder / (id + 1));

	return value;
}
void init_forcefield(Forcefield *force, bool flag) {
	force->_flag = flag;
	force->_vertical = 0;
	force->_horizontal = 0;
	force->_timer = 0;

	for (int count = 0; count < 40; count++)
		force->_seqId[count] = -1;

	if (force->_flag)
		g_engine->_soundManager->command(24);
}

void handle_forcefield(Forcefield *force, int16 *sprites) {
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
			posY = compute_scale(15, 119, id - 20);
			speedX = 1000 * (mirror ? -1 : 1);
			speedY = 0;
		} else if (randVal <= 50) {
			spriteId = 1;
			mirror = false;
			posX = compute_scale(21, 258, id);
			posY = 0;
			speedX = 0;
			speedY = 600;
		} else {
			spriteId = 0;
			mirror = false;
			posX = compute_scale(21, 258, id);
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

} // namespace Rooms
} // namespace RexNebular
} // namespace MADSV2
} // namespace MADS
